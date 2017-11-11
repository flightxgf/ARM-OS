#include "memory.h"

#define KERNEL_PAGE_TABLE_SIZE 16


// typedef struct {
//   pte_t *main;
//   int size;
//  } pte_section;

//  typedef struct {
//    uint32_t size;
//    uint32_t owners;
//    uint32_t 
//  }

// pte_section program_page_tables[64];
// pte_section kernel_page_table;
// pte_section io_page_table;

// typedef uint32_t pte_t;
// typedef struct {
//   pte_t page;
//   int   location;
// } addressed_pte_t;
typedef struct {
  uint32_t page;
  uint32_t location;
  uint32_t fd;
  uint8_t  swapped;
  uint8_t  assigned;
  uint8_t  shared;
  uint32_t owner;
  uint32_t  *patrons;
  uint32_t *patron_mount_point;
  uint32_t patron_amount;
} meta_pte_t;

typedef struct {
  meta_pte_t pages[4096];
  uint32_t *used_pages;
  uint32_t used_page_amount;
} page_table_segment;

typedef struct {
  uint32_t start;
  uint32_t end;
  uint8_t  *map;
} free_memory_map;


page_table_segment *process_page_table;
page_table_segment kernel_page_table;
meta_pte_t         *shared_page_table;
free_memory_map    physical_memory;
free_memory_map    shared_memory[64];
uint32_t current_page_table [4096] __attribute__ ((aligned (1 << 14)));

//didn't want to have to do this
uint32_t process_page_table_size;
uint32_t shared_page_table_size;

// extern uint32_t tos_usr;
uint32_t kernel_heap;
extern uint32_t sysmem_start;
extern uint32_t kernel_start;
extern uint32_t kernel_end;


void setup_mmu(){
  process_page_table_size = 0;

  physical_memory.start = 0x700;
  physical_memory.end   = 0x8d0;
  physical_memory.map   = malloc((physical_memory.end - physical_memory.start) * sizeof(uint8_t));
  
  for (int I = 0; I < 64; I++){
    shared_memory[I].start = 0xa00;
    shared_memory[I].end   = 0xaFF;
    shared_memory[I].map   = malloc((shared_memory[I].end - shared_memory[I].start) * sizeof(uint8_t));
  }

  create_kernel_page_table();
  load_kernel_page_table();
}

// void kernel_swap_page(uint32_t addr){
//   kernel_page_table[KERNEL_PAGE_TABLE_SIZE - 1] = (addr & 0xFFF00000) | 0x402;
// }

void cleanup_process(int pid){
  uint32_t old_process = current_process;
  switch_page_table(pid);
  for(int I = 0; I < process_page_table[pid].used_page_amount; I++){
    uint32_t index = process_page_table[pid].used_pages[I];
    meta_pte_t pte = process_page_table[pid].pages[index];
    memset((void*)(pte.location << 20), 0, 0x00100000);
    physical_memory.map[pte.page >> 20 - physical_memory.start] = 0;
  }
  memset(&process_page_table[pid], 0, sizeof(page_table_segment));
  for(int I = shared_page_table_size - 1; I > -1; I--){
    meta_pte_t page = shared_page_table[I];
    for(int J = page.patron_amount - 1; J > -1; J--){
      if (page.patrons[J] == pid){
        if (J < shared_page_table[I].patron_amount){
          memmove(&(shared_page_table[I].patrons[J]), &(shared_page_table[I].patrons[J + 1]), sizeof(uint32_t) * (page.patron_amount - J - 1));
          memmove(&(shared_page_table[I].patron_mount_point[J]), &(shared_page_table[I].patron_mount_point[J + 1]), sizeof(uint32_t) * (page.patron_amount - J - 1));
        }
        shared_page_table[I].patron_amount--;
        switch_page_table(pid);
        if(shared_page_table[I].patron_amount == 0){
          current_page_table[0x8ff] = shared_page_table[I].page & 0xFFF00000 | 0x402;
          mmu_flush();
          memset(0x8FF00000, 0, 0x00100000);
          physical_memory.map[(shared_page_table[I].page >> 20) - physical_memory.start] = 0;
          printf("Deleting page at 0x%08x\n", shared_page_table[I].page & 0xFFF00000);
          if (I < shared_page_table_size){
            memmove(&(shared_page_table[I]), &(shared_page_table[I + 1]), sizeof(meta_pte_t) * (shared_page_table_size - I - 1));
          }
          shared_page_table_size--;
          shared_page_table = realloc(shared_page_table, sizeof(meta_pte_t) * shared_page_table_size);
        }
      }
    }
  }
  memset(shared_memory[pid].map, 0, shared_memory[pid].end - shared_memory[pid].start);
  current_process = old_process;
  switch_page_table(current_process);
}

