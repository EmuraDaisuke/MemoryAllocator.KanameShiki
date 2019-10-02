


namespace KanameShiki {



thread_local uint8_t gvLocalCntx(0);

alignas(csCacheLine) std::atomic_uint64_t gnLocalCntx(0);



uint64_t NumLocalCntx() noexcept
{
	return gnLocalCntx.load(std::memory_order_acquire);
}



LocalCntx::~LocalCntx() noexcept
{
	if (gpLocalCntx){
		if (--gvLocalCntx == 0){
			for (Auto pPool : mapPool) if (pPool && pPool->Clearance() == 0) pPool->Delete();
			for (Auto pCram : mapCram) if (pCram && pCram->Clearance() == 0) pCram->Delete();
			
			mReserver.Destruct();
			
			assert(gpLocalCntx);
			gpLocalCntx = nullptr;
			
			#if KANAMESHIKI_DEBUG_LEVEL//[
			gnLocalCntx.fetch_sub(1, std::memory_order_acq_rel);
			#endif//]
		} else {
			this->~LocalCntx();
		}
	}
}



LocalCntx::LocalCntx(bool bInit)
:mapCram{}
,mapPool{}
,mReserver(true)
{
	if (gvLocalCntx++ == 0){
		assert(!gpLocalCntx);
		gpLocalCntx = this;
		
		#if KANAMESHIKI_DEBUG_LEVEL//[
		gnLocalCntx.fetch_add(1, std::memory_order_acq_rel);
		#endif//]
	}
}



void LocalCntx::ReserverFree(void* p) noexcept
{
	mReserver.Free(p);
}



void* LocalCntx::ReserverAlloc(std::size_t s) noexcept
{
	return mReserver.Alloc(s);
}



void* LocalCntx::Alloc(std::size_t s) noexcept
{
	if (s <= csPool){
		Auto o = ((s)? s-1: s) >> cbSizeT;
		Auto pPool = mapPool[o];
		return (pPool)? pPool->Alloc(): NewPool(o);
	} else if (s <= csCram){
		Auto b = Lzc::Msb(s);
		Auto pCram = mapCram[b];
		return (pCram)? pCram->Alloc(s): NewCram(b, s);
	} else {
		return NewAny(s);
	}
}



void* LocalCntx::NewPool(uint16_t o) noexcept
{
	Auto pPool = new(o, std::nothrow) LocalPool(this, o);
	mapPool[o] = pPool;
	return (pPool)? pPool->Alloc(): nullptr;
}



void* LocalCntx::NewCram(uint16_t b, std::size_t s) noexcept
{
	Auto pCram = new(b, std::nothrow) LocalCram(this, b);
	mapCram[b] = pCram;
	return (pCram)? pCram->Alloc(s): nullptr;
}



void* LocalCntx::NewAny(std::size_t s) noexcept
{
	Auto pAny = new(s, std::nothrow) LocalAny(s);
	return (pAny)? pAny->Alloc(): nullptr;
}



}
