


namespace KanameShiki {



// Segment

class LocalReserver::Segment final {
	public:
		~Segment() noexcept
		{
			Release();
		}
		
		
		
		Segment(uint32_t nReserver, uint16_t Realm)
		:ms(Tag::Size(Realm))
		,mnReserver(nReserver)
		,moReserver(0)
		,maReserver(reinterpret_cast<decltype(maReserver)>(this+1))
		{
			assert(Realm < cnRealm);
		}
		
		
		
		void Release() noexcept
		{
			auto& oReserver = moReserver;
			while (oReserver){
				GlobalReserverFree(maReserver[--oReserver].p);
			}
		}
		
		
		
		void Free(void* p) noexcept
		{
			auto& oReserver = moReserver;
			if (oReserver < mnReserver){
				maReserver[oReserver++].p = p;
			} else {
				GlobalReserverFree(p);
			}
		}
		
		
		
		void* Alloc() noexcept
		{
			auto& oReserver = moReserver;
			if (oReserver){
				return maReserver[--oReserver].p;
			} else {
				return GlobalReserverAlloc(ms);
			}
		}
		
		
		
		void* operator new(std::size_t sThis, uint32_t nReserver, const std::nothrow_t&) noexcept
		{
			Auto saReserver = sizeof(maReserver[0]) * nReserver;
			
			Auto sBudget = sThis + saReserver;
			return GlobalReserverAlloc(sBudget);
		}
		
		void operator delete(void* p, uint32_t nReserver, const std::nothrow_t&) noexcept	{ GlobalReserverFree(p); }
		void operator delete(void* p) noexcept												{ GlobalReserverFree(p); }
	
	
	private:
		struct Reserver {
			void* p;
		};
		
		const std::size_t ms;
		
		const uint32_t mnReserver;
		
		uint32_t moReserver;
		Reserver* const maReserver;
};



// LocalReserver

LocalReserver::~LocalReserver() noexcept
{
	for (auto& rpSegment : mapRealm){
		delete rpSegment;
	}
}



LocalReserver::LocalReserver()
{
	uint16_t Realm = 0;
	for (auto& rpSegment : mapRealm){
		Auto nReserver = NumReserver(Realm);
		rpSegment = new(nReserver, std::nothrow) LocalReserver::Segment(nReserver, Realm);
		assert(rpSegment);
		++Realm;
	}
}



void LocalReserver::Release() noexcept
{
	for (auto& rpSegment : mapRealm){
		rpSegment->Release();
	}
	GlobalReserverRelease();
}



void LocalReserver::Free(void* p) noexcept
{
	Auto pTag = Tag::CastTag(p);
	Auto Realm = pTag->Realm();
	
	assert(Realm < numof(mapRealm));
	Auto pSegment = mapRealm[Realm];
	pSegment->Free(p);
}



void* LocalReserver::Alloc(std::size_t s) noexcept
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
