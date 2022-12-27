/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <vector>
#include <math.h>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;
using std::unordered_map;
using std::set;
using std::vector;


class _Set{

public:

    vector<unsigned> ways; //pair of: (tag, last modification counter)
    vector<unsigned> modification_times;
    vector<bool> dirty;
    unsigned assoc;//assoc level
    unsigned counter;
    unsigned size;
    unsigned index_bits;
    unsigned access_time;
    unsigned total_mem_access;
    unsigned total_misses;


    _Set():ways(1), modification_times(1), dirty(1){
        counter=0;
        assoc = 0;//assoc level
        size = 0;
        index_bits = 0;
        access_time = 0;
        total_mem_access = 0;
        total_misses = 0;
    }

    unsigned get_tag_index(unsigned address){
        unsigned j = 0;
        for (auto & i : ways) {
            j++;
            if(i == address){
                return j;
            }
        }
        return -1;
    }

    unsigned find_lru_way_index (){
        unsigned min = 0;
        for(auto i = 0 ; i < modification_times.size() ; i++) {

            if (modification_times[min] > modification_times[i]) {
                min = i;
            }
        }
        cout << "lru index is " << min << " address is " << ways[min] << endl;
        return min;
    }

};


class Cache_sim{
public:
    //cace sim params:
    unsigned block_size;
    unsigned wr_alloc;
    unsigned mem_cycle;
    unsigned total_mem_access;
    unsigned total_accesses;
    unsigned total_time;

    //L1 related params:
    vector<_Set> L1;
    unsigned L1_size;//the size of the cache
    unsigned L1_set_num;//the number of sets in each cache = block/ways
    unsigned L1_ways;//
    unsigned L1_assoc;
    unsigned L1_index_bits;
    unsigned L1_access_time;
    unsigned L1_total_access;
    unsigned L1_total_misses;

    //L2 related params:
    vector<_Set> L2;
    unsigned L2_size;
    unsigned L2_set_num;
    unsigned L2_ways;
    unsigned L2_assoc;
    unsigned L2_index_bits;
    unsigned L2_access_time;
    unsigned L2_total_misses;
    unsigned L2_total_access;


    unsigned calc_set_num(unsigned cache_size, unsigned num_of_ways, unsigned block_size){
        unsigned temp = num_of_ways * block_size;
        return cache_size / temp;
    }
    void do_eviction(bool isL1, unsigned set_index, unsigned way_index){

        if(isL1) {
            cout << "eviction on L1" << endl;
            if(L1[set_index].ways[way_index] != -1) {
                if (L1[set_index].dirty[way_index]) {  // Write Back
                    unsigned address = L1[set_index].ways[way_index];
                    unsigned L2_index = address >> int(log2(block_size));
                    L2_index = L2_index % (L2_set_num);
                    unsigned L2_way_index = L2[L2_index].get_tag_index(L1[set_index].ways[way_index]);

                    L2[L2_index].dirty[L2_way_index] = true;
                    L2[L2_index].counter++;
                    L2[L2_index].modification_times[L2_way_index] = L2[L2_index].counter;

                }
                cout << "address is " << L1[set_index].ways[way_index] << endl;
                L1[set_index].ways[way_index] = -1;
                L1[set_index].dirty[way_index] = false;
            }
        }
        else{
            cout << "eviction on L2" << endl;
            if(L2[set_index].ways[way_index] != -1) {
                unsigned address = L2[set_index].ways[way_index];
                unsigned L1_empty_cell = 0;
                int L1_way_index = access_cache(true, address, &L1_empty_cell);
                L1_total_access--;
                total_time -= L1_access_time;
                if(L1_way_index >= 0){

                    unsigned L1_index = address >> int(log2(block_size));
                    L1_index = L1_index % (L1_set_num);
                    do_eviction(true, L1_index, L1_way_index);
                }
                cout << "address is " << L2[set_index].ways[way_index] << endl;
                L2[set_index].ways[way_index] = -1;
                L2[set_index].dirty[way_index] = 0;
            }
        }
    }

