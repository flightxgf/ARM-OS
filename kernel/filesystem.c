#include "filesystem.h"


typedef struct{
  char            name[32];
  uint32_t        filesize; 
  uint8_t         isdir;
  uint32_t        blocks[12];
  uint32_t        indirect;
  // uint32_t        double_indirect;
  // uint32_t        triple_indirect;
} inode;

typedef struct {
  uint32_t block_bitmap_index;
  uint32_t inode_bitmap_index;
  uint32_t inode_table_index;
  uint16_t free_block_count;
  uint16_t free_inode_count;
  uint8_t  padding[44];
} bg_descriptor;

typedef struct {
  uint32_t inode_count;
  uint32_t block_count;
  uint32_t NOT_IMPLEMENTED_0;
  uint32_t free_block_count;
  uint32_t free_inode_count;
  uint32_t superblock_index; //WTF we clearly already know this if we've read the superblock
  uint32_t log_block_size; 
  uint32_t log_frag_size;
  uint32_t blocks_per_group;
  uint32_t frags_per_group;
  uint32_t inodes_per_group;
  uint32_t NOT_IMPLEMENTED_1;
  uint32_t NOT_IMPLEMENTED_2;
  uint32_t NOT_IMPLEMENTED_3;
  uint32_t NOT_IMPLEMENTED_4;
  uint16_t magic;
  uint16_t NOT_IMPLEMENTED_5;
  uint16_t NOT_IMPLEMENTED_6;
  uint16_t NOT_IMPLEMENTED_7;
  uint32_t NOT_IMPLEMENTED_8;
  uint32_t NOT_IMPLEMENTED_9;
  uint32_t block_size;
  uint32_t revision_level;
  uint16_t NOT_IMPLEMENTED_11;
  uint16_t NOT_IMPLEMENTED_12;
  uint8_t  padding[940];
} superblock;

// typedef struct {
//   uint8_t  *bits;
// } bitmap;

// typedef struct { 
//   uint32_t id;
//   uint8_t *block_bitmap;
//   uint8_t *inode_bitmap;
//   inode   *inode_table;
//   uint8_t *blocks;
// } block_group;

typedef struct {
  superblock    super;
  bg_descriptor *bg_descriptor_table;
  uint32_t block_group_amount;
  uint32_t root_node;
  // block_group cached_bg;
} partition;

typedef struct {
  uint8_t name[32];
  uint32_t inode_index;
} directory_entry;

partition     *part;
int block_size;
int block_amount;
int drive_size;
int block_pointer_amount;
// inode*


void ext2_init();

void ext2_partition_create(partition *part, uint32_t block_count, uint32_t block_size);

int ext2_superblock_load(partition *part, uint32_t block_size);
void ext2_superblock_write(partition *part, uint32_t index);
void ext2_superblock_create(partition *part, uint32_t block_count, uint32_t block_size);
  
void ext2_bg_descriptor_create(partition *part, uint32_t block_bitmap_index,
     uint32_t inode_bitmap_index, uint32_t inode_table_index, uint32_t bg_index);
void ext2_bg_descriptor_table_load(partition *part, uint32_t index);
void ext2_bg_create(partition *part, uint32_t index, uint32_t bg_number);

uint32_t ext2_inode_create(partition *part, char *name, uint8_t isdir);
void ext2_inode_expand(partition *part, uint32_t inode_index, uint32_t delta);
inode ext2_inode_load(partition *part, uint32_t inode_index);
uint32_t ext2_inode_load_table(partition *part, uint32_t inode_index);

uint32_t ext2_block_get_free(partition *part, uint32_t bg_index);

uint32_t ext2_directory_create(partition *part, char *path, char *name);
void ext2_directory_add_file(partition *part, uint32_t dir_index, char *name, uint32_t inode_index);
void ext2_directory_list(partition *part, char *prefix, uint32_t start_index);

