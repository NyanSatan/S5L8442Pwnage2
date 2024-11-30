#include <stddef.h>
#include <stdint.h>
// that's what the baddest do!
#include "libc.c"
#include "hook.h"
#include "global_obj.h"

#define ROM_BASE    0x20000000
#define ROM_SIZE    0x10000

#define SRAM_BASE   0x62000000
#define SRAM_SIZE   0x80000

__attribute__((noreturn)) void hang(void) {
    while (1) {}
}

#define STACK_SKIP      0x560
#define LR_ON_STACK     0x200076E4
#define NEW_LR_STACK    0x200076B8

int fix_usb_lr() {
    register uintptr_t sp asm("sp");
    static const uintptr_t lr = LR_ON_STACK;

    volatile void **lr_pos = memmem((void *)sp + STACK_SKIP, 0x1000, &lr, sizeof(lr));
    if (!lr_pos) {
        return -1;
    }

    *lr_pos = (void *)NEW_LR_STACK;
    
    *(volatile uint8_t *)(GLOBAL_OBJ_ADDR + 5) = 0;

    return 0;
}

static void create_new_string_desc(void *dst, size_t dst_len, const char *str, size_t str_len) {
    volatile uint8_t *ptr = dst;

    if (str_len > dst_len) {
        str_len = dst_len;
    }

    *ptr++ = str_len * 2 + 2;
    *ptr++ = 0x3;  // type

    int str_cnt = 0;

    for (int i = 0; i < str_len * 2; i += 2) {
        *ptr++ = str[str_cnt];
        *ptr++ = 0x0;
        str_cnt++;
    }
}

#define MAX_NAME_LEN    14
#define NEW_USB_NAME    "S5L8442 pwnDFU"

#define SRNM_DESC       0x2000B424
#define SRNM_DESC_LEN   34

void usb_desc_fix() {
    void *new_name_desc = (void *)GLOBAL_OBJ_ADDR + 331;
    create_new_string_desc(new_name_desc, MAX_NAME_LEN, NEW_USB_NAME, sizeof(NEW_USB_NAME) - 1);

    void *new_srnm_desc = (void *)GLOBAL_OBJ_ADDR + 512;
    memcpy(new_srnm_desc, (void *)SRNM_DESC, SRNM_DESC_LEN);
    *(volatile void **)(GLOBAL_OBJ_ADDR + 404) = new_srnm_desc;
}

void hook_init() {
    register uintptr_t sp asm("sp");
    void *new_hook = (void *)sp - 0x1000;
    memcpy(new_hook, hook, sizeof(hook));
    *(volatile void **)(GLOBAL_OBJ_ADDR + 36) = new_hook;
}
