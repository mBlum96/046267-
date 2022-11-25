/* 046267 Computer Architecture - Spring 2019 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <math.h>
#include <iostream>


uint32_t getIndicatorByPC(uint32_t pc, int btbSize);
uint32_t getTagByPC(uint32_t pc,int tagSize);

class BTB_cell{
    uint32_t tag;
    uint32_t target;
    int history;
    int *psm;
    int psm_size;
public:
    BTB_cell(uint32_t rcvd_tag , uint32_t rcvd_target
            , int rcvd_history , int history_size , int fsm_state ,  bool global_table)
    //CTOR
    {
        tag = rcvd_tag;
        target = rcvd_target;    //should be zero
        history = rcvd_history;
        psm_size = (int)pow(2,history_size);
        if(!global_table){
            psm = new int[psm_size]; // when global , turn this off.
            for (int i=0;i<psm_size;i++) {
                psm[i] = fsm_state;
            }
        }else{
            psm=NULL;
        }
    }


    BTB_cell()       //DEFAULT CTOR
    {
        tag = -1;
        target = -1;
        history = -1;
        psm_size=0;
        psm = NULL;
    }



    BTB_cell( BTB_cell &copy)    //COPY CTOR
    {
        tag = copy.tag;
        target = copy.target;
        history = copy.history;
	if(psm){
		delete[] psm;
	}
        psm = copy.psm;
        copy.psm = NULL;
	psm_size = copy.psm_size;
    }

    BTB_cell& operator=(BTB_cell& copy){
	tag = copy.tag;
        target = copy.target;
        history = copy.history;
	if(psm){
		delete[] psm;
	}
        psm = copy.psm;
        copy.psm = NULL;
	psm_size = copy.psm_size;
	return *this;
    }



    ~BTB_cell()             //DTOR
    {
	if(psm){
        	delete[] psm;
	}
	psm=NULL;
    }


    bool operator ==(const BTB_cell &compared_cell) const
    {
        return tag==compared_cell.tag;
    }


    uint32_t get_tag()
    {
        return tag;
    }


    uint32_t get_target()
    {
        return target;
    }


    int get_history()
    {
        return history;
    }


    void set_fsm(int fsm)
    {
        for(int i=0 ; i<psm_size ; i++)
        {
            psm[i] = fsm;
        }
    }
    const int* getPsm(){
        return psm;
    }


    int updateHistory(bool taken , bool isGlobalHist , bool isGlobalTable , int shared
            ,int* globalHist,int* globalTable , uint32_t pc , unsigned int historySize ,
            unsigned int tagSize){
        if(taken) {
         	if (!isGlobalHist && !isGlobalTable) {
                	if (psm[history] < 3) {
                    		psm[history]=1+psm[history];
                	}
                	history=(history*2+1)%psm_size;
			//std::cout<<"this is bimodal: "<< psm[history]<<" of history "<<history<< std::endl;
		}

            if(isGlobalHist && isGlobalTable){      // global , global and


                int location = 0;
                if(shared == 1) {
                    pc=pc/4;
                    location = ((int)pc%psm_size)^(*globalHist)%psm_size;
                }
                else if(shared == 2){
                    pc = pc/((int)pow(2,16));
                    location = ((int)pc%psm_size)^(*globalHist)%psm_size;
                }
                else if(shared == 0){
                    location = *globalHist;
                }
		else{}
		//std::cout<<"this is bimodal: "<< globalTable[*globalHist]<<" of history "<<*globalHist<< std::endl;
                if(globalTable[location]<3){
                    globalTable[location]++;
                }
                *globalHist = ((*globalHist)*2+1)%psm_size;
            }
            if(!isGlobalHist && isGlobalTable){
                int location = 0;
                if(shared == 1) {
                    pc=pc/4;
                    location = (((int)pc%psm_size)^history)%psm_size;
                }
                else if(shared == 2){
                    pc = pc/((int)pow(2,16));
                    location = (((int)pc%psm_size)^history)%psm_size;
                }
                else if(shared == 0){
                    location = history;
                }
		else{}
		//std::cout<<"this is bimodal: "<< globalTable[location]<<" of history "<<location<< std::endl;
                if(globalTable[location] < 3){
                    globalTable[location]++;
                }
                history=(history*2+1)%psm_size;
            }
            if(isGlobalHist && !isGlobalTable){
			//std::cout<<"this is bimodal: "<< psm[*globalHist]<<" of history "<<*globalHist<< std::endl;
			if(psm[*globalHist]<3){
                    psm[*globalHist]++;
                }
                *globalHist = ((*globalHist)*2+1)%psm_size;
            }

        }
        else{  //if not taken
            if(!isGlobalHist && !isGlobalTable){
                if (psm[history] > 0) {
                    psm[history]=psm[history]-1;
                }
                history = (history * 2) % psm_size;
            }



            if(isGlobalHist && isGlobalTable){  //global global
                int location = 0;
                if(shared == 1) {
                    pc=pc/4;
                    location = (((int)pc%psm_size)^*globalHist)% psm_size;
                }
                else if(shared == 2){
                    pc = pc/((int)pow(2,16));
                    location = (((int)pc%psm_size)^*globalHist)% psm_size;
                }
                else if(shared == 0){
                    location = *globalHist;
		}
		else{}
                if(globalTable[location]>0){
                //    std::cout<<"this is bimodal: "<< globalTable[*globalHist]<<" of history "<<*globalHist<< std::endl;
                    globalTable[location]--;
                }
                *globalHist = ((*globalHist)*2) % psm_size;
            }
            if(!isGlobalHist && isGlobalTable){
                int location = 0;
                if(shared == 1) {
                    pc=pc/4;
                    location = ((int)pc%psm_size)^history;
                }
                if(shared == 2){
                    pc = pc/((int)pow(2,16));
                    location = ((int)pc%psm_size)^history;
                }

                if(shared == 0){
                    location=history;
                }
		//std::cout<<"this is bimodal: "<< globalTable[location]<<" of history "<<location<< std::endl;
                if(globalTable[location]>0){
                    globalTable[location]--;
                }
                history = (history * 2) % psm_size;
            }
            if(isGlobalHist && !isGlobalTable){
			//std::cout<<"this is bimodal: "<< psm[*globalHist]<<" of history "<<*globalHist<< std::endl;   
			if(psm[*globalHist]>0){
                    		psm[*globalHist]--;
                	}
                *globalHist = ((*globalHist)*2) % psm_size;
            }
        }
        return history;
    }
    void updateTarget(uint32_t new_targ){
        target=new_targ;
    }
};



class BTB_table{
    BTB_cell *btbTable;
    unsigned int btbSize;
    unsigned int historySize;
    unsigned int tagSize;
    unsigned int fsmState;
    bool isGlobalHist;
    int globalHist;
    bool isGlobalTable;
    int *globalTable;
    int shared;
    int totalUpdatedBranches;
    int totalFlushes;
    int sizeInBits;

public:
    BTB_table(unsigned int btb_size , unsigned int history_size ,
              unsigned int tag_size , unsigned int fsm_state , bool is_globalHist
            , bool is_globalTable , int is_shared) //CTOR
    {
        ////////////////////////////////////////////////////////////////////////////////
        /*this is for LOCAL , LOCAL */
	    btbSize = btb_size;
        btbTable = new BTB_cell[btb_size];
        historySize = history_size;
        tagSize = tag_size;
        fsmState = fsm_state;
        isGlobalHist = is_globalHist;
        isGlobalTable = is_globalTable;
        shared=is_shared;
        totalUpdatedBranches = 0;
        totalFlushes=0;
        globalHist = 0;
	if(!is_globalTable) {    //local table local history
            globalTable = NULL;
        }else        // global table global history , or global table local history
        {
            globalTable = new int[(int)pow(2,history_size)];
            for(int i=0;i<(int)pow(2,history_size);i++)
            {
                globalTable[i]=fsmState;
            }
        }
    }
    BTB_table()
    {
        btbSize = -1;
        historySize = -1;
        tagSize = -1;
        fsmState = -1;
        isGlobalTable = true;
        isGlobalHist = true;
        globalTable = NULL;
        globalHist = -1;
        shared = -1;
        totalUpdatedBranches = 0;
        totalFlushes=0;
    }

    BTB_table(BTB_table &copy)   //COPY CTOR
    {
	if(btbTable){
		delete[] btbTable;
	}
        btbTable = copy.btbTable;
        copy.btbTable = NULL;
        btbSize = copy.btbSize;
        historySize = copy.historySize;
        tagSize = copy.historySize;
        fsmState = copy.fsmState;
        globalHist = copy.globalHist;
	if(globalTable){
		delete[] globalTable;
	}
        globalTable = copy.globalTable;
        copy.globalTable=NULL;
        isGlobalHist = copy.isGlobalHist;
        isGlobalTable = copy.isGlobalTable;
        shared = copy.shared;
        totalUpdatedBranches = copy.totalUpdatedBranches;
        totalFlushes = copy.totalFlushes;
    }

    BTB_table& operator=(BTB_table &copy){	
	if(btbTable){
		delete[] btbTable;
	}
        btbTable = copy.btbTable;
        copy.btbTable = NULL;
        btbSize = copy.btbSize;
        historySize = copy.historySize;
        tagSize = copy.historySize;
        fsmState = copy.fsmState;
        globalHist = copy.globalHist;
	if(globalTable){
		delete[] globalTable;
	}
        globalTable = copy.globalTable;
        copy.globalTable=NULL;
        isGlobalHist = copy.isGlobalHist;
        isGlobalTable = copy.isGlobalTable;
        shared = copy.shared;
        totalUpdatedBranches = copy.totalUpdatedBranches;
        totalFlushes = copy.totalFlushes;
	return *this;
    }
    ~BTB_table()                 //DTOR
    {
	if(btbTable){
	        delete[] btbTable;
	}
        if(isGlobalTable){
	    //std::cout<< "ohhhh shit, psm global D'tor"<<std::endl;
       	    delete[] globalTable;
            globalTable=NULL;
        }
	btbTable=NULL;
    }
	unsigned int get_btbSize(){
		return btbSize;	
	}
	unsigned int get_tagSize(){
		return tagSize;
	}
	unsigned int get_historySize(){
		return historySize;
	}
    int get_totalFlushes()
    {
        return totalFlushes;
    }
    int get_totalUpdates(){
        return totalUpdatedBranches;
    }
    bool get_isGlobalHist()
    {
        return isGlobalHist;
    }
    bool get_isGlobalTable()
    {
        return isGlobalTable;
    }
    int* getGlobalHist(){
        return &globalHist;
    }
    int* getGlobalTable(){
        return globalTable;
    }


    uint32_t getTargetByPC(uint32_t pc, bool isGlobalHist , bool isGlobalTable , int* globalHist, int* globalTable,bool *ans){
	if(!btbTable){
           // std::cout<<"btbTable is NULL"<<std::endl;
        }
        int i=getIndicatorByPC(pc,btbSize);
	int lshare,gshare;	
	if(shared==0){
		lshare=btbTable[i].get_history();
		gshare=*globalHist;
	}	
	if(shared==1){	
		lshare=((pc/4)%(int)pow(2,historySize))^btbTable[i].get_history();     
		gshare=((pc/4)%(int)pow(2,historySize))^(*globalHist);
	}
	if(shared==2){
		lshare=((pc/(int)pow(2,16))%(int)pow(2,historySize))^(btbTable[i].get_history());     
		gshare=((pc/(int)pow(2,16))%(int)pow(2,historySize))^(*globalHist);
	}
        if(!isGlobalHist && !isGlobalTable) {
            if (btbTable[i].get_tag() == getTagByPC(pc, tagSize)) {
                if ((btbTable[i].getPsm())[btbTable[i].get_history()]>1) {
			*ans=true;                    
			return btbTable[i].get_target();
                }
            }else return pc+4;
        }


        else if(isGlobalHist && isGlobalTable){
            if (btbTable[i].get_tag() == getTagByPC(pc, tagSize)) {
                if (globalTable[gshare]>1){
			*ans=true;                    
			return btbTable[i].get_target();
                }
            }
            else return pc + 4;
        }


        else if(!isGlobalHist && isGlobalTable){           //local history , global table( psm )
		//std::cout<<"inside the scope"<<std::endl;            
		if(btbTable[i].get_tag() == getTagByPC(pc , tagSize)){
                	if(globalTable[lshare] >1){
				//std::cout<<"inside the if"<<std::endl;
				*ans=true;                	    
				return btbTable[i].get_target();
                	}
            	}
            	else return pc+4;
        }
        else{
            if(btbTable[i].get_tag() == getTagByPC(pc , tagSize)) {
                if ((btbTable[i].getPsm())[*globalHist] > 1) {
			*ans=true;                    
			return btbTable[i].get_target();
                }
            }
                    else return pc+4;
        }
        return pc+4;
    }




    //TODO , to the function updateHistory we should send the pc , because in share... xor... we will need it , read page 1 in the HW.
    void updateBranch(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
        totalUpdatedBranches++;
        int i=getIndicatorByPC(pc,btbSize);
        uint32_t good_tag=getTagByPC(pc,tagSize);
	if( (taken && ((int)targetPc != (int)pred_dst)) || (!taken && pred_dst!=(pc+4)) ) {
            totalFlushes++;
        }
	//std::cout << " tag in table: "<<btbTable[i].get_tag() << " tag now is: " << good_tag << " " ;
	if(btbTable[i].get_tag()!=good_tag){
        	BTB_cell new_cell(good_tag,targetPc,0,historySize,fsmState,isGlobalTable);            
		btbTable[i]=new_cell;
        }
	else if(btbTable[i].get_target()!= targetPc){
		btbTable[i].updateTarget(targetPc);
	}	
        btbTable[i].updateHistory(taken,isGlobalHist,isGlobalTable,shared,&globalHist,globalTable,pc,historySize,tagSize);
        return;
    }

};

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
BTB_table *theTable;
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
            bool isGlobalHist, bool isGlobalTable, int Shared){
	theTable = new BTB_table(btbSize,historySize,tagSize,fsmState,isGlobalHist,isGlobalTable,Shared);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	bool ans1=false;
	bool *ans=&ans1;
    *dst=theTable->getTargetByPC(pc,theTable->get_isGlobalHist(),theTable->get_isGlobalTable()
            ,theTable->getGlobalHist(),theTable->getGlobalTable(),ans);
	return *ans;    
	}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
        theTable->updateBranch(pc, targetPc, taken, pred_dst);
    	return;
}

