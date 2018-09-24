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

    // need to increment the tail. If the tail is past the number
    //    of elements, then it will reset back to 0
    t->tail_ptr = (t->tail_ptr + 1) % MAX_ROB_ENTRIES;

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

}


/////////////////////////////////////////////////////////////
// Check if the oldest ROB entry is ready for commit
/////////////////////////////////////////////////////////////

bool ROB_check_head(ROB *t){

}

/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t){
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
