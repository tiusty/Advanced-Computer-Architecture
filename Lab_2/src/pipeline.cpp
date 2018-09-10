/***********************************************************************
 * File         : pipeline.cpp
 * Author       : Soham J. Desai 
 * Date         : 14th January 2014
 * Description  : Superscalar Pipeline for Lab2 ECE 6100
 **********************************************************************/

#include "pipeline.h"
#include <cstdlib>
#include <vector>
#include <algorithm>

extern int32_t PIPE_WIDTH;
extern int32_t ENABLE_MEM_FWD;
extern int32_t ENABLE_EXE_FWD;
extern int32_t BPRED_POLICY;

/**********************************************************************
 * Support Function: Read 1 Trace Record From File and populate Fetch Op
 **********************************************************************/

void pipe_get_fetch_op(Pipeline *p, Pipeline_Latch* fetch_op){
    uint8_t bytes_read = 0;
    bytes_read = fread(&fetch_op->tr_entry, 1, sizeof(Trace_Rec), p->tr_file);

    // check for end of trace
    if( bytes_read < sizeof(Trace_Rec)) {
      fetch_op->valid=false;
      p->halt_op_id=p->op_id_tracker;
      return;
    }

    // got an instruction ... hooray!
    fetch_op->valid=true;
    fetch_op->stall=false;
    fetch_op->is_mispred_cbr=false;
    p->op_id_tracker++;
    fetch_op->op_id=p->op_id_tracker;
    
    return; 
}


/**********************************************************************
 * Pipeline Class Member Functions 
 **********************************************************************/

Pipeline * pipe_init(FILE *tr_file_in){
    printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);

    // Initialize Pipeline Internals
    Pipeline *p = (Pipeline *) calloc (1, sizeof (Pipeline));

    p->tr_file = tr_file_in;
    p->halt_op_id = ((uint64_t)-1) - 3;           

    // Allocated Branch Predictor
    if(BPRED_POLICY){
      p->b_pred = new BPRED(BPRED_POLICY);
    }

    return p;
}


/**********************************************************************
 * Print the pipeline state (useful for debugging)
 **********************************************************************/

void pipe_print_state(Pipeline *p){
    std::cout << "--------------------------------------------" << std::endl;
    std::cout <<"cycle count : " << p->stat_num_cycle << " retired_instruction : " << p->stat_retired_inst << std::endl;

    uint8_t latch_type_i = 0;   // Iterates over Latch Types
    uint8_t width_i      = 0;   // Iterates over Pipeline Width
    for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
        switch(latch_type_i) {
            case 0:
                printf(" FE: ");
                break;
            case 1:
                printf(" ID: ");
                break;
            case 2:
                printf(" EX: ");
                break;
            case 3:
                printf(" MEM: ");
                break;
            default:
                printf(" ---- ");
        }
    }
    printf("\n");
    for(width_i = 0; width_i < PIPE_WIDTH; width_i++) {
        for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
            if(p->pipe_latch[latch_type_i][width_i].valid == true) {
	      printf(" %6u ",(uint32_t)( p->pipe_latch[latch_type_i][width_i].op_id));
            } else {
                printf(" ------ ");
            }
        }
        printf("\n");
    }
    printf("\n");

}


/**********************************************************************
 * Pipeline Main Function: Every cycle, cycle the stage 
 **********************************************************************/

void pipe_cycle(Pipeline *p)
{
    p->stat_num_cycle++;
    pipe_cycle_WB(p);
    pipe_cycle_MEM(p);
    pipe_cycle_EX(p);
    pipe_cycle_ID(p);
    pipe_cycle_FE(p);

}
/**********************************************************************
 * -----------  DO NOT MODIFY THE CODE ABOVE THIS LINE ----------------
 **********************************************************************/

