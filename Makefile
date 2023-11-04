.PHONY: run build san debug

BASE_FLAGS := -O2 -Wall -Wextra -Wpedantic -Wuninitialized -fno-strict-aliasing -ogame -lraylib -lm -luv 
FILES := net.c game.c

build:
	gcc $(FILES) $(BASE_FLAGS)

run: build
	./game

san:
	gcc $(FILES) $(BASE_FLAGS) -g -fsanitize=undefined -fsanitize=address
	./game

debug:
	gcc $(FILES) $(BASE_FLAGS) -ggdb 	
	gdb ./game

