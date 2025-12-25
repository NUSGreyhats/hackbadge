TARGET:=breathing_led

# Specify the full MCU name
TARGET_MCU:=CH32V003

# Set both variables to the same path
CH32V003FUN:=../../env/rv003usb/ch32v003fun/ch32fun
CH32FUN:=../../env/rv003usb/ch32v003fun/ch32fun
MINICHLINK:=../../env/rv003usb/ch32v003fun/minichlink/minichlink

# System clock speed
FUNCONF_SYSTEM_CORE_CLOCK:=24000000

# Source files
SRCS:=main.c

# Include the ch32v003fun build system
include $(CH32V003FUN)/ch32fun.mk

flash: $(TARGET).bin
	sudo $(MINICHLINK) -w $(TARGET).bin 0x08000000 -c 0x1209b003 -b
	
# Flash using WCH-Link (removes bootloader)
# flash: $(TARGET).bin
#	sudo minichlink -E
#	sudo minichlink -a -w $(TARGET).bin 0x08000000 -B

clean:
	rm -rf *.o *.elf *.bin *.hex *.lst *.map