void pipe_cycle_WB(Pipeline *p) {
    int ii;
    for (ii = 0; ii < PIPE_WIDTH; ii++) {
        if (p->pipe_latch[MEM_LATCH][ii].valid) {
            p->stat_retired_inst++;
            if (p->pipe_latch[MEM_LATCH][ii].op_id >= p->halt_op_id) {
                p->halt = true;
            }
        }
    }
}

//--------------------------------------------------------------------//

void pipe_cycle_MEM(Pipeline *p){
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
      p->pipe_latch[MEM_LATCH][ii].valid = p->pipe_latch[EX_LATCH][ii].valid;
      if (p->pipe_latch[MEM_LATCH][ii].valid)
      {
          p->pipe_latch[MEM_LATCH][ii]=p->pipe_latch[EX_LATCH][ii];
      }
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_EX(Pipeline *p){
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
      p->pipe_latch[EX_LATCH][ii].valid = !p->pipe_latch[ID_LATCH][ii].stall;
      if (p->pipe_latch[EX_LATCH][ii].valid)
      {
          p->pipe_latch[EX_LATCH][ii]=p->pipe_latch[ID_LATCH][ii];
      }
  }
}

//--------------------------------------------------------------------//
void swap(Pipeline_Latch *xp, Pipeline_Latch *yp)
{
    Pipeline_Latch temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void pipe_cycle_ID(Pipeline *p) {
    int ii;
    bool early_cycle_stall = false;

    // A function to implement bubble sort
    int i, j;
    for (i = 0; i < PIPE_WIDTH - 1; i++)
        // Last i elements are already in place
        for (j = 0; j < PIPE_WIDTH - i - 1; j++)
            if (p->pipe_latch[FE_LATCH][j].op_id > p->pipe_latch[FE_LATCH][j + 1].op_id)
                swap(&p->pipe_latch[FE_LATCH][j], &p->pipe_latch[FE_LATCH][j + 1]);


    for (ii = 0; ii < PIPE_WIDTH; ii++) {

        p->pipe_latch[ID_LATCH][ii].stall = false;

        // If the instruction before this one has a stall in the fetch stage, then stall this one since it is an
        //  in order pipeline
        if (early_cycle_stall) {
            p->pipe_latch[ID_LATCH][ii].stall = true;
        }

        // Stall condition for EX stage for data dependency
        if(!ENABLE_EXE_FWD) {
            for (int jj = 0; jj < PIPE_WIDTH; jj++) {
                if (p->pipe_latch[EX_LATCH][jj].valid and p->pipe_latch[EX_LATCH][jj].tr_entry.dest_needed) {
                    if (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed
                        and
                        (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg == p->pipe_latch[EX_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    } else if (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed
                               and (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg ==
                                    p->pipe_latch[EX_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    }
                }
            }
        } else {
            for (int jj = 0; jj < PIPE_WIDTH; jj++) {
                if (p->pipe_latch[EX_LATCH][jj].valid and p->pipe_latch[EX_LATCH][jj].tr_entry.dest_needed) {
                    if (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed
                        and
                        (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg == p->pipe_latch[EX_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = p->pipe_latch[EX_LATCH][jj].tr_entry.op_type == OP_LD;
                    } else if (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed
                               and (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg ==
                                    p->pipe_latch[EX_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = p->pipe_latch[EX_LATCH][jj].tr_entry.op_type == OP_LD;
                    }
                }
            }
        }

        // Data dependency for MEM latch
        if (!ENABLE_MEM_FWD) {
            for (int jj = 0; jj < PIPE_WIDTH; jj++) {
                if (p->pipe_latch[MEM_LATCH][jj].valid and p->pipe_latch[MEM_LATCH][jj].tr_entry.dest_needed) {
                    if (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed
                        and
                        (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg == p->pipe_latch[MEM_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    }
                    if (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed
                               and (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg ==
                                    p->pipe_latch[MEM_LATCH][jj].tr_entry.dest)) {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    }
                }
            }
        }

        // check for dependencies with instructions that moved to decode stage in this cycle
        for (int jj=0; jj < ii; jj++)
        {
            if(p->pipe_latch[ID_LATCH][jj].valid and p->pipe_latch[ID_LATCH][jj].tr_entry.dest_needed)
            {
                if(p->pipe_latch[FE_LATCH][ii].tr_entry.src1_needed
                   and (p->pipe_latch[FE_LATCH][ii].tr_entry.src1_reg == p->pipe_latch[ID_LATCH][jj].tr_entry.dest))
                {
                    p->pipe_latch[ID_LATCH][ii].stall = true;
                }
                else if(p->pipe_latch[FE_LATCH][ii].tr_entry.src2_needed
                        and (p->pipe_latch[FE_LATCH][ii].tr_entry.src2_reg == p->pipe_latch[ID_LATCH][jj].tr_entry.dest))
                {
                    p->pipe_latch[ID_LATCH][ii].stall = true;
                }
            }
        }

        // Checks for cc_read dependency
        if(p->pipe_latch[FE_LATCH][ii].tr_entry.cc_read)
        {
            // Checks for the dependency in the EX and MEM stages
            for (int jj=0; jj < PIPE_WIDTH; jj++)
            {
                if(!ENABLE_EXE_FWD) {

                    if (p->pipe_latch[EX_LATCH][jj].valid and p->pipe_latch[EX_LATCH][jj].tr_entry.cc_write) {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    }
                } else {
                    if (p->pipe_latch[EX_LATCH][jj].valid and p->pipe_latch[EX_LATCH][jj].tr_entry.cc_write) {
                        p->pipe_latch[ID_LATCH][ii].stall = p->pipe_latch[EX_LATCH][jj].tr_entry.op_type == OP_LD;

                    }
                }

                if(!ENABLE_MEM_FWD)
                {
                    if(p->pipe_latch[MEM_LATCH][jj].valid and p->pipe_latch[MEM_LATCH][jj].tr_entry.cc_write)
                    {
                        p->pipe_latch[ID_LATCH][ii].stall = true;
                    }
                }

            }

            // Checks for the dependency instructions that just moved to decode stage in this iteration
            for (int jj=0; jj < ii; jj++)
            {
                if(p->pipe_latch[ID_LATCH][jj].valid and p->pipe_latch[ID_LATCH][jj].tr_entry.cc_write)
                {
                    p->pipe_latch[ID_LATCH][ii].stall = true;
                }
            }
        }

        // If the latch is not stalled then pull a new instruction. If it is stalled then the latch is no longer valid
        if (!p->pipe_latch[ID_LATCH][ii].stall) {
            p->pipe_latch[ID_LATCH][ii] = p->pipe_latch[FE_LATCH][ii];
        } else {
            p->pipe_latch[ID_LATCH][ii].valid = false;
        }

        // If the first pipeline was stalled then all future pipelines in this cycle need to be stalled
        early_cycle_stall = early_cycle_stall || p->pipe_latch[ID_LATCH][ii].stall;
    }
}

//--------------------------------------------------------------------//

void pipe_cycle_FE(Pipeline *p) {
    int ii;
    Pipeline_Latch fetch_op;
    bool tr_read_success;

    for (ii = 0; ii < PIPE_WIDTH; ii++) {
        // copy the op in FE LATCH
        if (!p->pipe_latch[ID_LATCH][ii].stall) {
            pipe_get_fetch_op(p, &fetch_op);

            if (BPRED_POLICY) {
                pipe_check_bpred(p, &fetch_op);
            }
            p->pipe_latch[FE_LATCH][ii] = fetch_op;
        }
    }
}

//--------------------------------------------------------------------//

void pipe_check_bpred(Pipeline *p, Pipeline_Latch *fetch_op){
  // call branch predictor here, if mispred then mark in fetch_op
  // update the predictor instantly
  // stall fetch using the flag p->fetch_cbr_stall
}


//--------------------------------------------------------------------//

