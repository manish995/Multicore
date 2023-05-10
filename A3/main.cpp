#include<bits/stdc++.h>
#include "structure.h"
using namespace std;

vector<struct l1_cache> l1_cache_array(8);
vector<struct l2_bank> l2_bank_array(8);
uint64 l1_accs, l1_misses;
map<int, int> l1_count, l2_count;
uint64 upgr_misses;
unordered_map<uint64, int> reason;

void send_msg(uint64 layer, uint64 recv, struct msg & mx){
    if(layer == 1){
        l1_count[mx.msg_name]++;
        l1_cache_array[recv].msg_from_l2_or_l1_cache.push_back(mx);
    }
    else{
        l2_count[mx.msg_name]++;
        l2_bank_array[recv].msg_incoming.push(mx);
    }
}


void process_l1_input(uint64 i , struct mem_access & ma){
    // printf("Input proc %llu\n", i);
    if(ma.addr == 0){
        return;
    }
    l1_accs++;
    if(ma.is_write == 0){
        // read
;        // if is cache hit
        // if(ma.addr == 34111106320ULL && ma.global_ctr == 0){
        //     cout << "Here\n";
        // }
        if(l1_cache_array[i].is_cache_hit(ma.addr)){
            // then return and update lru
            l1_cache_array[i].update_lru(ma.addr, Cycle);
            if(ma.global_ctr == 0){
                l1_cache_array[i].remove_orb_ma(ma);
            }
        }
        else{
            l1_misses++;
            if(reason[ma.addr]){
                upgr_misses++;
            }
            // else 
            // check for outstanding msg queue 
            if(ma.global_ctr == 0){
                uint64 home = ma.addr&7;
                struct msg mx = {MSG_GET, i, ma.addr, home};
                send_msg(2, home, mx);
                // l2_bank_array[home].msg_incoming.push(mx);
                return;
            }
            for(auto &x: l1_cache_array[i].outstanding_req_buffer){
                if(x.addr == ma.addr){
                    x.global_ctr = max(x.global_ctr, ma.global_ctr);
                    return;
                }
            }
            // otherwise send message to l2 homebank to 
            uint64 home = ma.addr&7;
            struct msg mx = {MSG_GET, i, ma.addr, home};
            send_msg(2, home, mx);
            // l2_bank_array[home].msg_incoming.push(mx);
            ma.orb_name = MSG_GET;
            l1_cache_array[i].outstanding_req_buffer.push_back(ma);
            // request for block and put this in outstanding vector
            return;
        }
    }
    else{
        
        if(l1_cache_array[i].is_cache_hit(ma.addr)){
            // write
            // if cache hit
            uint64 state = l1_cache_array[i].cache_block_state(ma.addr);
            if(state == EXCLUSIVE){
            // is state is E, silently modify data and change state to M  and update lru
                l1_cache_array[i].update_state(ma.addr, MODIFIED);
                if(ma.global_ctr == 0){
                    l1_cache_array[i].remove_orb_ma(ma);
                } 
            }
            else if(state == SHARED){
        //         if(ma.addr == 34356566963ULL){
        //     printf("SMgXIN: %lld, %lld, %lld, %d\n", ma.addr, ma.global_ctr, ma.is_write, 0);
        // }
                if(ma.global_ctr == 0){
                    uint64 home = ma.addr&7;
                    struct msg mx = {MSG_UPGR, i, ma.addr, home};
                    send_msg(2, home, mx);
                    // l2_bank_array[home].msg_incoming.push(mx);
                    return;
                }
            // if state is S, send upgrade request and check outstanding req buffer
                for(auto &x: l1_cache_array[i].outstanding_req_buffer){
                    if(x.addr == ma.addr && x.orb_name == MSG_UPGR){
                        x.global_ctr = max(x.global_ctr, ma.global_ctr);
                        return;
                    }
                }
                // otherwise send message to l2 homebank to 
                uint64 home = ma.addr&7;
                struct msg mx = {MSG_UPGR, i, ma.addr, home};
                send_msg(2, home, mx);
                // l2_bank_array[home].msg_incoming.push(mx);
                ma.orb_name = MSG_UPGR;
                l1_cache_array[i].outstanding_req_buffer.push_back(ma);
                // It is possible that due to race condition, the home bank
                // may send invalidation request for this block
                // IN such a case, we receive NACK request, due to which
                // we repeat the request again
            }
            else if(state == MODIFIED){
                // if(ma.addr == 34111106320ULL && ma.global_ctr == 1566015){
                //     cout << "Camer herer modi : " << ma.global_ctr << " " <<  ma.is_write << " \n";
                // }
                // if state is M, modify update lru 
                l1_cache_array[i].update_lru(ma.addr, Cycle);
                if(ma.global_ctr == 0){
                    l1_cache_array[i].remove_orb_ma(ma);
                } 
            }
            else{
                printf("Should never come here\n");
            }
            // and return
        }
        else{
            l1_misses++;
            if(reason[ma.addr]){
                upgr_misses++;
            }
            //  Write Miss
            if(ma.global_ctr == 0){
                uint64 home = ma.addr&7;
                struct msg mx = {MSG_GETX, i, ma.addr, home};
                send_msg(2, home, mx);
                // l2_bank_array[home].msg_incoming.push(mx);
                return;
            }
            for(auto &x: l1_cache_array[i].outstanding_req_buffer){
                if(x.addr == ma.addr && x.orb_name == MSG_GETX){
                    x.global_ctr = max(x.global_ctr, ma.global_ctr);
                    return;
                }
            }
            // otherwise send message to l2 homebank to 
            uint64 home = ma.addr&7;
            struct msg mx = {MSG_GETX, i, ma.addr, home};
            send_msg(2, home, mx);
            // l2_bank_array[home].msg_incoming.push(mx);
            ma.orb_name = MSG_GETX;
            l1_cache_array[i].outstanding_req_buffer.push_back(ma);
        }
        
        // else
        // check for outstanding msq queue for write request
        // to l2 home bank otherwise request for block
        // and put this in outstanding request queue. 
    }
}

