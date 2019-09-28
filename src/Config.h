#pragma once



namespace KanameShiki {



#ifndef NDEBUG//[
#define KANAMESHIKI_DEBUG_LEVEL 1	// 0:Disable 1:Loose 2:Strict
// The timing at which the thread instance is actually deleted seems to vary depending on the development environment.
// There seems to be a case where an excessively created thread of unknown cause leaks apart from the thread explicitly created by the application.
#endif//]

#ifndef KANAMESHIKI_HEAP_SPECIALIZATION//[
#define KANAMESHIKI_HEAP_SPECIALIZATION 1	// 0:Disable 1:Enable
#endif//]



// 
static constexpr uint8_t cbMemory = /*GiB*/30 + /*64*/6;	// 64GiB

static constexpr std::size_t cbSizeT = 3;
static constexpr std::size_t csSizeT = bit(cbSizeT);

static constexpr std::size_t cbCacheLine = 6;
static constexpr std::size_t csCacheLine = bit(cbCacheLine);
static constexpr std::size_t cmCacheLine = csCacheLine-1;

static constexpr std::size_t cbAlign = 4;
static constexpr std::size_t csAlign = bit(cbAlign);


// 
static constexpr uint16_t cbFrac = 4;
static constexpr uint16_t cnFrac = bit(cbFrac);
static constexpr uint16_t cmFrac = cnFrac-1;

static constexpr uint16_t cnExpo = cbMemory;
static constexpr uint16_t cnVoid = cbFrac << cbFrac;
static constexpr uint16_t cnRealm = (cnExpo << cbFrac) - cnVoid;


// 
static constexpr auto cbPool = 10;
static constexpr Auto csPool = bit(cbPool);
static constexpr Auto cnPool = csPool / csSizeT;

static constexpr auto cbPoolParcel = 5;
static constexpr Auto cnPoolParcel = bit(cbPoolParcel);


// 
static constexpr auto cbCram = 20;
static constexpr Auto csCram = bit(cbCram);

static constexpr auto cbCramParcel = 4;
static constexpr Auto cnCramParcel = bit(cbCramParcel);


// 
static constexpr std::size_t cbGlobalReserver = /*GiB*/30 + /*1*/0;		// 1GiB
static constexpr std::size_t csGlobalReserver = bit(cbGlobalReserver);

static constexpr std::size_t cbLocalReserver = /*MiB*/20 + /*1*/0;		// 1MiB
static constexpr std::size_t csLocalReserver = bit(cbLocalReserver);


// 
#if KANAMESHIKI_HEAP_SPECIALIZATION//[
static constexpr std::size_t cbGlobalHeap = /*GiB*/30 + /*8*/3;		// 8GiB
#else//][
static constexpr std::size_t cbGlobalHeap = /*MiB*/20 + /*8*/3;		// 8MiB
#endif//]
static constexpr std::size_t csGlobalHeap = bit(cbGlobalHeap);



}