void create_kernel_page_table(){
  // kernel_page_table.used_pages = malloc((0xFFF - 0x08F0) * sizeof(uint32_t));
  for(uint32_t I = 0x8d0; I < 0x900; I++){
    meta_pte_t pte;
    pte.page     = (I << 20) | 0xC02;
    pte.location = I;
    pte.assigned = 1;
    pte.swapped  = 0;
    pte.fd       = 0;
    pte.shared   = 0;
    kernel_page_table.pages[I]         = pte;

    uint32_t size = kernel_page_table.used_page_amount;
    size += 1;

    uint32_t *new_used_pages = malloc(size * sizeof(uint32_t));
    uint32_t *old_used_pages = kernel_page_table.used_pages;
    memcpy(new_used_pages, old_used_pages, (size - 1) * sizeof(uint32_t));
    new_used_pages[size - 1] = I;
    free(old_used_pages);
    kernel_page_table.used_pages      = new_used_pages;
    kernel_page_table.used_page_amount = size;
  }
  for(uint32_t I = 0x900; I < 0xa00; I++){
    meta_pte_t pte;
    pte.page     = ((I - 0x800) << 20) | 0x402; //should be 402
    pte.location = I;
    pte.assigned = 1;
    pte.swapped  = 0;
    pte.fd       = 0;
    pte.shared   = 0;
    kernel_page_table.pages[I]         = pte;

    uint32_t size = kernel_page_table.used_page_amount;
    size += 1;

    uint32_t *new_used_pages = malloc(size * sizeof(uint32_t));
    uint32_t *old_used_pages = kernel_page_table.used_pages;
    memcpy(new_used_pages, old_used_pages, (size - 1) * sizeof(uint32_t));
    new_used_pages[size - 1] = I;
    free(old_used_pages);
    kernel_page_table.used_pages      = new_used_pages;
    kernel_page_table.used_page_amount = size;
  }
  for(uint32_t I = 0xa00; I < 0x1000; I++){
    meta_pte_t pte;
    pte.page     = (I << 20) | 0x402;
    pte.location = I;
    pte.assigned = 1;
    pte.swapped  = 0;
    pte.fd       = 0;
    pte.shared   = 0;
    kernel_page_table.pages[I]         = pte;

    uint32_t size = kernel_page_table.used_page_amount;
    size += 1;

    uint32_t *new_used_pages = malloc(size * sizeof(uint32_t));
    uint32_t *old_used_pages = kernel_page_table.used_pages;
    memcpy(new_used_pages, old_used_pages, (size - 1) * sizeof(uint32_t));
    new_used_pages[size - 1] = I;
    free(old_used_pages);
    kernel_page_table.used_pages      = new_used_pages;
    kernel_page_table.used_page_amount = size;
  }
}

uint32_t request_free_memory(free_memory_map map){
  for(int I = map.start; I < map.end; I++){
    if (!map.map[I - map.start]){
      map.map[I - map.start] = 1;
      return (I << 20);
    }
  }
  //we've run out and are fucked
  //so fucked that we've forgotten the return type
  return -1;
}

void create_page_table(int pid){
  if (pid + 1 > process_page_table_size){
    process_page_table_size = pid + 1;
  }
  page_table_segment *new = malloc((process_page_table_size) * sizeof(page_table_segment));
  memset(&(new[process_page_table_size - 1]), 0, sizeof(page_table_segment));
  page_table_segment *old    = process_page_table;
  memcpy(new, old, (process_page_table_size - 1) * sizeof(page_table_segment));
  process_page_table = new;
  free(old);
}

int allocate_new_page(int pid, uint32_t addr){
  uint32_t I = (addr & 0xFFF00000) >> 20;
  meta_pte_t pte;
  pte.page     = request_free_memory(physical_memory) | 0xC02;
  pte.location = I; 
  pte.assigned = 1;
  pte.swapped  = 0;
  pte.fd       = 0;
  pte.owner    = pid;
  pte.shared   = 0;
  pte.patron_amount = 0;
  process_page_table[pid].pages[I]         = pte;

  uint32_t size = process_page_table[pid].used_page_amount;
  size += 1;

  uint32_t *new_used_pages = malloc(size * sizeof(uint32_t));
  uint32_t *old_used_pages = process_page_table[pid].used_pages;
  memcpy(new_used_pages, old_used_pages, (size - 1) * sizeof(uint32_t));
  new_used_pages[size - 1] = I;
  free(old_used_pages);
  process_page_table[pid].used_pages       = new_used_pages;
  process_page_table[pid].used_page_amount = size;
  return 0;
}

