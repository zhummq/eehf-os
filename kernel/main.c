#include "assert.h"
#include "console.h"
#include "font.h"
#include "fork.h"
#include "ide.h"
#include "init.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "shell.h"
#include "stdio-kernel.h"
#include "stdio.h"
#include "syscall.h"

void init(void);

int main(void) {
  put_str("I am kernel\n");
  init_all();
  printk("init_all!");

  int32_t fd = sys_open("/font", O_CREAT | O_RDWR);
  if (fd != -1) {
    uint32_t file_size = 29728;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk *sda = &ide_channels[0].devices[0];
    void *prog_buf = sys_malloc(sec_cnt * 512);
    ide_read(sda, 500, prog_buf, sec_cnt);

    printk("open success!");
    if (sys_write(fd, prog_buf, file_size) == -1) {
      printk("file write error!\n");
      while (1)
        ;
    }
    sys_free(prog_buf);
  }
  int32_t fd_doom = sys_open("/doom", O_CREAT | O_RDWR);
  if (fd_doom != -1) {
    uint32_t file_size = 1502780;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk *sda = &ide_channels[0].devices[0];
    void *prog_buf = sys_malloc(sec_cnt * 512);

    ide_read(sda, 800, prog_buf, sec_cnt);

    printk("open success!");
    if (sys_write(fd_doom, prog_buf, file_size) == -1) {
      printk("file write error!\n");
      while (1)
        ;
    }
    sys_free(prog_buf);
  }
  int32_t fd_wad = sys_open("/doom1.wad", O_CREAT | O_RDWR);
  if (fd_doom != -1) {
    uint32_t file_size = 4196020;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk *sda = &ide_channels[0].devices[0];
    void *prog_buf = sys_malloc(sec_cnt * 512);
    ide_read(sda, 5000, prog_buf, sec_cnt);

    printk("open success!");
    if (sys_write(fd_wad, prog_buf, file_size) == -1) {
      printk("file write error!\n");
      while (1)
        ;
    }
    sys_free(prog_buf);
  }

  font_init();
  // cls_screen();
  //  char name[9];
  // for (int i = 10; i < 64; i++) {
  //   snprintf(name, 9, "STCFN%s", "123");
  // }

  process_execute(init,
                  "init"); // 放在第一个初始化,这是第一个进程,init进程的pid为1
  thread_exit(running_thread(), TRUE);
  return 0;
}

/* init进程 */
void init(void) {
  uint32_t ret_pid = fork();
  if (ret_pid) { // 父进程
    int status;
    int child_pid;
    /* init在此处不停的回收僵尸进程 */
    while (1) {
      child_pid = wait(&status);
      printf("I`m init, My pid is 1, I recieve a child, It`s pid is %d, status "
             "is %d\n",
             child_pid, status);
    }
  } else { // 子进程
    my_shell();
  }
  panic("init: should not be here");
}
