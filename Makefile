.PHONY: all clean distclean

GNUARM_PREFIX ?= /opt/arm-gnu-toolchain-13.3.rel1-darwin-arm64-arm-none-eabi/bin/arm-none-eabi-

CC = $(GNUARM_PREFIX)gcc
CFLAGS = -Wall
CFLAGS += -Wno-long-long
CFLAGS += -Wno-multichar
CFLAGS += -Os
CFLAGS += -march=armv4
CFLAGS += -fcall-used-r9

LD = $(GNUARM_PREFIX)gcc
LDFLAGS = -nostdlib
PAYLOAD_LDFLAGS = -Tpayload.ld -Ttext=0x62000800
HOOK_LDFLAGS = -Thook.ld -fpie

OBJCOPY = $(GNUARM_PREFIX)objcopy

PAYLOAD_SOURCES = \
	payload.c

PAYLOAD_OBJECTS = $(PAYLOAD_SOURCES:.c=.o)

PAYLOAD_ELF = payload.elf
PAYLOAD_BIN = payload.bin

HOOK_SOURCES = \
	hook.c

HOOK_OBJECTS = $(HOOK_SOURCES:.c=.o)

HOOK_ELF = hook.elf
HOOK_BIN = hook.bin
HOOK_HEADER = hook.h

TEMPLATE_DFU = template.dfu
PWN_DFU = pwn.dfu

all: $(HOOK_HEADER) $(PWN_DFU)

$(PWN_DFU): $(PAYLOAD_BIN) | $(HOOK_HEADER) 
	cp -a $(TEMPLATE_DFU) $@
	dd if=$< of=$@ bs=1 conv=notrunc seek=$$((0x800))

$(PAYLOAD_BIN): $(PAYLOAD_ELF)
	$(OBJCOPY) -O binary $< $@

$(PAYLOAD_ELF): $(PAYLOAD_OBJECTS) | entry.o
	$(LD) -o $@ $(LDFLAGS) $(PAYLOAD_LDFLAGS) $^

$(PAYLOAD_OBJECTS): $(HOOK_HEADER)

$(HOOK_HEADER): $(HOOK_BIN)
	xxd -n hook -i $< $@

$(HOOK_BIN): $(HOOK_ELF)
	$(OBJCOPY) -O binary $< $@

$(HOOK_ELF): $(HOOK_OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $(HOOK_LDFLAGS) $^

.S.o:
	$(CC) -o $@ $(CFLAGS) -c $<

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	-$(RM) *.o *.elf *.a $(HOOK_HEADER)

distclean: clean
	-$(RM) *.bin
