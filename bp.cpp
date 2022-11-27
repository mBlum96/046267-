/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <math.h>
#include <unordered_map>

using std::unordered_map;





class BTB_cell{
public:
    uint32_t tag;
    uint32_t target;
    int *history;
    int history_size;
    int *fsm;
    int fsm_size;
    int fsm_state;
    BTB_cell(){
        tag = 0;
        target = 0;
        history = NULL;
        history_size = 0;
        fsm = NULL;
        fsm_size = 0;
        fsm_state = 0;
    }
    BTB_cell(uint32_t tag, uint32_t target, int *history, int history_size, int *fsm, int fsm_size, int fsm_state){
        this->tag = tag;
        this->target = target;
        this->history = history;
        this->history_size = history_size;
        this->fsm = fsm;
        this->fsm_size = fsm_size;
        this->fsm_state = fsm_state;
    }
    ~BTB_cell(){
        if (history != NULL)
            delete[] history;
        if (fsm != NULL)
            delete[] fsm;
    }
};

// unordered_map<uint32_t,BTB_cell> map;

class BTB_table{
public:
    BTB_cell *table;
    int size;
    int *history;
    int history_size;
    int *fsm;
    int fsm_size;
    int fsm_state;
    bool isGlobalHist;
    bool isGlobalFSM;
    unsigned tagSize;
    int *tagArray;
    BTB_table(){
        table = NULL;
        tagArray = NULL;
        size = 0;
        history = NULL;
        history_size = 0;
        fsm = NULL;
        fsm_size = 0;
        fsm_state = 0;
        isGlobalHist = false;
        isGlobalFSM = false;
        tagSize = 0;
    }
    BTB_table(int size, int history_size,unsigned tagSize, int fsm_size, bool isGlobalHist, bool isGlobalFSM){
        this->size = size;
        this->history_size = history_size;
        this->history = NULL;
        this->fsm_size = fsm_size;
        this->fsm = NULL;
        this->isGlobalHist = isGlobalHist;
        this->isGlobalFSM = isGlobalFSM;
        this -> tagSize = tagSize;
        table = new BTB_cell[size];
        tagArray = new int [pow(2,tagSize)];
        for (int i = 0; i < size; i++){
            table[i] = BTB_cell(0, 0, history, history_size, fsm, fsm_size, fsm_state);
        }
        for(int i=0; i<pow(2,tagSize); i++){
            tagArray[i] = 0;
        }
        if(isGlobalHist){
            this->history = new int[history_size];
            for (int i = 0; i < history_size; i++){
                this->history[i] = 0;
            }
        }else{
            for (int i = 0; i < size; i++){
                table[i].history = new int[history_size];
                for (int j = 0; j < history_size; j++){
                    table[i].history[j] = 0;
                }
            }
        }
        if(isGlobalFSM){
            this->fsm = new int[fsm_size];
            for (int i = 0; i < fsm_size; i++){
                this->fsm[i] = this->fsm_state;
            }
        }else{
            for (int i = 0; i < size; i++){
                table[i].fsm = new int[fsm_size];
                for (int j = 0; j < fsm_size; j++){
                    table[i].fsm[j] = this->fsm_state;
                }
            }
        }
    }
    //coppy constructor
    BTB_table(const BTB_table &other){
        this->size = other.size;
        this->history = other.history;
        this->history_size = other.history_size;
        this->fsm = other.fsm;
        this->fsm_size = other.fsm_size;
        this->isGlobalHist = other.isGlobalHist;
        this->isGlobalFSM = other.isGlobalFSM;
        this->table = new BTB_cell[size];
        for (int i = 0; i < size; i++){
            this->table[i] = BTB_cell(other.table[i].tag, other.table[i].target, other.table[i].history, other.table[i].history_size, other.table[i].fsm, other.table[i].fsm_size, other.table[i].fsm_state);
        }
        if(isGlobalHist){
            this->history = new int[history_size];
            for (int i = 0; i < history_size; i++){
                this->history[i] = other.history[i];
            }
        }else{
            for (int i = 0; i < size; i++){
                this->table[i].history = new int[history_size];
                for (int j = 0; j < history_size; j++){
                    this->table[i].history[j] = other.table[i].history[j];
                }
            }
        }
        if(isGlobalFSM){
            this->fsm = new int[fsm_size];
            for (int i = 0; i < fsm_size; i++){
                this->fsm[i] = other.fsm[i];
            }
        }else{
            for (int i = 0; i < size; i++){
                this->table[i].fsm = new int[fsm_size];
                for (int j = 0; j < fsm_size; j++){
                    this->table[i].fsm[j] = other.table[i].fsm[j];
                }
            }
        }
    }
    //copy assignment
    BTB_table& operator=(const BTB_table &other){
        if (this != &other){
            this->size = other.size;
            this->history = other.history;
            this->history_size = other.history_size;
            this->fsm = other.fsm;
            this->fsm_size = other.fsm_size;
            this->isGlobalHist = other.isGlobalHist;
            this->isGlobalFSM = other.isGlobalFSM;
            this->table = new BTB_cell[size];
            for (int i = 0; i < size; i++){
                this->table[i] = BTB_cell(other.table[i].tag, other.table[i].target, other.table[i].history, other.table[i].history_size, other.table[i].fsm, other.table[i].fsm_size, other.table[i].fsm_state);
            }
            if(isGlobalHist){
                this->history = new int[history_size];
                for (int i = 0; i < history_size; i++){
                    this->history[i] = other.history[i];
                }
            }else{
                for (int i = 0; i < size; i++){
                    this->table[i].history = new int[history_size];
                    for (int j = 0; j < history_size; j++){
                        this->table[i].history[j] = other.table[i].history[j];
                    }
                }
            }
            if(isGlobalFSM){
                this->fsm = new int[fsm_size];
                for (int i = 0; i < fsm_size; i++){
                    this->fsm[i] = other.fsm[i];
                }
            }else{
                for (int i = 0; i < size; i++){
                    this->table[i].fsm = new int[fsm_size];
                    for (int j = 0; j < fsm_size; j++){
                        this->table[i].fsm[j] = other.table[i].fsm[j];
                    }
                }
            }
        }
        return *this;
    }
    //destructor
    ~BTB_table(){
        if (table != NULL){
            for(int i = 0; i < size; i++)
                table[i].~BTB_cell();
            delete[] table;
        }
        if(history != NULL){
            if(isGlobalHist)
                delete[] history;
            else{
                for(int i = 0; i < size; i++)
                    delete[] table[i].history;
            }
        }
        if (fsm != NULL){
            if(isGlobalFSM)
                delete[] fsm;
            else{
                for(int i = 0; i < size; i++)
                    delete[] table[i].fsm;
            }
        }
    }
};

