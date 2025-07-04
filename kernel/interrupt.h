#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
typedef void* intr_handler;		//将intr_handler定义为void*同类型
void idt_init(void);

/* 定义中断的两种状态:
 * INTR_OFF值为0,表示关中断,
 * INTR_ON值为1,表示开中断 */
enum intr_status {		 // 中断状态
    INTR_OFF,			 // 中断关闭
    INTR_ON		         // 中断打开
};
void send_eoi(uint8_t irq);
enum intr_status intr_get_status(void);
enum intr_status intr_set_status (enum intr_status);
enum intr_status intr_enable (void);
enum intr_status intr_disable (void);
void register_handler(uint8_t vector_no, intr_handler function);
#endif
