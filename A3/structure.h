#include<bits/stdc++.h>

using namespace std;

#define INVALID 0
#define SHARED 1
#define MODIFIED 2
#define EXCLUSIVE 3

#define MSG_GET 0
#define MSG_GETX 1
#define MSG_UPGR 2
#define MSG_INVAL_ACK 3
#define MSG_NACK 4
#define MSG_INVAL 5
#define MSG_SWB 6
#define MSG_SIMPL_WB 7
#define MSG_PUT 8
#define MSG_PUTE 9
#define MSG_PUTX 10
#define MSG_WB_ACK 11
#define MSG_OT 12 // Ownership Transfer
#define MSG_INVAL_L2 13
#define MSG_INVAL_L2_REPLY 14

vector<string> msg_names = {"MSG_GET","MSG_GETX","MSG_UPGR","MSG_INVAL_ACK","MSG_NACK","MSG_INVAL","MSG_SWB","MSG_SIMPL_WB","MSG_PUT","MSG_PUTE","MSG_PUTX","MSG_WB_ACK","MSG_OT","MSG_INVAL_L2","MSG_INVAL_L2_REPLY"};

uint64_t Cycle = 0;

typedef unsigned long long uint64 ;

uint64 l2_misses = 0;

struct mem_access{
    uint64 global_ctr;
    uint64 addr;
    uint64 is_write;
    uint64 orb_name;

    mem_access(){
        global_ctr = 0;
        addr = 0;
        is_write = 0;
        orb_name = 50;
    }
};

struct msg{
    uint64 msg_name;
    uint64 sender;
    uint64 addr;
    uint64 receiver;
    uint64 inval_ack_exp; // With PUTX only
    uint64 is_upgr_reply; // With PUTX only
    uint64 nack_is_write; // With NACK only
    uint64 upgr_miss;

    int home(){
        return addr & 7;
    }

    friend ostream& operator<<(ostream& o, struct msg & a){
        o  << "NAME : " << a.msg_name << " SENDER: " << a.sender << " ADDR : " << a.addr << " RECEIVER : " << a.receiver << " FIElDS: " << a.inval_ack_exp << " " << a.is_upgr_reply << " " << a.nack_is_write;
        return o;
    }
};


uint64 l1_size = 32*1024;
uint64 l1_block_size=64;
uint64 l1_assoc = 8;
uint64 l1_sets = l1_size/(l1_block_size*l1_assoc);

struct l1_cache_block{
    bool is_valid;
    uint64 tag;
    uint64 states;
    uint64 lru;
    uint64 is_dirty;
    l1_cache_block(){
        is_valid = false;
        tag = 0;
        states = INVALID;
        lru = 0;
        is_dirty = 0;
    }
};


struct l1_cache_block_cmp {
    bool operator() (struct l1_cache_block b1, struct l1_cache_block b2){
        
        return b1.lru< b2.lru;
    }
};

struct putx_node {
    struct msg putx;
    struct msg pend;
};

struct l1_cache{
    queue<pair<uint64, struct mem_access>> snacks;
    queue<struct mem_access> req_from_processor;
    deque<struct msg> msg_from_l2_or_l1_cache;
    

    vector<struct mem_access> outstanding_req_buffer;
    vector<struct putx_node> putx_buf;

    vector<multiset<l1_cache_block,l1_cache_block_cmp>> all_cache_blocks;  

    l1_cache(){
        for(int i=0;i<l1_sets;i++){
            multiset<l1_cache_block,l1_cache_block_cmp> one_cache_line;
            for(int j=0;j<l1_assoc;j++){
                struct l1_cache_block one_cache_block;
                one_cache_line.insert(one_cache_block);
            }
            all_cache_blocks.push_back(one_cache_line);
        }
    }

    //all utility functions;

    uint64 is_cache_hit(uint64 addr ){
        uint64 set = addr%l1_sets;
        for(auto it=all_cache_blocks[set].begin() ; it!=all_cache_blocks[set].end();it++){
            if(it->tag==addr && it->is_valid == true){
                return 1;

            }
        }
        return 0;
    }

