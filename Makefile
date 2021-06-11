all:
	gcc -c src/block.c -o block.o -D_FILE_OFFSET_BITS=64 -lfuse -Wall
	gcc -c src/cache.c -o cache.o -D_FILE_OFFSET_BITS=64 -lfuse -Wall
	gcc -c src/inode.c -o inode.o -D_FILE_OFFSET_BITS=64 -lfuse -Wall
	gcc -c src/interface.c -o interface.o -D_FILE_OFFSET_BITS=64 -lfuse -Wall 
	gcc src/main.c block.o cache.o inode.o interface.o -o fuselab -D_FILE_OFFSET_BITS=64 -lfuse -Wall 
	gcc src/test.c block.o cache.o inode.o interface.o -o utiltest -D_FILE_OFFSET_BITS=64 -lfuse -Wall 

clean:
	rm *.o fuselab utiltest
