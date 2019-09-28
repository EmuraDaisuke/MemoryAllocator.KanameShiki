#pragma once



namespace KanameShiki {



template <uint16_t cnRevolver>
class Parallel final {
	public:
		~Parallel() noexcept = default;
		
		Parallel() = default;
		
		constexpr uint16_t RevolverMask() const noexcept { return (cnRevolver-1); }
		
		
		
		void CacheST(Parcel* pParcel) noexcept
		{
			pParcel->Free(mCacheST.p);
			mCacheST.p = pParcel;
		}
		
		
		
		bool CacheMT(Parcel* pParcel) noexcept
		{
			Auto oRevolver = mRevolverFree.o.fetch_add(1, std::memory_order_acq_rel);
			auto& rCacheMT = maCacheMT[oRevolver & RevolverMask()];
			Auto pCacheMT = rCacheMT.p.load(std::memory_order_acquire);
			while (pCacheMT != Parcel::cpInvalid){
				pParcel->Free(pCacheMT);
				if (rCacheMT.p.compare_exchange_strong(pCacheMT, pParcel, std::memory_order_acq_rel, std::memory_order_acquire)) return true;
			}
			return false;
		}
		
		
		
		uint16_t Clearance() noexcept
		{
			uint16_t nCache = 0;
			
			for (Auto pParcel = mCacheST.p; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
			
			Auto oRevolver = mRevolverAlloc.o;
			for (auto n = cnRevolver; n; --n, ++oRevolver){
				auto& rCacheMT = maCacheMT[oRevolver & RevolverMask()];
				Auto pParcel = rCacheMT.p.exchange(Parcel::cpInvalid, std::memory_order_acq_rel);
				for (; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
			}
			
			return nCache;
		}
	
	
	public:
		struct ARevolver { std::atomic_uint16_t o; };
		struct Revolver { uint16_t o; };
		
		struct Cache { Parcel* p; };
		struct ACache { std::atomic<Parcel*> p; };
		
		ARevolver mRevolverFree;
		Revolver mRevolverAlloc;
		
		Cache mCacheST;
		std::array<ACache, cnRevolver> maCacheMT;
};



}