uint32_t ext2_path_to_inode(partition *part, char *name);
uint32_t recursive_path_to_inode(partition *part, uint32_t start_index, char *name);


uint32_t ext2_inode_load_table(partition *part, uint32_t inode_index){
  uint32_t bg_index = inode_index / part->super.inodes_per_group;
  return part->bg_descriptor_table[bg_index].inode_table_index;
}

inode ext2_inode_load(partition *part, uint32_t inode_index){
  uint32_t index = ext2_inode_load_table(part, inode_index);
  inode *nodes = malloc(part->super.block_size);
  disk_rd(index, nodes, part->super.block_size);
  uint32_t precise_index = inode_index % part->super.inodes_per_group;
  return nodes[precise_index];
}
uint32_t ext2_path_to_inode(partition *part, char *path){
  char *next = malloc(32);
  memset(next, 0, 32);
  uint32_t pos;
  uint32_t final = 0;

  for(int I = 0; I < 32; I++){
    if (path[I] == '/'){
      pos = I + 1;
      break;
    }
    else if (path[I] == '\0'){
      final = 1;
      break;
    }
    else{
      next[I] = path[I];
    }
  }
  if(strcmp(next, "root") == 0){
    free(next);
    return recursive_path_to_inode(part, part->root_node, &(path[pos]));
  }
  else{
    free(next);
    return 0;
  }
}

uint32_t recursive_path_to_inode(partition *part, uint32_t start_index, char *path){
  directory_entry *dir = malloc(part->super.block_size);

  char *next = malloc(32);
  memset(next, 0, 32);
  uint32_t pos;
  uint32_t final = 0;

  for(int I = 0; I < 32; I++){
    if (path[I] == '/'){
      pos = I + 1;
      break;
    }
    else if (path[I] == '\0'){
      final = 1;
      break;
    }
    else{
      next[I] = path[I];
    }
  }

  inode i = ext2_inode_load(part, start_index);
  if (i.isdir){
    uint32_t index = i.blocks[0];
    disk_rd(index, dir, part->super.block_size);
    for(int i = 0; i < part->super.block_size / sizeof(directory_entry); i++){
      if (dir[i].name[0] == 0){
        return 0;
      }
      else{
        if (strcmp(dir[i].name, next) == 0){
          if (final == 0){
            free(next);
            return recursive_path_to_inode(part, dir[i].inode_index, &(path[pos]));
          }
          else{
            free(next);
            return dir[i].inode_index;
          }
        }
      }
    }
  }
  else{
    return 0;
  }
}

void ext2_directory_list(partition *part, char *prefix, uint32_t start_index){
  directory_entry *dir = malloc(part->super.block_size);
  inode i = ext2_inode_load(part, start_index);
  if (i.isdir){
    disk_rd(i.blocks[0], dir, part->super.block_size);
    for(int I = 0; I < part->super.block_size / sizeof(directory_entry); I++){
      if (dir[I].name[0] == 0){
        break;
      }
      else{
        printf("%s/%s\n", prefix, dir[I].name);
        ext2_directory_list(part, dir[I].name, dir[I].inode_index);
      }
    }
  }
}

void ext2_directory_add_file(partition *part, uint32_t dir_index, char *name, uint32_t inode_index){
  directory_entry new_dir;
  for(int I = 0; I < 32; I++){
    if (name[I] == '\0'){
      name[I] = '\0';
      break;
    }
    else{
      new_dir.name[I] = name[I];
    }
  }
  new_dir.inode_index = inode_index;

  uint32_t inode_table_block = ext2_inode_load_table(part, dir_index);
  uint32_t *temp = malloc(part->super.block_size);
  inode *inode_table = (inode *) temp;
  inode i = ext2_inode_load(part, dir_index);
  ext2_inode_expand(part, dir_index, sizeof(directory_entry));
  memset(temp, 0, part->super.block_size);
  directory_entry *dirs = (directory_entry*)temp;
  disk_rd(i.blocks[0], dirs, part->super.block_size);
  dirs[(i.filesize / sizeof(directory_entry))] = new_dir;
  disk_wr(i.blocks[0], dirs, part->super.block_size);
  i.filesize += sizeof(directory_entry);
  disk_rd(inode_table_block, inode_table, part->super.block_size);
  inode_table[dir_index % part->super.inodes_per_group] = i;
  disk_wr(inode_table_block, inode_table, part->super.block_size);
}

