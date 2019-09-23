#pragma once



#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>
#include <immintrin.h>

#ifdef _WIN32//[
#include <windows.h>
#include <tlhelp32.h>
#else//][
#include <sys/mman.h>
namespace std {
using atomic_int16_t = atomic<int16_t>;
using atomic_int64_t = atomic<int64_t>;
using atomic_uint16_t = atomic<uint16_t>;
using atomic_uint64_t = atomic<uint64_t>;
}
#endif//]



#include "./NonCopyable.h"
#include "./Macro.h"
#include "./Util.h"

#include "./Config.h"
#include "./Func.h"
#include "./Decl.h"
#include "./Lzc.h"

#include "./Tag.h"
#include "./Base.h"
#include "./Parcel.h"
#include "./Relay.h"

#include "./LocalPool.h"
#include "./LocalCram.h"
#include "./LocalAny.h"
#include "./LocalReserver.h"
#include "./LocalCntx.h"

#include "./GlobalHeap.h"
#include "./GlobalReserver.h"
#include "./GlobalCntx.h"
