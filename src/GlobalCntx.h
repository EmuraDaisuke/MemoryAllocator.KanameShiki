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
	
	
	private:
		GlobalHeap mHeap;
		GlobalReserver mReserver;
};



}
