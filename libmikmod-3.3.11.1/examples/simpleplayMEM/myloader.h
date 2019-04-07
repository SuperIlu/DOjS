#ifndef myloader_h
#define myloader_h

typedef struct _MY_MEMREADER {
	MREADER core;
	const void *buffer;
	long len;
	long pos;
} MY_MEMREADER;

MREADER *my_new_mem_reader(const void *buffer, long len);
void my_delete_mem_reader(MREADER* reader);

#endif

