#pragma once



#include <string>



class CLog final {
    public:
        ~CLog() noexcept;
        
        CLog();
        
        void Out() const noexcept;
        
        template <class Head, class... Tail>
        void Log(Head&& head, Tail&&... tail) noexcept
        {
            Put(head); Put();
            return Log(std::forward<Tail>(tail)...);
        }
    
    private:
        void Tail(int s) noexcept;
        char* Tail() const noexcept;
        
        void Put() noexcept;
        void Put(bool v) noexcept;
        void Put(int8_t v) noexcept;
        void Put(int16_t v) noexcept;
        void Put(int32_t v) noexcept;
        void Put(int64_t v) noexcept;
        void Put(uint8_t v) noexcept;
        void Put(uint16_t v) noexcept;
        void Put(uint32_t v) noexcept;
        void Put(uint64_t v) noexcept;
        void Put(float v) noexcept;
        void Put(double v) noexcept;
        void Put(const char* v) noexcept;
        void Put(const void* v) noexcept;
        void Put(const std::string& v) noexcept;
        
        void Log() noexcept;
    
    
    private:
        char maChar[4096];
        char* mpTail;
};



template <class... Args>
void clog(Args... args) noexcept
{
    CLog v;
    v.Log(args...);
    v.Out();
}