    //initializing the structure of cache simulator:
    Cache_sim(unsigned _mem_cycle, unsigned block_size_log, unsigned _wr_alloc,
              unsigned L1_size_log, unsigned _L1_assoc, unsigned _L1_access_time,
              unsigned L2_size_log, unsigned _L2_assoc,  unsigned _L2_access_time
    ):L1(1), L2(1){
        //initialize simulator:
        mem_cycle = _mem_cycle;
        block_size = pow(2, block_size_log);
        wr_alloc = _wr_alloc;
        total_mem_access = 0;
        total_accesses = 0;
        total_time = 0;

        //initialize L1 cache:
        L1_access_time = _L1_access_time;
        L1_total_access = 0;
        L1_total_misses = 0;
        L1_size = pow(2, L1_size_log);
        L1_assoc = _L1_assoc;
        L1_ways = pow(2, L1_assoc);
        L1_set_num = calc_set_num(L1_size,L1_ways, block_size);
        L1_index_bits = log2 (L1_set_num);
        //L1 = new vector<_Set>;
        L1.resize(L1_set_num);
        for (auto i = 0; i < L1.size(); ++i){
            L1[i].ways.resize(L1_ways);
            L1[i].dirty.resize(L1_ways);
            L1[i].modification_times.resize(L1_ways);
            for(auto j = 0; j < L1_ways; ++j){
                L1[i].ways[j] = -1;
                L1[i].dirty[j] = false;
                L1[i].modification_times[j] = 0;
            }
        }

        //initialize L2 cache:
        L2_access_time = _L2_access_time;
        L2_total_access = 0;
        L2_total_misses = 0;
        L2_size = pow(2, L2_size_log);
        L2_assoc = _L2_assoc;
        L2_ways = pow(2, L2_assoc);
        L2_set_num = calc_set_num(L2_size,L2_ways, block_size);
        L2_index_bits = log2 (L2_set_num);
        //L2 = new vector<_Set>;
        L2.resize(L2_set_num);
        for (auto i = 0; i < L2.size(); ++i){
            L2[i].ways.resize(L2_ways);
            L2[i].dirty.resize(L2_ways);
            L2[i].modification_times.resize(L2_ways);
            for(auto j = 0; j < L2_ways; ++j){
                L2[i].ways[j] = -1;
                L2[i].dirty[j] = false;
                L2[i].modification_times[j] = 0;
            }
        }
    }


    int access_cache(bool isL1, unsigned address, unsigned *empty_cell) {
        //search for the address, if found return the entry in the way vector.
        //if not, but there's an empty cell, return -1, if the cache is full, return -2;
        //this function suits both caches, (using isL1 flag)
        if (isL1) {
            L1_total_access++;
            total_time += L1_access_time;
            unsigned L1_index = address;
            L1_index = L1_index >> int(log2(block_size));
            L1_index = L1_index % (L1_set_num);
            bool is_empty = false;

            int i = 0;
            for (i=0; i < L1_ways; i++) {
                if (L1[L1_index].ways[i] == address) { //if the data is found
                    return i;
                }
                else if (this->L1[L1_index].ways[i] == -1) {
                    *empty_cell = i;
                    is_empty = true;
                }
            }

            if (is_empty) {
                return -1;
            } else {
                return -2;
            }
            //return -2;
        } else {
            L2_total_access++;
            total_time += L2_access_time;
            unsigned L2_index = address;
            L2_index = L2_index >> int(log2(block_size));
            L2_index = L2_index % (L2_set_num);
            bool is_empty = false;
            int i = 0;
            for (i = 0; i < L2_ways; i++) {
                if (L2[L2_index].ways[i] == address) { //if the data is found
                    return i;
                }
                else if (this->L2[L2_index].ways[i] == -1) {
                    *empty_cell = i;
                    is_empty = true;
                }
            }
            if (is_empty) {
                return -1;
            } else {
                return -2;
            }
            //return -2;
        }
    }