uint32_t ext2_directory_create(partition *part, char *path, char *name){
  uint32_t parent_index = ext2_path_to_inode(part, path);
  if ((parent_index) == 0){
    printf("invalid path: %s\n", path);
  }
  else{
    uint32_t inode_index = ext2_inode_create(part, name, 1);
    ext2_directory_add_file(part, parent_index, name, inode_index);
  }
}

uint32_t ext2_block_get_free(partition *part, uint32_t bg_index){
  uint8_t *temp = malloc(part->super.block_size);
  disk_rd(part->bg_descriptor_table[bg_index].block_bitmap_index, temp, part->super.block_size);
  uint32_t block_index;
  for(int I = 0; I < part->super.block_size; I++){
    if (temp[I] == 0){
      block_index = I;
      temp[I] = 1;
      break;
    }
  }
  disk_wr(part->bg_descriptor_table[bg_index].block_bitmap_index, temp, part->super.block_size);
  printf("Using block %d of bg %d\n", block_index, bg_index);
  return block_index;
}

void ext2_inode_expand(partition *part, uint32_t inode_index, uint32_t delta){
  uint32_t *temp = malloc(part->super.block_size);
  inode *inode_table = (inode *)temp;
  uint32_t index = inode_index / part->super.inodes_per_group;
  disk_rd(part->bg_descriptor_table[index].inode_table_index, inode_table, part->super.block_size);
  inode new_inode = inode_table[inode_index % part->super.inodes_per_group];
  uint32_t blocks_currently = new_inode.filesize / part->super.block_size + 1;
  uint32_t current_leftover_space =  part->super.block_size - (new_inode.filesize % part->super.block_size);
  uint32_t new_blocks;
  if (delta > current_leftover_space){
    uint32_t new_blocks = ((delta - current_leftover_space) / part->super.block_size);
  }
  else{
    uint32_t new_blocks = 0;
  }


  //less than 12
  for(int I = blocks_currently; I < new_blocks && I < 12; I++){
    new_inode.blocks[I] = ext2_block_get_free(part, index);
    new_blocks--;
  }

  //indirect
  memset(temp, 0, part->super.block_size);
  if(new_inode.indirect == 0){
    new_inode.indirect = ext2_block_get_free(part, index);
  }
  else{
    disk_rd(new_inode.indirect, temp, part->super.block_size);
  }

  uint32_t *indirect = (uint32_t *)temp;
  if (blocks_currently - 12 < 0){
    blocks_currently = 0;
  }
  for(int I = blocks_currently - 12; I < new_blocks && I < part->super.block_size / 4; I++){
    indirect[I] = ext2_block_get_free(part, index);
    new_blocks--;
  }

  memset(temp, 0, part->super.block_size);
  disk_rd(part->bg_descriptor_table[index].inode_table_index, inode_table, part->super.block_size);
  inode_table[inode_index % part->super.inodes_per_group] = new_inode;
  disk_wr(part->bg_descriptor_table[index].inode_table_index, inode_table, part->super.block_size);

}

