all:
	gcc -c src/block.c -o block.o -D_FILE_OFFSET_BITS=64 -lfuse 
	gcc -c src/cache.c -o cache.o -D_FILE_OFFSET_BITS=64 -lfuse 
	gcc -c src/inode.c -o inode.o -D_FILE_OFFSET_BITS=64 -lfuse 
	gcc -c src/interface.c -o interface.o -D_FILE_OFFSET_BITS=64 -lfuse 
	gcc src/main.c block.o cache.o inode.o interface.o -o fuselab -D_FILE_OFFSET_BITS=64 -lfuse 

clean:
	rm *.o
	rm fuselab
	rm single_thread_test
	rm multi_thread_test
