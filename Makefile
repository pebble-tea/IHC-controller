PORT 			= com4
TARGET 		= main
PROG_ID 	= arduino
CONFIG 		= C:\PROGRA~2\Arduino\hardware\tools\avr\etc\avrdude.conf
ARCH 			= atmega328p
BAUDRATE 	= 115200
COMPILE 	= avr-gcc -Wall -Os -mmcu=$(ARCH)

all: compile upload clean

compile:
	$(COMPILE) -c $(TARGET).c -o $(TARGET).o
	$(COMPILE) -o $(TARGET).elf $(TARGET).o
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	avr-size --format=avr --mcu=$(ARCH) $(TARGET).elf

upload:
	avrdude -v -p $(ARCH) -c $(PROG_ID) -C $(CONFIG) -P $(PORT) -b $(BAUDRATE) -U flash:w:$(TARGET).hex:i

clean:
	del $(TARGET).o $(TARGET).elf
