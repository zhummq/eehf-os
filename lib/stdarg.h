#ifndef _STDARG_H_
#define _STDARG_H_
typedef char *va_list;
#define va_start(ap, v) ap = (va_list)((char *)&v + sizeof(v));
#define va_arg(ap, t) (*(t *)((ap += sizeof(t)) - sizeof(t)))
#define va_end(ap) ap = NULL // 清除ap

#endif
