#include "init.h"
#include "bga.h"
#include "console.h"
#include "e1000.h"
#include "font.h"
#include "fs.h"
#include "ide.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "net.h"
#include "pci.h"
#include "print-bga.h"
#include "print.h"
#include "psf2.h"
#include "syscall-init.h"
#include "thread.h"
#include "timer.h"
#include "tss.h"

/*负责初始化所有模块 */
void init_all() {
  put_str("init_all\n");

  idt_init(); // 初始化中断
  mem_init(); // 初始化内存管理系统

  thread_init(); // 初始化线程相关结构

  timer_init();    // 初始化PIT
  console_init();  // 控制台初始化最好放在开中断之前
  keyboard_init(); // 键盘初始化
  tss_init();      // tss初始化
  syscall_init();  // 初始化系统调用
  pci_init();      // pci_init
  e1000_init();
  net_init();
  bga_init();
  intr_enable();  // 后面的ide_init需要打开中断
  ide_init();     // 初始化硬盘
  filesys_init(); // 初始化文件系统
}