void process_l1_msg_inc(uint64 i, struct msg & m){
    // printf("L1 proc %d , %lld, %d\n", i, m.addr, m.msg_name);
    if(m.addr == 0){
        return;
    }
    switch(m.msg_name){
        case MSG_GET: {
            // Expects forwarded GET request from L2:
            // Possible states: I, E, M; Couldn't have block in S state
            auto state = l1_cache_array[i].cache_block_state(m.addr);
            if(state == INVALID){
                for(auto& x: l1_cache_array[i].putx_buf){
                    if(x.putx.addr == m.addr){
                        // Special case
                        assert(x.pend.addr == 0);
                        x.pend = m;
                    }
                }

                // Eat 5 star and do nothing

            }
            else if(state == EXCLUSIVE || state == MODIFIED){
                // 
                l1_cache_array[i].update_state(m.addr, SHARED);
                uint64 home = m.addr&7;
                struct msg mx = {MSG_SWB, i, m.addr, home};
                send_msg(2, home, mx);
                // l2_bank_array[home].msg_incoming.push(mx);
                struct msg mx2 = {MSG_PUT, i, m.addr, m.sender}; // m.sender is original requester
                send_msg(1, m.sender, mx2);
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx2);
            }
            else{
                // printf("Should sometimes come here 3\n");
                for(auto& x: l1_cache_array[i].putx_buf){
                    if(x.putx.addr == m.addr){
                        // Special case
                        assert(x.pend.addr == 0);
                        x.pend = m;
                    }
                }
            }
            // If state is I: It has evicted dirty block from M to I (race condition)
                // Ignore GET and home will respond with WB_ACK for WB
                // Don't know why WB_ACK is required (else we will use a buffer to maintain outstanding reqs.)
            // If state is E or M: 
                // We will transition to S state
                // Home is in PSH state
                // We need to send SWB to home node and 
                // PUT request to  original requester (GET.sender)
            break;
        }
        case MSG_GETX: {
            // Expects forwarded GETX request from L2:
            // Possible states: I, E, M; Couldn't have block in S state

            auto state = l1_cache_array[i].cache_block_state(m.addr);
            if(state == INVALID){
                for(auto& x: l1_cache_array[i].putx_buf){
                    if(x.putx.addr == m.addr){
                        // Special case
                        assert(x.pend.addr == 0);
                        x.pend = m;
                    }
                }
                // Eat 5 star and do nothing
                // send_msg(1, m.sender, mx);
            }
            else if(state == EXCLUSIVE || state == MODIFIED){
                l1_cache_array[i].update_state(m.addr, INVALID);
                reason[m.addr] = 0;
                uint64 home = m.addr&7;
                struct msg mx = {MSG_OT, i, m.addr, home};
                send_msg(2, home, mx);
                // l2_bank_array[home].msg_incoming.push(mx);
                struct msg mx2 = {MSG_PUTX, i, m.addr, m.sender, 0, 0}; // m.sender is original requester
                send_msg(1, m.sender, mx2);
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx2);
            }
            else{
                // printf("Err\n");
                // Special race condition when directory apparently thinks
                //  we are in M state, but sadly we are not
                for(auto& x: l1_cache_array[i].putx_buf){
                    if(x.putx.addr == m.addr){
                        // Special case
                        assert(x.pend.addr == 0);
                        x.pend = m;
                    }
                }
            }

            // If state is I: It has evicted dirty block from M to I (race condition)
                // Ignore GET and home will respond with WB_ACK for WB
                // Don't know why WB_ACK is required (else we will use a buffer to maintain outstanding reqs.)
            
            // If state is E or M:
                // We will transition to I state by evicting the block (remove the block)
                // Home is in PDEX state; will transition to M state after receiving OT
                // We need to send PUTX to original requester(GETX.sender)
                // We need to send OT to home node

            break;
        }
        case MSG_INVAL: {
            // Possible states: I, S;
            auto state = l1_cache_array[i].cache_block_state(m.addr);
            if(state == INVALID){
                struct msg mx2 = {MSG_INVAL_ACK, i, m.addr, m.sender}; // m.sender is original requester
                send_msg(1, m.sender, mx2);
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx2);
            }
            else if(state == SHARED){

                // Invalidate the set
                if(m.upgr_miss){
                    reason[m.addr] = 1;
                }
                else{
                    reason[m.addr] = 0;
                }
                l1_cache_array[i].update_state(m.addr, INVALID);
                struct msg mx2 = {MSG_INVAL_ACK, i, m.addr, m.sender}; // m.sender is original requester
                send_msg(1, m.sender, mx2);
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx2);
            }
            // If state is I: Transitioned silently from S to I
                // We will send a INVAL_ACK to requester(INVAL.sender)
                // Sender can't be home, (it iwill send MSG_INVAL_L2) 
                // send INVAL_ACK
            // If state is S:
                // Invalidate the block
                // Send INVAL_ACK to requester(INVAL.sender) (L1)
            break;
        }
        case MSG_INVAL_ACK: {
            // Possible states: I, S
            auto state = l1_cache_array[i].cache_block_state(m.addr);
            uint64 done = 0;
            uint64 ctr = 0;
            for(auto &x : l1_cache_array[i].putx_buf){
                if(x.putx.addr == m.addr){
                    // Match found
                    done = true;
                    x.putx.inval_ack_exp--;
                    if(x.putx.inval_ack_exp == 0){
                        struct l1_cache_block evicted;
                        if(state == INVALID){
                            if(x.putx.is_upgr_reply){
                                // S to I 
                                // Re request from I to M
                                struct mem_access ma;
                                ma.global_ctr = 0;
                                ma.addr = m.addr;
                                ma.is_write = 1;
                                ma.orb_name = MSG_GETX;
                                l1_cache_array[i].snacks.push({Cycle + 5, ma});
                            }
                            else{
                                // Trying to go from I to M
                                evicted = l1_cache_array[i].insert(m.addr, MODIFIED);
                                if(evicted.states == EXCLUSIVE || evicted.states == MODIFIED){
                                    struct msg mx = {MSG_SIMPL_WB, i, evicted.tag, evicted.tag & 7};
                                    send_msg(2, evicted.tag & 7, mx);
                                    reason[evicted.tag] = 0;
                                    // l2_bank_array[evicted.tag & 7].msg_incoming.push(mx);
                                }
                                l1_cache_array[i].remove_orb(x.putx);
                            }
                            
                        }
                        else if(state == SHARED){
                            l1_cache_array[i].update_state(m.addr, MODIFIED);
                            l1_cache_array[i].remove_orb(x.putx);
                        }
                        else{
                            printf("Gadbad\n");
                        }
                        l1_cache_array[i].putx_buf.erase(l1_cache_array[i].putx_buf.begin() + ctr);
                        if(x.pend.addr != 0){
                            l1_cache_array[i].msg_from_l2_or_l1_cache.push_front(x.pend);
                        }
                    }
                    break;
                }
                ctr++;
            }
            if(!done){
                printf("INval ACK error\n");
            }
            // If state I: Do decrement of block counter in 
            // buffer till we reach the number specified in PUT msg
            // Check request type was GETX: Then insert that block and evict another block B2 by LRU
                // Make evict function for making space in the set
                // If this block B2 is in S state, we silently evict
                // Else if it is in E/M state, we send SIMPL_WB msg and wait for WB_ACK in a buffer
            // Check request type was Upgr: If the block has been evicted from S to I and is in I state, then re request for I to M (GETX) (add to out. req. buffer to )
            
            // If state S: DO decrement till we reach  0 from number
            // Then upgrade permission for the block to M state if te block is still in S
            break;
        }
        case MSG_NACK: {
            // Possible states: I, S, E, M
            struct mem_access ma;
            ma.global_ctr = 0;
            ma.addr = m.addr;
            ma.is_write = m.nack_is_write;
            ma.orb_name = m.msg_name;
            l1_cache_array[i].snacks.push({Cycle + 5, ma});
            // Insert the NACKed request to snack queue with updated counter (cycle+5)
            break;
        }
        case MSG_PUT: {
            l1_cache_array[i].remove_orb(m);
            // Remove corresponding GET request from out. req. buffer
            // Possible state: I
            struct l1_cache_block evicted;
            evicted = l1_cache_array[i].insert(m.addr, SHARED);
            if(evicted.states == MODIFIED || evicted.states == EXCLUSIVE){
                uint64 home = evicted.tag & 7;
                struct msg mx = {MSG_SIMPL_WB, i, evicted.tag, home};
                send_msg(2, home, mx);
                reason[evicted.tag] = 0;
                // l2_bank_array[home].msg_incoming.push(mx);
                // send simplwb
            }            
            // Insert the cache block received in S state after possibly evicting some other block (Use function)
            break;
        }
        case MSG_PUTE: {
            // Remove corresponding GET request from out. req. buffer
            l1_cache_array[i].remove_orb(m);
            struct l1_cache_block evicted;
            evicted = l1_cache_array[i].insert(m.addr, EXCLUSIVE);
            if(evicted.states == MODIFIED || evicted.states == EXCLUSIVE){
                uint64 home = evicted.tag & 7;
                struct msg mx = {MSG_SIMPL_WB, i, evicted.tag, home};
                send_msg(2, home ,mx);
                reason[evicted.tag] = 0;
                // l2_bank_array[home].msg_incoming.push(mx);
                // send simplwb
            }  
            // Possible states: I
            // Insert the cache block received in E state after possibly evicting some other block (Use function)
            break;
        }
        case MSG_PUTX: {
                 
            // Remove corresponding GETX/Upgr request from out. req. buffer
            auto state = l1_cache_array[i].cache_block_state(m.addr);
            if(state == INVALID){
            
                if(m.inval_ack_exp == 0){
                    struct l1_cache_block evicted;
                    evicted = l1_cache_array[i].insert(m.addr, MODIFIED);
                    if(evicted.states == MODIFIED || evicted.states == EXCLUSIVE){
                        uint64 home = evicted.tag & 7;
                        struct msg mx = {MSG_SIMPL_WB, i, evicted.tag, home};
                        send_msg(2,home, mx);
                        reason[evicted.tag] = 0;
                        // l2_bank_array[home].msg_incoming.push(mx);
                        // send simplwb
                    }  
                    l1_cache_array[i].remove_orb(m);
                }
                else{
                    if(m.is_upgr_reply){
                        // Fraud
                        struct mem_access ma;
                        ma.global_ctr = 0;
                        ma.addr = m.addr;
                        ma.is_write = 1;
                        ma.orb_name = MSG_GETX;
                        l1_cache_array[i].snacks.push({Cycle + 5, ma});
                    }
                    else{
                        l1_cache_array[i].putx_buf.push_back({m});
                    }
                }
            }
            else if(state == SHARED){        
                if(m.inval_ack_exp == 0){
                    // Update state S to M
                    l1_cache_array[i].update_state(m.addr, MODIFIED);                    
                    l1_cache_array[i].remove_orb(m);
                }
                else{
                    l1_cache_array[i].putx_buf.push_back({m});
                }
            }
            // Possible states: I, S
            // Setup counter in some buffer so that we receive that number of INVAL_ACK (number from PUTX)
            // Also setup which request type (GETX or Upgr) has (WTF DOnt know) 
            // triggered PUTX so that INVAL_ACK result can be interpretd corretly
            break;
        }
        case MSG_WB_ACK: {
            // Do nothing
            // Remove the corresponding WB request in the outstanding request buffer
            break;
        }
        case MSG_INVAL_L2: {
            // Possible states: I, S, E, M
            auto state = l1_cache_array[i].cache_block_state(m.addr);
            if(state == INVALID){
                for(auto&x: l1_cache_array[i].putx_buf){
                    if(x.putx.addr == m.addr){
                        // Special
                        assert(x.pend.addr == 0);
                        x.pend = m;
                    }
                }
            }
            else if(state == EXCLUSIVE || state == MODIFIED){
                uint64 home = m.addr & 7;
                struct msg mx = {MSG_INVAL_L2_REPLY, i, m.addr, home};
                send_msg(2, home, mx);
                l1_cache_array[i].update_state(m.addr, INVALID);
                reason[m.addr] = 0;
                // l2_bank_array[home].msg_incoming.push(mx);
            }
            else{
                l1_cache_array[i].update_state(m.addr, INVALID);
                reason[m.addr] = 0;
            }
             // Send MSG_INVAL_L2_REPLY. Add data if E/M state
            break;
        }
        default: {
            printf("Shouldn't come here %lld\n", m.msg_name);

        }
    }

}

