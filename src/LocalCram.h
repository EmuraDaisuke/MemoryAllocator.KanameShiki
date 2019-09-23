#pragma once



namespace KanameShiki {



uint64_t NumLocalCram() noexcept;



class LocalCram final : public Base, private NonCopyable<LocalCram> {
	public:
		~LocalCram() noexcept;
		
		LocalCram(LocalCntx* pOwner, uint16_t b);
		
		std::size_t Size(Parcel* pParcel) const noexcept;
		void Free(Parcel* pParcel) noexcept;
		void* Alloc(std::size_t s) noexcept;
		
		void Clearance() noexcept;
		bool Closed() const noexcept;
		
		void Delete() noexcept;
		
		void* operator new(std::size_t sThis, uint16_t b, const std::nothrow_t&) noexcept;
		
		void operator delete(void* p, uint16_t b, const std::nothrow_t&) noexcept	{ assert(false); }
		void operator delete(void* p) noexcept										{ assert(false); }
	
	private:
		void CacheST(Parcel* pParcel) noexcept;
		bool CacheMT(Parcel* pParcel) noexcept;
		
		int16_t DecCache() noexcept;
	
	
	private:
		struct Cache {
			Parcel* p;
		};
		struct alignas(csCacheLine) ACache {
			std::atomic<Parcel*> p;
		};
		
		std::thread::id mId;
		LocalCntx* mpOwner;
		uint16_t mb;
		uint16_t mRealm;
		
		std::size_t mvParcel;
		std::size_t meParcel;
		
		std::array<Cache, cnFrac+1> maCacheST;
		std::array<ACache, cnFrac+1> maCacheMT;
		
		alignas(csCacheLine) std::atomic_bool mbCache;
		alignas(csCacheLine) std::atomic_int16_t mnCache;
};



}
