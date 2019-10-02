#pragma once



namespace KanameShiki {



uint64_t NumLocalPool() noexcept;



class alignas(csCacheLine) LocalPool final : public Base, private NonCopyable<LocalPool> {
	public:
		~LocalPool() noexcept;
		
		LocalPool(LocalCntx* pOwner, uint16_t o);
		
		std::size_t Size(Parcel* pParcel) const noexcept;
		void Free(Parcel* pParcel) noexcept;
		void* Alloc() noexcept;
		
		uint16_t Clearance() noexcept;
		void Delete() noexcept;
		
		void* operator new(std::size_t sThis, uint16_t o, const std::nothrow_t&) noexcept;
		
		void operator delete(void* p, uint16_t o, const std::nothrow_t&) noexcept	{ assert(false); }
		void operator delete(void* p) noexcept										{ assert(false); }
	
	private:
		uint16_t DecCache(uint16_t nCache) noexcept;
	
	
	private:
		std::thread::id mId;
		LocalCntx* mpOwner;
		uint16_t mo;
		
		Cache<cnPoolParcel> mCache;
		
		alignas(csCacheLine) std::atomic_bool mbCache;
		alignas(csCacheLine) std::atomic_uint16_t mnCache;
};



}
