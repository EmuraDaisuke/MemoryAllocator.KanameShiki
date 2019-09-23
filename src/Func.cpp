


using namespace KanameShiki;

static GlobalCntx gGlobalCntx;
thread_local LocalCntx gLocalCntx;



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

void GlobalHeapFree(void* p) noexcept
{
	if (p) gGlobalCntx.HeapFree(p);
}



void* GlobalHeapAlloc(std::size_t s) noexcept
{
	Auto p = gGlobalCntx.HeapAlloc(s);
	#if !KANAMESHIKI_HEAP_SPECIALIZATION//[
	assert(p);
	#endif//]
	return p;
}



void GlobalReserverRelease() noexcept
{
	gGlobalCntx.ReserverRelease();
}



void GlobalReserverFree(void* p) noexcept
{
	if (p) gGlobalCntx.ReserverFree(p);
}



void* GlobalReserverAlloc(std::size_t s) noexcept
{
	Auto p = gGlobalCntx.ReserverAlloc(s);
	assert(p);
	return p;
}



uint32_t NumReserver(uint16_t Realm) noexcept
{
	return gGlobalCntx.NumReserver(Realm);
}



// Local

void LocalReserverFree(void* p) noexcept
{
	if (p) gLocalCntx.ReserverFree(p);
}



void* LocalReserverAlloc(std::size_t s) noexcept
{
	Auto p = gLocalCntx.ReserverAlloc(s);
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
	Auto p = gLocalCntx.Alloc(s);
	assert(p);
	return p;
}



void* Align(std::size_t a, std::size_t s) noexcept
{
	Auto sRelay = Relay::SizeofT();
	Auto sParcel = Parcel::SizeofT();
	Auto sHeader = sRelay + sParcel;
	
	Auto p = gLocalCntx.Alloc(sHeader + a + s);
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
			Auto pParcel = Parcel::CastParcel(pOld);
			Auto pOwner = pParcel->Owner();
			assert(pOwner);
			
			Auto sOld = pOwner->Size(pParcel);
			std::memcpy(pNew, pOld, (sNew < sOld)? sNew: sOld);
			Free(pOld);
		}
		return pNew;
	} else {
		Free(pOld);
		return nullptr;
	}
}



}
