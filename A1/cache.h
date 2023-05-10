#ifndef CACHE_H
#define CACHE_H

#include<vector>
#include<cmath>
#include<set>
#include<utility>
#include<map>
#include<unordered_map>
#include<queue>

#define INCLUSIVE_POLICY    0
#define NINE_POLICY         1
#define EXCLUSIVE_POLICY    2

class Block {
    public:
    unsigned long long tag;
    int valid;
    Block() : tag(0), valid(0) {}
    Block(unsigned long long tag, int valid) : tag(tag), valid(valid) {}
    bool operator<(const Block& rhs) const{
        return this->tag < rhs.tag;
    }
};

class Memory;

class LRUCache {
    private:
        int ways;
        unsigned long long index;
        int offset;
        std::vector<std::multiset<std::pair<int, Block>>> cache;
        std::set<unsigned long long> coldctr;
        std::map<long long, std::pair<int, Block>> fullAssoc; 
        std::vector<unsigned long long> history;
        int solvep2;
        Memory* mem;
        int misses;
        int hits;
        void initialise();

    public:
        LRUCache(int ways, int block_size, int sz, int flag = 0);
        inline unsigned long long getBlockAddr(unsigned long long addr) const;
        inline unsigned long long getTag(unsigned long long block_addr) const;
        inline unsigned long long getAddr(unsigned long long tag, int indexBits) const;
        bool search(unsigned long long addr);
        unsigned long long insert(unsigned long long addr, bool& evicted);
        void invalidate(unsigned long long addr);
        std::pair<unsigned long long, unsigned long long> getStats() const;
        void setMem(Memory* mem);
        unsigned long long getBeladyMisses();
        void reset();
        void update(unsigned long long addr);
};


class Memory {
    private:
        std::vector<LRUCache> cache_layers;
        int policy_id;
        int timer;
        bool solvep2;
        // Implements Inclusive Policy
        void implInclusivePolicy(unsigned long long addr, int hit_layer);
        // Implements Not-Inclusive-Not-Exclusive Policy
        void implNINEPolicy(unsigned long long addr, int hit_layer);
        // Implements Exclusive Policy
        void implExclusivePolicy(unsigned long long addr, int hit_layer);

    public:
        std::unordered_map<unsigned long long, std::queue<int>> bTrace;
        friend unsigned long long LRUCache::insert(unsigned long long addr, bool& evicted);
        friend bool LRUCache::search(unsigned long long addr);
        friend void LRUCache::setMem(Memory * ptr);
        Memory(std::vector<LRUCache> v, int policy_id);
        // Takes address which is to be loaded and returns the block 
        // which contains the address (block address is returned)
        void handlePkt(unsigned long long addr);
        void reset(int policy_id);
        std::vector<std::pair<unsigned long long, unsigned long long>> getStats();
        unsigned long long getBeladyMisses();
        void setTrace(std::vector<unsigned long long>& trace);
};


#endif