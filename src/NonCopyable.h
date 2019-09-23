#pragma once



namespace KanameShiki {



// Class

template <class T>
class NonCopyable {
	protected:
		~NonCopyable() noexcept = default;
		NonCopyable() = default;
	
	private:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator =(const NonCopyable&) = delete;
};



}
