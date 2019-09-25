


namespace KanameShiki {



GlobalCntx::~GlobalCntx() noexcept
{
	if (gpLocalCntx) gpLocalCntx->~LocalCntx();
	
	#if KANAMESHIKI_DEBUG_LEVEL == 1//[
	// If LocalPool, LocalCram, and LocalAny do not exist, excess LocalCntx is ignored.
	Auto ExistLocal = [](){ return (NumLocalCntx() && (NumLocalPool() | NumLocalCram() | NumLocalAny())); };
	#elif KANAMESHIKI_DEBUG_LEVEL == 2//][
	// Check for the existence of LocalCntx, LocalPool, LocalCram, and LocalAny.
	Auto ExistLocal = [](){ return (NumLocalCntx() | NumLocalPool() | NumLocalCram() | NumLocalAny()); };
	#endif//]
	
	#if KANAMESHIKI_DEBUG_LEVEL//[
	while (ExistLocal()){
		std::this_thread::yield();
		ReserverRelease();
	}
	#endif//]
	
	mReserver.Destruct();
	mHeap.Destruct();
	
	assert(gpGlobalCntx);
	gpGlobalCntx = nullptr;
}



GlobalCntx::GlobalCntx(bool bInit)
{
	new(&mHeap) GlobalHeap(csHeap);
	
	mnRevolver = std::thread::hardware_concurrency();
	mnRevolver += mnRevolver;
	mbRevolver = (mnRevolver)? Lzc::Msb(mnRevolver + mnRevolver - 1):0;
	msReserver = bit(cbMemory - mbRevolver - cbFrac);
	
	assert(!gpGlobalCntx);
	gpGlobalCntx = this;
	
	new(&mReserver) GlobalReserver(mbRevolver);
}



void GlobalCntx::HeapFree(void* p) noexcept
{
	mHeap.Free(p);
}



void* GlobalCntx::HeapAlloc(std::size_t s) noexcept
{
	Auto p = mHeap.AllocUpper(s);
	return (p)? p: mHeap.AllocLower(s);
}



void GlobalCntx::ReserverRelease() noexcept
{
	mReserver.Release();
}



void GlobalCntx::ReserverFree(void* p) noexcept
{
	mReserver.Free(p);
}



void* GlobalCntx::ReserverAlloc(std::size_t s) noexcept
{
	return mReserver.Alloc(s);
}



uint32_t GlobalCntx::NumReserver(uint16_t Realm) const noexcept
{
	Auto Ratio = msReserver / Tag::Size(Realm);
	Auto nReserver = Lzc::Msb(Ratio) + 1;
	return nReserver;
}



}