uint32_t ext2_inode_create(partition *part, char *name, uint8_t isdir){
  uint32_t bg_index;
  for(int I = 0; I < part->super.block_count; I++){
    if (part->bg_descriptor_table[I].free_block_count > 0){
      bg_index = I;
      break;
    }
  }

  inode new_inode;
  memset(&new_inode, 0, sizeof(inode));
  for(int I = 0; I < 32; I++){
    if (name[I] == '\0'){
      name[I] = '\0';
      break;
    }
    else{
      new_inode.name[I] = name[I];
    }
  }
  new_inode.isdir = isdir;
  new_inode.filesize = 0;
  memset(new_inode.blocks, 0, 48);
  new_inode.indirect = 0;

  new_inode.blocks[0] = ext2_block_get_free(part, bg_index);

  uint8_t *temp = malloc(part->super.block_size);
  disk_rd(part->bg_descriptor_table[bg_index].inode_bitmap_index, temp, part->super.block_size);
  uint32_t inode_index;
  for(int I = 0; I < part->super.block_size;I++){
    if (temp[I] == 0){
      inode_index = I;
      temp[I] = 1;
      break;
    }
  }
  disk_wr(part->bg_descriptor_table[bg_index].inode_bitmap_index, temp, part->super.block_size);
  
  memset(temp, 0, part->super.block_size);
  inode *inode_table = (inode *) temp;
  disk_rd(part->bg_descriptor_table[bg_index].inode_table_index, inode_table, part->super.block_size);
  inode_table[inode_index] = new_inode;
  disk_wr(part->bg_descriptor_table[bg_index].inode_table_index, inode_table, part->super.block_size);
  printf("Created %s at index %d\n", name, inode_index + bg_index * part->super.inodes_per_group);
  return inode_index + bg_index * part->super.inodes_per_group;
}

void ext2_bg_create(partition *part, uint32_t index, uint32_t bg_number){
  ext2_superblock_write(part, index);
  char *temp = malloc(part->super.block_size);
  memset(temp, 1, 5);
  disk_wr(index + 2, temp, part->super.block_size);
  ext2_bg_descriptor_create(part, index + 2, index + 3, index + 4, bg_number);
}

void ext2_bg_descriptor_table_write(partition *part, int index){
  uint8_t *temp = malloc(part->super.block_size);
  disk_wr(index, part->bg_descriptor_table, part->super.block_size);
}

void ext2_bg_descriptor_table_load(partition *part, uint32_t index){
  part->block_group_amount = part->super.block_size / sizeof(bg_descriptor);
  part->bg_descriptor_table = malloc(part->super.block_size);
  if(part->super.block_size >= 2048){
    disk_rd(index, part->bg_descriptor_table, part->super.block_size);
  }
  else{
    int index = (2048 / part->super.block_size);
    disk_rd(index, part->bg_descriptor_table, part->super.block_size);
  }
  for(int I = 0; I < part->block_group_amount; I++){
    bg_descriptor desc = part->bg_descriptor_table[I];
    if (desc.block_bitmap_index != 0){
      part->block_group_amount += 1;
      part->super.free_inode_count += desc.free_inode_count;
      part->super.free_block_count += desc.free_block_count;
    }
  }
}

void ext2_bg_descriptor_create(partition *part, uint32_t block_bitmap_index,
 uint32_t inode_bitmap_index, uint32_t inode_table_index, uint32_t bg_index){
  bg_descriptor desc;
  desc.block_bitmap_index = block_bitmap_index;
  desc.inode_bitmap_index = inode_bitmap_index;
  desc.inode_table_index  = inode_table_index;
  desc.free_block_count = part->super.blocks_per_group - 5;
  desc.free_inode_count = part->super.block_size / sizeof(inode);
  part->bg_descriptor_table[bg_index] = desc;
  ext2_bg_descriptor_table_write(part, part->super.superblock_index + 1);
}

void ext2_superblock_create(partition *part, uint32_t block_count, uint32_t block_size){
  part->super.inode_count        = block_size  * sizeof(inode);
  part->super.block_count        = block_count;
  part->super.free_block_count   = part->super.block_count - (part->block_group_amount * 5);
  part->super.free_inode_count   = part->block_group_amount * block_size / sizeof(inode);
  if (block_size <= 1024){
    part->super.superblock_index = 1024 / block_size;
  }
  else{
    part->super.superblock_index = 0;
  }
  part->super.log_block_size     = 0; // TODO FIXME
  part->super.log_frag_size      = -10;
  part->super.blocks_per_group   = block_size;
  part->super.frags_per_group    = 0;
  part->super.inodes_per_group   = block_size / sizeof(inode);
  part->super.magic              = 0xEF53;
  part->super.block_size         = block_size;
  part->super.revision_level     = 0;
}

