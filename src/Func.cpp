


using namespace KanameShiki;



#ifdef _WIN32//[
#pragma init_seg(lib)
static GlobalCntx gGlobalCntx;
static GlobalCntx* volatile gpGlobalCntx(nullptr);

thread_local LocalCntx gLocalCntx;
thread_local LocalCntx* volatile gpLocalCntx(nullptr);
#else//][
#error
//static GlobalCntx __attribute__((init_priority(101))) gGlobalCntx;
//static GlobalCntx* volatile gpGlobalCntx(nullptr);

//thread_local LocalCntx gLocalCntx;
//thread_local LocalCntx* volatile gpLocalCntx(nullptr);
#endif//]



namespace KanameShiki {



// System

void* SystemAlloc(std::size_t s) noexcept
{
	#ifdef _WIN32//[
	return VirtualAlloc(nullptr, s, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
	#else//][
	{	// 
		int prot = PROT_READ | PROT_WRITE;
		int flags = MAP_PRIVATE;
		
		#ifdef PROT_MAX//[
		prot = PROT_MAX(prot);
		#endif//]
		
		#ifdef MAP_NOCORE//[
		flags |= MAP_NOCORE;
		#endif//]
		#ifdef MAP_ANON//[
		flags |= MAP_ANON;
		#endif//]
		#ifdef MAP_ANONYMOUS//[
		flags |= MAP_ANONYMOUS;
		#endif//]
		#ifdef MAP_ALIGNED//[
		flags |= MAP_ALIGNED(cbCacheLine);
		#endif//]
		#ifdef MAP_UNINITIALIZED//[
		flags |= MAP_UNINITIALIZED;
		#endif//]
		
		Auto p = mmap(nullptr, s, prot, flags, -1, 0);
		return (p == MAP_FAILED)? nullptr: p;
	}
	#endif//]
}



void SystemFree(void* p, std::size_t s) noexcept
{
	#ifdef _WIN32//[
	Auto bResult = VirtualFree(p, 0, MEM_RELEASE);
	#else//][
	Auto bResult = (munmap(p, s) == 0);
	#endif//]
	assert(bResult);
}



// Global

GlobalCntx* GlobalCntxPtr() noexcept
{
	Auto pGlobalCntx = gpGlobalCntx;
	if (pGlobalCntx){
		return pGlobalCntx;
	} else {
		new(&gGlobalCntx) GlobalCntx(true);
		return gpGlobalCntx;
	}
}



void GlobalHeapFree(void* p) noexcept
{
	if (p) GlobalCntxPtr()->HeapFree(p);
}



void* GlobalHeapAlloc(std::size_t s) noexcept
{
	Auto p = GlobalCntxPtr()->HeapAlloc(s);
	#if !KANAMESHIKI_HEAP_SPECIALIZATION//[
	assert(p);
	#endif//]
	return p;
}



void GlobalReserverRelease() noexcept
{
	GlobalCntxPtr()->ReserverRelease();
}



void GlobalReserverFree(void* p) noexcept
{
	if (p) GlobalCntxPtr()->ReserverFree(p);
}



void* GlobalReserverAlloc(std::size_t s) noexcept
{
	Auto p = GlobalCntxPtr()->ReserverAlloc(s);
	assert(p);
	return p;
}



uint32_t NumReserver(uint16_t Realm) noexcept
{
	return GlobalCntxPtr()->NumReserver(Realm);
}



// Local

LocalCntx* LocalCntxPtr() noexcept
{
	Auto pLocalCntx = gpLocalCntx;
	if (pLocalCntx){
		return pLocalCntx;
	} else {
		new(&gLocalCntx) LocalCntx(true);
		return gpLocalCntx;
	}
}



void LocalReserverFree(void* p) noexcept
{
	if (p) LocalCntxPtr()->ReserverFree(p);
}



void* LocalReserverAlloc(std::size_t s) noexcept
{
	Auto p = LocalCntxPtr()->ReserverAlloc(s);
	assert(p);
	return p;
}



// Default

void Free(void* p) noexcept
{
	if (p){
		Auto pParcel = Parcel::CastParcel(p);
		Auto pOwner = pParcel->Owner();
		assert(pOwner);
		pOwner->Free(pParcel);
	}
}



void* Alloc(std::size_t s) noexcept
{
	Auto p = LocalCntxPtr()->Alloc(s);
	assert(p);
	return p;
}



void* Align(std::size_t a, std::size_t s) noexcept
{
	Auto sRelay = Relay::SizeofT();
	Auto sParcel = Parcel::SizeofT();
	Auto sHeader = sRelay + sParcel;
	
	Auto p = LocalCntxPtr()->Alloc(sHeader + a + s);
	assert(p);
	if (p){
		Auto pAlign = align_p(offset_p(p, sHeader), a);
		Auto pParcel = Parcel::CastParcel(pAlign);
		Auto pRelay = Relay::CastRelay(pParcel);
		pParcel->Alloc(pRelay);
		new(pRelay) Relay(p, s);
		
		return pAlign;
	}
	return p;
}



void* ReAlloc(void* p, std::size_t s) noexcept
{
	Auto pOld = p;
	Auto sNew = s;
	if (sNew){
		Auto pNew = Alloc(sNew);
		if (pNew && pOld){
			Auto sOld = Size(pOld);
			std::memcpy(pNew, pOld, (sNew < sOld)? sNew: sOld);
			Free(pOld);
		}
		return pNew;
	} else {
		Free(pOld);
		return nullptr;
	}
}



std::size_t Size(void* p) noexcept
{
	Auto pParcel = Parcel::CastParcel(p);
	Auto pOwner = pParcel->Owner();
	assert(pOwner);
	return pOwner->Size(pParcel);
}



}
