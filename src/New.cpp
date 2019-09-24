


using namespace KanameShiki;



#if NEW//[
void* operator new(std::size_t size)										{ return Alloc(size); }
void* operator new[](std::size_t size)										{ return Alloc(size); }
void operator delete(void* p) noexcept										{ Free(p); }
void operator delete[](void* p) noexcept									{ Free(p); }

void* operator new(std::size_t size, const std::nothrow_t& nt) noexcept		{ return Alloc(size); }
void* operator new[](std::size_t size, const std::nothrow_t& nt) noexcept	{ return Alloc(size); }
void operator delete(void* p, const std::nothrow_t& nt) noexcept			{ Free(p); }
void operator delete[](void* p, const std::nothrow_t& nt) noexcept			{ Free(p); }
#endif//]