void l2_evicted(uint64 l2id, struct l2_cache_block & b){
    if(b.dstate == L2_INVALID){
        return;
    }
    else if(b.dstate == L2_SHARED){
        for(uint64 i = 0; i<8; i++){
            struct msg mx = {MSG_INVAL_L2, l2id, b.tag, i, 0, 0, 0};
            if(b.dentry[i] == 1){
                // Send INVAL_L2
                send_msg(1, i, mx);
                // l1_cache_array[i].msg_from_l2_or_l1_cache.push_back(mx);
            }
        }
    }
    else if(b.dstate == L2_EM){
        for(uint64 i = 0; i<8; i++){
            struct msg mx = {MSG_INVAL_L2, l2id, b.tag, i, 0, 0, 0};
            if(b.dentry[i] == 1){
                // Send INVAL_L2
                send_msg(1,i, mx);
                // l1_cache_array[i].msg_from_l2_or_l1_cache.push_back(mx);
            }
        }
        l2_bank_array[l2id].evict_buffer.push_back(b);
    }
    else if(b.dstate == L2_PSH || b.dstate == L2_PDEX){
        l2_bank_array[l2id].pending_buffer.push_back(b);   
    }
}

void process_l2_msg_inc(uint64 i, struct msg & m){
    // printf("L2 proc %d , %lld, %d\n", i, m.addr, m.msg_name);
    if(m.addr == 0){
        return;
    }
    switch(m.msg_name){
        case MSG_GET: {
            // cout << "adsc\n"
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            // cout <<" sdvds\n";
            if(dstate == L2_INVALID){
                // L2 miss
                // cout << "Hello\n";
                struct l2_cache_block evicted = l2_bank_array[i].insert(m.addr, m.sender, L2_EM);
                l2_evicted(i, evicted);
                struct msg mx = {MSG_PUTE, i, m.addr, m.sender, 0, 0, 0};
                send_msg(1, m.sender, mx);
                
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
            }
            else{
                // L2 hit
                if(dstate == L2_PSH || dstate == L2_PDEX){
                    struct msg mx = {MSG_NACK, i, m.addr, m.sender, 0, 0, 0}; // nack+is_write = 0, due to get
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_UNOWNED){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    tdent[m.sender] = 1;
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_PUTE, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_EM){
                    m.receiver = l2_bank_array[i].get_owner(m.addr);
                    l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(m);
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    tdent[m.sender] = 1;
                    l2_bank_array[i].update_dad(m.addr, L2_PSH, tdent);
                    send_msg(1, m.receiver, m);
                }
                else if(dstate == L2_SHARED){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    tdent[m.sender] = 1;
                    l2_bank_array[i].update_dad(m.addr, L2_SHARED, tdent);
                    struct msg mx = {MSG_PUT, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
            }
            // Check if the block is in L2 cache:
            // If No: 
                // Then evict some block, put it in evict buffer and insert the current block
                // Set directory state of block to EM 
                // And send PUT_E to the corresponding L1 requester (GET.sender)
            // If Yes:
                // If directory state is pending:
                    // Return NACK msg to sender for corresponding request
                // Directory states: U, EM, S
                    // Unowned:
                        // Set the block to EM state in directory and reply with PUT_E
                    // EM:
                        // Forward the GET request after changing receiver to the owner marked in directory
                        // Set directory state to PSH
                    // Shared:
                        // Update the directory state for the new sharer:
                        // Send a PUT request to the sender (GET.sender)

            break;
        }
        case MSG_GETX: {
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            if(dstate == L2_INVALID){
            //          if(m.addr == 2198820285657ULL){
            //     cout << m.msg_name << " " << m.sender <<" "<< m.receiver <<"\n";
            // }
                // L2 miss
                struct l2_cache_block evicted = l2_bank_array[i].insert(m.addr, m.sender, L2_EM);
                l2_evicted(i, evicted);
                struct msg mx = {MSG_PUTX, i, m.addr, m.sender, 0, 0, 0};
                send_msg(1, m.sender, mx);
                // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
            }
            else{
                // L2 hit
                if(dstate == L2_PSH || dstate == L2_PDEX){
                    struct msg mx = {MSG_NACK, i, m.addr, m.sender, 0, 0, 1}; // nack_is_write = 1, due to getX
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_UNOWNED){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    tdent[m.sender] = 1;
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_PUTX, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_EM){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    auto owner = l2_bank_array[i].get_owner(m.addr);
                    if(m.sender == owner){
                        struct msg mx = {MSG_NACK, i, m.addr, owner, 0, 0, 1};
                        send_msg(1, owner, mx);
                        // l1_cache_array[owner].msg_from_l2_or_l1_cache.push_back(mx);
                    }
                    else{
                        tdent[m.sender] = 1;
                        tdent[owner] = 0;
                        m.receiver = owner;
                        send_msg(1, owner, m);
                        // if(m.addr == 34268577069ULL){
                        // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(m);
                        l2_bank_array[i].update_dad(m.addr, L2_PDEX, tdent);
                    }
                }
                else if(dstate == L2_SHARED){

                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    // if(tdent[m.sender] == 1){
                    //      struct msg mx = {MSG_NACK, i, m.addr, m.sender, 0, 0, 1}; // nack_is_write = 1, due to getX
                    //     send_msg(1, m.sender, mx);
                    //     // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                    //     return;
                    // }
                    uint64 sum = 0;
                    for(uint64 ctr = 0; ctr<8; ctr++){
                        if(tdent[ctr] == 1 && ctr != m.sender){
                            struct msg mx = {MSG_INVAL, m.sender, m.addr, ctr, 0, 0, 0}; // sender = original sender
                            send_msg(1, ctr, mx);
                            // l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(mx);
                            tdent[ctr] = 0;
                            sum++;
                        }
                    }
                    tdent[m.sender] = 1;
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_PUTX, i, m.addr, m.sender, sum, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
            }
            // Check if the block is in L2 cache:
            // If No: 
                // Then evict some block, put it in evict buffer and insert the current block
                // Set directory state of block to M
                // And send PUT_X to the corresponding L1 requester (GET.sender) NUmber of exp. INVAL_ACKs = 0
            // If Yes:
                // If directory state is pending:
                    // Return NACK msg to sender for corresponding request
                // Directory states: U, EM, S
                    // Unowned:
                        // Set the block to EM state in directory and reply with PUT_X (INVAL_ACKs = 0)
                    // EM:
                        // Forward the GET_X request after changing receiver to the owner marked in directory
                        // Change directory state to PDEX
                    // Shared:
                        // Send INVAL to all sharers with SENDER ID to send INVAL_ACKs
                        // Update the directory state to EM
                        // Send a PUTX request to the sender (GET.sender)
            break;
        }
        case MSG_UPGR: {
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            if(dstate == L2_INVALID){
                printf("Inclusivity error\n");
            }
            else{
                if(dstate == L2_PSH || dstate == L2_PDEX){
                    struct msg mx = {MSG_NACK, i, m.addr, m.sender, 0, 0, 1}; // nack_is_write = 1, due to Upgr
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_EM){
                    struct msg mx = {MSG_NACK, i, m.addr, m.sender, 0, 0, 1}; // nack_is_write = 1, due to Upgr
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_SHARED){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    uint64 sum = 0;
                    for(uint64 ctr = 0; ctr<8; ctr++){
                        if(tdent[ctr] == 1 && ctr != m.sender){
                            struct msg mx = {MSG_INVAL, m.sender, m.addr, ctr, 0, 0, 0, 1}; // sender = original sender
                            send_msg(1, ctr, mx);
                            // l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(mx);
                            tdent[ctr] = 0;
                            sum++;
                        }
                    }
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_PUTX, i, m.addr, m.sender, sum, 1, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
            }
            // Check if the block is in L2 cache:
            // If No: 
                // Not possible due to inclusivity
            // If Yes:
                // If directory state is pending:
                    // Return NACK msg to sender for corresponding request
                // Directory states: U, EM, S
                    // Unowned:
                        // Not possible
                    // EM:
                        // Contradiction
                        // Send NACK
                    // Shared:
                        // Send INVAL to all sharers except sender with SENDER ID to send INVAL_ACKs
                        // Update the directory state to EM
                        // Send a PUTX request to the sender (GET.sender) and set field
            break;
        }
        case MSG_SWB: {

            // Inconsistent dentry check
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            if(dstate == L2_INVALID){
                uint64 done = 0;
                uint64 ctr = 0;
                for(auto &x: l2_bank_array[i].pending_buffer){
                    if(x.tag == m.addr && x.dstate == L2_PSH){
                        // remove from pending buffer
                        for(uint64 y = 0; y<8; y++){
                            if(x.dentry[y] == 1){
                                struct msg mx = {MSG_INVAL_L2, i, m.addr, y, 0, 0, 0};
                                send_msg(1, y, mx);
                                // l1_cache_array[y].msg_from_l2_or_l1_cache.push_back(mx);
                            }
                        }
                        l2_bank_array[i].pending_buffer.erase(l2_bank_array[i].pending_buffer.begin() + ctr);
                        done = 1;
                    }
                    ctr++;
                }
                if(!done){
                    printf("SWB Invalid Error\n");
                }
            }
            else if(dstate == L2_PSH){
                auto tdent = l2_bank_array[i].get_dentry(m.addr);
                l2_bank_array[i].update_dad(m.addr, L2_SHARED, tdent);
            }
            else{
                printf("bug\n");
            }
            // Check if block is in pending state in L2 :
            // If No:
                // It should be in pending buffer
                // The eviction will send invalidations
            // If Yes:
                // You must in PSH
                // PSH: Set directory state to S
            break;
        }
        case MSG_SIMPL_WB: {
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            if(dstate == L2_INVALID){
                for(auto it = l2_bank_array[i].evict_buffer.begin(); it != l2_bank_array[i].evict_buffer.end(); it++){
                    if(it->tag == m.addr && it->dstate == L2_EM){
                        struct msg mx = {MSG_WB_ACK, i, m.addr, m.sender, 0, 0, 0};
                        send_msg(1, m.sender, mx);
                        // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                        l2_bank_array[i].evict_buffer.erase(it);
                        return;
                    }
                }
                for(auto it = l2_bank_array[i].pending_buffer.begin(); it != l2_bank_array[i].pending_buffer.end(); it++){
                    if(it->tag == m.addr){
                        for(uint64 ctr = 0; ctr<8; ctr++){
                            if(it->dentry[ctr] == 1 && ctr != m.sender){
                                struct msg nack = {MSG_NACK, i, m.addr, ctr, 0, 0, (it->dstate == L2_PDEX)};
                                send_msg(1, ctr, nack);
                                l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(nack);
                                break;
                            } 
                        }
                        struct msg mx = {MSG_WB_ACK, i, m.addr, m.sender, 0, 0, 0};
                        send_msg(1, m.sender, mx);
                        // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                        l2_bank_array[i].pending_buffer.erase(it);
                        return;
                    }
                }
                printf("Gadbad hai\n");

            }
            else{
                if(dstate == L2_PDEX){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    for(uint64 ctr = 0; ctr<8; ctr++){
                        if(tdent[ctr] == 1 && ctr != m.sender){
                            struct msg mx = {MSG_PUTX, i, m.addr, ctr, 0, 0, 0};
                            send_msg(1, ctr, mx);
                            // l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(mx);
                            tdent[m.sender] = 0; // not needed
                            break;
                        }
                    }
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_WB_ACK, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_PSH){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    for(uint64 ctr = 0; ctr<8; ctr++){
                        if(tdent[ctr] == 1 && ctr != m.sender){
                            struct msg mx = {MSG_PUTE, i, m.addr, ctr, 0, 0, 0};
                            send_msg(1, ctr, mx);
                            // l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(mx);
                            tdent[m.sender] = 0;
                            break;
                        }
                    }
                    l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
                    struct msg mx = {MSG_WB_ACK, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }
                else if(dstate == L2_EM){
                    auto tdent = l2_bank_array[i].get_dentry(m.addr);
                    tdent[m.sender] = 0;
                    l2_bank_array[i].update_dad(m.addr, L2_UNOWNED, tdent);
                    struct msg mx = {MSG_WB_ACK, i, m.addr, m.sender, 0, 0, 0};
                    send_msg(1, m.sender, mx);
                    // l1_cache_array[m.sender].msg_from_l2_or_l1_cache.push_back(mx);
                }

            }
            // Check is block is in L2 :
            // If no:
                // If block is in evicted buffer with EM state, then only send WB_ACK to requester
                // Else if block in pending buffer
                // NACK the request to try it again
                // Send the WB_ACK to the L1 requester who has evicted
            // If yes:
                // If pending state is PDEX:
                    // Send data to PDEX requester with PUT_X and set directory state to EM
                    // Send WB_ACK to the sender (SIMPL_WB.sender)
                // If pending state IS PSH
                    // Send data to PSH requester with PUT_E and set directory state to EM
                    // Send WB_ACK to the sender (SIMPL_WB.sender)
                // Else if EM state:
                    // Change directory state to Unowned state
                    // Send WB_ACK to the sender (SIMPL_WB.sender)
            break;
        }
        case MSG_OT: {
            auto dstate = l2_bank_array[i].get_dstate(m.addr);
            if(dstate == L2_INVALID){
                // IT MUST BE IN PENDING
                for(auto it = l2_bank_array[i].pending_buffer.begin(); it != l2_bank_array[i].pending_buffer.end(); it++){
                    if(it->tag == m.addr){
                        for(uint64 ctr = 0; ctr<8; ctr++){
                            if(it->dentry[ctr] == 1 && ctr != m.sender){
                                struct msg inval = {MSG_INVAL_L2, i, m.addr, ctr, 0, 0, 0};
                                send_msg(1, ctr, inval);
                                // l1_cache_array[ctr].msg_from_l2_or_l1_cache.push_back(inval);
                                break;
                            } 
                        }
                        l2_bank_array[i].evict_buffer.push_back(*it);
                        l2_bank_array[i].pending_buffer.erase(it);
                        return;
                    }
                }
            }
            else if(dstate == L2_PDEX){
                auto tdent = l2_bank_array[i].get_dentry(m.addr);
                l2_bank_array[i].update_dad(m.addr, L2_EM, tdent);
            }
            // Update directory state for block from PDEX to M for new owner(which is already set)
            break;
        }
        case MSG_INVAL_L2_REPLY: {
            for(auto it = l2_bank_array[i].evict_buffer.begin(); it != l2_bank_array[i].evict_buffer.end(); it++){
                if(it->tag == m.addr && it->dstate == L2_EM){
                    l2_bank_array[i].evict_buffer.erase(it);
                    return;
                }
            }
            // Remove block from evicted buffer if number of 
            // invalid replies match that is required after incrementing count (no need of count due to only EM owener present)
            break;
        }
        default: {
            printf("Shouldn't come here %lld\n", m.msg_name);
        }
    }
}

int main()
{

    //Initialisng 8 l1_cache    
    //Initialising 8 l2_cache_bank

    vector<ifstream > traces(8);
    
    for(uint64 i=0;i<8;i++){
        string name = "trace";
        traces[i].open(name + to_string(i) + ".out", ios::binary);

    }
    string myline;
    set<uint64> uni;
    for(uint64 i=0;i<8;i++){
        struct mem_access m; 
        while(!traces[i].eof())
        {
            traces[i].read((char*)&m.global_ctr, sizeof(uint64));
            traces[i].read((char*)&m.addr, sizeof(uint64));
            traces[i].read((char*)&m.is_write, sizeof(uint64));
            if(traces[i].eof()){
                break;
            }
            // auto cx = stringstream(myline);
            // cx >> m.global_ctr >> m.addr >> m.is_write;
            m.addr >>= 6;
            uni.insert(m.addr);
            l1_cache_array[i].req_from_processor.push(m);
        }
        cout <<"Trace"<<i<<" Size: "<<l1_cache_array[i].req_from_processor.size()<<"\n";
    }
    Cycle = 1;
    long long cur_gid = -1;
    while(1){
        uint64 flag = 0;
        vector<struct mem_access> temp_ma(8);
        vector<pair<long long, int>> reqv;
        
        // Pop queue and get data
        for(uint64 i = 0; i<8; i++){
            // Check for snacks
            if( l1_cache_array[i].snacks.size()){
                flag = true;
                if(l1_cache_array[i].snacks.front().first == Cycle){
                    temp_ma[i] = l1_cache_array[i].snacks.front().second;
                    l1_cache_array[i].snacks.pop();
                }
            }
            else{
                // Make sorting array for normal memory access
                if(l1_cache_array[i].req_from_processor.size()){
                    reqv.push_back({l1_cache_array[i].req_from_processor.front().global_ctr, i});
                    flag = true;
                }
            }
            
        }
        sort(reqv.begin(), reqv.end());
        for(auto x: reqv){
            // Check for normal memory accesses
            if(x.first == cur_gid + 1){        
                cur_gid++;
                temp_ma[x.second] = l1_cache_array[x.second].req_from_processor.front();
                l1_cache_array[x.second].req_from_processor.pop();
            }
            else{
                // Normal memory access condition was found to be not correct and no snacks 
                break;
            }
        }

        vector<struct msg> temp_msg_l1(8), temp_msg_l2(8);

        for(uint64 i= 0; i<8; i++){
            if(l1_cache_array[i].msg_from_l2_or_l1_cache.size()){
                temp_msg_l1[i] = l1_cache_array[i].msg_from_l2_or_l1_cache.front();
                l1_cache_array[i].msg_from_l2_or_l1_cache.pop_front();
                flag = true;
            }
        }

        for(uint64 i= 0; i<8; i++){
            if(l2_bank_array[i].msg_incoming.size()){
                temp_msg_l2[i] = l2_bank_array[i].msg_incoming.front();
                l2_bank_array[i].msg_incoming.pop();
                flag = true;
            }
        }

        // Process all data

        // Process L1 input
        for(uint64 i = 0; i<8; i++){
            // if(temp_ma[i].addr == 34356566963ULL){
            //     cout << temp_ma[i].global_ctr <<" " << temp_ma[i].is_write <<"\n";
            // }
            process_l1_input(i, temp_ma[i]);
        }

        // Process L1 incoming msg
        for(uint64 i = 0; i<8; i++){
            // if(temp_msg_l1[i].addr == 34356566963ULL){
            //     cout << temp_msg_l1[i].msg_name << " " << temp_msg_l1[i].sender << " " << temp_msg_l1[i].receiver << "\n";
            // }
            process_l1_msg_inc(i, temp_msg_l1[i]);
        }

        // Process L2 incoming msg
        for(uint64 i = 0; i<8; i++){
            // if(temp_msg_l2[i].addr == 34356566963ULL){
            //     cout << temp_msg_l2[i].msg_name << " " << temp_msg_l2[i].sender << " " << temp_msg_l2[i].receiver << "\n";
            // }
            process_l2_msg_inc(i, temp_msg_l2[i]);
        }

        if(!flag){
            break;
        }
        Cycle++;
    }

    cout<<"FINAL LENGTHS OF ALL 8 L1 CACHE AND 8 L2 CACHE QUEUES"<<endl;
    for(uint64 i = 0; i<8; i++){
        cout <<"L1 Outstanding reg buffer queue Size: "<< l1_cache_array[i].outstanding_req_buffer.size() << " L1 Msg Queue Size: " << l1_cache_array[i].msg_from_l2_or_l1_cache.size() << " L1 Putx Inval Ack Waiting Queue Size: " << l1_cache_array[i].putx_buf.size()<< " L1 Nacks Queue Size: " << l1_cache_array[i].snacks.size() << "\n";
        cout <<"L2 Incoming Queue Size: "<<  l2_bank_array[i].msg_incoming.size() << " L2 Evict Buffer Len: " << l2_bank_array[i].evict_buffer.size() << " L2 Pending state buffer Len: " << l2_bank_array[i].pending_buffer.size() << "\n";
        // for(auto &x : l1_cache_array[i].outstanding_req_buffer){
        //     cout << x.addr << " " << x.global_ctr <<" " << x.is_write << "\n";
        // }

        cout<<endl;
        traces[i].close();
    }
    cout << "RESULTS:\n";
    cout << "------------------------------\n";
    cout << "Number of cycles: " << Cycle << "\n";
    cout << "Number of unique block addresses: " << uni.size() << "\n";
    cout << "Number of L1 Accesses: " << l1_accs << " L1 Misses: " << l1_misses << "\n";
    cout << "Upgrade Misses: " << upgr_misses << "\n";
    cout << "Number of L2 Misses: " << l2_misses << "\n";
    cout << "L1 Message counts\n";
    for(auto& x : l1_count){
        cout << msg_names[x.first] << " " << x.second << "\n";
    }
    cout << "L2 Message counts\n";
    for(auto& x : l2_count){
        cout << msg_names[x.first] << " " << x.second << "\n";
    }
    return 0;
}