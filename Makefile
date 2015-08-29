OBJECTS = main.c lpc.c can.c sdo.c firmware.c progressbar.c util/io.c

all: $(OBJECTS)
	gcc --std=gnu99 -lm $(OBJECTS) -o uflash

%.o: %.c
	gcc --std=gnu99 -c $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f uflash
