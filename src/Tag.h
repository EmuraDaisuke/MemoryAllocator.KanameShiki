#pragma once



namespace KanameShiki {



class Tag final : private NonCopyable<Tag> {
	public:
		~Tag() noexcept										= default;
		
		Tag(uint16_t Realm)									:Tag(Realm, false){}
		Tag(uint16_t Realm, bool bVirtual)					:mRealm(Realm),mbVirtual(bVirtual){}
		
		uint16_t Realm() const noexcept						{ return mRealm; }
		bool Virtual() const noexcept						{ return mbVirtual; }
		
		void* CastData() const noexcept						{ return offset_p(this, SizeofT()); }
		
		static Tag* CastTag(void* p) noexcept				{ return offset_p<Tag*>(p, -SizeofT()); }
		
		static constexpr std::size_t SizeofT() noexcept		{ return sizeof(Tag); }
		
		
		
		static uint16_t Realm(std::size_t s) noexcept
		{
			{	// 
				Auto sLimit = bit(cbFrac);
				s = (s > sLimit)? s: sLimit;
				
				Auto bAlign = Lzc::Msb(s);
				Auto sAlign = bit(bAlign - cbFrac);
				s = align_t(s, sAlign);
			}
			
			{	// 
				Auto bExpo = Lzc::Msb(s);
				Auto sFrac = s;
				sFrac >>= bExpo - cbFrac;
				sFrac &= cmFrac;
				
				uint16_t Realm = ((bExpo << cbFrac) | sFrac);
				return Realm - cnVoid;
			}
		}
		
		
		
		static std::size_t Size(uint16_t Realm) noexcept
		{
			Realm += cnVoid;
			
			std::size_t bExpo = Realm >> cbFrac;
			std::size_t sFrac = Realm & cmFrac;
			return bit(bExpo) | (sFrac << (bExpo - cbFrac));
		}
	
	
	private:
		const uint16_t mRealm;
		const bool mbVirtual;
};



}
