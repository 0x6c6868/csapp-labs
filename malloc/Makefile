all:
	gcc -pthread -o mymalloc ../lib/csapp.c memlib.c mm.c mymalloc.c -I ../include/ -I ./

clean:
	rm -rf mymalloc

format:
	clang-format -i memlib.c memlib.h mm.c mm.h mymalloc.c
