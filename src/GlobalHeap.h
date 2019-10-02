#pragma once



namespace KanameShiki {



class GlobalHeap final : private NonCopyable<GlobalHeap> {
	public:
		~GlobalHeap() noexcept				= default;
		
		GlobalHeap()						= default;
		GlobalHeap(bool bInit);
		
		void Destruct() noexcept;
		
		void Free(void* p) noexcept;
		void* AllocUpper(std::size_t s) noexcept;
		void* AllocLower(std::size_t s) noexcept;
	
	private:
		class Node;
		
		void AddFree(Node* pFree, std::size_t s) noexcept;
		
		void* Alloc(Node* pFree, uint8_t oFree, std::size_t s) noexcept;
		
		void FreeOn(uint8_t oFree) noexcept;
		void FreeOff(uint8_t oFree) noexcept;
		
		static uint8_t Lower(std::size_t s) noexcept;
		static uint8_t Upper(std::size_t s) noexcept;
	
	
	private:
		struct List { Node* pHead; };
		
		std::mutex mMutex;
		std::size_t ms;
		void* mp;
		
		int64_t mnAlloc;
		
		List mOrder;
		std::array<List, 64> maFree;
		
		uint64_t mabFree;
};



}