void BP_GetStats(SIM_stats *curStats){
    	curStats->br_num = theTable->get_totalUpdates();
    	curStats->flush_num = theTable->get_totalFlushes();
	bool H=theTable->get_isGlobalHist();
	bool T=theTable->get_isGlobalTable();
	unsigned int bs=theTable->get_btbSize();
	unsigned int ts=theTable->get_tagSize();
	unsigned int hs=theTable->get_historySize();
	if(!H && !T){
		curStats->size=bs*(ts+30+hs+(int)pow(2,hs+1));		
	}
	else if(H && T){
		curStats->size=bs*(ts+30)+(hs+(int)pow(2,hs+1));
	}
	else if(!H && T){
		curStats->size=bs*(ts+30+hs)+(int)pow(2,hs+1);
	}else{
		curStats->size=bs*(ts+30+(int)pow(2,hs+1))+hs;
	}
    delete theTable;
	//std::cout <<"fucking sam has done free!!"<<std::endl;
    return;
}
uint32_t getIndicatorByPC(uint32_t pc, int btbSize){
    pc=pc/4;
    return pc%btbSize;
}
uint32_t getTagByPC(uint32_t pc,int tagSize){
	//std::cout<< pc <<std::endl;    
	pc=pc/4;
	return pc%(int)(pow(2,tagSize));
}


