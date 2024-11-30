#ifndef GLOBAL_OBJ_H
#define GLOBAL_OBJ_H

#include <stdint.h>

#define GLOBAL_OBJ_PTR  0x6207FFF8

#define GLOBAL_OBJ_ADDR     (*((volatile uintptr_t *)GLOBAL_OBJ_PTR))
#define GLOBAL_OBJ_USB_DESC (*(volatile uint32_t **)(GLOBAL_OBJ_ADDR + 0x58C))

#endif
