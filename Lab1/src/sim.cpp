/********************************************************************  *
 * File         : reader.cpp
 * Author       : Soham J. Desai 
 * Date         : 2nd January 2014
 * Description  : Trace Redear and Analyzer for Lab 1 of ECE6100
 ********************************************************************/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "trace.h"
#include <vector>
#include <algorithm>


/*********************************************************************
 * Globals and Statistics
 ********************************************************************/

FILE     *tr_file;
Trace_Rec tr_entry;

uint64_t stat_num_inst = 0;
uint64_t stat_num_cycle = 0;
uint64_t stat_optype_dyn[NUM_OP_TYPE] = { 0 }; // dynamic count for each op type
uint64_t stat_unique_pc = 0;  // number of unique PCs
std::vector<uint64_t> v; //stores PCs that have been executed

/*********************************************************************
 * Functions
 ********************************************************************/

void die_message(const char *msg) {
    printf("Error! %s. Exiting...\n", msg);
    exit(1);
}
void print_stats();
void analyze_trace_record(Trace_Rec *t);    

/*********************************************************************
 * Main
 *********************************************************************/

int main(int argc, char *argv[]){
  char tr_filename[1024];
  char cmd_string[256];
  
  if(argc == 1) {
    die_message("Must Provide a Trace File"); 
  }
  
  strcpy(tr_filename, argv[1]);
  
  // ------- Open Trace File -------------------------------------------
  
  sprintf(cmd_string,"gunzip -c %s", tr_filename);
  if ((tr_file = popen(cmd_string, "r")) == NULL){
    printf("Command string is %s\n", cmd_string);
    die_message("Unable to open the trace file with gzip option \n")  ;
  } else {
    printf("Opened file with command: %s \n", cmd_string);
  }
  
  // ------- Read From Trace File --------------------------------------
  uint8_t bytes_read = 0;  
  
  while(!feof(tr_file)) {
    bytes_read = fread(&tr_entry, 1, sizeof(Trace_Rec), tr_file);

    if( bytes_read < sizeof(Trace_Rec)) {
      break; 
    }
    
    stat_num_inst++;
    analyze_trace_record(&tr_entry);
  }
  
  // ------- Print Statistics ------------------------------------------
  print_stats(); 
  fclose(tr_file);
  return 0;	

}

/*********************************************************************
 * Print Statistics --------- DO NOT MODIFY THIS FUNCTION ------------
 *********************************************************************/

void print_stats(){
  char header[256];
  sprintf(header, "LAB1");
  
  if(stat_num_inst==0){
    stat_num_inst=1; // To avoid divide by zero errors
  }
  double cpi = (double)(stat_num_cycle)/(double)(stat_num_inst);
  
  printf("\n%s_NUM_INST           \t : %10lu", header, stat_num_inst)  ;
  printf("\n%s_NUM_CYCLES         \t : %10lu", header, stat_num_cycle  );
  
  printf("\n%s_CPI                \t : %6.3f",  header, cpi);
  printf("\n%s_UNIQUE_PC          \t : %10lu", header, stat_unique_pc  );
  
  printf("\n");
  
  printf("\n%s_NUM_ALU_OP         \t : %10lu", header, stat_optype_dyn[OP_ALU]);
  printf("\n%s_NUM_LD_OP          \t : %10lu", header, stat_optype_dyn[OP_LD]);
  printf("\n%s_NUM_ST_OP          \t : %10lu", header, stat_optype_dyn[OP_ST]);
  printf("\n%s_NUM_CBR_OP         \t : %10lu", header, stat_optype_dyn[OP_CBR]);
  printf("\n%s_NUM_OTHER_OP       \t : %10lu", header, stat_optype_dyn[OP_OTHER]);
  
  printf("\n");
  
  printf("\n%s_PERC_ALU_OP        \t : %6.3f", header, 100.0*(double)(  stat_optype_dyn[OP_ALU])/(double)(stat_num_inst));
  printf("\n%s_PERC_LD_OP         \t : %6.3f", header, 100.0*(double)(  stat_optype_dyn[OP_LD])/(double)(stat_num_inst));
  printf("\n%s_PERC_ST_OP         \t : %6.3f", header, 100.0*(double)(  stat_optype_dyn[OP_ST])/(double)(stat_num_inst));
  printf("\n%s_PERC_CBR_OP        \t : %6.3f", header, 100.0*(double)(  stat_optype_dyn[OP_CBR])/(double)(stat_num_inst));
  printf("\n%s_PERC_OTHER_OP      \t : %6.3f", header, 100.0*(double)(  stat_optype_dyn[OP_OTHER])/(double)(stat_num_inst));
  
  printf("\n\n");
}

/*********************************************************************
 * ------------- DO NOT MODIFY THE CODE ABOVE THIS LINE -------------*
 *********************************************************************/
  
/*********************************************************************
 * Trace Analysis (Students need to write this function)
 *********************************************************************/
void analyze_trace_record(Trace_Rec *t){
    assert(t);

    
    // Switch case counts number of each instruction and also adds the number of
    // cycles taken. Number of cycles per instruction is defined by lab.
    switch ( t->opcode ) {
        case OP_ALU:
            stat_optype_dyn[OP_ALU]++;
            stat_num_cycle++;
            break;
        case OP_LD:
            stat_optype_dyn[OP_LD]++;
            stat_num_cycle += 2;
            break;
        case OP_ST:
            stat_optype_dyn[OP_ST]++;
            stat_num_cycle += 2;
            break;
        case OP_CBR:
            stat_optype_dyn[OP_CBR]++;
            stat_num_cycle += 3;
            break;
        case OP_OTHER:
            stat_optype_dyn[OP_OTHER]++;
            stat_num_cycle += 1;
            break;
        default:
            printf("Opcode not qualified\n");
            break;
    }

    // This statement looks to see if the inst_addr already exists in the vector
    // If it already exists then it has already been executed and is not a
    // unique PC. If it doesn't exist, then add the inst_addr to the vector and
    // increment the unique_pc value.
    if(std::find(v.begin(), v.end(), t->inst_addr) == v.end()) {
        v.push_back(t->inst_addr);
        stat_unique_pc++;
    }
}
