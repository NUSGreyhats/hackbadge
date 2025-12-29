BUILD_DIR:=build
TARGET:=main

# Specify the full MCU name
TARGET_MCU:=CH32V003

# Set both variables to the same path
CH32V003FUN:=../env/rv003usb/ch32v003fun/ch32fun
CH32FUN:=../env/rv003usb/ch32v003fun/ch32fun
MINICHLINK:=../env/rv003usb/ch32v003fun/minichlink/minichlink

# System clock speed
FUNCONF_SYSTEM_CORE_CLOCK:=24000000

# Additional compiler flags
EXTRA_CFLAGS:=-Wextra -Wno-unused-variable

# Source files
SRCS:=main.c wch/ch32v00x_exti.c

# Default target - defined before including ch32fun.mk to override its default
.DEFAULT_GOAL := all

# Include the ch32v003fun build system
include $(CH32V003FUN)/ch32fun.mk

# Override default build target to move artifacts to build directory
all: $(TARGET).bin
	@mkdir -p $(BUILD_DIR)
	@mv -f $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(BUILD_DIR)/ 2>/dev/null || true
	@echo "Build artifacts moved to $(BUILD_DIR)/"

flash: all
	sudo $(MINICHLINK) -w $(BUILD_DIR)/$(TARGET).bin 0x08000000 -c 0x1209b003 -b
	
# Flash using WCH-Link (removes bootloader)
# flash: $(TARGET).bin
#	sudo minichlink -E
#	sudo minichlink -a -w $(TARGET).bin 0x08000000 -B

clean:
	rm -rf *.o *.elf *.bin *.hex *.lst *.map $(BUILD_DIR)
