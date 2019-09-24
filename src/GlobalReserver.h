#pragma once



namespace KanameShiki {



class GlobalReserver final : private NonCopyable<GlobalReserver> {
	public:
		~GlobalReserver() noexcept			= default;
		
		GlobalReserver()					= default;
		GlobalReserver(uint8_t bRevolver);
		
		void Destruct() noexcept;
		void Release() noexcept;
		
		void Free(void* p) noexcept;
		void* Alloc(std::size_t s) noexcept;
	
	private:
		class Segment;
	
	
	private:
		std::array<Segment*, cnRealm> mapRealm;
};



}
