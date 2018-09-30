#include <stdio.h>
#include <assert.h>

#include "rob.h"


extern int32_t NUM_ROB_ENTRIES;

/////////////////////////////////////////////////////////////
// Init function initializes the ROB
/////////////////////////////////////////////////////////////

ROB* ROB_init(void){
  int ii;
  ROB *t = (ROB *) calloc (1, sizeof (ROB));
  for(ii=0; ii<MAX_ROB_ENTRIES; ii++){
    t->ROB_Entries[ii].valid=false;
    t->ROB_Entries[ii].ready=false;
  }
  t->head_ptr=0;
  t->tail_ptr=0;
  return t;
}

/////////////////////////////////////////////////////////////
// Print State
/////////////////////////////////////////////////////////////
void ROB_print_state(ROB *t){
 int ii = 0;
  printf("Printing ROB \n");
  printf("Entry  Inst   Valid   ready\n");
  for(ii = 0; ii < 7; ii++) {
    printf("%5d ::  %d\t", ii, (int)t->ROB_Entries[ii].inst.inst_num);
    printf(" %5d\t", t->ROB_Entries[ii].valid);
    printf(" %5d\n", t->ROB_Entries[ii].ready);
  }
  printf("\n");
}

/////////////////////////////////////////////////////////////
//------- DO NOT CHANGE THE CODE ABOVE THIS LINE -----------
/////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
// If there is space in ROB return true, else false
/////////////////////////////////////////////////////////////

bool ROB_check_space(ROB *t){

    // Since the ROB is a circular buffer and the tail
    //  always points to where a new instruction will go
    //  (i.e to an empty space), the only time that there won't
    //  be space is when the head == tail cus then the circular
    //  buffer filled up
    if(t->tail_ptr == t->head_ptr)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/////////////////////////////////////////////////////////////
// insert entry at tail, increment tail (do check_space first)
/////////////////////////////////////////////////////////////

int ROB_insert(ROB *t, Inst_Info inst){

    // Inserts an element in the tail
    t->ROB_Entries[t->tail_ptr].inst = inst;

    // Now mark the entry as valid but not ready
    t->ROB_Entries[t->tail_ptr].valid = true;
    t->ROB_Entries[t->tail_ptr].ready = false;

    // The int associated with the tail_ptr should be the int associated with the ROB_id
    int prf_id = t->tail_ptr;

    // need to increment the tail. If the tail is past the number
    //    of elements, then it will reset back to 0
    t->tail_ptr = (t->tail_ptr + 1) % MAX_ROB_ENTRIES;

    return prf_id;

}

/////////////////////////////////////////////////////////////
// Once an instruction finishes execution, mark rob entry as done
/////////////////////////////////////////////////////////////

void ROB_mark_ready(ROB *t, Inst_Info inst) {

    // Loop through all the elements in the ROB,
    //  if the entry is valid and the instruction
    //  numbers match, then mark the entry as done, by
    //  setting ready = 1
    for (int i=0; i<MAX_ROB_ENTRIES; i++)
    {
        // When the corresponding instruction is found, then mark the instruction as ready to
        //  indicate that the instruction is done execution
        if(t->ROB_Entries[i].valid && t->ROB_Entries[i].inst.inst_num == inst.inst_num)
        {
            t->ROB_Entries[i].ready = true;
            break;
        }
    }

}

/////////////////////////////////////////////////////////////
// Find whether the prf-rob entry is ready
/////////////////////////////////////////////////////////////

bool ROB_check_ready(ROB *t, int tag){

    // Checks to see if the corresponding ROB entry is valid
    //  and ready
    if (t->ROB_Entries[tag].valid && t->ROB_Entries[tag].ready)
    {
        return true;
    }
    else
    {
        return false;
    }

}


/////////////////////////////////////////////////////////////
// Check if the oldest ROB entry is ready for commit
/////////////////////////////////////////////////////////////

bool ROB_check_head(ROB *t){

    // Checks to see if the instruction at the head is ready and valid
    if(t->ROB_Entries[t->head_ptr].valid && t->ROB_Entries[t->head_ptr].ready)
    {
        return true;
    }
    else
    {
        return false;
    }

}

/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t){

    // To remove the oldest entry, mark the entry as not valid and not ready
    //  and then increment the head prt

    // Store the committed instruction so the value can be returned
    Inst_Info committed_inst = t->ROB_Entries[t->head_ptr].inst;

    // Mark the entry as not ready and not valid
    t->ROB_Entries[t->head_ptr].valid = false;
    t->ROB_Entries[t->head_ptr].ready = false;

    // Increment the head ptr
    t->head_ptr = (t->head_ptr + 1) % MAX_ROB_ENTRIES;

    // Return the instruction that was committed
    return committed_inst;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
