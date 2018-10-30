//#include <iostream> 
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

FILE *outfile;
uint32_t outfile_traces=0;

#define MAX_ITER 100

void dump_trace(uint32_t iaddr, uint32_t type, uint32_t vaddr);

int main (int argc, char **argv) {
  int ii, iterid;
  char command_string2[1024];
  sprintf(command_string2,"gzip -f > %s", "out.mtr.gz");
  outfile = popen(command_string2, "w"); 
  printf("%s opened for writing trace\n", "out.mtr.gz");

  for(iterid=0; iterid < 100 ; iterid++){
    for(ii=0; ii< 33; ii++){
      dump_trace(0, 1, 1024*1024*ii);
    }
  }

  printf("Outfile should have %u traces\n", outfile_traces);

  fclose(outfile);
}




void dump_trace(uint32_t iaddr, uint32_t type, uint32_t vaddr){
  
  outfile_traces++;
  fwrite (&iaddr, 4, 1, outfile);
  fwrite (&type, 1, 1, outfile);
  fwrite (&vaddr, 4, 1, outfile);
}
