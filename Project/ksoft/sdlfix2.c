#include <stdio.h>

FILE* __cdecl __iob_func(void)
{
	FILE _iob[] = { *stdin, *stdout, *stderr };
	return _iob;

	//setvbuf()
	//setbuf
	//remove
	//freopen
}

int __cdecl __setvbuf(FILE *stream, char* buffer, int mode, size_t size)
{
	return setvbuf(stream, buffer, mode, size);
}

void __cdecl __setbuf(FILE *stream, char* buffer)
{
	setbuf(stream, buffer);
}

int __cdecl __remove(const char *filename)
{
	return remove(filename);
}

FILE * __cdecl __freopen(const char *filename, const char *mode, FILE *stream)
{
	return freopen(filename, mode, stream);
}