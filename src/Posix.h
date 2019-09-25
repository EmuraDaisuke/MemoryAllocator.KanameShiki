#pragma once



extern "C" {



#if _WIN32//[
#define MALLOC_THROW			
#define FREE_THROW				
#define CALLOC_THROW			
#define REALLOC_THROW			

#define POSIX_MEMALIGN_THROW	throw()
#define ALIGNED_ALLOC_THROW		throw()
#define VALLOC_THROW			throw()

#define MEMALIGN_THROW			
#define PVALLOC_THROW			
#else//][
#define MALLOC_THROW			throw()
#define FREE_THROW				throw()
#define CALLOC_THROW			throw()
#define REALLOC_THROW			throw()

#define POSIX_MEMALIGN_THROW	throw()
#define ALIGNED_ALLOC_THROW		throw()
#define VALLOC_THROW			throw()

#define MEMALIGN_THROW			
#define PVALLOC_THROW			
#endif//]



#if !_WIN32//[
void *malloc(size_t size) MALLOC_THROW;
void free(void *ptr) FREE_THROW;
void *calloc(size_t nmemb, size_t size) CALLOC_THROW;
void *realloc(void *ptr, size_t size) REALLOC_THROW;
#endif//]

int posix_memalign(void **memptr, size_t alignment, size_t size) POSIX_MEMALIGN_THROW;
void *aligned_alloc(size_t alignment, size_t size) ALIGNED_ALLOC_THROW;
void *valloc(size_t size) VALLOC_THROW;

void *memalign(size_t alignment, size_t size) MEMALIGN_THROW;
void *pvalloc(size_t size) PVALLOC_THROW;

size_t malloc_usable_size(void *ptr);



}
