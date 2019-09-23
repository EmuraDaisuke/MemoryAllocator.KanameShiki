


namespace KanameShiki {



// Segment

class GlobalReserver::Segment final {
	public:
		~Segment() noexcept
		{
			Release();
			
			#if KANAMESHIKI_DEBUG_LEVEL == 2//[
			assert(mnAllocHeap.load(std::memory_order_acquire) == 0);
			assert(mnAllocVirtual.load(std::memory_order_acquire) == 0);
			#endif//]
		}
		
		
		
		Segment(uint16_t nRevolver, uint32_t nReserver, uint16_t Realm)
		:mRealm(Realm)
		,ms(Tag::Size(Realm))
		,mnAllocHeap(0)
		,mnAllocVirtual(0)
		,mnReserver(nReserver)
		,mnRevolver(nRevolver)
		,mmRevolver(nRevolver-1)
		,moRevolverFree(0)
		,moRevolverAlloc(0)
		,maSpinlock(reinterpret_cast<decltype(maSpinlock)>(this+1))
		,maaReserver(reinterpret_cast<decltype(maaReserver)>(&maSpinlock[nRevolver]))
		,maoReserver(reinterpret_cast<decltype(maoReserver)>(&maaReserver[nReserver * nRevolver]))
		{
			assert(!(to_t(this) & cmCacheLine));
			assert(Realm < cnRealm);
			
			for (uint16_t oRevolver = 0; oRevolver < mnRevolver; ++oRevolver){
				maSpinlock[oRevolver].f.clear(std::memory_order_release);
				maoReserver[oRevolver].o = 0;
			}
		}
		
		
		
		void Release() noexcept
		{
			for (uint16_t oRevolver = 0; oRevolver < mnRevolver; ++oRevolver){
				if (ReserverOffset(oRevolver)){
					Auto rFlag = Flag(oRevolver);
					Lock(rFlag);
					
					Auto aReserver = ReserverArray(oRevolver);
					Auto oReserver = ReserverOffset(oRevolver);
					while (oReserver){
						TaggedFree(aReserver[--oReserver].p);
					}
					Unlock(rFlag);
				}
			}
		}
		
		
		
		void Free(void* p) noexcept
		{
			Auto oRevolver = moRevolverFree.fetch_add(1, std::memory_order_acq_rel) & mmRevolver;
			Auto rFlag = Flag(oRevolver);
			Lock(rFlag);
			
			Auto oReserver = ReserverOffset(oRevolver);
			if (oReserver < mnReserver){
				Auto aReserver = ReserverArray(oRevolver);
				aReserver[oReserver++].p = p;
				Unlock(rFlag);
			} else {
				Unlock(rFlag);
				TaggedFree(p);
			}
		}
		
		
		
		void* Alloc() noexcept
		{
			Auto oRevolver = moRevolverAlloc.fetch_add(1, std::memory_order_acq_rel) & mmRevolver;
			Auto rFlag = Flag(oRevolver);
			Lock(rFlag);
			
			Auto oReserver = ReserverOffset(oRevolver);
			if (oReserver){
				Auto aReserver = ReserverArray(oRevolver);
				Auto p = aReserver[--oReserver].p;
				Unlock(rFlag);
				return p;
			} else {
				Unlock(rFlag);
				return TaggedAlloc();
			}
		}
		
		
		
		void* operator new(std::size_t sThis, uint16_t nRevolver, uint32_t nReserver, const std::nothrow_t&) noexcept
		{
			Auto saSpinlock = sizeof(maSpinlock[0]) * nRevolver;
			Auto saaReserver = sizeof(maaReserver[0]) * nReserver * nRevolver;
			Auto saoReserver = sizeof(maoReserver[0]) * nRevolver;
			
			Auto sBudget = sThis + saSpinlock + saaReserver + saoReserver;
			return GlobalHeapAlloc(sBudget);
		}
		
		
		
		void operator delete(void* p, uint16_t nRevolver, uint32_t nReserver, const std::nothrow_t&) noexcept	{ GlobalHeapFree(p); }
		void operator delete(void* p) noexcept																	{ GlobalHeapFree(p); }
	
	private:
		struct Reserver;
		
		std::atomic_flag& Flag(uint16_t oRevolver) noexcept		{ return maSpinlock[oRevolver].f; }
		
		static void Lock(std::atomic_flag& rFlag) noexcept		{ while (rFlag.test_and_set(std::memory_order_acquire)); }
		static void Unlock(std::atomic_flag& rFlag) noexcept	{ rFlag.clear(std::memory_order_release); }
		
