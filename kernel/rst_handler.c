#include "rst_handler.h"


extern uint32_t _kernel_heap_start;
extern void     main_console(); 
extern void     main_P1(); 
extern void     main_P2(); 
extern void     main_P3(); 
extern void     main_P4(); 
// extern void     main_P5(); 
extern void     main_P6(); 

void rst_handler(ctx_t *ctx) {
  kernel_heap = &_kernel_heap_start;
  setup_mmu();
  create_page_table(0);
  mmu_set_ptr0(current_page_table);
  // mmu_set_dom( 0, 0x3 ); // set domain 0 to 11_{(2)} => manager (i.e., not checked)
  mmu_set_dom( 0, 0x1 ); // set domain 0 to 01_{(2)} => client  (i.e.,     checked)
  mmu_set_dom( 1, 0x1 ); // set domain 1 to 01_{(2)} => client  (i.e.,     checked)
  mmu_enable();

  working_printf("STARTING KERNEL\n");
  printf("%s: %d + %d = 0x%08x\n","TESTING", 5, 7, 12);


  process_queue = malloc(sizeof(bin_heap));
  heap_init(process_queue);

  ext2_init();




  init_processes();
  add_process((uint32_t)&main_console);
  // add_process((uint32_t)&main_P3);
  // add_process((uint32_t)&main_P6);
  // setpriority(2, PRIORITY_HIGH);
  switch_page_table(0);
  // add_process((uint32_t)&main_P2);
  // add_process((uint32_t)&main_P2);
  // add_process((uint32_t)&main_P2);

  // add_process((uint32_t)&main_P1);
  // add_process((uint32_t)&main_P2);
  // add_process((uint32_t)&main_P3);
  // add_process((uint32_t)&main_P4);
  // add_process((uint32_t)&main_P5);

  current_process = 0;
  memcpy(ctx, &(pcb[current_process].ctx), sizeof(ctx_t));

  TIMER0->Timer1Load  = 0x00010000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  UART0->IMSC       |= 0x00000010; // enable UART    (Rx) interrupt
  UART0->CR          = 0x00000301; // enable UART (Tx+Rx)

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICD0->ISENABLER1  |= 0x00001000; // enable UART          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor


  printf("STARTING TERMINAL\n");
}