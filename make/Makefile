##############################################################################
BUILD = build
BIN = 67key
BOSSA = ~/.arduino15/packages/arduino/tools/bossac/1.7.0/bossac
PORT = /dev/serial/by-id/usb-DNBDMR_67keyboard_123456-if00
BOOTPORT = /dev/serial/by-id/usb-DNBDMR_67_Keyboard_Boot_C3CA2B4750525151332E3120FF19240E-if00
##############################################################################
.PHONY: all directory clean size

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

#CFLAGS += -Wextra -Wall --std=gnu11 -DDEBUG -ggdb3 -Og
CFLAGS += -Wextra -Wall --std=gnu11 -Os
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0plus -mthumb
CFLAGS += -MD -MP -MT $(BUILD)/$(*F).o -MF $(BUILD)/$(@F).d

LDFLAGS += -mcpu=cortex-m0plus -mthumb
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--start-group -lm -Wl,--end-group
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,-Map=$(BUILD)/$(BIN).map
LDFLAGS += -Wl,--script=../linker/samd21g18.ld

INCLUDES += \
  -I../include \
  -I../tinyusb/src \
  -I..

SRCS += \
  ../main.c \
  ../startup_samd21.c \
  ../tinyusb/src/tusb.c \
  ../tinyusb/src/common/tusb_fifo.c \
  ../tinyusb/src/device/usbd.c \
  ../tinyusb/src/device/usbd_control.c \
  ../tinyusb/src/portable/microchip/samd/dcd_samd.c \
  ../tinyusb/src/class/cdc/cdc_device.c \
  ../tinyusb/src/class/hid/hid_device.c \
  ../usb_descriptors.c \
  ../led.c \
  ../spi_master.c \
  ../trackpoint.c \
  ../config.c \
  ../i2c_master.c \
  ../matrix.c \
  ../keymap.c

DEFINES += \
  -D__SAMD21E18A__ \
  -DDONT_USE_CMSIS_INIT \
  -DF_CPU=48000000

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD)/, $(notdir %/$(subst .c,.o, $(SRCS))))

all: directory $(BUILD)/$(BIN).elf $(BUILD)/$(BIN).hex $(BUILD)/$(BIN).bin size

$(BUILD)/$(BIN).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(BIN).hex: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex $^ $@

$(BUILD)/$(BIN).bin: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %/$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

uf2: $(BUILD)/$(BIN).hex
	@echo PROGRAM $^
	./force_reset.py --uf2 $(PORT)
	./uf2conv.py $^

disasm: $(BUILD)/$(BIN).elf
	@echo DISASSEMBLE $^
	arm-none-eabi-objdump -D -S $^ > $(BUILD)/$(BIN).lss

bossa: $(BUILD)/$(BIN).bin
	@echo BOSSA @^
	./force_reset.py --caterina $(PORT)
	$(BOSSA) -e -w -v -R -U true -p $(BOOTPORT) $^

directory:
	@mkdir -p $(BUILD)

size: $(BUILD)/$(BIN).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)

