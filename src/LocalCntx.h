#pragma once



namespace KanameShiki {



uint64_t NumLocalCntx() noexcept;



class LocalCntx final : private NonCopyable<LocalCntx> {
	public:
		~LocalCntx() noexcept;
		
		LocalCntx() = default;
		LocalCntx(bool bInit);
		
		void ReserverFree(void* p) noexcept;
		void* ReserverAlloc(std::size_t s) noexcept;
		
		void* Alloc(std::size_t s) noexcept;
		
		void* NewPool(uint16_t o) noexcept;
		void* NewCram(uint16_t b, std::size_t s) noexcept;
		void* NewAny(std::size_t s) noexcept;
	
	
	private:
		LocalReserver mReserver;
		std::array<LocalPool*, cnPool+1> mapPool;
		std::array<LocalCram*, cbCram+1> mapCram;
};



}
