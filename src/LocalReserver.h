#pragma once



namespace KanameShiki {



class LocalReserver final : private NonCopyable<LocalReserver> {
	public:
		~LocalReserver() noexcept;
		
		LocalReserver();
		
		void Release() noexcept;
		
		void Free(void* p) noexcept;
		void* Alloc(std::size_t s) noexcept;
	
	private:
		class Segment;
	
	
	private:
		std::array<Segment*, cnRealm> mapRealm;
};



}
