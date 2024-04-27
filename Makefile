OUT := game
FILES := tools.o game.c
TRACK := game.c game.h controls.h
BASE_FLAGS := -fuse-ld=mold -std=gnu2x -O2 -o$(OUT)

WARN_FLAGS := -Wall -Wextra -Wpedantic -Wuninitialized -Wcast-qual -Wdisabled-optimization -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wswitch-default -Wundef -Wstrict-prototypes -Wpointer-to-int-cast -Wint-to-pointer-cast -Wconversion -Wduplicated-cond -Wduplicated-branches -Wformat=2 -Wshift-overflow=2 -Wint-in-bool-context -Wlong-long -Wvector-operation-performance -Wvla -Wdisabled-optimization -Wredundant-decls -Wmissing-parameter-type -Wold-style-declaration -Wlogical-not-parentheses -Waddress -Wmemset-transposed-args -Wmemset-elt-size -Wsizeof-pointer-memaccess -Wwrite-strings -Wbad-function-cast -Wtrampolines -Werror=implicit-function-declaration

PATH_FLAGS := -I/usr/local/lib
LINK_FLAGS := -lraylib -lm

export GCC_COLORS = warning=01;33

build: $(TRACK)
	gcc $(WARN_FLAGS) $(PATH_FLAGS) $(BASE_FLAGS) $(FILES) $(LINK_FLAGS)

run: build
	./$(OUT)

san: $(TRACK)
	gcc $(WARN_FLAGS) $(PATH_FLAGS) -g -fsanitize=undefined -fsanitize=address $(BASE_FLAGS) $(FILES) $(LINK_FLAGS)
	./$(OUT)

debug: $(TRACK)
	gcc $(WARN_FLAGS) $(PATH_FLAGS) -ggdb $(BASE_FLAGS) $(FILES) $(LINK_FLAGS)
	gdb ./$(OUT)