void ext2_superblock_write(partition *part, uint32_t index){
  superblock *new_super = malloc(part->super.block_size);
  if (part->super.block_size > 1024){
    memcpy(new_super + (part->super.block_size - 1024), &part->super, sizeof(superblock));
    disk_wr(index, new_super, part->super.block_size);
  }
  else{
    uint32_t length_in_blocks = 1024 / part->super.block_size;
    memcpy(new_super, &part->super, sizeof(superblock));
    for(int I = 0; I < length_in_blocks; I++){
      disk_wr(index + I, &(new_super[I * part->super.block_size]), part->super.block_size);
    }
  }
}

int ext2_superblock_load(partition *part, uint32_t block_size){
  superblock new_super;
  if (block_size > 1024){
    uint8_t *temp = malloc(block_size);
    disk_rd(0, temp, block_size);
    memcpy(&new_super, temp + (block_size - 1024), 1024);
  }
  else{
    int I = 1024 / block_size;
    disk_rd(I, &part->super, block_size);
  }
  printf("MAGIC: 0x%08x\n", new_super.magic);
  if (part->super.magic == 0xEF53){
    return 0;
  }
  else{
    printf("SUPERBLOCK NOT FOUND\n");
    return 1;
  }
}

void ext2_partition_create(partition *part, uint32_t block_count, uint32_t block_size){
    ext2_superblock_create(part, block_count, block_size);  
    part->bg_descriptor_table = malloc(part->super.block_size);
    uint32_t descriptor_index = 0;
    for(int I = part->super.superblock_index;
    I < part->super.block_count - part->super.blocks_per_group;
    I += part->super.blocks_per_group){
      printf("Creating bg_descriptors %d / %d\n", descriptor_index, part->block_group_amount);
      ext2_bg_create(part, I, descriptor_index);
      descriptor_index++;
    }

    uint32_t root = ext2_inode_create(part, "root", 1);
    //testing code
    uint32_t test_dir = ext2_inode_create(part, "test_dir", 1);
    ext2_directory_add_file(part, root, "test_dir", test_dir);
    ext2_directory_create(part, "root/test_dir", "test_subdir");
    printf("File test_dir at block %d\n", ext2_path_to_inode(part, "root/test_dir"));
    printf("File test_dir/test_subdir at block %d\n", ext2_path_to_inode(part, "root/test_dir/test_subdir"));
    printf("File fake_file at block %d\n", ext2_path_to_inode(part, "root/fake_file"));

    for(int I = part->super.superblock_index;
    I < part->super.block_count - part->super.blocks_per_group;
    I += part->super.blocks_per_group){
      ext2_superblock_write(part, I);
      ext2_bg_descriptor_table_write(part, I+1);
    }
}

void ext2_init(){
  uint32_t block_size =  disk_get_block_len();
  uint32_t block_count = disk_get_block_num();
  part = malloc(sizeof(partition));
  printf("DISK: %d blocks of size %d\n", block_count, block_size);
  part->block_group_amount = (block_count / (block_size)) - 1;
  if (ext2_superblock_load(part, block_size)){
    printf("DISK: partition not found\n");
    printf("DISK: creating partition\n");
    ext2_partition_create(part, block_count, block_size);  
  }
  else{
    printf("DISK: found ext2 partition\n");
    ext2_bg_descriptor_table_load(part, part->super.superblock_index + 1);
  }
  printf("Loaded directories\n");
  // ext2_directory_list(part,"root", part->root_node);
}
