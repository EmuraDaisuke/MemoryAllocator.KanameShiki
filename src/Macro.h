#pragma once



#define Auto decltype(auto)

#ifndef numof//[
#define numof(x) (sizeof(x) / sizeof((x)[0]))
#endif//]
