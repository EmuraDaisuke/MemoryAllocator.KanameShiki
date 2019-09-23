


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
,mpCacheST(nullptr)
,mpCacheMT(nullptr)
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
		CacheST(pParcel);
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
			CacheST(pParcel); return;
		} else {
			if (CacheMT(pParcel)) return;
		}
	}
	if (DecCache() == 0) Delete();
}



void* LocalPool::Alloc() noexcept
{
	Auto pParcel = mpCacheST;
	if (pParcel){
		mpCacheST = pParcel->Alloc(this);
		return pParcel->CastData();
	} else {
		pParcel = mpCacheMT.exchange(nullptr, std::memory_order_acq_rel);
		if (pParcel){
			mpCacheST = pParcel->Alloc(this);
			return pParcel->CastData();
		} else {
			Clearance();
			return mpOwner->NewPool(mo);
		}
	}
}



void LocalPool::Clearance() noexcept
{
	int16_t nCache = 0;
	
	Auto pParcel = mpCacheST;
	for (; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
	
	pParcel = mpCacheMT.exchange(Parcel::cpInvalid, std::memory_order_acq_rel);
	for (; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
	
	mnCache.fetch_sub(nCache, std::memory_order_acq_rel);
	mbCache.store(false, std::memory_order_release);
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



void LocalPool::CacheST(Parcel* pParcel) noexcept
{
	pParcel->Free(mpCacheST);
	mpCacheST = pParcel;
}



bool LocalPool::CacheMT(Parcel* pParcel) noexcept
{
	Auto pCacheMT = mpCacheMT.load(std::memory_order_acquire);
	while (pCacheMT != Parcel::cpInvalid){
		pParcel->Free(pCacheMT);
		if (mpCacheMT.compare_exchange_strong(pCacheMT, pParcel, std::memory_order_acq_rel, std::memory_order_acquire)) return true;
	}
	return false;
}



int16_t LocalPool::DecCache() noexcept
{
	return mnCache.fetch_sub(1, std::memory_order_acq_rel) - 1;
}



}
