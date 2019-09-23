#pragma once



namespace KanameShiki {



bool IsBootGlobalCntx() noexcept;



class GlobalCntx final : private NonCopyable<GlobalCntx> {
	public:
		~GlobalCntx() noexcept;
		
		GlobalCntx();
		
		void HeapFree(void* p) noexcept;
		void* HeapAlloc(std::size_t s) noexcept;
		
		void ReserverRelease() noexcept;
		void ReserverFree(void* p) noexcept;
		void* ReserverAlloc(std::size_t s) noexcept;
		
		uint32_t NumReserver(uint16_t Realm) const noexcept;
	
	
	private:
		GlobalHeap mHeap;
		
		const uint32_t mnRevolver;
		const uint32_t mbRevolver;
		const std::size_t msReserver;
		GlobalReserver mReserver;
};



}
