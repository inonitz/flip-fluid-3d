#include "util/file.hpp"
#include "util/ifcrash.hpp"
#include "util/util.hpp"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd // stupid MSFT "deprecation" warning
#elif defined __linux__
    #include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>


bool util::loadFile(
	const char*   path, /* filepath 													*/ 
	unsigned int* size, /* the size of the out array that was pre-allocated.            */
	char*		  out   /* Where to place the file contents (Buffer of Min-Size 'size') */
) {
	uint64_t fsize;
	FILE* to_open = nullptr;

	to_open = fopen(path, "rb");
	ifcrashfmt(!to_open, "Couldn't get handle to file at Path [%s]\n", path);
	fseek(to_open, 0, SEEK_END);
	fsize = ftell(to_open);
	fseek(to_open, 0, SEEK_SET);

	if(*size < fsize || out == nullptr) {
		/* A buffer wasn't allocated OR the size of the buffer is too small. */
		*size = fsize;
		fclose(to_open);
		return false;
	}


	fsize = fread(out, sizeof(unsigned char), *size, to_open);
	ifcrashdo(fsize != *size, {
		fclose(to_open); 
		printf("Something went wrong - file size is %llu bytes, read only %llu bytes\n", __scast(uint64_t, *size), fsize);
	});


	fsize = fclose(to_open);
	ifcrashfmt(fsize, "Couldn't close file handle. ERROR CODE: %lld\n", fsize, errno);
	return true;
}


/* 
	Thanks to: 
	https://stackoverflow.com/questions/2868680/what-is-a-cross-platform-way-to-get-the-current-directory 
*/
void util::current_path(
	unsigned int*  size, 
	char* 		   out
) {
	char*    allocated_path = getcwd(NULL, 0);
	uint32_t pathlength	    = strlen(allocated_path); /* string is null terminated already */
	ifcrashfmt(allocated_path == nullptr, "Couldn't get current path. ERROR CODE %lld", errno);


	if(*size < pathlength || out == nullptr) {
		/* A buffer wasn't allocated OR the size of the buffer is too small. */
		*size = pathlength;
		free(allocated_path);
		return;
	}
	util::__memcpy(out, allocated_path, pathlength);
	free(allocated_path);
	return;
}