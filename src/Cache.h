#pragma once



namespace KanameShiki {



template <uint16_t cnRevolver>
class alignas(csCacheLine) Cache final : private NonCopyable<Cache<cnRevolver>> {
	public:
		~Cache() noexcept = default;
		
		Cache() = default;
		
		constexpr uint16_t RevolverMask() const noexcept { return (cnRevolver-1); }
		
		
		
		void ListST(Parcel* pParcel) noexcept
		{
			pParcel->Free(mListST.p);
			mListST.p = pParcel;
		}
		
		
		
		bool ListMT(Parcel* pParcel) noexcept
		{
			Auto oRevolver = mRevolverFree.o.fetch_add(1, std::memory_order_acq_rel);
			auto& rListMT = maListMT[oRevolver & RevolverMask()];
			Auto pListMT = rListMT.p.load(std::memory_order_acquire);
			while (pListMT != Parcel::cpInvalid){
				pParcel->Free(pListMT);
				if (rListMT.p.compare_exchange_strong(pListMT, pParcel, std::memory_order_acq_rel, std::memory_order_acquire)) return true;
			}
			return false;
		}
		
		
		
		uint16_t Clearance() noexcept
		{
			uint16_t nCache = 0;
			
			for (Auto pParcel = mListST.p; pParcel; pParcel = pParcel->Alloc(nullptr), ++nCache);
			
			Auto oRevolver = mRevolverAlloc.o;
			for (auto n = cnRevolver; n; --n, ++oRevolver){
				auto& rListMT = maListMT[oRevolver & RevolverMask()];
				Auto pParcel = rListMT.p.exchange(Parcel::cpInvalid, std::memory_order_acq_rel);
				nCache += (pParcel)? 1:0;
			}
			
			return nCache;
		}
	
	
	public:
		struct ARevolver { std::atomic_uint16_t o; };
		struct Revolver { uint16_t o; };
		
		struct List { Parcel* p; };
		struct AList { std::atomic<Parcel*> p; };
		
		alignas(csCacheLine) ARevolver mRevolverFree;
		alignas(csCacheLine) Revolver mRevolverAlloc;
		
		alignas(csCacheLine) List mListST;
		alignas(csCacheLine) std::array<AList, cnRevolver> maListMT;
};



}
