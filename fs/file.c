#include "file.h"
#include "stdio-kernel.h"
#include "super_block.h"
#include "fs.h"
#include "string.h"
#include "interrupt.h"

FILE * stderr;
/* 文件表 */
struct file file_table[MAX_FILE_OPEN];

struct index get_index(uint32_t size){
  uint32_t count = DIV_ROUND_UP(size,BLOCK_SIZE);
  struct index index;
  index.direct = 0;
  index.sing =0;
  index.dou = 0;
  if (count <= 12){
    index.direct = count;
    return index;
  } else if (count <= 12 + 128){
    index.sing = count - 12;
    return index;
  } else{
    index.dou = 1;
    count -= 140;
    index.sing = count / 128;
    index.direct = count % 128;
    return index;
  }
}
/* 从文件表file_table中获取一个空闲位,成功返回下标,失败返回-1 */
int32_t get_free_slot_in_global(void)
{
    uint32_t fd_idx = 3;
    while (fd_idx < MAX_FILE_OPEN)
    {
        if (file_table[fd_idx].used == 0)
        {
            file_table[fd_idx].used = 1;
            break;
        } else {
        fd_idx++;
        }
    }
    if (fd_idx == MAX_FILE_OPEN)
    {
        printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/* 将全局描述符下标安装到进程或线程自己的文件描述符数组fd_table中,
 * 成功返回下标,失败返回-1 */
int32_t pcb_fd_install(int32_t globa_fd_idx)
{
    struct task_struct *cur = running_thread();
    uint8_t local_fd_idx = 3; // 跨过stdin,stdout,stderr
    while (local_fd_idx < MAX_FILES_OPEN_PER_PROC)
    {
        if (cur->fd_table[local_fd_idx] == -1)
        { // -1表示free_slot,可用
            cur->fd_table[local_fd_idx] = globa_fd_idx;
            break;
        }
        local_fd_idx++;
    }
    if (local_fd_idx == MAX_FILES_OPEN_PER_PROC)
    {
        printk("exceed max open files_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/* 分配一个i结点,返回i结点号 */
int32_t inode_bitmap_alloc(struct partition *part)
{
    int32_t bit_idx = bitmap_scan(&part->inode_bitmap, 1);
    if (bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->inode_bitmap, bit_idx, 1);
    return bit_idx;
}

/* 分配1个扇区,返回其扇区地址 */
int32_t block_bitmap_alloc(struct partition *part)
{
    int32_t bit_idx = bitmap_scan(&part->block_bitmap, 1);
    if (bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->block_bitmap, bit_idx, 1);
    /* 和inode_bitmap_malloc不同,此处返回的不是位图索引,而是具体可用的扇区地址 */
    return (part->sb->data_start_lba + bit_idx);
}

/* 将内存中bitmap第bit_idx位所在的512字节同步到硬盘 */
void bitmap_sync(struct partition *part, uint32_t bit_idx, uint8_t btmp_type)
{
    uint32_t off_sec = bit_idx / 4096;        // 本i结点索引相对于位图的扇区偏移量
    uint32_t off_size = off_sec * BLOCK_SIZE; // 本i结点索引相对于位图的字节偏移量
    uint32_t sec_lba;
    uint8_t *bitmap_off;

    /* 需要被同步到硬盘的位图只有inode_bitmap和block_bitmap */
    switch (btmp_type)
    {
    case INODE_BITMAP:
        sec_lba = part->sb->inode_bitmap_lba + off_sec;
        bitmap_off = part->inode_bitmap.bits + off_size;
        break;

    case BLOCK_BITMAP:
        sec_lba = part->sb->block_bitmap_lba + off_sec;
        bitmap_off = part->block_bitmap.bits + off_size;
        break;
    }
    ide_write(part->my_disk, sec_lba, bitmap_off, 1);
}

/* 创建文件,若成功则返回文件描述符,否则返回-1 */
int32_t file_create(struct dir *parent_dir, char *filename, uint8_t flag)
{
    /* 后续操作的公共缓冲区 */
    void *io_buf = sys_malloc(1024);
    if (io_buf == NULL)
    {
        printk("in file_creat: sys_malloc for io_buf failed\n");
        return -1;
    }

    uint8_t rollback_step = 0; // 用于操作失败时回滚各资源状态

    /* 为新文件分配inode */
    int32_t inode_no = inode_bitmap_alloc(cur_part);
    if (inode_no == -1)
    {
        printk("in file_creat: allocate inode failed\n");
        goto rollback;
    }

    /* 此inode要从堆中申请内存,不可生成局部变量(函数退出时会释放)
     * 因为file_table数组中的文件描述符的inode指针要指向它.*/
    struct inode *new_file_inode = (struct inode *)sys_malloc(sizeof(struct inode));
    if (new_file_inode == NULL)
    {
        printk("file_create: sys_malloc for inode failded\n");
        rollback_step = 1;
        goto rollback;
    }
    inode_init(inode_no, new_file_inode); // 初始化i结点

    /* 返回的是file_table数组的下标 */
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1)
    {
        printk("exceed max open files\n");
        rollback_step = 2;
        goto rollback;
    }

    file_table[fd_idx].fd_inode = new_file_inode;
    file_table[fd_idx].fd_pos = 0;
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    struct dir_entry new_dir_entry;
    memset(&new_dir_entry, 0, sizeof(struct dir_entry));

    create_dir_entry(filename, inode_no, FT_REGULAR, &new_dir_entry); // create_dir_entry只是内存操作不出意外,不会返回失败

    /* 同步内存数据到硬盘 */
    /* a 在目录parent_dir下安装目录项new_dir_entry, 写入硬盘后返回true,否则false */
    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf))
    {
        printk("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    memset(io_buf, 0, 1024);
    /* b 将父目录i结点的内容同步到硬盘 */
    inode_sync(cur_part, parent_dir->inode, io_buf);

    memset(io_buf, 0, 1024);
    /* c 将新创建文件的i结点内容同步到硬盘 */
    inode_sync(cur_part, new_file_inode, io_buf);

    /* d 将inode_bitmap位图同步到硬盘 */
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    /* e 将创建的文件i结点添加到open_inodes链表 */
    list_push(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;

    sys_free(io_buf);
    return pcb_fd_install(fd_idx);

/*创建文件需要创建相关的多个资源,若某步失败则会执行到下面的回滚步骤 */
rollback:
    switch (rollback_step)
    {
    case 3:
        /* 失败时,将file_table中的相应位清空 */
        memset(&file_table[fd_idx], 0, sizeof(struct file));
    case 2:
        sys_free(new_file_inode);
    case 1:
        /* 如果新文件的i结点创建失败,之前位图中分配的inode_no也要恢复 */
        bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
        break;
    }
    sys_free(io_buf);
    return -1;
}

/* 打开编号为inode_no的inode对应的文件,若成功则返回文件描述符,否则返回-1 */
int32_t file_open(uint32_t inode_no, uint8_t flag)
{
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1)
    {
        printk("exceed max open files\n");
        return -1;
    }
    file_table[fd_idx].fd_inode = inode_open(cur_part, inode_no);
    file_table[fd_idx].fd_pos = 0; // 每次打开文件,要将fd_pos还原为0,即让文件内的指针指向开头
    file_table[fd_idx].fd_flag = flag;
    bool *write_deny = &file_table[fd_idx].fd_inode->write_deny;

    if (flag & O_WRONLY || flag & O_RDWR)
    { // 只要是关于写文件,判断是否有其它进程正写此文件
        // 若是读文件,不考虑write_deny
        /* 以下进入临界区前先关中断 */
        enum intr_status old_status = intr_disable();
        if (!(*write_deny))
        {                                // 若当前没有其它进程写该文件,将其占用.
            *write_deny = true;          // 置为true,避免多个进程同时写此文件
            intr_set_status(old_status); // 恢复中断
        }
        else
        { // 直接失败返回
            intr_set_status(old_status);
            file_table[fd_idx].fd_inode = NULL;
            printk("file can`t be write now, try again later\n");
            return -1;
        }
    } // 若是读文件或创建文件,不用理会write_deny,保持默认
    return pcb_fd_install(fd_idx);
}

/* 关闭文件 */
int32_t file_close(struct file *file)
{
    if (file == NULL)
    {
        return -1;
    }
    file->fd_inode->write_deny = false;
    inode_close(file->fd_inode);
    file->fd_inode = NULL; // 使文件结构可用
    file->used = 0;
    return 0;
}
static uint32_t get_lba(void)
{
  int32_t block_lba = block_bitmap_alloc(cur_part);
  if (block_lba == -1)
  {
    while(1);
  }
  int32_t block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
  ASSERT(block_bitmap_idx != 0);
  bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
  return (uint32_t)block_lba;
}
static void buf_write(uint32_t* i_sectors,uint32_t used_size,uint32_t new_size,uint32_t used_index,uint32_t new_index,uint8_t*buf,uint8_t*io_buf)
{
      uint32_t block_offset = used_size % BLOCK_SIZE;
  uint32_t bytes_written = 0;
      if (block_offset)
      {
        if (used_index == new_index)
        {
          uint32_t lba = i_sectors[used_index -1];
          ide_read(cur_part->my_disk,lba,io_buf,1);
          memcpy(io_buf+block_offset,buf,new_size - used_size);
          ide_write(cur_part->my_disk,lba,io_buf,1);
      return;
        } else 
        {
          bytes_written = BLOCK_SIZE - block_offset;
          uint32_t lba = i_sectors[used_index -1];
          ide_read(cur_part->my_disk,lba,io_buf,1);
          memcpy(io_buf+block_offset,buf,bytes_written);
          ide_write(cur_part->my_disk,lba,io_buf,1);
        }
      }
      for (uint32_t i = used_index ;i<new_index -1;i++)
      {

        uint32_t lba = get_lba();
        i_sectors[i] = lba;
        ide_write(cur_part->my_disk,lba,buf + bytes_written,1);
        bytes_written += BLOCK_SIZE;
      }
      uint32_t end_offset = new_size % BLOCK_SIZE;
      uint32_t lba = get_lba();
      i_sectors[new_index -1] = lba;
      end_offset = end_offset ? end_offset : BLOCK_SIZE;
      memcpy(io_buf,buf+bytes_written,end_offset);
      ide_write(cur_part->my_disk,lba,io_buf,1);
}
int32_t file_write(struct file *file, const void *buf, uint32_t count){
  if ((file->fd_inode->i_size + count) > (BLOCK_SIZE * (12 + 128 + 128 * 128)))
  { // 文件目前最大只支持512*140=71680字节
     printk("exceed max file_size 71680 bytes, write file failed\n");
     return -1;
  }
  uint8_t* io_buf = sys_malloc(BLOCK_SIZE);
  uint32_t* lba_buf = sys_malloc(BLOCK_SIZE);
  uint32_t* double_buf = sys_malloc(BLOCK_SIZE);
  if (io_buf == NULL)
  {
      printk("file_write: sys_malloc for io_buf failed\n");
      return -1;
  }
  uint8_t * buf_dst = (uint8_t*)buf;
  uint32_t used_size = file->fd_inode->i_size;
  uint32_t new_size = file->fd_inode->i_size + count;
  struct index used_index = get_index(used_size);
  struct index new_index = get_index(new_size);
  file->fd_pos = file->fd_inode->i_size - 1;
  if ((used_index.dou == 0&& used_index.direct != 0)|| used_size == 0)
  {
    if (new_index.dou == 0&& new_index.direct != 0)
    {
      buf_write(file->fd_inode->i_sectors,used_size,new_size,used_index.direct,new_index.direct,buf_dst,io_buf);
    } else if (new_index.dou == 0&& new_index.sing != 0)
    {
      uint32_t bytes_written = 0;
      buf_write(file->fd_inode->i_sectors,used_size,12*BLOCK_SIZE,used_index.direct,12,buf_dst,io_buf);
      bytes_written += 12*BLOCK_SIZE - used_size;
      uint32_t sing_lba = get_lba();
      file->fd_inode->i_sectors[12] = sing_lba;
      memset(lba_buf,0,BLOCK_SIZE);
      buf_write(lba_buf,12*BLOCK_SIZE,new_size,0,new_index.sing,buf_dst+bytes_written,io_buf);
      ide_write(cur_part->my_disk,sing_lba,lba_buf,1);
    } else 
    {
      uint32_t bytes_written = 0;
      buf_write(file->fd_inode->i_sectors,used_size,12*BLOCK_SIZE,used_index.direct,12,buf_dst,io_buf);
      bytes_written += 12*BLOCK_SIZE - used_size;
      uint32_t sing_lba = get_lba();
      file->fd_inode->i_sectors[12] = sing_lba;
      memset(lba_buf,0,BLOCK_SIZE);
      buf_write(lba_buf,12*BLOCK_SIZE,140*BLOCK_SIZE,0,128,buf_dst+bytes_written,io_buf);
      ide_write(cur_part->my_disk,sing_lba,lba_buf,1);
      bytes_written += 128*BLOCK_SIZE;
      uint32_t double_lba= get_lba();
      file->fd_inode->i_sectors[13] = double_lba;
      memset(lba_buf,0,BLOCK_SIZE);
      for (int32_t i = 0;i<(int32_t)new_index.sing;i++)
      {

        uint32_t lba = get_lba();
        lba_buf[i] = lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,0,0,128,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = get_lba();
        lba_buf[new_index.sing] = lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,new_size,0,new_index.direct,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
      }
      ide_write(cur_part->my_disk,double_lba,lba_buf,1);

    }
  } else if (used_index.dou == 0&& used_index.sing != 0)
  {
    if (new_index.dou == 0&& new_index.sing != 0)
    {
      uint32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
      buf_write(lba_buf,used_size,new_size,used_index.sing,new_index.sing,buf_dst,io_buf);
    } else 
    {
      uint32_t bytes_written = 0;
      uint32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
      buf_write(lba_buf,used_size,140*BLOCK_SIZE,used_index.sing,128,buf_dst,io_buf);
      ide_write(cur_part->my_disk,sing_lba,lba_buf,1);
      bytes_written += 140*BLOCK_SIZE - used_size;
      uint32_t double_lba= get_lba();
      file->fd_inode->i_sectors[13] = double_lba;
      memset(lba_buf,0,BLOCK_SIZE);
      for (int32_t i = 0;i<(int32_t)new_index.sing;i++)
      {

        uint32_t lba = get_lba();
        lba_buf[i] = lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,0,0,128,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = get_lba();
        lba_buf[new_index.sing] = lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,new_size,0,new_index.direct,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
 
      }
      ide_write(cur_part->my_disk,double_lba,lba_buf,1);

    }
  } else 
  {
    uint32_t double_lba = file->fd_inode->i_sectors[13];
    memset(lba_buf,0,BLOCK_SIZE);
    ide_read(cur_part->my_disk,double_lba,lba_buf,1);
    if (used_index.sing == new_index.sing)
    {
      uint32_t lba = lba_buf[used_index.sing];
      memset(double_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,lba,double_buf,1);
      buf_write(double_buf,used_size,new_size,used_index.direct,new_index.direct,buf_dst,io_buf);
      ide_write(cur_part->my_disk,lba,double_buf,1);
    } else 
    {
      uint32_t bytes_written = 0;
      if (used_index.direct != 0)
      {
        uint32_t lba  = lba_buf[used_index.sing];
        memset(double_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,double_buf,1);
        buf_write(double_buf,used_size,0,used_index.direct,128,buf_dst,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
        bytes_written += (used_index.sing + 1)*128*BLOCK_SIZE + 140*BLOCK_SIZE - used_size;
      }
      for (uint32_t i = used_index.sing+1;i<new_index.sing;i++)
      {
        uint32_t lba = get_lba();
        lba_buf[i] = lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,0,0,128,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = get_lba();
        lba_buf[new_index.sing]= lba;
        memset(double_buf,0,BLOCK_SIZE);
        buf_write(double_buf,0,new_size,0,new_index.direct,buf_dst+bytes_written,io_buf);
        ide_write(cur_part->my_disk,lba,double_buf,1);
      }
      ide_write(cur_part->my_disk,double_lba,lba_buf,1);
    }
  }

  file->fd_inode->i_size += count;
  inode_sync(cur_part, file->fd_inode, io_buf);
  sys_free(double_buf);
  sys_free(lba_buf);
  return count;
}

/* 把buf中的count个字节写入file,成功则返回写入的字节数,失败则返回-1 */
/*
int32_t file_write(struct file *file, const void *buf, uint32_t count)
{
    if ((file->fd_inode->i_size + count) > (BLOCK_SIZE * (12 + 128 + 128 * 128)))
    { // 文件目前最大只支持512*140=71680字节
        printk("exceed max file_size 71680 bytes, write file failed\n");
        return -1;
    }
    uint8_t *io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL)
    {
        printk("file_write: sys_malloc for io_buf failed\n");
        return -1;
    }
    uint32_t *all_blocks = (uint32_t *)sys_malloc(BLOCK_SIZE + 48); // 用来记录文件所有的块地址
    if (all_blocks == NULL)
    {
        printk("file_write: sys_malloc for all_blocks failed\n");
        return -1;
    }

    const uint8_t *src = buf;      // 用src指向buf中待写入的数据
    uint32_t bytes_written = 0;    // 用来记录已写入数据大小
    uint32_t size_left = count;    // 用来记录未写入数据大小
    int32_t block_lba = -1;        // 块地址
    uint32_t block_bitmap_idx = 0; // 用来记录block对应于block_bitmap中的索引,做为参数传给bitmap_sync
    uint32_t sec_idx;              // 用来索引扇区
    uint32_t sec_lba;              // 扇区地址
    uint32_t sec_off_bytes;        // 扇区内字节偏移量
    uint32_t sec_left_bytes;       // 扇区内剩余字节量
    uint32_t chunk_size;           // 每次写入硬盘的数据块大小
    int32_t indirect_block_table;  // 用来获取一级间接表地址
    uint32_t block_idx;            // 块索引

    if (file->fd_inode->i_sectors[0] == 0)
    {
        block_lba = block_bitmap_alloc(cur_part);
        if (block_lba == -1)
        {
            printk("file_write: block_bitmap_alloc failed\n");
            return -1;
        }
        file->fd_inode->i_sectors[0] = block_lba;

        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx != 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }

    uint32_t file_has_used_blocks = file->fd_inode->i_size / BLOCK_SIZE + 1;
    // 其实这两行应该用DIV_ROUND_UP进行计算，但是改了之后，后面会涉及大量小细节修改。反正这个能跑，错就错吧

    uint32_t file_will_use_blocks = (file->fd_inode->i_size + count) / BLOCK_SIZE + 1;

    ASSERT(file_will_use_blocks <= 140);

    uint32_t add_blocks = file_will_use_blocks - file_has_used_blocks;

    if (add_blocks == 0)
    {
        if (file_has_used_blocks <= 12)
        {                                         // 文件数据量将在12块之内
            block_idx = file_has_used_blocks - 1; // 指向最后一个已有数据的扇区
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }
        else
        {
            ASSERT(file->fd_inode->i_sectors[12] != 0);
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
        }
    }
    else
    {
        if (file_will_use_blocks <= 12)
        {
            block_idx = file_has_used_blocks - 1;
            ASSERT(file->fd_inode->i_sectors[block_idx] != 0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            block_idx = file_has_used_blocks; // 指向第一个要分配的新扇区
            while (block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1)
                {
                    printk("file_write: block_bitmap_alloc for situation 1 failed\n");
                    return -1;
                }

                ASSERT(file->fd_inode->i_sectors[block_idx] == 0); // 确保尚未分配扇区地址
                file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;

                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

                block_idx++; // 下一个分配的新扇区
            }
        }
        else if (file_has_used_blocks <= 12 && file_will_use_blocks > 12)
        {

            block_idx = file_has_used_blocks - 1; // 指向旧数据所在的最后一个扇区
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            block_lba = block_bitmap_alloc(cur_part);
            if (block_lba == -1)
            {
                printk("file_write: block_bitmap_alloc for situation 2 failed\n");
                return -1;
            }

            ASSERT(file->fd_inode->i_sectors[12] == 0); // 确保一级间接块表未分配
            indirect_block_table = file->fd_inode->i_sectors[12] = block_lba;

            block_idx = file_has_used_blocks; // 第一个未使用的块,即本文件最后一个已经使用的直接块的下一块
            while (block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1)
                {
                    printk("file_write: block_bitmap_alloc for situation 2 failed\n");
                    return -1;
                }

                if (block_idx < 12)
                {                                                      // 新创建的0~11块直接存入all_blocks数组
                    ASSERT(file->fd_inode->i_sectors[block_idx] == 0); // 确保尚未分配扇区地址
                    file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
                }
                else
                { // 间接块只写入到all_block数组中,待全部分配完成后一次性同步到硬盘
                    all_blocks[block_idx] = block_lba;
                }

                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

                block_idx++; // 下一个新扇区
            }
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); // 同步一级间接块表到硬盘
        }
        else if (file_has_used_blocks > 12)
        {
            ASSERT(file->fd_inode->i_sectors[12] != 0);           // 已经具备了一级间接块表
            indirect_block_table = file->fd_inode->i_sectors[12]; // 获取一级间接表地址

            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); // 获取所有间接块地址

            block_idx = file_has_used_blocks; // 第一个未使用的间接块,即已经使用的间接块的下一块
            while (block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1)
                {
                    printk("file_write: block_bitmap_alloc for situation 3 failed\n");
                    return -1;
                }
                all_blocks[block_idx++] = block_lba;

                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
            }
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); // 同步一级间接块表到硬盘
        }
    }

    bool first_write_block = true; // 含有剩余空间的扇区标识
    file->fd_pos = file->fd_inode->i_size - 1; // 置fd_pos为文件大小-1,下面在写数据时随时更新
    while (bytes_written < count)
    { // 直到写完所有数据
        memset(io_buf, 0, BLOCK_SIZE);
        sec_idx = file->fd_inode->i_size / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
        if (first_write_block)
        {
            ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
            first_write_block = false;
        }
        memcpy(io_buf + sec_off_bytes, src, chunk_size);
        ide_write(cur_part->my_disk, sec_lba, io_buf, 1);
        printk("file write at lba 0x%x\n", sec_lba); // 调试,完成后去掉

        src += chunk_size;                    // 将指针推移到下个新数据
        file->fd_inode->i_size += chunk_size; // 更新文件大小
        file->fd_pos += chunk_size;
        bytes_written += chunk_size;
        size_left -= chunk_size;
    }
    inode_sync(cur_part, file->fd_inode, io_buf);
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_written;
}
*/
/*
int32_t file_read(struct file *file, void *buf, uint32_t count)
{
    uint8_t *buf_dst = (uint8_t *)buf;
    uint32_t size = count, size_left = size;

    if ((file->fd_pos + count) > file->fd_inode->i_size)
    {
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if (size == 0)
        { // 若到文件尾则返回-1
            return -1;
        }
    }

    uint8_t *io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL)
    {
        printk("file_read: sys_malloc for io_buf failed\n");
    }
    uint32_t *all_blocks = (uint32_t *)sys_malloc(BLOCK_SIZE + 48); // 用来记录文件所有的块地址
    if (all_blocks == NULL)
    {
        printk("file_read: sys_malloc for all_blocks failed\n");
        return -1;
    }

    uint32_t block_read_start_idx = file->fd_pos / BLOCK_SIZE;        // 数据所在块的起始地址
    uint32_t block_read_end_idx = (file->fd_pos + size) / BLOCK_SIZE; // 数据所在块的终止地址
    uint32_t read_blocks = block_read_start_idx - block_read_end_idx; // 如增量为0,表示数据在同一扇区
    ASSERT(block_read_start_idx < 139 && block_read_end_idx < 139);   // 应该改为140

    int32_t indirect_block_table; // 用来获取一级间接表地址
    uint32_t block_idx;           // 获取待读的块地址

    if (read_blocks == 0)
    { // 在同一扇区内读数据,不涉及到跨扇区读取
        ASSERT(block_read_end_idx == block_read_start_idx);
        if (block_read_end_idx < 12)
        { // 待读的数据在12个直接块之内
            block_idx = block_read_end_idx;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }
        else
        { // 若用到了一级间接块表,需要将表中间接块读进来
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
        }
    }
    else
    { // 若要读多个块
        if (block_read_end_idx < 12)
        { // 数据结束所在的块属于直接块
            block_idx = block_read_start_idx;
            while (block_idx <= block_read_end_idx)
            {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
        }
        else if (block_read_start_idx < 12 && block_read_end_idx >= 12)
        {
            block_idx = block_read_start_idx;
            while (block_idx < 12)
            {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
            ASSERT(file->fd_inode->i_sectors[12] != 0); // 确保已经分配了一级间接块表

            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); // 将一级间接块表读进来写入到第13个块的位置之后
        }
        else
        {
            ASSERT(file->fd_inode->i_sectors[12] != 0);                            // 确保已经分配了一级间接块表
            indirect_block_table = file->fd_inode->i_sectors[12];                  // 获取一级间接表地址
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); // 将一级间接块表读进来写入到第13个块的位置之后
        }
    }

    uint32_t sec_idx, sec_lba, sec_off_bytes, sec_left_bytes, chunk_size;
    uint32_t bytes_read = 0;
    while (bytes_read < size)
    { // 直到读完为止
        sec_idx = file->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_pos % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes; // 待读入的数据大小

        memset(io_buf, 0, BLOCK_SIZE);
        ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
        memcpy(buf_dst, io_buf + sec_off_bytes, chunk_size);

        buf_dst += chunk_size;
        file->fd_pos += chunk_size;
        bytes_read += chunk_size;
        size_left -= chunk_size;
    }
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_read;
}*/
static void buf_read(uint32_t* i_sectors,uint8_t* buf_dst,uint32_t used_index,uint32_t new_index,uint32_t used_size,uint32_t new_size,void* io_buf,uint32_t size_left)
{
      uint32_t block_offset = used_size % BLOCK_SIZE;
  
      uint32_t bytes_written = 0;
      if (block_offset)
      {
        if (used_index== new_index)
        {
          uint32_t block_lba = i_sectors[used_index- 1];
          ide_read(cur_part->my_disk,block_lba,io_buf,1);
          memcpy(buf_dst,io_buf + block_offset,size_left);
      return;
        } else
        {
          uint32_t block_lba = i_sectors[used_index -1];
          ide_read(cur_part->my_disk,block_lba,io_buf,1);
          memcpy(buf_dst,io_buf + block_offset,BLOCK_SIZE - block_offset);
          bytes_written += BLOCK_SIZE - block_offset;
       }
      }
      uint32_t end_offset= new_size % BLOCK_SIZE;
      for (uint32_t i = used_index; i < new_index-1; i++)
      {
        uint32_t lba = i_sectors[i];
        ide_read(cur_part->my_disk,lba,io_buf,1);
        memcpy(buf_dst + bytes_written,io_buf,BLOCK_SIZE);
        bytes_written += BLOCK_SIZE;
      }
      uint32_t left_size = end_offset ? end_offset : BLOCK_SIZE;
      uint32_t lba = i_sectors[new_index-1];
      ide_read(cur_part->my_disk,lba,io_buf,1);
      memcpy(buf_dst + bytes_written,io_buf,left_size);
      bytes_written += left_size;
}
int32_t file_read(struct file *file, void *buf, uint32_t count)
{
    uint8_t *buf_dst = (uint8_t *)buf;
    uint32_t size = count, size_left = size;

    /* 若要读取的字节数超过了文件可读的剩余量, 就用剩余量做为待读取的字节数 */
    if ((file->fd_pos + count) > file->fd_inode->i_size)
    {
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if (size == 0)
        { // 若到文件尾则返回-1
            return -1;
        }
    }

    uint8_t *io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL)
    {
        printk("file_read: sys_malloc for io_buf failed\n");
    }

  uint32_t* lba_buf = sys_malloc(BLOCK_SIZE);
  uint32_t* double_buf = sys_malloc(BLOCK_SIZE);
  uint32_t used_size = file->fd_pos;
  uint32_t new_size = file->fd_pos + size_left;
  struct index used_index = get_index(used_size);
  struct index new_index = get_index(new_size);
  if ((used_index.dou == 0 && used_index.direct != 0) || used_size == 0)
  {
    if (new_index.dou == 0 && new_index.direct != 0){
      buf_read(file->fd_inode->i_sectors,buf_dst,used_index.direct,new_index.direct,used_size,new_size,io_buf,size_left);
    } else if (new_index.dou == 0 && new_index.sing != 0)
    {
      uint32_t bytes_written = 0;
      buf_read(file->fd_inode->i_sectors,buf_dst,used_index.direct,12,used_size,12*BLOCK_SIZE,io_buf,12*BLOCK_SIZE - used_size);
      bytes_written += 12*BLOCK_SIZE - used_size;
      uint32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
      buf_read(lba_buf,buf_dst + bytes_written,0,new_index.sing,12*BLOCK_SIZE,new_size,io_buf,new_size - bytes_written);
    } else 
    {
      uint32_t bytes_written = 0;
      buf_read(file->fd_inode->i_sectors,buf_dst,used_index.direct,12,used_size,12*BLOCK_SIZE,io_buf,12*BLOCK_SIZE - used_size);
      bytes_written += 12*BLOCK_SIZE - used_size;
      uint32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
 
      buf_read(lba_buf,buf_dst + bytes_written,0,128,12*BLOCK_SIZE,140*BLOCK_SIZE,io_buf,128*BLOCK_SIZE);

      bytes_written += 128*BLOCK_SIZE;
      uint32_t double_lba = file->fd_inode->i_sectors[13];
      memset(double_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,double_lba,double_buf,1);
      for (uint32_t i = 0; i < new_index.sing; i++){
        uint32_t lba = double_buf[i];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,128,0,0,io_buf,128*BLOCK_SIZE);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = double_buf[new_index.sing];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,new_index.direct,0,new_size,io_buf,new_size - bytes_written);
      }

    }
  } else if (used_index.dou == 0 && used_index.sing != 0)
  {
    if (new_index.dou == 0 && new_index.sing != 0)
    {
      int32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
      buf_read(lba_buf,buf_dst,used_index.sing,new_index.sing,used_size,new_size,io_buf,size_left);
    } else
    {
      uint32_t bytes_written = 0;
      uint32_t sing_lba = file->fd_inode->i_sectors[12];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,sing_lba,lba_buf,1);
      buf_read(lba_buf,buf_dst,used_index.sing,128,used_size,140*BLOCK_SIZE,io_buf,140*BLOCK_SIZE - used_size);
      bytes_written += 140*BLOCK_SIZE - used_size;
      uint32_t double_lba = file->fd_inode->i_sectors[13];
      memset(double_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,double_lba,double_buf,1);
      for (uint32_t i = 0; i < new_index.sing; i++){
        uint32_t lba = double_buf[i];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,128,0,0,io_buf,128*BLOCK_SIZE);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = double_buf[new_index.sing];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,new_index.direct,0,new_size,io_buf,new_size - bytes_written);
      }
     
    }
  } else
  {
    uint32_t double_lba = file->fd_inode->i_sectors[13];
    memset(double_buf,0,BLOCK_SIZE);
    ide_read(cur_part->my_disk,double_lba,double_buf,1);
    if (used_index.sing == new_index.sing)
    {
      uint32_t lba = double_buf[used_index.sing];
      memset(lba_buf,0,BLOCK_SIZE);
      ide_read(cur_part->my_disk,lba,lba_buf,1);
      buf_read(lba_buf,buf_dst,used_index.direct,new_index.direct,used_size,new_size,io_buf,size_left);
    } else
    {
      uint32_t bytes_written =0;
      if (used_index.direct != 0)
      {
        uint32_t lba = double_buf[used_index.sing];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst,used_index.direct,128,used_size,0,io_buf,140*BLOCK_SIZE + (used_index.sing +1)*128*BLOCK_SIZE - used_size);
        bytes_written += 140*BLOCK_SIZE + (used_index.sing +1)*128*BLOCK_SIZE - used_size;
      }
     for (uint32_t i = used_index.sing +1; i < new_index.sing ; i++)
      {
        uint32_t lba = double_buf[i];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,128,0,0,io_buf,128*BLOCK_SIZE);
        bytes_written += 128*BLOCK_SIZE;
      }
      if (new_index.direct != 0)
      {
        uint32_t lba = double_buf[new_index.sing];
        memset(lba_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,lba,lba_buf,1);
        buf_read(lba_buf,buf_dst + bytes_written,0,new_index.direct,0,new_size,io_buf,new_size - bytes_written);
      }
    }
  }
  sys_free(io_buf);
  sys_free(double_buf);
  sys_free(lba_buf);
  return size_left;
}