    void isMiss(unsigned address, bool isWrite){
        //calculate the actual address from the given address
        address = address - address%block_size;

        //calculate l1_index
        unsigned L1_index = address >> int(log2(block_size));
        L1_index = L1_index % (L1_set_num);

        //access l1 to get the way index:
        unsigned L1_empty_cell = 0;
        int L1_way_index = access_cache(true, address, &L1_empty_cell);
        if(L1_way_index >= 0){ //means that we found the way index in l1 cache
            if(isWrite){
                L1[L1_index].dirty[L1_way_index] = true;
            }
            L1[L1_index].counter++;
            L1[L1_index].modification_times[L1_way_index] = L1[L1_index].counter;
            cout << "hit in L1 " << endl;
            return;
        }
        //if we got here, it means that the address was not found
        //update the simulator with l1 miss:
        L1_total_misses++;

        //check L2:
        unsigned L2_index = address >> int(log2(block_size));
        L2_index = L2_index % (L2_set_num);
        unsigned L2_empty_cell = 0;
        int L2_way_index = access_cache(false, address, &L2_empty_cell);
        if(L2_way_index >= 0 && !wr_alloc){//match in l2, and wr_allocate is NO write allocate
            if(isWrite){
                L2[L2_index].dirty[L2_way_index] = true;
            }
            L1[L2_index].counter++;
            L1[L2_index].modification_times[L2_way_index] = L1[L2_index].counter;
            return;
        }

        if(L2_way_index >= 0 && L1_way_index == -2){ // need to update l1
            L2[L2_index].counter++;
            L2[L2_index].modification_times[L2_way_index] = L2[L2_index].counter;
            unsigned L1_victim = L1[L1_index].find_lru_way_index();
            //do eviction for victim:
            do_eviction(true, L1_index, L1_victim);
            L1_way_index = L1_victim;
        }
        if(L2_way_index >= 0){
            L1[L1_index].ways[L1_way_index] = address;
            L1[L1_index].counter++;
            L1[L1_index].modification_times[L1_way_index] = L1[L1_index].counter;
            L1[L1_index].dirty[L1_way_index] = false;
            if(isWrite){
                L1[L1_index].dirty[L1_way_index] = true;
            }
            return;

        }
        //if we got here, it means that the address was not found
        //update the simulator with l2 miss:
        L2_total_misses++;

        //need to access to memory:
        total_time += mem_cycle;
        if(isWrite){
            if(!wr_alloc){
                return;
            }
        }
        if(L2_way_index == -2){ //means L2 is full, need to do eviction
            unsigned L2_victim = L2[L2_index].find_lru_way_index();
            //do eviction for victim:
            do_eviction(false, L2_index, L2_victim);
            L2_way_index = L2_victim;
        }
        else{
            L2_way_index = L2_empty_cell;
        }

        L2[L2_index].ways[L2_way_index] = address;

        L2[L2_index].dirty[L2_way_index] = false;
        if(isWrite){
            L2[L2_index].dirty[L2_way_index] = true;
        }
        L2[L2_index].counter++;
        L2[L2_index].modification_times[L2_way_index] = L2[L2_index].counter;

        L1_way_index = access_cache(true, address, &L1_empty_cell);
        L1_total_access--;
        total_time -= L1_access_time;
        if(L1_way_index == -2){ //means L2 is full, need to do eviction
            unsigned L1_victim = L1[L1_index].find_lru_way_index();
            //do eviction for victim:
            do_eviction(true, L1_index, L1_victim);
            L1_way_index = L1_victim;
        }
        else{
            L1_way_index = L1_empty_cell;
        }

        L1[L1_index].ways[L1_way_index] = address;
        L1[L1_index].counter++;
        L1[L1_index].modification_times[L1_way_index] = L1[L1_index].counter;
        L1[L1_index].dirty[L1_way_index] = false;
        if(isWrite){
            L1[L1_index].dirty[L1_way_index] = true;
        }
        //return;

    }
    double L1_miss_rate(){
        cout << " L1: total misses " << L1_total_misses << ", total access " << L1_total_access<< endl;
        return double(L1_total_misses) / L1_total_access;
    }
    double L2_miss_rate(){
        cout << " L2: total misses " << L2_total_misses << ", total access " << L2_total_access<< endl;
        return double(L2_total_misses) / L2_total_access;
    }
    double avg_time(){
        cout << " cache: total time " << total_time << ",  L1 total access " << L1_total_access<< endl;
        return double(total_time)/(L1_total_access);
    }




};

