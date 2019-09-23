#pragma once



namespace KanameShiki {



class Relay final : public Base, private NonCopyable<Relay> {
	public:
		~Relay() noexcept									= default;
		
		Relay(void* p, std::size_t s)						:mp(p),ms(s){}
		
		std::size_t Size(Parcel* pParcel) const noexcept	{ return ms; }
		void Free(Parcel* pParcel) noexcept					{ Auto pRelay = CastRelay(pParcel); KanameShiki::Free(pRelay->mp); }
		
		void* CastData() const noexcept						{ return offset_p(this, SizeofT()); }
		
		static Relay* CastRelay(void* p) noexcept			{ return offset_p<Relay*>(p, -SizeofT()); }
		
		static constexpr std::size_t SizeofT() noexcept		{ return sizeof(Relay); }
	
	
	private:
		void* mp;
		std::size_t ms;
};



}
