#pragma once
#include <cstdint>
#include <type_traits>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <string_view>
#include <unordered_map>
#ifdef BUF_CHK
#define DO_BUF_CHK() assert(datamax > data)
#define BUF_CHK_VAR uintptr_t datamax
#else
#define DO_BUF_CHK()
#define BUF_CHK_VAR
#endif
using std::string, std::string_view;
template <class T>
struct is_safe_obj : std::integral_constant<bool,
    !std::is_class<T>::value>
{
};

typedef unsigned int bsize_t;
class RBStream
{
public:
    uintptr_t data;
    BUF_CHK_VAR;
    RBStream(void* dat, size_t len)
    {
        data = (uintptr_t)dat;
#ifdef BUF_CHK
        datamax = data;
        datamax += len;
#endif
    }
    RBStream(string_view x) noexcept {
        *this = { (void*)x.data(), (size_t)x.size() };
    }

private:
    template<typename T1, typename T2>
    void __get(std::unordered_map<T1, T2>& x) {
        bsize_t sz;
        __get(sz);
        x.reserve(sz);
        for (bsize_t i = 0; i < sz; ++i)
        {
            T1 local;
            T2 local2;
            __get(local);
            __get(local2);
            x.emplace(local, local2);
        }
    }
    template <typename T1>
    void __get(std::vector<T1>& x)
    {
        bsize_t sz;
        __get(sz);
        x.reserve(sz);
        for (bsize_t i = 0; i < sz; ++i)
        {
            T1 local;
            __get(local);
            x.push_back(local);
        }
    }
    template <typename T1>
    void __get(std::list<T1>& x)
    {
        bsize_t sz;
        __get(sz);
        for (bsize_t i = 0; i < sz; ++i)
        {
            T1 local;
            __get(local);
            x.push_back(local);
        }
    }
    void __get(string& x)
    {
        bsize_t sz;
        __get(sz);
        x.reserve(sz);
        x.append((const char*)data, sz);
        data += sz;
        DO_BUF_CHK();
    }
    template <typename T>
    void __get(T& x)
    {
        if constexpr (is_safe_obj<T>())
        {
            static_assert(!std::is_reference<T>());
            memcpy(&x, (void*)data, sizeof(x));
            data += sizeof(T);
            DO_BUF_CHK();
        }
        else
        {
            x.unpack(*this);
        }
    }

public:
    template <typename... T>
    void apply(T&&... args) noexcept
    {
        (__get(std::forward<T>(args)), ...);
    }
    void read(void* dst, size_t n)
    {
        memcpy(dst, (void*)data, n);
        data += n;
        DO_BUF_CHK();
    }
};
template<typename container>
class WBStreamImpl
{
public:
    container data;

private:
    template<typename T1, typename T2>
    void __put(const std::unordered_map<T1, T2>& x) {
        bsize_t sz = x.size();
        __put(sz);
        for (auto& [k, v] : x) {
            __put(k);
            __put(v);
        }
    }
    template <typename T2>
    void __put(const std::vector<T2>& x)
    {
        bsize_t sz = x.size();
        __put(sz);
        for (auto i = x.begin(); i != x.end(); ++i)
        {
            __put(*i);
        }
    }
    template <typename T2>
    void __put(const std::list<T2>& x)
    {
        bsize_t sz = x.size();
        __put(sz);
        for (auto i = x.begin(); i != x.end(); ++i)
        {
            __put(*i);
        }
    }
    void __put(const string& x)
    {
        __put((bsize_t)x.size());
        data.append(x);
    }
    void __put(const string_view x)
    {
        __put((bsize_t)x.size());
        data.append(x);
    }
    template <typename T>
    void __put(T&& x)
    {
        if constexpr (is_safe_obj<T>())
        {
            data.append((const char*)&x, sizeof(T));
        }
        else
        {
            x.pack(*this);
        }
    }

public:
    WBStreamImpl() {}
    WBStreamImpl(container&& x) :data(x) {}
    template <typename... T>
    void apply(T&&... args) noexcept
    {
        (__put(std::forward<T>(args)), ...);
    }
    void write(const void* src, size_t n)
    {
        data.append((const char*)src, n);
    }
    operator string_view() {
        return data;
    }
};
using WBStream = WBStreamImpl<std::string>;