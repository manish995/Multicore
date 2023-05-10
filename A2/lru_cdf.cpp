#include<bits/stdc++.h>
using namespace std;

struct cache_block{
    long long valid=0;
    long long lru=0;
    long long tag=0;
};

struct mem_access{
    long long addr;
    long long tid;
    long long len;
};


struct stats_of_cache{
    long long hits=0;
    long long misses=0;
    long long only_insert_called=0;
    long long evict_insert_called=0;
};

struct stats_of_cache stats;

void print_cache_blk(struct cache_block blk){
    cout<<blk.valid<<" "<<blk.tag<<" "<<blk.lru<<endl;
}


long long capacity = 2*1024*1024;
long long assoc=16;
long long block_size=64;
long long num_blocks= capacity/(block_size);
long long num_sets=num_blocks/assoc;

vector<vector<cache_block>> cache(num_sets,vector<cache_block>(assoc));
long long counter=0;


fstream file;
vector<long long> misses;
unordered_map<long long,vector<long long>> blocks;
map<long long,long long>cdf;

void evict(long long eff_addr){
    long long set = eff_addr%num_sets;
    long long mini_index=-1;
    long long mini_lru=LONG_LONG_MAX;
    for(long long i=0;i<assoc;i++){
        if(mini_lru>cache[set][i].lru && cache[set][i].valid==1){
            mini_index=i;
            mini_lru=cache[set][i].lru;
        }
        else if(mini_lru>cache[set][i].lru && cache[set][i].valid==0){
            cout<<"Wrong insertion implemented"<<endl;
        }

    }
    if(mini_index==-1){cout<<"Some Error in Eviction"<<endl;return;}

    cache[set][mini_index].valid=0;
}


void insert(long long eff_addr){
    long long set=eff_addr%num_sets;
    // cout<<"eff_addr "<<eff_addr<<endl;
    for(long long i=0;i<assoc;i++){
        if(cache[set][i].valid == 0){
            struct cache_block new_cache_block ;
            new_cache_block.valid=1;
            new_cache_block.lru=counter;
            new_cache_block.tag=eff_addr;
            cache[set][i]=new_cache_block;

            // print_cache_blk(new_cache_block);
            // print_cache_blk(cache[set][i]);
            
            stats.only_insert_called++;
            return;
        }
    }
    evict(eff_addr);
    stats.evict_insert_called++;
    insert(eff_addr);


}

void CacheWalk(long long eff_addr){
    long long set=eff_addr%num_sets;
    for(long long i=0;i<assoc;i++){
        if(cache[set][i].tag==eff_addr && cache[set][i].valid==1){
            cache[set][i].lru=counter;
            stats.hits++;
            return;
        }
        
    }
    stats.misses++;
    misses.push_back(eff_addr);
    insert(eff_addr);//Miss
}


void print_cache_stats(){

    cout<<"STATS: "<<endl;
    cout<<"Total hit in cache: "<<stats.hits<<endl;
    cout<<"Total misses in cache: "<<stats.misses<<endl;
    cout<<"Insert: "<<stats.only_insert_called<<endl;
    cout<<"Evict: "<<stats.evict_insert_called<<endl;

}


void print_cache_characterstics(){
    cout<<"Capacity: "<<capacity<<endl;
    cout<<"Number of sets: "<<num_sets<<endl;
    cout<<"Associativity: "<<assoc<<endl;
    cout<<"Number of blocks: "<<num_blocks<<endl;
}

void split_line(string line,struct mem_access * temp){
    char *token = strtok((char *)line.c_str()," ");
  
    // Keep printing tokens while one of the
    // delimiters present in str[].
    long long i=0;
    while (token != NULL)
    {
        // printf("%s\n", token);
        if(i==0){
            temp->tid= stoi(token);
            i++;
        }
        else if(i==1){
            temp->addr= stoll(token);
            i++;
        }
        else if(i==2){
            temp->len= stoi(token);
            i++;
        }
        else{
            cout<<"Error: More than 3 words in a line"<<endl;
        }
        token = strtok(NULL, " ");
    }
}


unordered_set<long long> diff_blocks;

int main(int argc,char ** argv){

    

    if(argc!=2){
        cout<<"Please Enter the file name"<<endl;
        exit(-1);
    }
    file.open(argv[1],ios::in);
    if (file.is_open()){ 
      string line;
      struct mem_access temp;
      while(getline(file, line)){ 
        //  cout << line << "\n"; 
         split_line(line,&temp);   
         CacheWalk(temp.addr/64);
         diff_blocks.insert(temp.addr/64);
        //  cout<<temp.addr<<endl;
         counter++;
        //  num_of_lines--;
      }
      file.close(); 
   }


   for(long long i=0;i<misses.size();i++){
    blocks[misses[i]].push_back(i);
   }


    // int num_unique_blocks;
   for(auto it=blocks.begin();it!=blocks.end();it++){
    vector<long long> curr_block_access=it->second;
    for(long long i=0;i<curr_block_access.size()-1;i++){
        cdf[curr_block_access[i+1]-curr_block_access[i]]++;
    }
   }
   cout<<"Total Misses: "<<blocks.size()<<endl;
   cout<<"Unique Blocks: "<<diff_blocks.size()<<endl;


    cout<<"PRINTING CDF DATA"<<endl;
   for(auto it=cdf.begin();it!=cdf.end();it++){
    cout<<it->first<<" "<<it->second<<endl;
   }


    print_cache_stats();
    print_cache_characterstics();
   

   

}