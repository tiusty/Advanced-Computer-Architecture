#include <stdio.h>
#include <assert.h>

#include "rest.h"

extern int32_t NUM_REST_ENTRIES;

/////////////////////////////////////////////////////////////
// Init function initializes the Reservation Station
/////////////////////////////////////////////////////////////

REST* REST_init(void){
  int ii;
  REST *t = (REST *) calloc (1, sizeof (REST));
  for(ii=0; ii<MAX_REST_ENTRIES; ii++){
    t->REST_Entries[ii].valid=false;
  }
  assert(NUM_REST_ENTRIES<=MAX_REST_ENTRIES);
  return t;
}

////////////////////////////////////////////////////////////
// Print State
/////////////////////////////////////////////////////////////
void REST_print_state(REST *t){
 int ii = 0;
  printf("Printing REST \n");
  printf("Entry  Inst Num  S1_tag S1_ready S2_tag S2_ready  Vld Scheduled\n");
  for(ii = 0; ii < NUM_REST_ENTRIES; ii++) {
    printf("%5d ::  \t\t%d\t", ii, (int)t->REST_Entries[ii].inst.inst_num);
    printf("%5d\t\t", t->REST_Entries[ii].inst.src1_tag);
    printf("%5d\t\t", t->REST_Entries[ii].inst.src1_ready);
    printf("%5d\t\t", t->REST_Entries[ii].inst.src2_tag);
    printf("%5d\t\t", t->REST_Entries[ii].inst.src2_ready);
    printf("%5d\t\t", t->REST_Entries[ii].valid);
    printf("%5d\n", t->REST_Entries[ii].scheduled);
    }
  printf("\n");
}

/////////////////////////////////////////////////////////////
//------- DO NOT CHANGE THE CODE ABOVE THIS LINE -----------
/////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
// If space return true else return false
/////////////////////////////////////////////////////////////

bool  REST_check_space(REST *t){

    // Checks every entry in the REST table and sees if there is
    //  an opening. An opening is defined by having a valid
    //  bit as false
    for (int i=0; i < MAX_REST_ENTRIES; i++)
    {
        // If there is an entry with the valid bit as false
        //  then an open spot was found
        if (!t->REST_Entries[i].valid)
        {
            return true;
        }
    }

    // If the for loop finishes and it couldn't find an entry
    //  with the valid bit as false, then the scheduler
    //  is full
    return false;

}

/////////////////////////////////////////////////////////////
// Insert an inst in REST, must do check_space first
/////////////////////////////////////////////////////////////

void  REST_insert(REST *t, Inst_Info inst){

    // Insert instruction into the index where
    //  the valid = false entry was found
    for (int i=0; i < MAX_REST_ENTRIES; i++)
    {
        // If an index is found with the valid bit
        //  as false, then the index is open to be scheduled.
        // Store the instruction at that index and then break
        //  the loop
        if(!t->REST_Entries[i].valid)
        {
            t->REST_Entries->inst = inst;
            break;
        }
    }

}

/////////////////////////////////////////////////////////////
// When instruction finishes execution, remove from REST
/////////////////////////////////////////////////////////////

void  REST_remove(REST *t, Inst_Info inst){

    // When an instruction finishes execution, then look for
    //  that instruction in the REST table and remove it by
    //  marking the valid bit as false

    for (int i=0; i< MAX_REST_ENTRIES; i++)
    {
        // When the matching instruction (via the instruction number)
        //  is found, then mark the valid bit as false to indicate,
        //  that it is done and being removed from the REST table.
        // Also mark the entry has not scheduled so when the entry becomes
        //  valid again, it doesn't try to get scheduled
        if(t->REST_Entries[i].valid && t->REST_Entries[i].inst.inst_num == inst.inst_num)
        {
            t->REST_Entries[i].scheduled = false;
            t->REST_Entries[i].valid = false;
        }
    }

}

/////////////////////////////////////////////////////////////
// For broadcast of freshly ready tags, wakeup waiting inst
/////////////////////////////////////////////////////////////

void  REST_wakeup(REST *t, int tag){

    // Check all the instructions in the REST table and mark any srcs that
    //  match the tag as ready
    for (int i=0; i<MAX_REST_ENTRIES; i++)
    {
        // If the REST entry is not valid then ignore
        if(t->REST_Entries[i].valid)
        {
            // If the src1_tag of the instruction matches the tag, then mark it as ready
            if(t->REST_Entries[i].inst.src1_tag != -1 && t->REST_Entries[i].inst.src1_tag == tag)
            {
                t->REST_Entries[i].inst.src1_ready = true;
            }

            // if the src2_rag of the instruction matches the tag, then mark it as ready
            if(t->REST_Entries[i].inst.src2_tag != -1 && t->REST_Entries[i].inst.src2_tag == tag)
            {
                t->REST_Entries[i].inst.src1_ready = true;
            }
        }
    }

}

/////////////////////////////////////////////////////////////
// When an instruction gets scheduled, mark REST entry as such
/////////////////////////////////////////////////////////////

void  REST_schedule(REST *t, Inst_Info inst){

    // If an instruction gets scheduled, find where the instruction
    //  is stored in the REST table and mark it has scheduled
    for (int i=0; i<MAX_REST_ENTRIES; i++)
    {
        //  When the corresponding entry is found then mark
        //  that it has been scheduled
        if(t->REST_Entries[i].valid && t->REST_Entries[i].inst.inst_num == inst.inst_num)
        {
            t->REST_Entries[i].scheduled = true;
            break;
        }
    }

}
