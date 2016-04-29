CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11

default:
	$(CC) $(CFLAGS) -g main2.c
	./a.out

clang:
	clang $(CFLAGS) -g main2.c
	./a.out

gcc:
	gcc $(CFLAGS) -g main2.c
	./a.out

musl:
	musl-gcc $(CFLAGS) -Os -static main2.c
	./a.out

memcheck:
	gcc $(CFLAGS) -g main2.c
	valgrind --leak-check=full --show-leak-kinds=all ./a.out

lldb:
	clang $(CFLAGS) -g main2.c
	lldb ./a.out

gdb:
	gcc $(CFLAGS) -g main2.c
	gdb ./a.out

clean:
	rm -f a.out
