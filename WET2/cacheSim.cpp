/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <vector>

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

class Node{
public:
	unsigned long int address;
	Node* next;
	Node* prev;
	Node(unsigned long int address, Node* next, Node* prev):
		address(address), next(next), prev(prev) {};
	~Node();
};

//this is an LRUcache class including: cache size, block size, associativity level
//(how many ways), access time (how many clock cycles), write allocate policy (bool)
//this cache should be able to store addresses of 4 bytes and determine if it is a hit or miss
//eviction policy is LRU
//the cache is implemented using a hash table
class Cache{
private:
	unsigned int level;
	unsigned int size;
	unsigned int blockSize;
	unsigned int assoc;
	unsigned int accessTime;
	bool writeAlloc;
	vector<Node*> head;
	vector<Node*> tail;
	// unordered_map<unsigned long int, Node*> cache;
	vector <unordered_map<unsigned long int, Node*>> cache;


public:
	Cache(){
		level = 0;
		size = 0;
		blockSize = 0;
		assoc = 0;
		accessTime = 0;
		writeAlloc = false;
		for(int i = 0; i < assoc; i++){
			unordered_map<unsigned long int, Node*> temp;
			cache.push_back(temp);
			head[i] = new Node(0, nullptr, nullptr);
			tail[i] = new Node(0, nullptr, nullptr);
			head[i]->next = tail[i];
			tail[i]->prev = head[i];
		}

	}
	Cache(unsigned int level, unsigned int size, unsigned int blockSize, unsigned int assoc
			, unsigned int accessTime, bool writeAlloc):
		level(level), size(size), blockSize(blockSize), assoc(assoc), accessTime(accessTime),
		writeAlloc(writeAlloc){
			head.resize(assoc);
			tail.resize(assoc);

			for(int i = 0; i < assoc; i++){
				unordered_map<unsigned long int, Node*> temp;
				cache.push_back(temp);
				head[i] = new Node(0, nullptr, nullptr);
				tail[i] = new Node(0, nullptr, nullptr);
				head[i]->next = tail[i];
				tail[i]->prev = head[i];
			}
			if(assoc == 0){
				unordered_map<unsigned long int, Node*> temp;
				cache.push_back(temp);
				head[0] = new Node(0, nullptr, nullptr);
				tail[0] = new Node(0, nullptr, nullptr);
				head[0]->next = tail[0];
				tail[0]->prev = head[0];
			}
			cache.resize(assoc);
	}
	~Cache(){
		for(int i = 0; i < assoc; i++){
			Node* temp = head[i];
			while(temp != nullptr){
				Node* temp2 = temp->next;
				delete temp;
				temp = temp2;
			}
		}
		if(assoc == 0){
			Node* temp = head[0];
			while(temp != nullptr){
				Node* temp2 = temp->next;
				delete temp;
				temp = temp2;
			}
		}
	}
	// in order to look for the block in the cache we are going to set all of the
	//offset bits to 0
	unsigned long int getBlockAddress(unsigned long int address){
		return address >> this->blockSize;
	} 

	//this function finds the set in the cache
	int findSet(unsigned long int address){
		if(this->assoc==0) return 0;
		address = getBlockAddress(address);
		uint32_t set = address % this->assoc; //this is the set (explaination:
		//taking the modulo of a binary by n is the same as taking the last n bits)
		//mod of 0 is undefined in cpp
		return set;
	}



	//this function puts the block in the cache
	void putBlock(unsigned long int address){
		int set = findSet(address);
		address = getBlockAddress(address);
		if(cache[set].size() == this->size/this->assoc){
			Node* temp = tail[set]->prev;
			temp->prev->next = tail[set];
			tail[set]->prev = temp->prev;
			cache[set].erase(temp->address);
			delete temp;
		}
		Node* temp = new Node(address, head[set]->next, head[set]);
		head[set]->next->prev = temp;
		head[set]->next = temp;
		cache[set][address] = temp;
 	}
	//this function gets the block from the cache
	int getBlock(unsigned long int address){
		int set = findSet(address);
		address = getBlockAddress(address);
		if(cache[set].find(address) == cache[set].end()){
			return -1;
		}
		Node* temp = cache[set][address];
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
		head[set]->next->prev = temp;
		temp->next = head[set]->next;
		temp->prev = head[set];
		head[set]->next = temp;
		return 0;
	}

};





double L1MissRate;
double L2MissRate;
double avgAccTime;


//this is the main function
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
		Cache L1(1, L1Size, BSize, L1Assoc, L1Cyc,WrAlloc);
		Cache L2(2, L2Size, BSize, L2Assoc, L2Cyc,WrAlloc);
	}

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

	}

	// double L1MissRate;
	// double L2MissRate;
	// double avgAccTime;

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
