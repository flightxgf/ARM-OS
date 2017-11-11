#include "irq_handler.h"

void irq_handler(ctx_t* ctx) {
  int_unable_irq();
  uint32_t id = GICC0->IAR;

  if(id == GIC_SOURCE_UART0){
    // printf("HANDLING IRQ FROM UART0\n");
    char c = PL011_getc( UART0, true );
    if (c == '\r'){
      c = '\n';
    }
    PL011_putc(UART0, c, true);
    stdin_insert(c);
    UART0->ICR = 0x10;
  }
  if(id == GIC_SOURCE_TIMER0){
    // printf("HANDLING IRQ FROM TIMER0\n");
    TIMER0->Timer1IntClr = 0x01;
    scheduler(ctx);
  }


  GICC0->EOIR = id;
  return;
}