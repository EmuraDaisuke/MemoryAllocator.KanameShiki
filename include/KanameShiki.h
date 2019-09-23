#pragma once



namespace KanameShiki {



void Free(void* p) noexcept;
void* Alloc(std::size_t s) noexcept;
void* Align(std::size_t a, std::size_t s) noexcept;
void* ReAlloc(void* p, std::size_t s) noexcept;



}
