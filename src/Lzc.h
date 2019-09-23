#pragma once



namespace KanameShiki {
namespace Lzc {



inline uint32_t Msb(uint32_t v){ return 31 - _lzcnt_u32(v); }
inline uint64_t Msb(uint64_t v){ return 63 - _lzcnt_u64(v); }
inline uint32_t Lsb(uint32_t v){ return _tzcnt_u32(v); }
inline uint64_t Lsb(uint64_t v){ return _tzcnt_u64(v); }



}
}
