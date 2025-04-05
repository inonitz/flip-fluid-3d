#ifndef __UTIL_LOAD_FILE__
#define __UTIL_LOAD_FILE__
#include "util_api.h"


namespace util {


/* 
	Recommended to set size{0}, out{nullptr} to find the size of the buffer that should be allocated 
	(if you didn't know the filesize beforehand)
	when 'size' is known, allocate a buffer and pass it in 'out', 
	After which the function will proceed with file loading
*/
UTIL_API bool loadFile(
	const char*   path, /* filepath 													*/ 
	unsigned int* size, /* the size of the out array that was pre-allocated.            */
	char*		  out   /* Where to place the file contents (Buffer of Min-Size 'size') */
);


/*

*/
UTIL_API void current_path(
	unsigned int*  size, /* */
	char* 		   out 	 /* Where the */
);


}


#endif