    uint64 cache_block_state(uint64 addr){
        uint64 set = addr%l1_sets;
        for(auto it=all_cache_blocks[set].begin() ;it!=all_cache_blocks[set].end();it++){
            if(it->tag==addr && it->is_valid == true){
                return it->states;
            }
        }
        // cout<<"Checking permission of a block which is not in cache"<<endl;
        return INVALID;
    }

    void remove_orb(struct msg & m){
        int id = -1;
        for(int i = 0; i<outstanding_req_buffer.size(); i++){
            if(outstanding_req_buffer[i].addr == m.addr){
                if(outstanding_req_buffer[i].is_write == 0 && (m.msg_name == MSG_PUT || m.msg_name == MSG_PUTE)){
                    id = i;
                    break;
                    // REMOVE
                }
                else if(outstanding_req_buffer[i].is_write == 1 && (m.msg_name == MSG_PUTX || m.msg_name == MSG_PUTE)){
                    id = i;
                    break;
                    // REMOVE
                }

            }
        }
        // if(m.addr == 34356566963ULL){
        //     printf("SMg: %lld, %lld, %lld, %lld, %d\n", m.addr, m.sender, m.receiver, m.msg_name, id);
        // }
        if(id == -1){
            // printf("SMg: %lld, %lld, %lld, %lld\n", m.addr, m.sender, m.receiver, m.msg_name);
            printf("Kuch to gadbad hai daya\n");
            return;
        }
        outstanding_req_buffer.erase(outstanding_req_buffer.begin() + id);
    }

    int dev(){
        int x = 0;
        return x + 1;
    }


    void remove_orb_ma(struct mem_access & ma){
        
        int id = -1;
        for(int i = 0; i<outstanding_req_buffer.size(); i++){
            if(outstanding_req_buffer[i].addr == ma.addr){
                if(outstanding_req_buffer[i].is_write == ma.is_write){
                    id = i;
                    break;
                    // REMOVE
                }
            }
        }
        if(id == -1){
            // printf("Kuch to gadbad hai daya xy\n");
            return;
        }
        outstanding_req_buffer.erase(outstanding_req_buffer.begin() + id);
    }
    struct l1_cache_block insert(uint64 addr, uint64 state){
        struct l1_cache_block ret, newb;
        uint64 set = addr%l1_sets;
        ret = *all_cache_blocks[set].begin();
        all_cache_blocks[set].erase(all_cache_blocks[set].begin());
        newb.is_dirty = (state== MODIFIED);
        newb.is_valid = true;
        newb.lru = Cycle;
        newb.states = state;
        newb.tag = addr;
        all_cache_blocks[set].insert(newb);
        return ret;
    }

    void update_lru(uint64 addr, uint64 ctr){
        uint64 set = addr%l1_sets;
        for(auto it=all_cache_blocks[set].begin() ;it!=all_cache_blocks[set].end();it++){
            if(it->tag==addr && it->is_valid == true){
                auto x = *it;
                all_cache_blocks[set].erase(it);
                x.lru = ctr;
                all_cache_blocks[set].insert(x);
            }
        }
    }

    void update_state(uint64 addr, int state){
        int done = 0;
        struct l1_cache_block x;
        uint64 set = addr % l1_sets;
        for(auto it=all_cache_blocks[set].begin() ;it!=all_cache_blocks[set].end();it++){
            if(it->tag==addr && it->is_valid == true){
                x.is_dirty = (state == MODIFIED);
                x.is_valid = (state != INVALID);
                x.lru = (state == INVALID ? 0 : Cycle);
                x.states = state;
                x.tag = addr;
                all_cache_blocks[set].erase(it);
                all_cache_blocks[set].insert(x);
                done = 1;
                break;
            }
        }
        if(!done){
            cout << "Update error: " << addr <<"  " << state << "\n";
        }
    }


};


#define L2_INVALID 0
#define L2_UNOWNED 1
#define L2_SHARED 2
#define L2_EM 3
#define L2_PSH 4
#define L2_PDEX 5


struct l2_cache_block{
    bool is_valid;
    uint64 tag;
    uint64 dirty;
    vector<uint64> dentry;
    uint64 dstate;
    uint64 lru;

