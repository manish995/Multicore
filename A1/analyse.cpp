#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<fstream>
#include <cassert>
#include "cache.h"

struct entry {
    char i_or_d;
    char type;
    unsigned long long addr;
    unsigned pc;
};

void printHelp(){
    std::cout << "\nUsage: ./analyse <policy> where policy can be 'i' for Inclusive Policy, 'n' for Not-Inclusive-Not-Exclusive Policy, 'e' for Exclusive Policy.\n";
    std::cout << "The program will output the results using that policy.\n";
}

std::map<std::string, std::vector<std::string>> apps = {
    {"bzip2", {"bzip2.log_l1misstrace_0", "bzip2.log_l1misstrace_1"}}, 
    {"gcc", {"gcc.log_l1misstrace_0", "gcc.log_l1misstrace_1"}}, 
    {"gromacs", {"gromacs.log_l1misstrace_0"}}, 
    {"h264ref", {"h264ref.log_l1misstrace_0"}}, 
    {"hmmer", {"hmmer.log_l1misstrace_0"}}, 
    {"sphinx3", {"sphinx3.log_l1misstrace_0", "sphinx3.log_l1misstrace_1"}}
};
std::string BASE_PATH = "./traces/";

// Function that returns trace for Belady replacment policy
std::vector<unsigned long long> getTrace(std::pair<const std::string, std::vector<std::string>> &app){
    std::vector<unsigned long long> trace;
    for(auto path: app.second){
        std::fstream file;
        struct entry temp;
        file.open(BASE_PATH + path, std::ios::in|std::ios::binary);
        if(file){
            while(!file.eof()){
                file.read((char *)&temp.i_or_d, sizeof(char));
                file.read((char *)&temp.type, sizeof(char));
                file.read((char *)&temp.addr, sizeof(unsigned long long));
                file.read((char *)&temp.pc, sizeof(unsigned));
                if(temp.type){
                    trace.push_back(temp.addr);
                }
                
            }
            file.close();
        }   
    }
    return trace;
}


// Processes all L1 miss addresses
std::vector<std::pair<unsigned long long, unsigned long long>> solve(Memory& mem, std::pair<const std::string, std::vector<std::string>> &app){
    for(auto path: app.second){
        std::fstream file;
        struct entry temp;
        file.open(BASE_PATH + path, std::ios::in|std::ios::binary);
        if(file){
            while(!file.eof()){
                file.read((char *)&temp.i_or_d, sizeof(char));
                file.read((char *)&temp.type, sizeof(char));
                file.read((char *)&temp.addr, sizeof(unsigned long long));
                file.read((char *)&temp.pc, sizeof(unsigned));
                if(temp.type){
                    mem.handlePkt(temp.addr);
                }
            }
            file.close();
        }   
    }
    return mem.getStats();
}

int main(int argc, char** argv){
    std::cout << "Application to analyse traces\n";
    if(argc != 2){
        printHelp();
        exit(0);
    }

    // Choosing the policy for inclusivity
    int policy = INCLUSIVE_POLICY;
    bool solveP2 = false;
    if(argv[1][0] == 'i'){
        std::cout << "Choosing Inclusive Policy\n";
        policy = INCLUSIVE_POLICY;
    }
    else if(argv[1][0] == 'n'){
        std::cout << "Choosing NINE Policy\n";
        policy = NINE_POLICY;
    }
    else if(argv[1][0] == 'e'){
        std::cout << "Choosing Exclusive Policy\n";
        policy = EXCLUSIVE_POLICY;
    }
    else if(argv[1][0] == '2'){
        std::cout << "Problem 2 : Inclusive Policy. Will show results for problem 1 and 2\n";
        policy = INCLUSIVE_POLICY;
        solveP2 = true;
    }
    else{
        printHelp();
        exit(0);
    }

    // Setting up memory and cache layers
    int BLOCK_SIZE = 64;
    int L2_WAYS = 8;
    int L2_SIZE = 512 * 1024;
    int L3_WAYS = 16;
    int L3_SIZE = 2*1024*1024;
    LRUCache L2(L2_WAYS, BLOCK_SIZE, L2_SIZE), L3(L3_WAYS, BLOCK_SIZE, L3_SIZE);
    Memory mem({L2, L3}, policy);

    for(auto app: apps){
        mem.reset(policy);
        std::cout << "\nAnalysing trace for " << app.first << "\n";
        auto stats = solve(mem, app);
        std::cout << "Printing Statistics\n";
        int layer_id = 2;
        for(const auto& stat: stats){
            std::cout << "Layer " << layer_id << " Hits and Misses:\n";
            unsigned long long hits = stat.first, misses = stat.second;
            long long per = 100.0 * hits / ( hits + misses);
            std::cout << hits << "(" << (per) << "%)" << "\t" << misses << "(" << (100 - (per))<< "%)" << "\n";
            layer_id++;
        }
        if(solveP2){
            std::cout << "Printing Statistics for Problem 2\n";
            std::cout << "Layer 3 misses:\n";

            // Already counted current misses
            long long currentMisses = stats.back().second;

            // Counting cold misses for L3
            LRUCache L2__(L2_WAYS, BLOCK_SIZE, L2_SIZE), L3__(L3_WAYS, BLOCK_SIZE, L3_SIZE, 1);
            Memory mem3({L2__, L3__}, INCLUSIVE_POLICY);
            long long coldMisses = solve(mem3, app).back().second;

            // Counting misses with using a fully associative 
            // cache with LRU replacement policy  for L3
            LRUCache L2_(L2_WAYS, BLOCK_SIZE, L2_SIZE), L3_(L3_SIZE/BLOCK_SIZE , BLOCK_SIZE, L3_SIZE, 2);
            Memory mem2({L2_, L3_}, INCLUSIVE_POLICY);
            long long fullAssocMisses = solve(mem2, app).back().second;
            std::cout << "Full Associative Cache Misses: " << fullAssocMisses << "\n";
            
            // Outputing results
            std::cout << "Cold Misses: " << coldMisses << "\n";
            std::cout << "Capacity Misses: " << fullAssocMisses - coldMisses << "\n";
            std::cout << "Conflict Misses: " << currentMisses - fullAssocMisses << "\n";
            
            
            // Counting misses with using a fully associative 
            // cache with Belady replacement policy for L3
            std::vector<unsigned long long> trace = getTrace(app);
            LRUCache L2B(L2_WAYS, BLOCK_SIZE, L2_SIZE, 4), L3B(L3_SIZE/BLOCK_SIZE, BLOCK_SIZE, L3_SIZE, 3);
            Memory mem4({L2B, L3B}, INCLUSIVE_POLICY);
            mem4.setTrace(trace);
            long long fullAssocMissesBelady = solve(mem4, app).back().second;
            std::cout << "Full Associative Cache Misses (With Belady): " << fullAssocMissesBelady << "\n";

            // Accounting for belady's optimal policy in fully associative case
            std::cout << "\nMisses with Belady:\n";
            std::cout << "Cold Misses: " << coldMisses << "\n";
            std::cout << "Capacity Misses: " << fullAssocMissesBelady - coldMisses << "\n";
            std::cout << "Conflict Misses: " << currentMisses - fullAssocMissesBelady << "\n";
        }
    }
    
    return 0;
}