BTB_table *BTB;

//this function initializes the predictor
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
    //initialize the global variables
    unsigned BTB_size = btbSize;
    unsigned history_size = historySize;
    unsigned tag_size = tagSize;
    unsigned fsm_state = fsmState;
    bool isGlobalHist = isGlobalHist;
    bool isGlobalFSM = isGlobalFSM;
    int isShared = Shared;
    //initialize the BTB
    BTB = new BTB_table(BTB_size, history_size, tag_size, fsm_state, isGlobalHist, isGlobalFSM);
    return 0;
}

//this function is called for every branch instruction
bool BP_predict(uint32_t pc, uint32_t *dst){
    //get the index of the BTB cell
    unsigned index = pc % BTB->size;
    //get the tag of the BTB cell
    unsigned tag = pc >> (BTB->tagSize);
    //check if the tag is the same
    if (BTB->table[index].tag == tag){
        //get the history
        unsigned history = 0;
        if (BTB->isGlobalHist){
            for (int i = 0; i < BTB->history_size; i++){
                history = history << 1;
                history = history | BTB->history[i];
            }
        }else{
            for (int i = 0; i < BTB->history_size; i++){
                history = history << 1;
                history = history | BTB->table[index].history[i];
            }
        }
        //get the fsm state
        unsigned fsm_state = 0;
        if (BTB->isGlobalFSM){
            for (int i = 0; i < BTB->fsm_size; i++){
                fsm_state = fsm_state << 1;
                fsm_state = fsm_state | BTB->fsm[i];
            }
        }else{
            for (int i = 0; i < BTB->fsm_size; i++){
                fsm_state = fsm_state << 1;
                fsm_state = fsm_state | BTB->table[index].fsm[i];
            }
        }
        //get the target
        unsigned target = BTB->table[index].target;
        bool prediction = false;
        //get the prediction if fsm is local and history is local
        if(!BTB->isGlobalHist && !BTB->isGlobalFSM){
            if (BTB->table[index].fsm_state == 0 || BTB->table[index].fsm_state == 1){
                prediction = false;
            }else{
                prediction = true;
            }
        }
        //get the prediction if fsm is local and history is global
        if(BTB->table[index].fsm[history]==0|| BTB->table[index].fsm[history]==1){
            prediction = false;
        }else{
            prediction = true;
        }
        //get the prediction if fsm is global and history is local
        if(BTB->fsm[history]==0|| BTB->fsm[history]==1){
            prediction = false;
        }else{
            prediction = true;
        }
        //get the prediction if fsm is global and history is global
        if(BTB->fsm[history]==0|| BTB->fsm[history]==1){
            prediction = false;
        }else{
            prediction = true;
        }
        //update the fsm state
        if (prediction){
            if (BTB->table[index].fsm_state < (1 << (BTB->fsm_size - 1)) - 1)
                BTB->table[index].fsm_state++;
        }else{
            if (BTB->table[index].fsm_state > 0)
                BTB->table[index].fsm_state--;
        }
        //update the history
        if (prediction){
            if (BTB->isGlobalHist){
                for (int i = 0; i < BTB->history_size - 1; i++){
                    BTB->history[i] = BTB->history[i + 1];
                }
                BTB->history[BTB->history_size - 1] = 1;
            }else{
                for (int i = 0; i < BTB->history_size - 1; i++){
                    BTB->table[index].history[i] = BTB->table[index].history[i + 1];
                }
                BTB->table[index].history[BTB->history_size - 1] = 1;
            }
        }else{
            if (BTB->isGlobalHist){
                for (int i = 0; i < BTB->history_size - 1; i++){
                    BTB->history[i] = BTB->history[i + 1];
                }
                BTB->history[BTB->history_size - 1] = 0;
            }else{
                for (int i = 0; i < BTB->history_size - 1; i++){
                    BTB->table[index].history[i] = BTB->table[index].history[i + 1];
                }
                BTB->table[index].history[BTB->history_size - 1] = 0;
            }
        }
        //update the fsm
        if (prediction){
            if (BTB->isGlobalFSM){
                for (int i = 0; i < BTB->fsm_size - 1; i++){
                    BTB->fsm[i] = BTB->fsm[i + 1];
                }
                BTB->fsm[BTB->fsm_size - 1] = 1;
            }else{
                for (int i = 0; i < BTB->fsm_size - 1; i++){
                    BTB->table[index].fsm[i] = BTB->table[index].fsm[i + 1];
                }
                BTB->table[index].fsm[BTB->fsm_size - 1] = 1;
            }
        }else{
            if (BTB->isGlobalFSM){
                for (int i = 0; i < BTB->fsm_size - 1; i++){
                    BTB->fsm[i] = BTB->fsm[i + 1];
                }
                BTB->fsm[BTB->fsm_size - 1] = 0;
            }else{
                for (int i = 0; i < BTB->fsm_size - 1; i++){
                    BTB->table[index].fsm[i] = BTB->table[index].fsm[i + 1];
                }
                BTB->table[index].fsm[BTB->fsm_size - 1] = 0;
            }
        }
        //return the prediction
        return prediction;
    }else{
        //if the tag is not the same, return false
        return false;
    }
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}
