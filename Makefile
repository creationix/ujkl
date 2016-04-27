musl:
	musl-gcc -Wall -Werror -std=c11 -Os -static main2.c
	./a.out

memcheck:
	gcc -Wall -Werror -std=c11 -g main2.c
	valgrind --leak-check=full --show-leak-kinds=all ./a.out

lldb:
	clang -Wall -Werror -std=c11 -g main2.c
	lldb ./a.out

gdb:
	gcc -Wall -Werror -std=c11 -g main2.c
	gdb ./a.out

clean:
	rm -f a.out
