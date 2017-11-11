SECTIONS {
  . = . + 0x70000000;
  sysmem_start = .;
  . = . + 0x00010000;
  /* place initial bootloader        */
  .bootloader_text : { kernel/bootloader.o(.text .rodata) }
  .bootloader_data : { kernel/bootloader.o(.data) } 
  .bootloader_bss  : { kernel/bootloader.o(.bss)  } 
  . = 0x8d000000;
  kernel_start = .;
  /* place text segment(s)           */
  .text : { *(EXCLUDE_FILE (bootloader.o) .text .rodata) }
  /* place data segment(s)           */        
  .data : { *(EXCLUDE_FILE (bootloader.o) .data) }
  /* place bss  segment(s)           */        
  .bss  : { *(EXCLUDE_FILE (bootloader.o) .bss COMMON) }
  /* align       address (per AAPCS) */
  .       = ALIGN( 8 );        
  /* allocate stack for irq mode     */
  .       = . + 0x00001000;  
  tos_irq = .;
  /* allocate stack for abt mode     */
  .       = . + 0x00100000;  
  tos_abt = .;
  /* allocate stack for svc mode     */
  .       = . + 0x00100000;  
  tos_svc = .;
  /* allocate stack for usr mode     */
  .       = . + 0x00100000;  
  .heap : {
  end = .;
  _kernel_heap_start = .;
  . = 0x8FF00000;
  _kernel_heap_end = .;
  }
  kernel_end = .;
}
