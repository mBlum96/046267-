/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"

class BTB_cell{
public:
    uint32_t tag;
    uint32_t target;
    int *history;
    int history_size;
    int *fsm;
    int fsm_size;
    int fsm_state;
    BTB_cell{
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
    ~BTB_cell{
        if (history != NULL)
            delete[] history;
        if (fsm != NULL)
            delete[] fsm;
    }
};


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
    BTB_table{
        table = NULL;
        size = 0;
        history = NULL;
        history_size = 0;
        fsm = NULL;
        fsm_size = 0;
        fsm_state = 0;
        isGlobalHist = false;
        isGlobalFSM = false;
    }
    BTB_table(int size, int *history, int history_size, int *fsm, int fsm_size, bool isGlobalHist, bool isGlobalFSM){
        this->size = size;
        this->history = history;
        this->history_size = history_size;
        this->fsm = fsm;
        this->fsm_size = fsm_size;
        this->isGlobalHist = isGlobalHist;
        this->isGlobalFSM = isGlobalFSM;
        table = new BTB_cell[size];
        for (int i = 0; i < size; i++){
            table[i] = BTB_cell(0, 0, history, history_size, fsm, fsm_size, fsm_state);
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
        if(isglobalFSM){
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
        if(isglobalFSM){
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
            if(isglobalFSM){
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
    ~BTB_table{
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

//this function initializes the predictor
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
    //initialize the global variables
    BTB_size = btbSize;
    history_size = historySize;
    tag_size = tagSize;
    fsm_state = fsmState;
    isGlobalHist = isGlobalHist;
    isGlobalFSM = isGlobalFSM;
    isShared = Shared;
    //initialize the BTB
    BTB = new BTB_table(BTB_size, history_size, tag_size, fsm_state, isGlobalHist, isGlobalFSM);
    return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}

