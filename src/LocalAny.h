#pragma once



namespace KanameShiki {



uint64_t NumLocalAny() noexcept;



class LocalAny final : public Base, private NonCopyable<LocalAny> {
	public:
		~LocalAny() noexcept;
		
		LocalAny(std::size_t s);
		
		std::size_t Size(Parcel* pParcel) const noexcept;
		void Free(Parcel* pParcel) noexcept;
		void* Alloc() noexcept;
		
		void* operator new(std::size_t sThis, std::size_t s, const std::nothrow_t&) noexcept;
		
		void operator delete(void* p, std::size_t s, const std::nothrow_t&) noexcept		{ assert(false); }
		void operator delete(void* p) noexcept												{ assert(false); }
	
	
	private:
		std::thread::id mId;
		std::size_t ms;
};



}
