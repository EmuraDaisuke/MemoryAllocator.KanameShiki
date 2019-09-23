#pragma once



namespace KanameShiki {



class Base : private NonCopyable<Base> {
	public:
		virtual ~Base() noexcept = default;
		
		Base() = default;
		
		virtual std::size_t Size(Parcel* pParcel) const noexcept = 0;
		virtual void Free(Parcel* pParcel) noexcept = 0;
};



}
