# (c) 2016-01-20 Jens Hauke <jens@4k2.de>

all:

MCU_TARGET=attiny13a

PROGS += pov.elf
pov.elf: pov.o

pov.o: banner1.inc


include ./make.rules


%.inc: %.txt banner_txt2c.awk
	./banner_txt2c.awk $< > $@


#PROGRAMMER=usbasp
#PROGRAMMER=stk500hvsp
#PROGRAMMER_OPTIONS=-b57600 -P/dev/ttyUSB0

