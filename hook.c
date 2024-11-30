#include <stddef.h>
// that's what the baddest do!
#include "libc.c"
#include "global_obj.h"

#define SRAM_BASE   0x62000000
#define MAX_OUT_LEN 0x40

enum {
    kAESKeyUser = 0,    // probably
    kAESKeyGID = 1,
    kAESKeyUID = 2
};

static void (*usb_shutdown)() = (void *)0x20008DE0;
static void (*reboot)() __attribute__((noreturn)) = (void *)0x2000834C;
static void (*aes_encrypt)(void *buf, size_t len, int key, void *user_key, void *user_iv) = (void *)0x20000C80;
static void (*aes_decrypt)(void *buf, size_t len, int key, void *user_key, void *user_iv) = (void *)0x20000BA4;
static void (*prepare_and_jump)(uintptr_t addr) __attribute__((noreturn)) = (void *)0x200077EC;

typedef enum {
    kHookCommandReset = 'rest',
    kHookCommandDump = 'dump',
    kHookCommandAESEncrypt = 'aese',
    kHookCommandAESDecrypt = 'aesd',
} hook_cmd_t;

struct cmd {
    hook_cmd_t cmd;
    uint32_t args[];
};

#define RESET_VECTOR    0xEA00000A

int hook(void **state) {
    struct cmd *cmd = *state;

    switch (cmd->cmd) {
        case kHookCommandReset: {
            usb_shutdown();
            reboot();
        }

        case kHookCommandDump: {
            void *addr = (void *)cmd->args[0];
            size_t len = cmd->args[1];

            if (len > MAX_OUT_LEN) {
                return -1;
            }

            memcpy(*state, addr, len);
            break;
        }

        case kHookCommandAESEncrypt:
        case kHookCommandAESDecrypt: {
            size_t len = cmd->args[0];
            int key = cmd->args[1];
            void *iv = &cmd->args[2];
            void *buf = &cmd->args[6];

            if (len > MAX_OUT_LEN) {
                return -1;
            }

            if (cmd->cmd == kHookCommandAESDecrypt) {
                aes_decrypt(buf, len, key, NULL, iv);
            } else {
                aes_encrypt(buf, len, key, NULL, iv);
            }

            memcpy(*state, buf, len);
            break;
        }

        default: {
            if (*(uint32_t *)SRAM_BASE == RESET_VECTOR) {
                usb_shutdown();
                prepare_and_jump(SRAM_BASE);
            }
        }
    }

    return 0;
}
