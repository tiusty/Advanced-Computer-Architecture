#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"



//---- Your answers must be stored in this (top N used for N-way cache)
extern uns conflict_list[8];


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
unsigned int power_2(uns64 number)
{
  int power = -1;
  int value = 1;
  while(value < number)
  {
    value = 1;
    power += 1;
    for(int i=0; i<power; i++)
    {
      value = value *2;
    }
  }
  return power;
}

unsigned createMask(unsigned a, unsigned b)
{
  unsigned r = 0;
  for (unsigned i=a; i<=b; i++)
    r |= 1 << i;

  return r;
}


void fill_conflict_list(Cache *c, uns num_ways, uns probe_addr){
  uns miss = 0;
  uns cline_addr1=rand()%(1<<20);
  uns cline_addr2=rand()%(1<<20);
  uns cline_addr3=rand()%(1<<20);
  uns cline_addr4=rand()%(1<<20);

  // Store the probe_addr in cache
  cache_access_install(c, probe_addr);





  //---- The function you should use to determine conflicts
  // --- The second argument must be less than (1<<20);

  for (int i=0; i<2^10; i++){
      cache_access_install(c, i);
    if( cache_access_install(c, probe_addr)==MISS ){
      uns tag = probe_addr >> 10;
      uns addr = tag << 10;
      addr = addr | i;
      miss++;

      if (miss == 1) {
        cline_addr1 = addr;
      } else if (miss == 2) {
        cline_addr2 =addr;
      }
    }

    if (miss == num_ways) {
      break;
    }

  }


  //------- Your code goes above this line

  conflict_list[0]=cline_addr1; //--- for 1 way
  conflict_list[1]=cline_addr2; //--- for 2 way
  conflict_list[2]=cline_addr3; //--- for 4 way
  conflict_list[3]=cline_addr4; //--- for 4 way 
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

