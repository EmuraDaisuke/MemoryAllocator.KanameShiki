


#if !_WIN32//[
#include <unistd.h>
#endif//]



extern "C" {



using namespace KanameShiki;



static size_t SystemPageSize()
{
	#ifdef _WIN32//[
	SYSTEM_INFO Info;
	GetSystemInfo(&Info);
	return Info.dwPageSize;
	#else//][
	return sysconf(_SC_PAGESIZE);
	#endif//]
}



static bool IsPowerOf2(size_t alignment)
{
	return ((alignment & -alignment) == alignment);
}



void *malloc(size_t size) MALLOC_THROW
{
	return Alloc(size);
}



void free(void *ptr) FREE_THROW
{
	Free(ptr);
}



void *calloc(size_t nmemb, size_t size) CALLOC_THROW
{
	Auto s = nmemb * size;
	Auto p = malloc(s);
	if (p) std::memset(p, 0, s);
	return p;
}



void *realloc(void *ptr, size_t size) REALLOC_THROW
{
	return ReAlloc(ptr, size);
}



int posix_memalign(void **memptr, size_t alignment, size_t size) POSIX_MEMALIGN_THROW
{
	Auto bAlign = (IsPowerOf2(alignment) && alignment >= sizeof(void*));
	*memptr = (bAlign)? memalign(alignment, size): nullptr;
	return (*memptr)? 0: (bAlign)? ENOMEM: EINVAL;
}



void *aligned_alloc(size_t alignment, size_t size) ALIGNED_ALLOC_THROW
{
	Auto bAlign = ((size & -size) >= alignment);
	return (bAlign)? Align(alignment, size): nullptr;
}



void *valloc(size_t size) VALLOC_THROW
{
	return memalign(SystemPageSize(), size);
}



void *memalign(size_t alignment, size_t size) MEMALIGN_THROW
{
	return (IsPowerOf2(alignment))? aligned_alloc(size, alignment): nullptr;
}



void *pvalloc(size_t size) PVALLOC_THROW
{
	return valloc(align_t(size, SystemPageSize()));
}



size_t malloc_usable_size(void *ptr)
{
	return Size(ptr);
}



}
