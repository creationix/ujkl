CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11 -Os

default:
	$(CC) $(CFLAGS) -g main.c
	./a.out

clang:
	clang $(CFLAGS) -g main.c
	./a.out

gcc:
	gcc $(CFLAGS) -g main.c
	./a.out

musl:
	musl-gcc $(CFLAGS) -static main.c
	./a.out

memcheck:
	gcc $(CFLAGS) -g main.c
	valgrind --leak-check=full --show-leak-kinds=all ./a.out

lldb:
	clang $(CFLAGS) -g main.c
	lldb ./a.out

gdb:
	gcc $(CFLAGS) -g main.c
	gdb ./a.out

clean:
	rm -f a.out
