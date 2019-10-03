


#if !defined(FILL)//[
#define FILL 0
#endif//]

#if !defined(SINGLE)//[
#define SINGLE 0
#endif//]

#if FILL
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>
#include <cstring>
#include <atomic>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include "./CLog.h"
#include "./Lapse.h"

#if MIMALLOC//[
#include "./mimalloc.h"
#elif TCMALLOC//][
#include "./tcmalloc.h"
#elif JEMALLOC//][
#include "./jemalloc.h"
#elif KANAMESHIKI//][
#include "./include/KanameShiki.h"
#endif//]



#if 0
#define CLOG(...)   clog(__VA_ARGS__)
#else
#define CLOG(...)
#endif



constexpr std::size_t bit(int b){ return (1ULL<<b); }
constexpr std::size_t size(int b){ return (b)? bit(b-1):0; }
constexpr std::size_t KiB(std::size_t v){ return (v<<10); }
constexpr std::size_t MiB(std::size_t v){ return (v<<20); }
constexpr std::size_t GiB(std::size_t v){ return (v<<30); }
uint8_t code(void* p){ return (reinterpret_cast<std::size_t>(p)>>4); }

#if SINGLE//[
static constexpr std::size_t T = bit(0);
#else//][
static constexpr std::size_t T = bit(3);
#endif//]

#if CATEGORY == 1
static constexpr std::size_t N = bit(5);
static constexpr std::size_t S = bit(10);
static constexpr std::size_t B0 = 0;
static constexpr std::size_t B1 = 10+1;
#elif CATEGORY == 2
static constexpr std::size_t N = bit(5);
static constexpr std::size_t S = bit(9);
static constexpr std::size_t B0 = 11+1;
static constexpr std::size_t B1 = 15+1;
#elif CATEGORY == 3
static constexpr std::size_t N = bit(5);
static constexpr std::size_t S = bit(8);
static constexpr std::size_t B0 = 16+1;
static constexpr std::size_t B1 = 20+1;
#elif CATEGORY == 4
static constexpr std::size_t N = bit(5);
static constexpr std::size_t S = bit(7);
static constexpr std::size_t B0 = 21+1;
static constexpr std::size_t B1 = 25+1;
#else
static constexpr std::size_t N = bit(5);
static constexpr std::size_t S = bit(7);
static constexpr std::size_t B0 = 0;
static constexpr std::size_t B1 = 25+1;
#endif



struct Value {
    std::size_t s;
    void* p;
};



class Thread final {
    public:
        using Func = void (*)(Value);
    
    public:
        ~Thread() noexcept
        {
            mb.store(false, std::memory_order_release);
            Call(nullptr, Value{});
            if (mt.joinable()) mt.join();
        }
        
        Thread()
        :mb(true)
        ,mf(0)
        ,mv{}
        ,mt(&Thread::Work, this)
        {}
        
        void Wait() noexcept
        {
            std::unique_lock<std::mutex> Lock(mMutexWait);
            while (mf.load(std::memory_order_acquire)) mWait.wait(Lock);
        }
        
        void Call(Func f, Value v) noexcept
        {
            Wait();
            
            {   // 
                std::lock_guard<std::mutex> Lock(mMutexCall);
                mf.store(reinterpret_cast<std::size_t>(f), std::memory_order_release);
                mv.store(v, std::memory_order_release);
                mCall.notify_one();
            }
        }
    
    private:
        void Work() noexcept
        {
            bool b;
            std::size_t f;
            do {
                {   // 
                    std::unique_lock<std::mutex> Lock(mMutexCall);
                    while ((b = mb.load(std::memory_order_acquire)) && (f = mf.load(std::memory_order_acquire)) == 0) mCall.wait(Lock);
                }
                
                if (f){
                    reinterpret_cast<Thread::Func>(f)(mv.load(std::memory_order_acquire));
                    mf.store(0, std::memory_order_release);
                }
                
                {   // 
                    std::lock_guard<std::mutex> Lock(mMutexWait);
                    mWait.notify_one();
                }
            } while (b);
        }
    
    
    private:
        alignas(64) std::mutex mMutexWait;
        alignas(64) std::mutex mMutexCall;
        alignas(64) std::condition_variable mWait;
        alignas(64) std::condition_variable mCall;
        
        alignas(64) std::atomic_bool mb;
        alignas(64) std::atomic_size_t mf;
        alignas(64) std::atomic<Value> mv;
        std::thread mt;
};



alignas(64) std::atomic_int snNullptr;

void NullptrReset()     { snNullptr.store(0); }
void NullptrInc()       { snNullptr.fetch_add(1); }
int NullptrNum()        { return snNullptr.load(); }



using ArrayT = std::array<Thread, T>;
using ArrayV = std::array<Value, S>;



void Alloc(Value& rv, std::size_t s)
{
    {   // 
        rv.s = s;
        #if MIMALLOC
        rv.p = mi_malloc(rv.s);
        #elif TCMALLOC
        rv.p = tc_malloc(rv.s);
        #elif JEMALLOC
        rv.p = je_malloc(rv.s);
        #elif KANAMESHIKI
        rv.p = KanameShiki::Alloc(rv.s);
        #else
        rv.p = malloc(rv.s);
        #endif
        CLOG("Alloc", rv.p, rv.s);
        if (!rv.p) NullptrInc();
    }
    
    #if FILL
    if (rv.p){
        auto c = code(rv.p);
        std::memset(rv.p, c, rv.s);
    }
    #endif
}