int main(int argc, char **argv) {

    if (argc < 19) {
        cerr << "Not enough arguments" << endl;
        return 0;
    }

    // Get input arguments

    // File
    // Assuming it is the first argument
    char* fileString = argv[1];
    ifstream file(fileString); //input file stream
    string line;
    if (!file || !file.good()) {
        // File doesn't exist or some other error
        cerr << "File not found" << endl;
        return 0;
    }

    unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
            L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

    for (int i = 2; i < 19; i += 2) {
        string s(argv[i]);
        if (s == "--mem-cyc") {
            MemCyc = atoi(argv[i + 1]);
        } else if (s == "--bsize") {
            BSize = atoi(argv[i + 1]);
        } else if (s == "--l1-size") {
            L1Size = atoi(argv[i + 1]);
        } else if (s == "--l2-size") {
            L2Size = atoi(argv[i + 1]);
        } else if (s == "--l1-cyc") {
            L1Cyc = atoi(argv[i + 1]);
        } else if (s == "--l2-cyc") {
            L2Cyc = atoi(argv[i + 1]);
        } else if (s == "--l1-assoc") {
            L1Assoc = atoi(argv[i + 1]);
        } else if (s == "--l2-assoc") {
            L2Assoc = atoi(argv[i + 1]);
        } else if (s == "--wr-alloc") {
            WrAlloc = atoi(argv[i + 1]);
        } else {
            cerr << "Error in arguments" << endl;
            return 0;
        }
    }
    /*unsigned _mem_cycle, unsigned block_size_log, unsigned _wr_alloc,
              unsigned L1_size_log, unsigned _L1_assoc, unsigned _L1_access_time,
              unsigned L2_size_log, unsigned _L2_assoc,  unsigned _L2_access_time*/
    Cache_sim cache(MemCyc, BSize, WrAlloc, L1Size, L1Assoc, L1Cyc, L2Size, L2Assoc, L2Cyc);

    while (getline(file, line)) {

        stringstream ss(line);
        string address;
        char operation = 0; // read (R) or write (W)
        if (!(ss >> operation >> address)) {
            // Operation appears in an Invalid format
            cout << "Command Format error" << endl;
            return 0;
        }

        // DEBUG - remove this line
        cout << "operation: " << operation;

        string cutAddress = address.substr(2); // Removing the "0x" part of the address

        // DEBUG - remove this line
        cout << ", address (hex)" << cutAddress;

        unsigned long int num = 0;
        num = strtoul(cutAddress.c_str(), NULL, 16);


        // DEBUG - remove this line
        cout << " (dec) " << num << endl;
        bool isWrite = false;
        if(operation == 'w')
            isWrite = true;
        cache.isMiss(num, isWrite);

    }

    double L1MissRate = cache.L1_miss_rate();
    double L2MissRate = cache.L2_miss_rate();
    double avgAccTime = cache.avg_time();

    printf("L1miss=%.03f ", L1MissRate);
    printf("L2miss=%.03f ", L2MissRate);
    printf("AccTimeAvg=%.03f\n", avgAccTime);

    return 0;
}
