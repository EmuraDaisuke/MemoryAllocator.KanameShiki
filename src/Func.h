#pragma once



namespace KanameShiki {



// System
void SystemFree(void* p, std::size_t s) noexcept;
void* SystemAlloc(std::size_t s) noexcept;

// Global
void GlobalHeapFree(void* p) noexcept;
void* GlobalHeapAlloc(std::size_t s) noexcept;

void GlobalReserverRelease() noexcept;
void GlobalReserverFree(void* p) noexcept;
void* GlobalReserverAlloc(std::size_t s) noexcept;

uint32_t NumReserver(uint16_t Realm) noexcept;

// Local
void LocalReserverFree(void* p) noexcept;
void* LocalReserverAlloc(std::size_t s) noexcept;

// Default
void Free(void* p) noexcept;
void* Alloc(std::size_t s) noexcept;
void* Align(std::size_t a, std::size_t s) noexcept;
void* ReAlloc(void* p, std::size_t s) noexcept;



}
