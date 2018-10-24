#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"


extern uns64 cycle; // You can use this as timestamp for LRU
extern uns64 CACHE_LINESIZE;
extern uns64 REPL_POLICY; // 0:LRU 1:RAND

////////////////////////////////////////////////////////////////////
// ------------- DO NOT MODIFY THE INIT FUNCTION -----------
////////////////////////////////////////////////////////////////////

Cache  *cache_new(uns64 size, uns64 assoc, uns64 linesize, uns64 repl_policy){

   Cache *c = (Cache *) calloc (1, sizeof (Cache));
   c->num_ways = assoc;
   c->repl_policy = repl_policy;

   if(c->num_ways > MAX_WAYS){
     printf("Change MAX_WAYS in cache.h to support %llu ways\n", c->num_ways);
     exit(-1);
   }

   // determine num sets, and init the cache
   c->num_sets = size/(linesize*assoc);
   c->sets  = (Cache_Set *) calloc (c->num_sets, sizeof(Cache_Set));

   return c;
}

////////////////////////////////////////////////////////////////////
// ------------- DO NOT MODIFY THE PRINT STATS FUNCTION -----------
////////////////////////////////////////////////////////////////////

void    cache_print_stats    (Cache *c, char *header){
  double read_mr =0;
  double write_mr =0;

  if(c->stat_read_access){
    read_mr=(double)(c->stat_read_miss)/(double)(c->stat_read_access);
  }

  if(c->stat_write_access){
    write_mr=(double)(c->stat_write_miss)/(double)(c->stat_write_access);
  }

  printf("\n%s_READ_ACCESS    \t\t : %10llu", header, c->stat_read_access);
  printf("\n%s_WRITE_ACCESS   \t\t : %10llu", header, c->stat_write_access);
  printf("\n%s_READ_MISS      \t\t : %10llu", header, c->stat_read_miss);
  printf("\n%s_WRITE_MISS     \t\t : %10llu", header, c->stat_write_miss);
  printf("\n%s_READ_MISSPERC  \t\t : %10.3f", header, 100*read_mr);
  printf("\n%s_WRITE_MISSPERC \t\t : %10.3f", header, 100*write_mr);
  printf("\n%s_DIRTY_EVICTS   \t\t : %10llu", header, c->stat_dirty_evicts);

  printf("\n");
}



////////////////////////////////////////////////////////////////////
// Note: the system provides the cache with the line address
// Return HIT if access hits in the cache, MISS otherwise 
// Also if is_write is TRUE, then mark the resident line as dirty
// Update appropriate stats
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

Flag cache_access(Cache *c, Addr lineaddr, uns is_write, uns core_id){
  Flag outcome=MISS;

  // Your Code Goes Here

  //check to see if line is in the cache
  uns index_mask = createMask(power_2(CACHE_LINESIZE), power_2(CACHE_LINESIZE) + power_2(c->num_ways)-1);
  uns index = (uns) (lineaddr & index_mask);
  uns tag = (uns) ((lineaddr / (CACHE_LINESIZE)) / c->num_ways);

  if (c->sets->line[index].valid && c->sets->line[index].tag == tag)
  {
      outcome=HIT;
      // Only need to set dirty bit on cache write?
      c->sets->line[index].dirty = TRUE;
      c->sets->line[index].last_access_time = cycle;
  }
  else
  {
      outcome = MISS;
  }

  return outcome;
}

////////////////////////////////////////////////////////////////////
// Note: the system provides the cache with the line address
// Install the line: determine victim using repl policy (LRU/RAND)
// copy victim into last_evicted_line for tracking writebacks
////////////////////////////////////////////////////////////////////

void cache_install(Cache *c, Addr lineaddr, uns is_write, uns core_id){

  // Your Code Goes Here
    uns index_mask = createMask(power_2(CACHE_LINESIZE), power_2(CACHE_LINESIZE) + power_2(c->num_ways)-1);
    uns index = (uns) (lineaddr & index_mask);
    uns tag = (uns) ((lineaddr / (CACHE_LINESIZE)) / c->num_ways);
    // make sure to set last access time
//    cache_find_victim(c, )
  // Find victim using cache_find_victim
  // Initialize the evicted entry
  // Initialize the victime entry

}

////////////////////////////////////////////////////////////////////
// You may find it useful to split victim selection from install
////////////////////////////////////////////////////////////////////


uns cache_find_victim(Cache *c, uns set_index, uns core_id){
  uns victim=0;
  uns last_access_time;

  // TODO: Write this using a switch case statement
  for(int i=0; i<c->num_ways; i++) {
      if (!c->sets->line[i].valid) {
          return (uns) i;
      }
  }

  switch (REPL_POLICY)
  {
      case 0: //LRU
            // since this will only occur if all the cache lines are valid, then we just take the first valid line
            //  as the last_access_time (we don't have to worry about getting non-valid line)
            last_access_time = c->sets->line[0].last_access_time;
            for(int j=0; j<c->num_ways; j++)
            {
                if (c->sets->line[j].last_access_time < last_access_time)
                {
                    last_access_time = c->sets->line[j].last_access_time;
                    victim = j;
                }
            }
            break;
      case 1: //RAND
            return cycle % c->num_ways;
      default:break;
  }

  return victim;
}

