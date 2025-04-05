#ifndef __UTIL_NEW_DELETE_OVERRIDE__
#define __UTIL_NEW_DELETE_OVERRIDE__
/*
    Needs to be implemented eventually -
    multiple dll's accessing different heaps
    doesn't sound like a good idea
*/


void* operator new(decltype(sizeof(0)) n) noexcept(false);
void  operator delete(void * p)           noexcept(true);

void* operator new[](decltype(sizeof(0)) n) noexcept(false);
void  operator delete[](void * p)           noexcept(true);


#endif