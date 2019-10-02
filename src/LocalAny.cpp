


namespace KanameShiki {



alignas(csCacheLine) std::atomic_uint64_t gnLocalAny(0);



uint64_t NumLocalAny() noexcept
{
	return gnLocalAny.load(std::memory_order_acquire);
}



LocalAny::~LocalAny() noexcept
{
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalAny.fetch_sub(1, std::memory_order_acq_rel);
	#endif//]
}



LocalAny::LocalAny(std::size_t s)
:mId(std::this_thread::get_id())
,ms(s)
{
	#if KANAMESHIKI_DEBUG_LEVEL//[
	gnLocalAny.fetch_add(1, std::memory_order_acq_rel);
	#endif//]
}



std::size_t LocalAny::Size(Parcel* pParcel) const noexcept
{
	return ms;
}



void LocalAny::Free(Parcel* pParcel) noexcept
{
	Auto bSelf = (mId == std::this_thread::get_id());
	
	this->~LocalAny();
	
	if (bSelf){
		LocalCntxPtr()->ReserverFree(this);
	} else {
		GlobalCntxPtr()->ReserverFree(this);
	}
}



void* LocalAny::Alloc() noexcept
{
	Auto sParcelT = Parcel::SizeofT();
	
	Auto vParcel = align_t(to_t(this+1) + sParcelT, csAlign) - sParcelT;
	Auto pParcel = reinterpret_cast<Parcel*>(vParcel);
	pParcel->Alloc(this);
	return pParcel->CastData();
}



void* LocalAny::operator new(std::size_t sThis, std::size_t s, const std::nothrow_t&) noexcept
{
	Auto sParcelS = Parcel::SizeofS(s);
	
	Auto sBudget = sThis + csAlign + sParcelS;
	return LocalCntxPtr()->ReserverAlloc(sBudget);
}



}