void Free(Value v)
{
    #if FILL
    if (v.p){
        auto s = v.s;
        auto p = static_cast<uint8_t*>(v.p);
        auto c = code(v.p);
        bool b = true;
        for (; s; --s, ++p) b &= (c == *p);
        assert(b);
    }
    #endif
    
    {   // 
        #if MIMALLOC
        mi_free(v.p);
        #elif TCMALLOC
        tc_free(v.p);
        #elif JEMALLOC
        je_free(v.p);
        #elif KANAMESHIKI
        KanameShiki::Free(v.p);
        #else
        free(v.p);
        #endif
        CLOG("Free ", v.p, v.s);
    }
}



void testA(std::size_t s)
{
    ArrayV av;
//  Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        for (auto& v : av){ Alloc(v, s); Free(v); }
    }
}



void testB(std::size_t s)
{
    ArrayV av;
//  Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        for (auto& v : av) Alloc(v, s);
        for (auto& v : av) Free(v);
    }
}



void testC(std::size_t s)
{
    ArrayV av;
//  Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        for (auto& v : av) Alloc(v, s);
        for (auto iv = av.rbegin(); iv != av.rend(); ++iv) Free(*iv);
    }
}



void testD(std::size_t s)
{
    ArrayV av;
//  Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        bool e;
        for (auto& v : av) Alloc(v, s);
        e = true;   for (auto& v : av){ if (e) Free(v); e = !e; }
        e = false;  for (auto& v : av){ if (e) Free(v); e = !e; }
    }
}



void testE(std::size_t s)
{
    ArrayV av;
//  Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        bool e;
        for (auto& v : av) Alloc(v, s);
        e = true;   for (auto& v : av){ if (e) Free(v); e = !e; }
        e = true;   for (auto& v : av){ if (e) Alloc(v, s); e = !e; }
        e = false;  for (auto& v : av){ if (e) Free(v); e = !e; }
        e = true;   for (auto& v : av){ if (e) Free(v); e = !e; }
    }
}



void testF(std::size_t s)
{
    ArrayT at;
    ArrayV av;
    Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        std::size_t ot = 0;
        for (auto& v : av){
            Alloc(v, s);
            at[ot].Call(Free, v);
            ot = ++ot % at.size();
        }
    }
    for (auto& t : at) t.Wait();
}



void testG(std::size_t s)
{
    ArrayT at;
    ArrayV av;
    Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        for (auto& v : av) Alloc(v, s);
        
        std::size_t ot = 0;
        for (auto& v : av){
            at[ot].Call(Free, v);
            ot = ++ot % at.size();
        }
    }
    for (auto& t : at) t.Wait();
}



void testH(std::size_t s)
{
    Value v;
    ArrayT at;
    std::array<ArrayV, T> aav;
    Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        for (auto& av : aav){
            for (auto& v : av) Alloc(v, s);
        }
        
        std::size_t ot = 0;
        for (auto& av : aav){
            v.p = &av;
            at[ot++].Call([](Value pav){
                auto& av = *reinterpret_cast<ArrayV*>(pav.p);
                for (auto& v : av) Free(v);
            }, v);
        }
        for (auto& t : at) t.Wait();
    }
}



void testI(std::size_t s)
{
    Value v;
    ArrayT at;
    ArrayV av;
    Lapse l(__FUNCTION__, s);
    for (auto n = N; n; --n){
        {   // 
            v.s = s;
            v.p = &av;
            std::thread t([](Value& rv){
                auto& av = *reinterpret_cast<ArrayV*>(rv.p);
                for (auto& v : av) Alloc(v, rv.s);
            }, std::ref(v));
            if (t.joinable()) t.join();
        }
        
        for (auto& t : at){
            t.Call([](Value cv){
                ArrayV av;
                for (auto& v : av) Alloc(v, cv.s);
                for (auto& v : av) Free(v);
            }, v);
        }
        
        std::size_t ot = 0;
        for (auto& v : av){
            at[ot].Call(Free, v);
            ot = ++ot % at.size();
        }
        
        for (auto& t : at){
            t.Call([](Value cv){
                ArrayV av;
                for (auto& v : av) Alloc(v, cv.s);
                for (auto& v : av) Free(v);
            }, v);
        }
    }
    for (auto& t : at) t.Wait();
}



void testA(Value v)
{
    testA(v.s);
}



void testB(Value v)
{
    testB(v.s);
}



void testC(Value v)
{
    testC(v.s);
}



void testD(Value v)
{
    testD(v.s);
}



void testE(Value v)
{
    testE(v.s);
}



int main(int argc, char* argv[])
{
    {   // 
        Value v;
        ArrayT at;
        Lapse l("total", 0);
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                v.s = size(b);
                Lapse l("testA", v.s);
                for (auto& t : at) t.Call(testA, v);
                for (auto& t : at) t.Wait();
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                v.s = size(b);
                Lapse l("testB", v.s);
                for (auto& t : at) t.Call(testB, v);
                for (auto& t : at) t.Wait();
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                v.s = size(b);
                Lapse l("testC", v.s);
                for (auto& t : at) t.Call(testC, v);
                for (auto& t : at) t.Wait();
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                v.s = size(b);
                Lapse l("testD", v.s);
                for (auto& t : at) t.Call(testD, v);
                for (auto& t : at) t.Wait();
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                v.s = size(b);
                Lapse l("testE", v.s);
                for (auto& t : at) t.Call(testE, v);
                for (auto& t : at) t.Wait();
            }
            clog(NullptrNum());
        }
        
        #if !SINGLE//[
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                testF(size(b));
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                testG(size(b));
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                testH(size(b));
            }
            clog(NullptrNum());
        }
        
        {   // 
            NullptrReset();
            for (auto b = B0; b <= B1; ++b){
                testI(size(b));
            }
            clog(NullptrNum());
        }
        #endif//]
    }
    
    clog("***");
    return 0;
}
