#pragma once



namespace KanameShiki {



class GlobalCntx final : private NonCopyable<GlobalCntx> {
	public:
		~GlobalCntx() noexcept;
		
		GlobalCntx() = default;
		GlobalCntx(bool bInit);
		
		void HeapFree(void* p) noexcept;
		void* HeapAlloc(std::size_t s) noexcept;
		
		void ReserverRelease() noexcept;
		void ReserverFree(void* p) noexcept;
		void* ReserverAlloc(std::size_t s) noexcept;
		
		uint32_t NumReserver(uint16_t Realm) const noexcept;
	
	
	private:
		GlobalHeap mHeap;
		
		uint32_t mnRevolver;
		uint32_t mbRevolver;
		std::size_t msReserver;
		GlobalReserver mReserver;
};



}
