all:
	gcc -pthread -o mywebserver ../lib/csapp.c mywebserver.c -I ../include/ -I ./

clean:
	rm -rf mywebserver

format:
	clang-format -i mywebserver.c
