#ifndef __LIB_STDINT_H
#define __LIB_STDINT_H
#define MAC_LEN     6
#define IPV4_LEN    4
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef uint8_t mac_addr[MAC_LEN];
typedef uint8_t ip_addr[IPV4_LEN];
#endif
