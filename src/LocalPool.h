#pragma once



namespace KanameShiki {



uint64_t NumLocalPool() noexcept;



class LocalPool final : public Base, private NonCopyable<LocalPool> {
	public:
		~LocalPool() noexcept;
		
		LocalPool(LocalCntx* pOwner, uint16_t o);
		
		std::size_t Size(Parcel* pParcel) const noexcept;
		void Free(Parcel* pParcel) noexcept;
		void* Alloc() noexcept;
		
		void Clearance() noexcept;
		bool Closed() const noexcept;
		
		void Delete() noexcept;
		
		void* operator new(std::size_t sThis, uint16_t o, const std::nothrow_t&) noexcept;
		
		void operator delete(void* p, uint16_t o, const std::nothrow_t&) noexcept	{ assert(false); }
		void operator delete(void* p) noexcept										{ assert(false); }
	
	private:
		void CacheST(Parcel* pParcel) noexcept;
		bool CacheMT(Parcel* pParcel) noexcept;
		
		int16_t DecCache() noexcept;
	
	
	private:
		std::thread::id mId;
		LocalCntx* mpOwner;
		uint16_t mo;
		
		Parcel* mpCacheST;
		alignas(csCacheLine) std::atomic<Parcel*> mpCacheMT;
		
		alignas(csCacheLine) std::atomic_bool mbCache;
		alignas(csCacheLine) std::atomic_int16_t mnCache;
};



}
