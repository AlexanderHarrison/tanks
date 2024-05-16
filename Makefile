.PHONY: hot_reload build run san debug

OUT := game
FILES := /usr/local/lib/tools.o game.c
TRACK := game.c game.h controls.h
BASE_FLAGS := -fuse-ld=mold -std=gnu2x -O2

WARN_FLAGS := -Wall -Wextra -Wpedantic -Wuninitialized -Wcast-qual -Wdisabled-optimization -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wswitch-default -Wundef -Wstrict-prototypes -Wpointer-to-int-cast -Wint-to-pointer-cast -Wduplicated-cond -Wduplicated-branches -Wformat=2 -Wshift-overflow=2 -Wint-in-bool-context -Wlong-long -Wvector-operation-performance -Wvla -Wdisabled-optimization -Wredundant-decls -Wmissing-parameter-type -Wold-style-declaration -Wlogical-not-parentheses -Waddress -Wmemset-transposed-args -Wmemset-elt-size -Wsizeof-pointer-memaccess -Wwrite-strings -Wbad-function-cast -Wtrampolines -Werror=implicit-function-declaration -Wno-incompatible-pointer-types

LINK_FLAGS := -lraylib -lm -ldl

export GCC_COLORS = warning=01;33

build: $(TRACK)
	gcc $(WARN_FLAGS) $(BASE_FLAGS) $(FILES) $(LINK_FLAGS) -o$(OUT)

run: build
	./$(OUT)

san: $(TRACK)
	gcc $(WARN_FLAGS) -g -fsanitize=undefined -fsanitize=address $(BASE_FLAGS) $(FILES) $(LINK_FLAGS) -o$(OUT)
	./$(OUT)

debug: $(TRACK)
	gcc $(WARN_FLAGS) -ggdb $(BASE_FLAGS) $(FILES) $(LINK_FLAGS)
	gdb ./$(OUT)

hot_reload: build hot_reload.c
	./$(OUT) & watch gcc $(WARN_FLAGS) $(BASE_FLAGS) -fPIC -shared hot_reload.c $(LINK_FLAGS) -o libhot_reload.so