    l2_cache_block(){
        is_valid = false;
        tag = 0;
        dirty = 0;
        dstate = 0;
        dentry = vector<uint64>(8, 0);
        lru = 0;
    }

};

struct l2_cache_block_cmp {
    bool operator() (struct l2_cache_block b1, struct l2_cache_block b2){
        
        return b1.lru< b2.lru;
    }
};

uint64 l2_bank_size = 512*1024;
uint64 l2_bank_line_size=64;
uint64 l2_bank_assoc = 16;
uint64 l2_bank_sets = l2_bank_size/(l2_bank_line_size*l2_bank_assoc);



struct l2_bank{

    queue<struct msg> msg_incoming;

    vector<multiset<l2_cache_block,l2_cache_block_cmp>> all_cache_blocks;
    vector<l2_cache_block> pending_buffer; 
    vector<l2_cache_block> evict_buffer;

    //all utility functions
    l2_bank(){
        for(int i=0;i<l2_bank_sets;i++){
            multiset<l2_cache_block,l2_cache_block_cmp> one_cache_line;
            for(int j=0;j<l2_bank_assoc;j++){
                struct l2_cache_block one_cache_block;
                one_cache_line.insert(one_cache_block);
            }
            all_cache_blocks.push_back(one_cache_line);
        }
    }

    uint64 get_dstate(uint64 addr){
        auto set = addr % l2_bank_sets;
        for(auto &x : all_cache_blocks[set]){
            if(x.tag == addr && x.dstate != L2_INVALID){
                return x.dstate;
            }
        }
        return L2_INVALID;
    }

    vector<uint64> get_dentry(uint64 addr){
        vector<uint64> ret(8, 11);
        auto set = addr % l2_bank_sets;
        for(auto &x : all_cache_blocks[set]){
            if(x.tag == addr && x.dstate != L2_INVALID){
                return x.dentry;
            }
        }
        return ret;
    }
    
    int deb(){
        int x = 34;
        x++;
        return x;
    }

    uint64 get_owner(uint64 addr){
        int oid = -1;
        auto set = addr % l2_bank_sets;
        for(auto &x : all_cache_blocks[set]){
            if(x.tag == addr && x.dstate != L2_INVALID){
                for(int i = 0; i<8; i++){
                    if(x.dentry[i] == 1){
                        return i;
                    }
                }
            }
        }
        deb();
        printf("Get Owner Error\n");
        return oid;
    }

    struct l2_cache_block insert(uint64 addr, int l1id, uint64 dstate){
        l2_misses++;
        // cout << "Insert " << addr << " "<< l1id <<  " " << dstate << "\n";
        struct l2_cache_block newb;
        auto set = addr % l2_bank_sets;
        auto start = *all_cache_blocks[set].begin();
        all_cache_blocks[set].erase(all_cache_blocks[set].begin());
        newb.dstate = dstate;
        newb.dirty = (dstate == L2_EM);
        newb.is_valid = (dstate != L2_INVALID);
        newb.tag = addr;
        newb.lru = (dstate == L2_INVALID ? 0 : Cycle);
        newb.dentry[l1id] = 1; // Assuming only L2_EM 
        all_cache_blocks[set].insert(newb);
        return start;
    }

    void update_dad(uint64 addr, uint64 dstate, vector<uint64> dentry){
        struct l2_cache_block newb;
        auto set = addr % l2_bank_sets;
        for(auto it = all_cache_blocks[set].begin(); it != all_cache_blocks[set].end(); it++){
            if(it->tag == addr && it->is_valid == true){
                newb.dstate = dstate;
                newb.lru = (dstate == L2_INVALID ? 0 : Cycle);
                if(dstate == L2_PSH || dstate == L2_PDEX){
                    newb.lru += 1e9;
                }
                newb.dirty = (dstate == L2_EM);
                newb.dentry = dentry;
                newb.is_valid = (dstate != L2_INVALID);
                newb.tag = addr;
                all_cache_blocks[set].erase(it);
                all_cache_blocks[set].insert(newb);
                return;
            }
        }
        cout << Cycle << " " << addr << " " << dstate << " ";
        for(auto x : dentry){
            cout << x << ",";
        }
        cout<<"\n";
    }
};

 