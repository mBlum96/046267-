/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <cmath>



class BTB_cell{
public:
    uint32_t tag;
    uint32_t target;
    int valid;
    BTB_cell(){
        tag = 0;
        target = 0;
        valid = 0;
    }
    BTB_cell(uint32_t tag, uint32_t target){
        this->tag = tag;
        this->target = target;
        this->valid = 0;
    }
    ~BTB_cell()= default;

};


unsigned log2_func(unsigned x)
{
    unsigned res = 0;

    while (pow(2, res) < x)
    {
        res++;
    }
    return res;
}


class BTB_table{
public:
    BTB_cell *table;
    unsigned size;
    unsigned *history;
    unsigned history_size;
    unsigned **fsm;
    unsigned fsm_size;
    int fsm_state;
    bool isGlobalHist;
    bool isGlobalFSM;
    unsigned tagSize;
    int shared;

    SIM_stats stats;
    BTB_table(){
        table = NULL;
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
    BTB_table(unsigned size, unsigned history_size,unsigned tagSize, unsigned fsm_state, bool isGlobalHist, bool isGlobalFSM, int isShared){
        this->size = size;
        this->history_size = history_size;
        this->history = NULL;
        this->fsm_state = fsm_state;
        this->fsm = NULL;
        this->isGlobalHist = isGlobalHist;
        this->isGlobalFSM = isGlobalFSM;
        this -> tagSize = tagSize;
        this->shared = isShared;
        this->stats = {0, 0, 0};

        table = new BTB_cell[size];
        for (int i = 0; i < size; i++){
            table[i] = BTB_cell(0, 0);
        }
        this->fsm_size = (unsigned )pow(2, history_size);
        if (isGlobalFSM){
            this->fsm = new unsigned* ;
            *this->fsm = new unsigned[this->fsm_size];
        } else{
            this->fsm = new unsigned *[size];

            for (unsigned i = 0; i < size; i++)
            {
                this->fsm[i] = new unsigned[this->fsm_size];
            }
        }
        if(!isGlobalFSM){


            for (int j = 0; j < this->size; j++)
            {
                for (int i = 0; i < this->fsm_size; i++)
                {
                    this->fsm[j][i] = this->fsm_state;
                }
            }
        }else{

            for (int i = 0; i < this->fsm_size; i++)
            {
                this->fsm[0][i] = this->fsm_state;

            }
        }
        if(!isGlobalHist){
            this->history = new unsigned[size];
            for (int i = 0; i < size; ++i) {
                this->history[i] = 0;
            }
        }else{
            this->history = new unsigned;
            *(this->history) = 0;
        }


    }
    //destructor
    ~BTB_table(){
        if(history != NULL){
            if(isGlobalHist)
                delete[] history;

        }
        if (fsm != NULL){
            if(isGlobalFSM)
                delete[] fsm;

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
    BTB = new BTB_table(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    return 0;
}


//this function is called for every branch instruction
bool BP_predict(uint32_t pc, uint32_t *dst){
    //get the index of the BTB cell
    unsigned temp = (unsigned)pow(2, log2_func(BTB->size));
    unsigned btb_index = pc >> 2;
    unsigned index_in_table = btb_index % temp;
    //get the tag of the BTB cell
    unsigned btbIndexLength = log2_func(BTB->size);
    unsigned tag = pc >> (2 + btbIndexLength);
    tag = tag % int(pow(2, BTB->tagSize));
    //check if the tag is the same
    if (BTB->table[index_in_table].tag != tag || BTB->table[index_in_table].valid == 0)
    {
        *dst = pc + 4;
        return false;

    }
    else {
        unsigned index_in_fsm;
        unsigned fsm_state = 0;
        unsigned shared = 0;
        unsigned temp_size = unsigned(pow(2, BTB->history_size));
        if (BTB->shared == 2)
        {
            shared = pc >> 16;
            shared = shared % (temp_size);
        }
        if (BTB->shared == 1)
        {
            shared = pc >> 2;
            shared = shared % (temp_size);
        }
        if (BTB->isGlobalHist){
            index_in_fsm = *BTB->history;
        } else{
            index_in_fsm = BTB->history[index_in_table];
        }

        index_in_fsm =  index_in_fsm ^ shared;

        if (BTB->isGlobalFSM)
        {
            fsm_state = (*BTB->fsm)[index_in_fsm];
        } else{
            fsm_state = BTB->fsm[index_in_table][index_in_fsm];
        }
        bool prediction = false;
        if (fsm_state == 0 || fsm_state == 1){
            prediction= false;

        } else{
            prediction = true;
        }

        if (prediction)
        {
            *dst = BTB->table[index_in_table].target;
        } else
        {
            *dst = pc + 4;
        }
        return prediction;
    }
}


unsigned fromPCToTag(uint32_t pc)
{
    int btbIndexLength = log2_func(BTB->size);
    unsigned tag = pc >> (2 + btbIndexLength); //allign address
    return tag % int(pow(2, BTB->tagSize));
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    //update the history
    bool prediction;
    unsigned btb_index = pc >> 2;
    unsigned index_in_table = btb_index % unsigned(pow(2, log2_func(BTB->size)));
    unsigned index_in_fsm;
    unsigned fsm_state = 0;
    unsigned shared = 0;
    unsigned temp_size = unsigned(pow(2, BTB->history_size));
    unsigned tag = fromPCToTag(pc);
    if( BTB->table[index_in_table].valid == 0 || BTB->table[index_in_table].tag != tag){
        BTB->table[index_in_table].target = targetPc;
        BTB->table[index_in_table].valid = 1;
        BTB->table[index_in_table].tag = tag;


        if (!BTB->isGlobalFSM)
        {
            for (int i = 0; i < temp_size; i++){
                BTB->fsm[index_in_table][i] = BTB->fsm_state;
            }
        }
        if (!BTB->isGlobalHist)
        {
            BTB->history[index_in_table] = 0;
        }


    }
    if (BTB->shared == 2)
    {
        shared = pc >> 16;
        shared = shared % (temp_size);
    }
    if (BTB->shared == 1)
    {
        shared = pc >> 2;
        shared = shared % (temp_size);
    }
    if (BTB->isGlobalHist){
        index_in_fsm = *BTB->history;
    } else{
        index_in_fsm = BTB->history[index_in_table];
    }

    index_in_fsm =  index_in_fsm ^ shared;

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


    BTB->table[index_in_table].target = targetPc;

    //update the fsm

    if (!BTB->isGlobalFSM){
        if (BTB->fsm[index_in_table][index_in_fsm] > 0 && !taken)
            BTB->fsm[index_in_table][index_in_fsm]--;

        if (BTB->fsm[index_in_table][index_in_fsm] < 3 && taken)
            BTB->fsm[index_in_table][index_in_fsm]++;


    }else {

        if ((*BTB->fsm)[index_in_fsm] > 0 && !taken)
            (*BTB->fsm)[index_in_fsm]--;


        if ((*BTB->fsm)[index_in_fsm] < 3 && taken)
            (*BTB->fsm)[index_in_fsm] += 1;

    }

    //update history

    if (BTB->isGlobalHist){
        *BTB->history = (*BTB->history) << 1;
        if(taken){
            *BTB->history += 1;
        }
        *BTB->history = *BTB->history % temp_size;

    }else{

        BTB->history[index_in_table] = BTB->history[index_in_table] << 1;
        if(taken){
            BTB->history[index_in_table] += 1;
        }

        BTB->history[index_in_table] = BTB->history[index_in_table] % temp_size;
    }


}

void BP_GetStats(SIM_stats *curStats){
    curStats->br_num = BTB->stats.br_num;
    curStats->flush_num = BTB->stats.flush_num;


    curStats->size = BTB->size * (31 + BTB->tagSize); //sizeof(BTB->table->target)-2
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