int __attribute__((optimize("O0"))) allocate_new_shared_page(int parent_pid, int child_pid){
  uint32_t addr = request_free_memory(shared_memory[parent_pid]);
  uint32_t I = (addr & 0xFFF00000) >> 20;
  meta_pte_t pte;
  pte.page     = request_free_memory(physical_memory);
  pte.location = I; 
  pte.assigned = 1;
  // pte.swapped  = 0;
  // pte.fd       = 0;
  pte.owner    = parent_pid;
  pte.shared   = 1;
  pte.patron_amount = 1;
  pte.patrons = malloc(sizeof(uint32_t));
  pte.patrons[0] = child_pid;
  pte.patron_mount_point = malloc(sizeof(uint32_t));
  pte.patron_mount_point[0] = request_free_memory(shared_memory[child_pid]) >> 20;

  shared_page_table_size++;
  shared_page_table = realloc(shared_page_table, shared_page_table_size * sizeof(meta_pte_t));
  shared_page_table[shared_page_table_size - 1] = pte;

  return pte.page & 0xFFF00000;
}

int allocate_new_page_at_address(int pid, uint32_t addr, uint32_t physical_addr){
  uint32_t I = (addr & 0xFFF00000) >> 20;
  meta_pte_t pte;
  pte.page     = physical_addr | 0xC02;
  pte.location = I;
  pte.assigned = 1;
  pte.swapped  = 0;
  pte.fd       = 0;
  pte.owner     = pid;
  pte.shared   = 0;
  process_page_table[pid].pages[I]         = pte;

  uint32_t size = process_page_table[pid].used_page_amount;
  size += 1;

  uint32_t *new_used_pages = malloc(size * sizeof(uint32_t));
  uint32_t *old_used_pages = process_page_table[pid].used_pages;
  memcpy(new_used_pages, old_used_pages, (size - 1) * sizeof(uint32_t));
  new_used_pages[size - 1] = I;
  free(old_used_pages);
  process_page_table[pid].used_pages       = new_used_pages;
  process_page_table[pid].used_page_amount = size;
  return 0;
}

void fork_copy_ro(int pid, int new_pid){
  for (int I = 0; I < shared_page_table_size; I++){
    for(int J = 0; J < shared_page_table[I].patron_amount;J++){
      if(shared_page_table[I].patrons[J] == pid){
        shared_page_table[I].patron_amount++;
        shared_page_table[I].patrons = realloc(shared_page_table[I].patrons, shared_page_table[I].patron_amount);
        shared_page_table[I].patrons[shared_page_table[I].patron_amount - 1] = new_pid;
      }
    }
  }
  shared_page_table = realloc(shared_page_table, sizeof(meta_pte_t) * (shared_page_table_size +
   process_page_table[pid].used_page_amount));
  for (int I = 0; I < process_page_table[pid].used_page_amount; I++){
    uint32_t index = process_page_table[pid].used_pages[I];
    shared_page_table[shared_page_table_size] = process_page_table[pid].pages[index];
    shared_page_table[shared_page_table_size].page &= ~0x08C00;
    // shared_page_table[shared_page_table_size].page |=  0x00802; // should be 802
    shared_page_table[shared_page_table_size].shared = 1;
    shared_page_table[shared_page_table_size].patron_amount += 2;
    shared_page_table[shared_page_table_size].patrons = realloc(shared_page_table[shared_page_table_size].patrons, 
    shared_page_table[shared_page_table_size].patron_amount);
    shared_page_table[shared_page_table_size].patrons[shared_page_table[shared_page_table_size].patron_amount - 2] = pid;
    shared_page_table[shared_page_table_size].patrons[shared_page_table[shared_page_table_size].patron_amount - 1] = new_pid;
    shared_page_table[shared_page_table_size].patron_mount_point = realloc(shared_page_table[shared_page_table_size].patron_mount_point, 
    shared_page_table[shared_page_table_size].patron_amount);
    shared_page_table[shared_page_table_size].patron_mount_point[shared_page_table[shared_page_table_size].patron_amount - 2] = index;
    shared_page_table[shared_page_table_size].patron_mount_point[shared_page_table[shared_page_table_size].patron_amount - 1] = index;
    shared_page_table[shared_page_table_size].owner = 0;
    shared_page_table_size++;
  }

  process_page_table[pid].used_page_amount = 0;
  memset(&(process_page_table[pid].pages), 0, 4096 * sizeof(meta_pte_t));
  memset(&(process_page_table[new_pid].pages), 0, 4096 * sizeof(meta_pte_t));
  // memcpy(&(process_page_table[new_pid]), &(process_page_table[pid]), sizeof(page_table_segment));
}

