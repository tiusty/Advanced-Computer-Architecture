#include "bpred.h"

#define TAKEN   true
#define NOTTAKEN false

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BPRED::BPRED(uint32_t policy) {
    if (policy == 1)
    {
        BPRED::policy = BPRED_ALWAYS_TAKEN;
    }
    else if (policy == 2)
    {
        BPRED::policy = BPRED_GSHARE;
    }
    else
    {
        BPRED::policy = BPRED_PERFECT;
    }
    GHR = 0;
    std::fill(PHT, PHT + 4096, 2);
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool BPRED::GetPrediction(uint32_t PC){
    if(policy == BPRED_ALWAYS_TAKEN)
    {
        return TAKEN;
    } else if (policy == BPRED_GSHARE) {
        unsigned int index = (GHR & 0xFFF) ^ (PC & 0xFFF);

        if (PHT[index] >= 2) {
            return TAKEN;
        } else {
            return NOTTAKEN;
        }

    } else {
        return TAKEN;
    }
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  BPRED::UpdatePredictor(uint32_t PC, bool resolveDir, bool predDir) {
    stat_num_branches++;
    if(resolveDir != predDir) {
        stat_num_mispred++;
    }

    if(policy == BPRED_GSHARE)
    {
        unsigned int index = (GHR & 0xFFF) ^ (PC & 0xFFF);
        if(resolveDir == TAKEN)
        {
            PHT[index] = SatIncrement(PHT[index], max_counter);
        } else {
            PHT[index] = SatDecrement(PHT[index]);
        }

        GHR = GHR << 1;
        GHR = GHR + resolveDir;
    }


}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

