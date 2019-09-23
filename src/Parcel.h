#pragma once



namespace KanameShiki {



class Parcel final : private NonCopyable<Parcel> {
	public:
		~Parcel() noexcept									= default;
		
		Parcel()											= default;
		
		Base* Owner() const noexcept						{ return mpOwner; }
		
		void Free(Parcel* pNext) noexcept					{ mpOwner = nullptr; mpNext = pNext; }
		Parcel* Alloc(Base* pOwner) noexcept				{ mpOwner = pOwner; return mpNext; }
		
		void* CastData() const noexcept						{ return offset_p(this, SizeofT()); }
		
		static Parcel* CastParcel(void* p) noexcept			{ return offset_p<Parcel*>(p, -SizeofT()); }
		
		static constexpr std::size_t SizeofT() noexcept					{ return offsetof(Parcel, mpNext); }
		static constexpr std::size_t SizeofS(std::size_t s) noexcept	{ return align_t(SizeofT() + s, csAlign); }
	
	
	private:
		static Parcel Invalid;
	
	public:
		static constexpr Parcel* const cpInvalid = &Invalid;
	
	private:
		Base* mpOwner;
		Parcel* mpNext;		// Free:mpNext or Alloc:Data
};



}
