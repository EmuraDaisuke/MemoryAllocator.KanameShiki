#pragma once



namespace KanameShiki {



class GlobalReserver final : private NonCopyable<GlobalReserver> {
	public:
		~GlobalReserver() noexcept			= default;
		
		GlobalReserver()					= default;
		GlobalReserver(bool bInit);
		
		void Destruct() noexcept;
		void Release() noexcept;
		
		void Free(void* p) noexcept;
		void* Alloc(std::size_t s) noexcept;
	
	private:
		class Segment;
		
		uint16_t NumReserver(uint16_t Realm) const noexcept;
	
	
	private:
		std::array<Segment*, cnRealm> mapRealm;
};



}
