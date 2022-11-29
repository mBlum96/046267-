/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <cmath>



unsigned log2_func(unsigned x)
{
    int res = 0;

    while (pow(2, res) < x)
    {
        res++;
    }
    return res;
}


class BTB_cell{
public:
    uint32_t tag;
    uint32_t target;
    int *history;
    int history_size;
    int *fsm;
    int fsm_size;
    int fsm_state;
    int valid;
    BTB_cell(){
        tag = 0;
        target = 0;
        history = NULL;
        history_size = 0;
        fsm = NULL;
        fsm_size = 0;
        fsm_state = 0;
        valid = 0;
    }
    BTB_cell(uint32_t tag, uint32_t target, int *history, int history_size, int *fsm, int fsm_size, int fsm_state, bool isGlobal){
        this->tag = tag;
        this->target = target;
        this->history = history;
        this->history_size = history_size;
        this->valid = 0;
        this->fsm_size = fsm_size;


    }
    ~BTB_cell(){
//        if (history != NULL)
//            delete[] history;
//        if (fsm != NULL)
//            delete[] fsm;
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
    int shared;
    SIM_stats stats;
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
        stats = {0, 0, 0};

    }
    BTB_table(int size, int history_size,unsigned tagSize, int fsm_size, bool isGlobalHist, bool isGlobalFSM, int isShared){
        this->size = size;
        this->history_size = history_size;
        this->history = NULL;
        this->fsm_size = fsm_size;
        this->fsm = NULL;
        this->isGlobalHist = isGlobalHist;
        this->isGlobalFSM = isGlobalFSM;
        this -> tagSize = tagSize;
        this->shared = isShared;
        this->stats = {0, 0, 0};

        table = new BTB_cell[size];
        int tagArraySize = pow(2,tagSize);
        tagArray = new int[tagArraySize];
        for (int i = 0; i < size; i++){
            table[i] = BTB_cell(0, 0, history, history_size, fsm, fsm_size, fsm_state, isGlobalFSM);
        }
        for(int i=0; i<tagArraySize; i++){
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
        this->stats = other.stats;

        this->table = new BTB_cell[size];

        for (int i = 0; i < size; i++){
            this->table[i] = BTB_cell(other.table[i].tag, other.table[i].target, other.table[i].history,
                                      other.table[i].history_size, other.table[i].fsm, other.table[i].fsm_size,
                                      other.table[i].fsm_state, other.isGlobalFSM);
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
                this->table[i] = BTB_cell(other.table[i].tag, other.table[i].target, other.table[i].history,
                                          other.table[i].history_size, other.table[i].fsm, other.table[i].fsm_size,
                                          other.table[i].fsm_state, other.isGlobalFSM);
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
        if (table != NULL){
            for(int i = 0; i < size; i++)
                table[i].~BTB_cell();
            delete[] table;
        }
    }
};

BTB_table *BTB;

//this function initializes the predictor
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
            bool isGlobalHist, bool isGlobalTable, int Shared){

    //initialize the global variables
//    unsigned BTB_size = btbSize;
//    unsigned history_size = historySize;
//    unsigned tag_size = tagSize;
//    unsigned fsm_state = fsmState;
//    bool isGlobalHist = isGlobalHist;
//    bool isGlobalFSM = isGlobalFSM;
//    int isShared = Shared;
    //initialize the BTB
    BTB = new BTB_table(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    return 0;
}

unsigned getHistory(uint32_t pc){
    unsigned index = (pc/4) % BTB->size;
    //get the history
    unsigned history = 0;
    if (BTB->isGlobalHist)
    {
        for (int i = 0; i < BTB->history_size; i++)
        {
            history = history << 1;
            history = history | BTB->history[i];
        }
    } else
    {
        for (int i = 0; i < BTB->history_size; i++)
        {
            history = history << 1;
            history = history | BTB->table[index].history[i];
        }
    }
    if(BTB->isGlobalFSM)
    {
        if (BTB->shared == 1)
        {
            pc = pc >> 2;
            history ^= pc;
        }
        if (BTB->shared == 2)
        {
            pc = pc >> 16;
            history ^= pc;
        }
    }
    return history;
}

//this function is called for every branch instruction
bool BP_predict(uint32_t pc, uint32_t *dst){
    //get the index of the BTB cell
    unsigned btb_index = pc >> 2;
    unsigned index = btb_index % unsigned(pow(2, log2_func(BTB->size)));
    //get the tag of the BTB cell
//    unsigned tag = pc >> (BTB->tagSize);
    int btbIndexLength = log2_func(BTB->size);
    unsigned tag = pc >> (2 + btbIndexLength); //allign address
    tag = tag % int(pow(2, BTB->tagSize));
    //check if the tag is the same
    if (BTB->table[index].tag == tag && BTB->table[index].valid != 0)
    {
        unsigned history = getHistory(pc);
        //get the fsm state
        unsigned fsm_state = 0;
        if (BTB->isGlobalFSM)
        {
            for (int i = 0; i < BTB->fsm_size; i++)
            {
                fsm_state = fsm_state << 1;
                fsm_state = fsm_state | BTB->fsm[i];
            }
        } else
        {
            for (int i = 0; i < BTB->fsm_size; i++)
            {
                fsm_state = fsm_state << 1;
                fsm_state = fsm_state | BTB->table[index].fsm[i];
            }
        }
        //get the target
        unsigned target = BTB->table[index].target;
        bool prediction = false;

        //get the prediction if fsm is local and history is local
        if (!BTB->isGlobalHist && !BTB->isGlobalFSM)
        {
            if (BTB->table[index].fsm_state == 0 || BTB->table[index].fsm_state == 1)
            {
                prediction = false;

            } else
            {
                prediction = true;
            }
        }

        //get the prediction if fsm is local and history is global
        if(BTB->isGlobalHist && !BTB->isGlobalFSM){
            if (BTB->table[index].fsm[history] == 0 || BTB->table[index].fsm[history] == 1)
            {
                prediction = false;
            } else
            {
                prediction = true;
            }
        }
        //get the prediction if fsm is global and history is local
        if(!BTB->isGlobalHist && BTB->isGlobalFSM) {
            if (BTB->fsm[history] == 0 || BTB->fsm[history] == 1) {
                prediction = false;
            } else {
                prediction = true;
            }
        }
        //get the prediction if fsm is global and history is global
        if(BTB->isGlobalHist && BTB->isGlobalFSM) {
            if(BTB->fsm_state > 1){
                prediction = true;
            }else{
                prediction = false;
            }

        }

        if (prediction)
        {
            *dst = (target);
        } else
        {
            *dst = pc + 4;
        }
        return prediction;
    }
    *dst = pc+4;
    return false;

}


unsigned getTagFromPC(uint32_t pc)
{
    int index_len = log2_func(BTB->size);
    unsigned tag = pc >> (2 + index_len);
    return tag % int(pow(2, BTB->tagSize));
}
void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    //update the history
    bool prediction;
    unsigned btb_index = pc >> 2;
    unsigned index = btb_index % unsigned(pow(2, log2_func(BTB->size)));
    unsigned history = getHistory(pc);
    if(pred_dst == pc+4)
    {
        prediction = false;
    }else
    {
        prediction = true;
    }
    if(prediction!=taken){
        BTB->stats.flush_num ++;
    }
    BTB->stats.br_num++;

    if(!taken && BTB->table[index].valid == 0){
        BTB->table[index].target = targetPc;
        BTB->table[index].valid = 1;
        BTB->table[index].tag = getTagFromPC(pc);
        BTB->table[index].history_size = BTB->history_size;
        BTB->table[index].fsm_size = BTB->fsm_size;
        BTB->table[index].fsm_state = BTB->fsm_state;

    }
    //update the fsm
//    if (){
    if (BTB->isGlobalFSM){
        if(BTB->fsm[history]<3 && taken){
            //BTB->fsm_state++;
            BTB->fsm[history]++;
        }else{
            if(BTB->fsm[history]>0 && !taken){
                //BTB->fsm_state--;
                BTB->fsm[history]--;
            }
        }

    }else{
        if(BTB->table[index].fsm_state<3 && taken)
        {
            BTB->table[index].fsm_state++;
        }else{
            if(BTB->table[index].fsm_state>0 && !taken){
                BTB->table[index].fsm_state--;
            }
        }
    }


    //update history
    if (taken){
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



}

void BP_GetStats(SIM_stats *curStats){
    curStats->br_num = BTB->stats.br_num;
    curStats->flush_num = BTB->stats.flush_num;


    curStats->size = BTB->size * (1 + 2*BTB->tagSize); //sizeof(BTB->table->target)-2
    if (!BTB->isGlobalHist)
    {
        curStats->size += BTB->size * BTB->history_size;
    }
    else
    {
        curStats->size += BTB->history_size;
    }

    if (!BTB->isGlobalFSM)
    {
        curStats->size += BTB->size * pow(2, (BTB->history_size + 1));
    }
    else
    {
        curStats->size += pow(2, (BTB->history_size + 1));
    }

}
