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
std::vector<uint8_t> reg_used;

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
    pipe_print_state(p);
	    
}
/**********************************************************************
 * -----------  DO NOT MODIFY THE CODE ABOVE THIS LINE ----------------
 **********************************************************************/

void pipe_cycle_WB(Pipeline *p) {
    int ii;
    for (ii = 0; ii < PIPE_WIDTH; ii++) {
        if (p->pipe_latch[MEM_LATCH][ii].valid) {

            p->stat_retired_inst++;
            if (p->pipe_latch[MEM_LATCH][ii].tr_entry.dest_needed) {
                std::vector<uint8_t, std::allocator<uint8_t >>::iterator it;
                it = std::find(reg_used.begin(), reg_used.end(), p->pipe_latch[MEM_LATCH][ii].tr_entry.dest);
                if (it != reg_used.end()) {
                    reg_used.erase(it);
                }
            }

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

      // If the ID latch is marked as not valid because of halting, then finish the last transfer but then
      // don't transfer anymore until the ID latch is valid again
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_ID(Pipeline *p) {
    int ii;
    for (ii = 0; ii < PIPE_WIDTH; ii++) {
        // If the register does not have any dependency's then no stalling

        // If the register does have dependency's then check to see if the dependencies are in the list
        //  if it is then stall, if not then continue

        if (!p->pipe_latch[ID_LATCH][ii].stall) {
            p->pipe_latch[ID_LATCH][ii] = p->pipe_latch[FE_LATCH][ii];
        }

        // Checks to see if there are any needed registers, i.e depedencies
        if (p->pipe_latch[ID_LATCH][ii].tr_entry.src1_needed || p->pipe_latch[ID_LATCH][ii].tr_entry.src2_needed ||
            p->pipe_latch[ID_LATCH][ii].tr_entry.dest_needed) {

            // Checks to see if the dependencies exist in the used registers
            if (std::find(reg_used.begin(), reg_used.end(), p->pipe_latch[ID_LATCH]->tr_entry.src1_reg) ==
                reg_used.end()
                and std::find(reg_used.begin(), reg_used.end(), p->pipe_latch[ID_LATCH]->tr_entry.src2_reg) ==
                    reg_used.end()
                and
                std::find(reg_used.begin(), reg_used.end(), p->pipe_latch[ID_LATCH]->tr_entry.dest) ==
                reg_used.end()) {

                // If it doesn't then add the list to the chart and contiunue
                if (p->pipe_latch[ID_LATCH][ii].tr_entry.dest_needed) {
                    reg_used.push_back(p->pipe_latch[ID_LATCH][ii].tr_entry.dest);
                }
                p->pipe_latch[ID_LATCH][ii].stall = false;
            } else {
                p->pipe_latch[ID_LATCH][ii].stall = true;
            }
        } else {
            p->pipe_latch[ID_LATCH][ii].stall = false;
        }

            if (ENABLE_MEM_FWD) {
                // todo
            }

            if (ENABLE_EXE_FWD) {
                // todo
            }
        }

}

//--------------------------------------------------------------------//

void pipe_cycle_FE(Pipeline *p) {
    int ii;
    Pipeline_Latch fetch_op;
    bool tr_read_success;

    for (ii = 0; ii < PIPE_WIDTH; ii++) {
        // copy the op in FE LATCH
        if (!p->pipe_latch[FE_LATCH][ii].stall) {
            pipe_get_fetch_op(p, &fetch_op);

            if (BPRED_POLICY) {
                pipe_check_bpred(p, &fetch_op);
            }

            p->pipe_latch[FE_LATCH][ii] = fetch_op;

        }
        p->pipe_latch[FE_LATCH][ii].stall = p->pipe_latch[ID_LATCH][ii].stall;

    }
}

//--------------------------------------------------------------------//

void pipe_check_bpred(Pipeline *p, Pipeline_Latch *fetch_op){
  // call branch predictor here, if mispred then mark in fetch_op
  // update the predictor instantly
  // stall fetch using the flag p->fetch_cbr_stall
}


//--------------------------------------------------------------------//