		Reserver* ReserverArray(uint16_t oRevolver) noexcept	{ return &maaReserver[mnReserver * oRevolver]; }
		uint32_t& ReserverOffset(uint16_t oRevolver) noexcept	{ return maoReserver[oRevolver].o; }
		
		
		
		void TaggedFree(void* p) noexcept
		{
			bool bVirtual;
			{	// 
				Auto pTag = Tag::CastTag(p);
				assert(mRealm == pTag->Realm());
				bVirtual = pTag->Virtual();
			}
			p = offset_p(p, -csCacheLine);
			
			Auto s = ms + csCacheLine;
			#if KANAMESHIKI_HEAP_SPECIALIZATION//[
			(bVirtual)? SystemFree(p, s): GlobalHeapFree(p);
			#else//][
			assert(bVirtual);
			SystemFree(p, s);
			#endif//]
			
			#if KANAMESHIKI_DEBUG_LEVEL == 2//[
			(bVirtual)? mnAllocVirtual.fetch_sub(1, std::memory_order_acq_rel): mnAllocHeap.fetch_sub(1, std::memory_order_acq_rel);
			#endif//]
		}
		
		
		
		void* TaggedAlloc() noexcept
		{
			Auto s = ms + csCacheLine;
			#if KANAMESHIKI_HEAP_SPECIALIZATION//[
			Auto p = (ms < csVirtual)? GlobalHeapAlloc(s): nullptr;
			bool bVirtual = !p;
			p = (p)? p: SystemAlloc(s);
			#else//][
			Auto p = SystemAlloc(s);
			bool bVirtual = true;
			#endif//]
			
			if (p){
				p = offset_p(p, csCacheLine);
				{	// 
					Auto pTag = Tag::CastTag(p);
					new(pTag) Tag(mRealm, bVirtual);
				}
				
				#if KANAMESHIKI_DEBUG_LEVEL == 2//[
				(bVirtual)? mnAllocVirtual.fetch_add(1, std::memory_order_acq_rel): mnAllocHeap.fetch_add(1, std::memory_order_acq_rel);
				#endif//]
			}
			return p;
		}
	
	
	private:
		struct alignas(csCacheLine) Spinlock {
			std::atomic_flag f;
		};
		struct Reserver {
			void* p;
		};
		struct Offset {
			uint32_t o;
		};
		
		const uint16_t mRealm;
		const std::size_t ms;
		
		alignas(csCacheLine) std::atomic_int64_t mnAllocHeap;
		alignas(csCacheLine) std::atomic_int64_t mnAllocVirtual;
		
		const uint32_t mnReserver;
		
		const uint16_t mnRevolver;
		const uint16_t mmRevolver;
		alignas(csCacheLine) std::atomic_uint16_t moRevolverFree;
		alignas(csCacheLine) std::atomic_uint16_t moRevolverAlloc;
		
		Spinlock* const maSpinlock;
		
		Reserver* const maaReserver;
		Offset* const maoReserver;
};



// GlobalReserver

GlobalReserver::~GlobalReserver() noexcept
{
	for (auto& rpSegment : mapRealm){
		delete rpSegment;
	}
}



GlobalReserver::GlobalReserver(uint8_t bRevolver)
{
	uint16_t nRevolver = bit(bRevolver);
	
	uint16_t Realm = 0;
	for (auto& rpSegment : mapRealm){
		Auto nReserver = NumReserver(Realm);
		rpSegment = new(nRevolver, nReserver, std::nothrow) GlobalReserver::Segment(nRevolver, nReserver, Realm);
		assert(rpSegment);
		++Realm;
	}
}



void GlobalReserver::Release() noexcept
{
	for (auto& rpSegment : mapRealm){
		rpSegment->Release();
	}
}



void GlobalReserver::Free(void* p) noexcept
{
	Auto pTag = Tag::CastTag(p);
	Auto Realm = pTag->Realm();
	
	assert(Realm < numof(mapRealm));
	Auto pSegment = mapRealm[Realm];
	pSegment->Free(p);
}



void* GlobalReserver::Alloc(std::size_t s) noexcept
{
	Auto Realm = Tag::Realm(s);
	
	assert(Realm < numof(mapRealm));
	Auto pSegment = mapRealm[Realm];
	Auto p = pSegment->Alloc();
	if (p) return p;
	
	Release();
	return pSegment->Alloc();
}



}