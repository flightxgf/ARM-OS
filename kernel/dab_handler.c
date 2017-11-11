#include "dab_handler.h"

void dab_handler(uint32_t instr, ctx_t *ctx) {
  // printf("HANDLING DATA ABORT FROM PROCESS %d\n", current_process);

  char negative_offset =  !((instr & 0x800000) >> 23);
  char register_offset =  (instr & 0x2000000) >> 25;
  char base_register =  ((instr & 0xF0000)  >> 16);
  uint32_t raw_offset;
  char shift_right;
  uint32_t shift_amount;

  if (register_offset){
    uint32_t offset_register = instr & 0xF; 
    if (offset_register < 13){
      raw_offset = ctx->gpr[base_register];
    }
    else if (offset_register == 13){
      raw_offset = ctx->sp;
    }
    else if (offset_register == 14){
      raw_offset = ctx->lr;
    }
    else if (offset_register == 15){
      raw_offset = ctx->pc;
    }
    else{
      printf("UNKNOWN OFFSET REGISTER\n");
    }

    shift_amount = (instr & 0xF80) >> 7;
    shift_right  = (instr & 0x20)  >> 5;
    printf("base_register: %d\n", base_register);
    printf("offset_register: %d\n", offset_register);
    printf("shift_amount: %d\n", shift_amount);
    printf("shift_right: %d\n", shift_right);
    if (shift_right){
      raw_offset = raw_offset >> shift_amount;
    }
    else{
      raw_offset = raw_offset << shift_amount;
    }
  } 
  else{
    raw_offset  =  ((instr & 0xFFF));
    shift_amount = 0;
    shift_right = 0;
  }
  int offset;
  if (negative_offset){
    offset = -1 * raw_offset;
  }
  else{
    offset = raw_offset;
  }

  uint32_t register_value;
  if (base_register < 13){
    register_value = ctx->gpr[base_register];
  }
  else if (base_register == 13){
    register_value = ctx->sp;
  }
  else if (base_register == 14){
    register_value = ctx->lr;
  }
  else if (base_register == 15){
    register_value = ctx->pc;
  }
  else{
    printf("UNKNOWN BASE REGISTER\n");
  }

  uint32_t address = register_value + offset;
  // printf("ADDRESS: 0x%08x\n", address);

  if (address > 0x8D000000){
    printf("ACCESS TO RESTRICTED PAGE\n");
    sleep(10);
  } 
  else{
 handle_data_abort(current_process, address);
     printf("ALLOCATED NEW PAGE AT 0x%08x\n", (address & 0xFFF00000));
  }
  // sleep(1);
  return;
}