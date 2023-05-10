#include "cache.h"
#include <cassert>
#include <iostream>

#define X_MASK(p) (unsigned long long)((1<<(p)) - 1)

// LRUCache
LRUCache::LRUCache(int ways, int block_size, int sz, int flag){
    this->ways = ways;
    this->offset = ceil(log2(block_size));
    int sets =  sz / ( ways * block_size);
    assert(sets >= 1);
    this->index = ceil(log2(sets));
    this->misses = 0;
    this->hits = 0;
    this->mem = nullptr;
    this->solvep2 = flag;
    this->initialise();
}

// Initialise data structures
void LRUCache::initialise(){
    this->cache.clear();
    this->coldctr.clear();
    this->fullAssoc.clear();
    this->history.clear();
    for(int i = 0; i<(1<<(this->index)); i++){
        std::multiset<std::pair<int, Block>> temp;
        for(int j = 0; j<this->ways; j++){
            temp.insert({INT32_MIN,{0ULL, 0}});
        }
        this->cache.push_back(temp);
    }
}

// Sets trace for use by belady algorithm
void Memory::setTrace(std::vector<unsigned long long>& hist){
    int i = 0;
    for(const auto& addr: hist){
        unsigned long long block_addr = addr / 64;
        this->bTrace[block_addr].push(i);
        i++;
    }    
    for(auto& p: this->bTrace){
        this->bTrace[p.first].push(INT32_MAX);
    }
}

inline unsigned long long LRUCache::getBlockAddr(unsigned long long addr) const {
    return addr >> (this->offset);
}

inline unsigned long long LRUCache::getTag(unsigned long long block_addr) const {
    return block_addr >> (this->index);
}

inline unsigned long long LRUCache::getAddr(unsigned long long tag, int indexBits) const{
    return ((tag << this->index) + indexBits) << this->offset;
}

// Update cache data structure's lru state
void LRUCache::update(unsigned long long block_addr){
    assert(this->solvep2 == 3);
    int indexBits = block_addr & X_MASK(this->index);
    unsigned long long tag = this->getTag(block_addr);
    if(fullAssoc.count(tag) && fullAssoc[tag].second.valid){
        this->cache[indexBits].erase(this->cache[indexBits].lower_bound(this->fullAssoc[tag]));
        this->fullAssoc.erase(tag);
        long long lru = this->mem->bTrace[block_addr].front();
        this->cache[indexBits].insert({-lru, {tag, 1}});
        this->fullAssoc[tag] = {-lru, {tag, 1}};
    }
}

bool LRUCache::search(unsigned long long addr) {
    unsigned long long block_addr = getBlockAddr(addr);
    if(this->solvep2 == 1){
        this->coldctr.insert(block_addr);
    }
    if(this->solvep2 == 4){
        this->mem->bTrace[block_addr].pop();
        this->mem->cache_layers[1].update(block_addr);
    }
    int indexBits = block_addr & X_MASK(this->index);
    unsigned long long tag = this->getTag(block_addr);
    if(this->solvep2 == 2){
        this->history.push_back(addr);
        if(this->fullAssoc.count(tag) && this->fullAssoc[tag].second.valid){
            this->hits++;
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(this->fullAssoc[tag]));
            this->fullAssoc.erase(tag);
            this->cache[indexBits].insert({this->mem->timer, {tag, 1}});
            this->fullAssoc[tag] = {this->mem->timer, {tag, 1}};
            return true;
        }
        else{
            this->misses++;
            return false;
        }
    }
    else if(this->solvep2 == 3){
        if(this->fullAssoc.count(tag) && this->fullAssoc[tag].second.valid){
            this->hits++;
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(this->fullAssoc[tag]));
            this->fullAssoc.erase(tag);
            long long lru = this->mem->bTrace[block_addr].front();
            this->cache[indexBits].insert({-lru, {tag, 1}});
            this->fullAssoc[tag] = {-lru, {tag, 1}};
            return true;
        }
        else{
            this->misses++;
            return false;
        }
    }
    for(auto& block: this->cache[indexBits]){
        if(block.second.valid && block.second.tag == tag){
            this->hits++;
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(block));
            this->cache[indexBits].insert({this->mem->timer, {tag, 1}});
            // Hits in this layer of cache
            return true;
        }
    }
    // Missed in this layer of cache
    this->misses++;
    return false;
}

