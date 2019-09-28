


namespace KanameShiki {



alignas(csCacheLine) std::atomic_uint64_t gnLocalPool(0);



uint64_t NumLocalPool() noexcept
{
	return gnLocalPool.load(std::memory_order_acquire);
}



LocalPool::~LocalPool() noexcept
{
	assert(Closed());
	
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalPool.fetch_sub(1, std::memory_order_acq_rel);
	#endif//]
}



LocalPool::LocalPool(LocalCntx* pOwner, uint16_t o)
:mId(std::this_thread::get_id())
,mpOwner(pOwner)
,mo(o)
,mParallel{}
,mbCache(true)
,mnCache(cnPoolParcel)
{
	assert(!(to_t(this) & cmCacheLine));
	
	Auto s = (o+1) * csSizeT;
	Auto sParcelT = Parcel::SizeofT();
	Auto sParcelS = Parcel::SizeofS(s);
	
	Auto vParcel = align_t(to_t(this+1) + sParcelT, csAlign) - sParcelT;
	for (auto n = mnCache.load(std::memory_order_acquire); n; --n){
		Auto pParcel = reinterpret_cast<Parcel*>(vParcel);
		mParallel.CacheST(pParcel);
		vParcel += sParcelS;
	}
	
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalPool.fetch_add(1, std::memory_order_acq_rel);
	#endif//]
}



std::size_t LocalPool::Size(Parcel* pParcel) const noexcept
{
	return (mo+1) * csSizeT;
}



void LocalPool::Free(Parcel* pParcel) noexcept
{
	if (mbCache.load(std::memory_order_acquire)){
		if (mId == std::this_thread::get_id()){
			mParallel.CacheST(pParcel); return;
		} else {
			if (mParallel.CacheMT(pParcel)) return;
		}
	}
	if (DecCache() == 0) Delete();
}



void* LocalPool::Alloc() noexcept
{
	auto& rCacheST = mParallel.mCacheST;
	Auto pParcel = rCacheST.p;
	if (pParcel){
		rCacheST.p = pParcel->Alloc(this);
		return pParcel->CastData();
	} else {
		Auto oRevolver = mParallel.mRevolverAlloc.o & mParallel.RevolverMask();
		pParcel = mParallel.maCacheMT[oRevolver].p.exchange(nullptr, std::memory_order_acq_rel);
		if (pParcel){
			mParallel.mRevolverAlloc.o = ++oRevolver;
			rCacheST.p = pParcel->Alloc(this);
			return pParcel->CastData();
		} else {
			Clearance();
			return mpOwner->NewPool(mo);
		}
	}
}



void LocalPool::Clearance() noexcept
{
	mbCache.store(false, std::memory_order_release);
	
	Auto nCache = mParallel.Clearance();
	mnCache.fetch_sub(nCache, std::memory_order_acq_rel);
}



bool LocalPool::Closed() const noexcept
{
	return (mnCache.load(std::memory_order_acquire) == 0);
}



void LocalPool::Delete() noexcept
{
	Auto bSelf = (mId == std::this_thread::get_id());
	
	this->~LocalPool();
	
	if (bSelf){
		LocalReserverFree(this);
	} else {
		GlobalReserverFree(this);
	}
}



void* LocalPool::operator new(std::size_t sThis, uint16_t o, const std::nothrow_t&) noexcept
{
	Auto s = (o+1) * csSizeT;
	Auto sParcelS = Parcel::SizeofS(s);
	
	Auto sBudget = sThis + csAlign + (sParcelS * cnPoolParcel);
	return LocalReserverAlloc(sBudget);
}



uint16_t LocalPool::DecCache() noexcept
{
	return mnCache.fetch_sub(1, std::memory_order_acq_rel) - 1;
}



}
