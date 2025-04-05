#ifndef __UTIL_ALIGNED_MEMORY_ALLOCATOR__
#define __UTIL_ALIGNED_MEMORY_ALLOCATOR__
#include "util_api.h"
#include "base_type.h"


namespace util {


template<u16 alignment> UTIL_API void* aligned_malloc(u64 bytes);
                        UTIL_API void  aligned_free(void* ptr);


} // namespace util


#endif