unsigned long long LRUCache::insert(unsigned long long addr, bool& evicted){
    unsigned long long victim = 0;
    unsigned long long block_addr = getBlockAddr(addr);
    int indexBits = block_addr & X_MASK(this->index);

    // Pointer to last block in set which will be invalid or LRU replacement victim
    auto block = this->cache[indexBits].begin();
    if(block->second.valid == false){
        evicted = false;
        if(solvep2 == 3){
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(*block));
            long long lru = this->mem->bTrace[block_addr].front();
            this->cache[indexBits].insert({-lru, {this->getTag(block_addr), 1}});
            this->fullAssoc[this->getTag(block_addr)] = std::make_pair(-lru, Block(this->getTag(block_addr), 1)); 

        }
        else{
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(*block));
            this->cache[indexBits].insert({this->mem->timer, {this->getTag(block_addr), 1}});
            if(solvep2 == 2){
                this->fullAssoc[this->getTag(block_addr)] = std::make_pair(this->mem->timer, Block(this->getTag(block_addr), 1)); 
            }   
        }
        
    }
    else{
        // Evicting this block
        evicted = true;
        if(solvep2 == 3){
            long long lru = this->mem->bTrace[block_addr].front();
            this->fullAssoc.erase(block->second.tag);
            this->fullAssoc[this->getTag(block_addr)] = std::make_pair(-lru, Block(this->getTag(block_addr), 1)); 
            victim = this->getAddr(block->second.tag, indexBits);
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(*block));
            this->cache[indexBits].insert({-lru, {this->getTag(block_addr), 1}});
        }
        else{
            if(solvep2 == 2){
                this->fullAssoc.erase(block->second.tag);
                this->fullAssoc[this->getTag(block_addr)] = std::make_pair(this->mem->timer, Block(this->getTag(block_addr), 1)); 
            }
            victim = this->getAddr(block->second.tag, indexBits);
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(*block));
            this->cache[indexBits].insert({this->mem->timer, {this->getTag(block_addr), 1}});
        }
    }
    return victim;
}

// Invalidates block with given address if found in the cache
void LRUCache::invalidate(unsigned long long addr){
    unsigned long long block_addr = getBlockAddr(addr);
    int indexBits = block_addr & X_MASK(this->index);
    auto tag = getTag(block_addr);
    if(solvep2 == 2){
        if(this->fullAssoc.count(tag) && this->fullAssoc[tag].second.valid){
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(this->fullAssoc[tag]));
            this->fullAssoc.erase(tag);
            this->cache[indexBits].insert({INT32_MIN,{0ULL, 0}});
        }
        return;
    }
    for(auto block: this->cache[indexBits]){
        if(block.second.tag == tag && block.second.valid){
            this->cache[indexBits].erase(this->cache[indexBits].lower_bound(block));
            this->cache[indexBits].insert({INT32_MIN,{0ULL, 0}});
            break;
        }
    }
}

void LRUCache::setMem(Memory* ptr){
    this->mem = ptr;
}

std::pair<unsigned long long, unsigned long long> LRUCache::getStats() const {
    
    if(solvep2 == 0){
        return {this->hits, this->misses};
    }
    else if(solvep2 == 1){
        // Returning cold misses
        return {this->hits, this->coldctr.size()};
    }
    else if(solvep2 == 2){
        return {this->hits, this->misses};
    }
    else if(solvep2 == 3){
        return {this->hits, this->misses};
    }
    else if(solvep2 == 4){
        return {this->hits, this->misses};
    }
    return {0, 0};
}

