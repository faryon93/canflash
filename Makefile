OBJECTS = main.o lpc.o can.o sdo.o firmware.o progressbar.o util/io.o

all: $(OBJECTS)
	gcc --std=gnu99 -lm $(OBJECTS) -o uflash

%.o: %.c
	gcc --std=gnu99 -c $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f uflash