void load_kernel_page_table(){
  for(int I = 0; I < 0xFFF; I++){
    current_page_table[I] = 0;
  }
  for(int I = 0; I < kernel_page_table.used_page_amount; I++){
    uint32_t index = kernel_page_table.used_pages[I];
    current_page_table[index] = kernel_page_table.pages[index].page;
  }
  mmu_flush();
}

void __attribute__((optimize("O0"))) handle_data_abort(int pid, uint32_t addr){
  uint32_t index = addr >> 20;
  for(int I = shared_page_table_size - 1; I > -1; I--){
    if (shared_page_table[I].location == index){
      meta_pte_t page = shared_page_table[I];
      for(int J = page.patron_amount - 1; J > -1; J--){
        if (page.patrons[J] == pid){
          uint32_t physical_addr = request_free_memory(physical_memory);
          printf("Duplicating ro page 0x%08x for process %d at 0x%08x\n", index << 20, pid, physical_addr);
          current_page_table[0x8FF] = physical_addr | 0x402;
          mmu_flush();
          memcpy(0x8FF00000, (addr & 0xFFF00000), 0x00100000);
          allocate_new_page_at_address(pid, addr, physical_addr);

          if (J < shared_page_table[I].patron_amount){
            memmove(&(shared_page_table[I].patrons[J]), &(shared_page_table[I].patrons[J + 1]), sizeof(uint32_t) * (page.patron_amount - J - 1));
            memmove(&(shared_page_table[I].patron_mount_point[J]), &(shared_page_table[I].patron_mount_point[J + 1]), sizeof(uint32_t) * (page.patron_amount - J - 1));
          }

          // memcpy(shared_page_table[I].patrons, page.patrons, sizeof(uint32_t) * J);
          // memcpy(shared_page_table[I].patron_mount_point, page.patron_mount_point, sizeof(uint32_t) * J);

          // memcpy(&(shared_page_table[I].patrons[J]), &(page.patrons[J + 1]), sizeof(uint32_t) * page.patron_amount - J - 1);
          // memcpy(&(shared_page_table[I].patron_mount_point[J]), &(page.patron_mount_point[J + 1]), sizeof(uint32_t) * page.patron_amount - J - 1);
          shared_page_table[I].patron_amount--;
          switch_page_table(pid);
          if(shared_page_table[I].patron_amount == 0){
            current_page_table[0x8ff] = page.page & 0xFFF00000 | 0x402;
            mmu_flush();
            memset(0x8FF00000, 0, 0x00100000);
            physical_memory.map[(page.page >> 20) - physical_memory.start] = 0;
            printf("Deleting page at 0x%08x\n", page.page & 0xFFF00000);
            if (I < shared_page_table_size){
              memmove(&(shared_page_table[I]), &(shared_page_table[I + 1]), sizeof(meta_pte_t) * (shared_page_table_size - I - 1));
            }
            shared_page_table_size--;
            shared_page_table = realloc(shared_page_table, sizeof(meta_pte_t) * shared_page_table_size);
          }
          return;
        }
      }
    }
  }
  printf("allocating new page\n");
  allocate_new_page(pid, addr);
  switch_page_table(pid);
}

void __attribute__((optimize("O0"))) switch_page_table(int pid){
  for(int I = 0; I < 0x8d0; I++){
      current_page_table[I] = 0;
  }
  for(int I = 0; I < process_page_table[pid].used_page_amount; I++){
    uint32_t index = process_page_table[pid].used_pages[I];
    current_page_table[index] = process_page_table[pid].pages[index].page;
  }
  for(int I = 0; I < shared_page_table_size; I++){
    if (shared_page_table[I].owner == pid && pid != 0){
      current_page_table[shared_page_table[I].location] = shared_page_table[I].page | 0xC02;
    }
    else{
      for(int J = 0; J < shared_page_table[I].patron_amount;J++){
        if(shared_page_table[I].patrons[J] == pid){
          current_page_table[shared_page_table[I].patron_mount_point[J]]
          = shared_page_table[I].page | 0x802;
        }
      }
    }
  }
  mmu_flush();
 }