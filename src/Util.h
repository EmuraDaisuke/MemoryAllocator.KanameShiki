#pragma once



namespace KanameShiki {



// value
static constexpr uint64_t bit(uint8_t v) noexcept	{ return 1ULL<<v; }

static constexpr uint64_t KiB(uint64_t v) noexcept	{ return v<<10; }
static constexpr uint64_t MiB(uint64_t v) noexcept	{ return v<<20; }
static constexpr uint64_t GiB(uint64_t v) noexcept	{ return v<<30; }
static constexpr uint64_t TiB(uint64_t v) noexcept	{ return v<<40; }
static constexpr uint64_t PiB(uint64_t v) noexcept	{ return v<<50; }
static constexpr uint64_t EiB(uint64_t v) noexcept	{ return v<<60; }

// t
static constexpr std::size_t bound_t(std::size_t v, std::size_t b) noexcept					{ return v & -b; }
static constexpr std::size_t align_t(std::size_t v, std::size_t a) noexcept					{ return bound_t(v + (a-1), a); }

template <class T>	constexpr std::size_t to_t(T p) noexcept								{ return reinterpret_cast<std::size_t>(p); }

// p
template <class T = void*>	constexpr T align_p(const void* p, std::size_t a) noexcept		{ return reinterpret_cast<T>(align_t(to_t(p), a)); }
template <class T = void*>	constexpr T offset_p(const void* p, std::size_t o) noexcept		{ return reinterpret_cast<T>(to_t(p) + o); }



}
