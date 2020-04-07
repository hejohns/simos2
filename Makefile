# shamelessly robbed basically everything from arduino ide
CC := avr-gcc
mmcu := atmega2560
F_CPU := 16000000UL
ARDUINO := 105
USB_PID := null
CFLAGS := -g -Os -Wall -Wextra -fno-exceptions -ffunction-sections -fdata-sections -mmcu=$(mmcu) -DF_CPU=$(F_CPU) -MMD -DUSB_VID=$(USB_PID) -DUSB_PID=$(USB_PID) -DARDUINO=$(ARDUINO) -D__PROG_TYPES_COMPAT__

LDFLAGS := --gc-sections,--relax

UPLOAD_BAUD := 115200
PROGRAMMER_ID := stk500v2
PORT := /dev/ttyACM0
AVRDUDE_CONF := avrdude/avrdude.conf
AVRDUDE := avrdude

AVROBJCOPY := avr-objcopy

export CPATH := .:

.PHONY: test upload clean

default:
	make main.elf

test: 

usart.o: AVR-UART-lib/usart.c AVR-UART-lib/usart.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: main.o usart.o kernel.o
	$(CC) $(CFLAGS) -Wl,$(LDFLAGS) $^ -o $@
main.hex: main.elf
#	$(AVROBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< main.eep
	$(AVROBJCOPY) -O ihex -R .eeprom $< $@

upload: main.hex
	$(AVRDUDE) -C$(AVRDUDE_CONF) -v -v -v -v -p$(mmcu) -c$(PROGRAMMER_ID) -P$(PORT) -b$(UPLOAD_BAUD) -D -Uflash:w:$<:i

clean: 
	rm -r *.o *.d *.elf *.hex
