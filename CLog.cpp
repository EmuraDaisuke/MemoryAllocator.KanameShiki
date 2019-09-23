


#include <cstdio>
#include "./CLog.h"

#ifdef _WIN32//[
#define PUT_BOOL "%d"
#define PUT_INT8_T "%hhd"
#define PUT_INT16_T "%hd"
#define PUT_INT32_T "%ld"
#define PUT_INT64_T "%lld"
#define PUT_UINT8_T "%hhu"
#define PUT_UINT16_T "%hu"
#define PUT_UINT32_T "%lu"
#define PUT_UINT64_T "%llu"
#else//][
#define PUT_BOOL "%d"
#define PUT_INT8_T "%hhd"
#define PUT_INT16_T "%hd"
#define PUT_INT32_T "%d"
#define PUT_INT64_T "%ld"
#define PUT_UINT8_T "%hhu"
#define PUT_UINT16_T "%hu"
#define PUT_UINT32_T "%u"
#define PUT_UINT64_T "%lu"
#endif//]



CLog::~CLog() noexcept                          {}

CLog::CLog()                                    :mpTail(maChar){}

void CLog::Tail(int s) noexcept                 { mpTail += s; }
char* CLog::Tail() const noexcept               { return mpTail; }

void CLog::Out() const noexcept                 { std::puts(maChar); std::fflush(stdout); }

void CLog::Put() noexcept                       { Tail(std::sprintf(Tail(), " ")); }
void CLog::Put(bool v) noexcept                 { Tail(std::sprintf(Tail(), PUT_BOOL, (v)? 1:0)); }
void CLog::Put(int8_t v) noexcept               { Tail(std::sprintf(Tail(), PUT_INT8_T, v)); }
void CLog::Put(int16_t v) noexcept              { Tail(std::sprintf(Tail(), PUT_INT16_T, v)); }
void CLog::Put(int32_t v) noexcept              { Tail(std::sprintf(Tail(), PUT_INT32_T, v)); }
void CLog::Put(int64_t v) noexcept              { Tail(std::sprintf(Tail(), PUT_INT64_T, v)); }
void CLog::Put(uint8_t v) noexcept              { Tail(std::sprintf(Tail(), PUT_UINT8_T, v)); }
void CLog::Put(uint16_t v) noexcept             { Tail(std::sprintf(Tail(), PUT_UINT16_T, v)); }
void CLog::Put(uint32_t v) noexcept             { Tail(std::sprintf(Tail(), PUT_UINT32_T, v)); }
void CLog::Put(uint64_t v) noexcept             { Tail(std::sprintf(Tail(), PUT_UINT64_T, v)); }
void CLog::Put(float v) noexcept                { Tail(std::sprintf(Tail(), "%1.8f", v)); }
void CLog::Put(double v) noexcept               { Tail(std::sprintf(Tail(), "%1.8f", v)); }
void CLog::Put(const char* v) noexcept          { Tail(std::sprintf(Tail(), "%s", (v)? v:"-nullstr-")); }
void CLog::Put(const void* v) noexcept          { Tail(std::sprintf(Tail(), "%p", v)); }
void CLog::Put(const std::string& v) noexcept   { Tail(std::sprintf(Tail(), "%s", v.c_str())); }

void CLog::Log() noexcept                       { Tail()[0] = 0; }
