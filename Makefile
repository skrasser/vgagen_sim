.PHONY: clean

CC := gcc
CFLAGS += $(shell sdl2-config --cflags) -std=c99 -Wall -Werror
LFLAGS += $(shell sdl2-config --libs)

OBJ := gfxsim.o

gfxsim: $(OBJ)
	$(CC) -o $@ $(LFLAGS) $^

clean:
	rm -f $(OBJ) gfxsim
