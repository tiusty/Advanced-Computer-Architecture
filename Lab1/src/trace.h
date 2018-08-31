#ifndef _TRACE_H_
#define _TRACE_H_

#include <inttypes.h>

typedef enum Op_Type_Enum{
  OP_ALU,       // ALU(ADD/SUB/MUL/DIV) operaiton
  OP_LD,        // load operation
  OP_ST,        // store operation
  OP_CBR,       // Conditional Branch
  OP_OTHER,     // Other Ops
  NUM_OP_TYPE
} Op_Type;



/* Data structure for trace record */ 
typedef struct Trace_Rec_Struct{
  uint64_t inst_addr;     // instruction address 
  uint8_t  opcode;       // opcode (see enum above)
} Trace_Rec;

#endif
