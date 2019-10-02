#pragma once



namespace KanameShiki {



// System
void SystemFree(void* p, std::size_t s) noexcept;
void* SystemAlloc(std::size_t s) noexcept;

uint16_t SystemRevolver() noexcept;

// Global
GlobalCntx* GlobalCntxPtr() noexcept;

// Local
LocalCntx* LocalCntxPtr() noexcept;

// Default
void Free(void* p) noexcept;
void* Alloc(std::size_t s) noexcept;
void* Align(std::size_t a, std::size_t s) noexcept;
void* ReAlloc(void* p, std::size_t s) noexcept;
std::size_t Size(void* p) noexcept;



}
