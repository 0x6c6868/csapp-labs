all:
	gcc -pthread -o myshell ../lib/csapp.c myshell.c -I ../include/ -I ./

clean:
	rm -rf myshell

format:
	clang-format -i myshell.c