unsigned long long LRUCache::getBeladyMisses(){
    int ways = this->ways;
    int i = 0;
    std::unordered_map<unsigned long long, std::queue<int>> trace;
    for(const auto& addr: this->history){
        unsigned long long block_addr = addr / 64;
        trace[block_addr].push(i);
        i++;
    }    
    for(auto& p: trace){
        trace[p.first].push(INT32_MAX);
    }
    std::set<std::pair<long long, unsigned long long>> beladyCache;
    std::map<unsigned long long, int> fullAssocMap;
    unsigned long long misses = 0;
    for(const auto& addr: this->history){
        unsigned long long block_addr = addr / 64;
        auto cur = trace[block_addr].front();
        trace[block_addr].pop();
        auto next = trace[block_addr].front();
        assert(beladyCache.size() <= ways);
        if(fullAssocMap.count(block_addr)){
            assert(fullAssocMap[block_addr] == cur);
            auto ptr = beladyCache.find({-fullAssocMap[block_addr], block_addr});
            beladyCache.erase(ptr);
            beladyCache.insert({-next, block_addr});
            fullAssocMap[block_addr] = next;
        }
        else{
            misses++;
            if(beladyCache.size() < ways){
                // Invalid ways present
                beladyCache.insert({-next, block_addr});
                fullAssocMap[block_addr] = next;

            }
            else{
                // Evict some block
                auto ptr = beladyCache.begin();
                fullAssocMap.erase(ptr->second);
                beladyCache.erase(ptr);
                beladyCache.insert({-next, block_addr});
                fullAssocMap[block_addr] = next;
            }
        }
    }  
    return misses;
}


void LRUCache::reset(){
    this->misses = 0;
    this->hits = 0;
    this->initialise();
}

// Memory
Memory::Memory(std::vector<LRUCache> v, int policy_id){
    this->cache_layers = v;
    for(auto& cache: this->cache_layers){
        cache.setMem(this);
    }
    this->policy_id = policy_id;
    this->timer = 0;
}

void Memory::handlePkt(unsigned long long addr){
    int i = 0;
    for(i = 0; i<this->cache_layers.size(); i++){
        if(this->cache_layers[i].search(addr)){
            // Hit in this cache layer
            break;
        }
    }
    switch(policy_id){
        case INCLUSIVE_POLICY:
            { 
                this->implInclusivePolicy(addr, i);
                break;
            }
        case NINE_POLICY:
            { 
                this->implNINEPolicy(addr, i);
                break;
            }
        case EXCLUSIVE_POLICY:
            { 
                this->implExclusivePolicy(addr, i);
                break;
            }
        default:
            std::cout << "Policy not implemented\n";
            exit(0);

    }
    this->timer++;
}

void Memory::implInclusivePolicy(unsigned long long addr, int hit_layer){
    
    for(int j = hit_layer - 1; j>=0; j--){
        bool evicted;
        unsigned long long victim = this->cache_layers[j].insert(addr, evicted);
        if(evicted){
            for(int k = j-1; k>=0; k--){
                this->cache_layers[k].invalidate(victim);
            }
        }
    }
}

void Memory::implNINEPolicy(unsigned long long addr, int hit_layer){
    for(int j = hit_layer - 1; j>=0; j--){
        bool evicted;
        unsigned long long victim = this->cache_layers[j].insert(addr, evicted);
    }
}

void Memory::implExclusivePolicy(unsigned long long addr, int hit_layer){
    if(hit_layer != 0){
        if(hit_layer < this->cache_layers.size()){
            this->cache_layers[hit_layer].invalidate(addr);
        }
        bool evicted;
        unsigned long long victim = this->cache_layers[0].insert(addr, evicted);
        int j = 1;
        while(evicted && j < this->cache_layers.size()){
            victim = this->cache_layers[j].insert(victim, evicted);
            j++;
        }
    }
}


std::vector<std::pair<unsigned long long, unsigned long long>> Memory::getStats(){
    std::vector<std::pair<unsigned long long, unsigned long long>> v;
    for(const auto& cache: this->cache_layers){
        v.push_back(cache.getStats());
    }
    return v;
}

// With using L3 Trace history with mem2
unsigned long long Memory::getBeladyMisses(){
    return this->cache_layers[1].getBeladyMisses();
}

void Memory::reset(int policy_id){
    this->policy_id = policy_id;
    for(auto& cache: this->cache_layers){
        cache.reset();
    }
    this->timer = 0;
}

