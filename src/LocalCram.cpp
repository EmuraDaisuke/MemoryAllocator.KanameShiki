


namespace KanameShiki {



alignas(csCacheLine) std::atomic_uint64_t gnLocalCram(0);



uint64_t NumLocalCram() noexcept
{
	return gnLocalCram.load(std::memory_order_acquire);
}



LocalCram::~LocalCram() noexcept
{
	assert(Closed());
	
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalCram.fetch_sub(1, std::memory_order_acq_rel);
	#endif//]
}



LocalCram::LocalCram(LocalCntx* pOwner, uint16_t b)
:mId(std::this_thread::get_id())
,mpOwner(pOwner)
,mb(b)
,mRealm(Tag::Realm(bit(b)))
,mvParcel(to_t(this+1))
,meParcel(mvParcel + (bit(b) * cnCramParcel))
,maCacheST{}
,maCacheMT{}
,mbCache(true)
,mnCache(0)
{
	assert(!(to_t(this) & cmCacheLine));
	
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalCram.fetch_add(1, std::memory_order_acq_rel);
	#endif//]
}



std::size_t LocalCram::Size(Parcel* pParcel) const noexcept
{
	Auto pTag = Tag::CastTag(pParcel);
	Auto RealmT = pTag->Realm();
	return Tag::Size(RealmT + mRealm);
}



void LocalCram::Free(Parcel* pParcel) noexcept
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



void* LocalCram::Alloc(std::size_t s) noexcept
{
	Auto RealmS = Tag::Realm(s);
	Auto RealmT = RealmS - mRealm;
	
	assert(RealmT < numof(maCacheST));
	auto& rCacheST = maCacheST[RealmT];
	Auto pParcel = rCacheST.p;
	if (pParcel){
		rCacheST.p = pParcel->Alloc(this);
		return pParcel->CastData();
	} else {
		assert(RealmT < numof(maCacheMT));
		auto& rCacheMT = maCacheMT[RealmT];
		pParcel = rCacheMT.p.exchange(nullptr, std::memory_order_acq_rel);
		if (pParcel){
			rCacheST.p = pParcel->Alloc(this);
			return pParcel->CastData();
		} else {
			Auto vParcel = mvParcel;
			{	// 
				Auto sTagT = Tag::SizeofT();
				Auto sParcelT = Parcel::SizeofT();
				vParcel = align_t(vParcel + sTagT + sParcelT, csAlign) - sParcelT;
				pParcel = reinterpret_cast<Parcel*>(vParcel);
			}
			Auto p = pParcel->CastData();
			
			vParcel = to_t(p) + Tag::Size(RealmS);
			if (vParcel <= meParcel){
				mvParcel = vParcel;
				
				pParcel->Alloc(this);
				{	// 
					Auto pTag = Tag::CastTag(pParcel);
					new(pTag) Tag(RealmT);
				}
				
				mnCache.fetch_add(1, std::memory_order_acq_rel);
				return p;
			} else {
				Clearance();
				return mpOwner->NewCram(mb, s);
			}
		}
	}
}



void LocalCram::Clearance() noexcept
{
	int16_t nCache = 0;
	
	for (auto& rCacheST : maCacheST){
		Auto pParcel = rCacheST.p;
		for (; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
	}
	
	for (auto& rCacheMT : maCacheMT){
		Auto pParcel = rCacheMT.p.exchange(Parcel::cpInvalid, std::memory_order_acq_rel);
		for (; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
	}
	
	mnCache.fetch_sub(nCache, std::memory_order_acq_rel);
	mbCache.store(false, std::memory_order_release);
}



bool LocalCram::Closed() const noexcept
{
	return (mnCache.load(std::memory_order_acquire) == 0);
}



void LocalCram::Delete() noexcept
{
	Auto bSelf = (mId == std::this_thread::get_id());
	
	this->~LocalCram();
	
	if (bSelf){
		LocalReserverFree(this);
	} else {
		GlobalReserverFree(this);
	}
}



void* LocalCram::operator new(std::size_t sThis, uint16_t b, const std::nothrow_t&) noexcept
{
	Auto sBudget = sThis + (bit(b) * cnCramParcel);
	return LocalReserverAlloc(sBudget);
}



void LocalCram::CacheST(Parcel* pParcel) noexcept
{
	Auto pTag = Tag::CastTag(pParcel);
	Auto RealmT = pTag->Realm();
	
	assert(RealmT < numof(maCacheST));
	auto& rCacheST = maCacheST[RealmT];
	pParcel->Free(rCacheST.p);
	rCacheST.p = pParcel;
}



bool LocalCram::CacheMT(Parcel* pParcel) noexcept
{
	Auto pTag = Tag::CastTag(pParcel);
	Auto RealmT = pTag->Realm();
	
	assert(RealmT < numof(maCacheMT));
	auto& rCacheMT = maCacheMT[RealmT];
	Auto pCacheMT = rCacheMT.p.load(std::memory_order_acquire);
	while (pCacheMT != Parcel::cpInvalid){
		pParcel->Free(pCacheMT);
		if (rCacheMT.p.compare_exchange_strong(pCacheMT, pParcel, std::memory_order_acq_rel, std::memory_order_acquire)) return true;
	}
	return false;
}



int16_t LocalCram::DecCache() noexcept
{
	return mnCache.fetch_sub(1, std::memory_order_acq_rel) - 1;
}



}
