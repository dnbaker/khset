#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include "klib/khash.h"


namespace kh {

/*

Complete: khset32_t, khset64_t
TODO: khsetstr_t
TODO: khmap, esp w.r.t. moving objects. (This will likely require updating khash code directly.)

*/

KHASH_SET_INIT_INT(set)
KHASH_SET_INIT_INT64(set64)
KHASH_SET_INIT_STR(cs)

using u32 = ::std::uint32_t;
using u64 = ::std::uint64_t;

struct EmptyKhBase {};
struct EmptyKhSet: public EmptyKhBase {};
struct EmptyKhMap: public EmptyKhBase {};

#define IS_KH_DEC(kh, type) \
template<typename T>\
struct is_##kh {\
    static constexpr bool value = std::is_base_of_v<type, T>;\
};\
template<typename T> inline constexpr bool is_##kh##_v = is_##kh<T>::value;

IS_KH_DEC(kh, EmptyKhBase)
IS_KH_DEC(set, EmptyKhSet)
IS_KH_DEC(map, EmptyKhMap)
template<typename Key, class Compare, class Allocator>
struct is_set<std::set<Key, Compare, Allocator>> {static constexpr bool value = true;};
template<typename Key, class Compare, class Hash, class Allocator>
struct is_set<std::unordered_set<Key, Compare, Hash, Allocator>> {static constexpr bool value = true;};
template<typename Key, typename T, class Compare, class Allocator>
struct is_map<std::map<Key, T, Compare, Allocator>> {static constexpr bool value = true;};
template<typename Key, typename T, class Hash, class Compare, class Allocator>
struct is_map<std::unordered_map<Key, T, Hash, Compare, Allocator>> {static constexpr bool value = true;};
#undef IS_KH_DEC
#undef IS_KH_DEC

#define __FE__ \
    template<typename Functor>\
    void for_each(const Functor &func) {\
        for(khiter_t ki = 0; ki < this->n_buckets; ++ki)\
            if(kh_exist(this, ki))\
                func(this->keys[ki]);\
    }\
    template<typename Functor>\
    void for_each(const Functor &func) const {\
        for(khiter_t ki = 0; ki < this->n_buckets; ++ki)\
            if(kh_exist(this, ki))\
                func(this->keys[ki]);\
    }


// Steal everything, take no prisoners.
#define KH_MOVE_DEC(t) \
   t(t &&other) {std::memcpy(this, &other, sizeof(*this)); std::memset(&other, 0, sizeof(other));}

#define KH_COPY_DEC(t) \
    t(const t &other) {\
        if(other.size()) {\
            std::memcpy(this, &other, sizeof(*this));\
            auto memsz = other.capacity() * sizeof(*keys);\
            keys = static_cast<decltype(keys)>(std::malloc(memsz));\
            if(!keys) throw std::bad_alloc();\
            std::memcpy(keys, other.keys, memsz);\
            memsz = __ac_fsize(other.capacity()) * sizeof(u32);\
            flags = static_cast<u32 *>(std::malloc(memsz));\
            if(!flags) throw std::bad_alloc();\
            std::memcpy(flags, other.flags, memsz);\
            if constexpr(::kh::is_map_v<std::decay_t<decltype(*this)>>)\
                std::memcpy(vals, other.vals, other.capacity() * sizeof(*vals));\
        } else std::memset(this, 0, sizeof(*this));\
    }



#define DECLARE_KHSET(name, nbits) \
struct khset##nbits##_t: EmptyKhSet, khash_t(name) {\
    khset##nbits##_t() {std::memset(this, 0, sizeof(*this));}\
    khset##nbits##_t(size_t reserve_size) {std::memset(this, 0, sizeof(*this)); reserve(reserve_size);}\
    ~khset##nbits##_t() {std::free(this->flags); std::free(this->keys);}\
    KH_COPY_DEC(khset##nbits##_t)\
    KH_MOVE_DEC(khset##nbits##_t)\
    /* For each*/ \
    __FE__\
    void swap(khset##nbits##_t &other) {\
        std::swap_ranges(reinterpret_cast<uint8_t *>(this), reinterpret_cast<uint8_t *>(this) + sizeof(*this), reinterpret_cast<uint8_t *>(std::addressof(other)));\
    }\
    operator khash_t(name) &() {return *reinterpret_cast<khash_t(name) *>(this);}\
    operator khash_t(name) *() {return reinterpret_cast<khash_t(name) *>(this);}\
    operator const khash_t(name) &() const {return *reinterpret_cast<const khash_t(name) *>(this);}\
    operator const khash_t(name) *() const {return reinterpret_cast<const khash_t(name) *>(this);}\
    khash_t(name) *operator->() {return static_cast<khash_t(name) *>(*this);}\
    const khash_t(name) *operator->() const {return static_cast<const khash_t(name) *>(*this);}\
    auto insert(u##nbits val) {\
        int khr;\
        return kh_put(name, this, val, &khr);\
    }\
    auto get(u##nbits x) const {return kh_get(name, this, x);}\
    auto erase(u##nbits val) {\
        if(auto it = get(val); it != capacity())\
            kh_del(name, this, it);\
    }\
    template<typename ItType, typename ItType2>\
    void insert(ItType i1, ItType2 i2) {\
        while(i1 != i2) insert(*i1++);\
    }\
    void clear() {kh_clear(name, this);}\
    bool contains(u##nbits x) const {return get(x) != kh_end(this);}\
    size_t size() const {return kh_size(static_cast<const khash_t(name) *>(this));}\
    size_t capacity() const {return this->n_buckets;}\
    void reserve(size_t sz) {if(kh_resize(name, this, sz) < 0) throw std::bad_alloc();}\
}; \
void swap(khset##nbits##_t &a, khset##nbits##_t &b) {\
    a.swap(b);\
}

DECLARE_KHSET(set, 32)
DECLARE_KHSET(set64, 64)
#undef DECLARE_KHSET
struct khset_cstr_t: EmptyKhSet, khash_t(cs) {
    khset_cstr_t() {std::memset(this, 0, sizeof(*this));}
    ~khset_cstr_t() {
        this->for_each([](const char *s) {std::free(const_cast<char *>(s));});
        std::free(this->flags); std::free(this->keys);
    }
    KH_COPY_DEC(khset_cstr_t)
    KH_MOVE_DEC(khset_cstr_t)
    /* For each*/
    __FE__
    operator khash_t(cs) &() {return *reinterpret_cast<khash_t(cs) *>(this);}
    operator khash_t(cs) *() {return reinterpret_cast<khash_t(cs) *>(this);}
    operator const khash_t(cs) &() const {return *reinterpret_cast<const khash_t(cs) *>(this);}
    operator const khash_t(cs) *() const {return reinterpret_cast<const khash_t(cs) *>(this);}
    khash_t(cs) *operator->() {return static_cast<khash_t(cs) *>(*this);}
    const khash_t(cs) *operator->() const {return static_cast<const khash_t(cs) *>(*this);}
    auto insert_move(const char *s) {
        // Takes ownership
        int khr;
        auto ret = kh_put(cs, this, s, &khr);
        switch(khr) {
            case -1: throw std::bad_alloc();
            case 0: break; // Present: do nothing.
            case 1: case 2: this->keys[ret] = s; break;
            default: __builtin_unreachable();
        }
        return ret;
    }
    auto get(const char *s) const {
        return kh_get(cs, this, s);
    }
    auto del(const char *s) {
        if(auto it = get(s); it != capacity()) std::free(const_cast<char *>(this->keys[it]));
    }
    khiter_t insert(const char *s) {return this->insert(s, std::strlen(s));}
    khiter_t insert(const char *s, size_t l) {
        // Copies
        int khr;
        auto ret = kh_put(cs, this, s, &khr);
        const char *tmp;
        switch(khr) {
            case -1: throw std::bad_alloc();
            case 0: break; // Present: do nothing.
            case 1: case 2: if((tmp = static_cast<const char *>(std::malloc(l + 1))) == nullptr) throw std::bad_alloc();
                std::memcpy(const_cast<char *>(tmp), this->keys[ret], l);
                this->keys[ret] = tmp;
                const_cast<char *>(this->keys[ret])[l] = '\0';
                break;
            default: __builtin_unreachable();
        }
        return ret;
    }
    template<typename ItType, typename ItType2>
    void insert(ItType i1, ItType2 i2) {
        while(i1 < i2) insert(*i1++);
    }
    void clear() {kh_clear(cs, this);}
    bool contains(const char *s) const {return kh_get(cs, this, s) != kh_end(this);}
    size_t size() const {return kh_size(static_cast<const khash_t(cs) *>(this));}
    size_t capacity() const {return this->n_buckets;}
    void reserve(size_t sz) {if(__builtin_expect(kh_resize(cs, this, sz) < 0, 0)) throw std::bad_alloc();}
};

#define KHASH_MAP_INIT_INT32 KHASH_SET_INIT_INT

/*
Note:
*/
#define DECLARE_KHMAP(name, VType, init_statement, nbits) \
\
init_statement(name, VType)\
struct khmap_##name##_t: EmptyKhSet, khash_t(name) {\
    using value_type = std::decay_t<decltype(*vals)>;\
    khmap_##name##_t() {std::memset(this, 0, sizeof(*this));}\
    ~khmap_##name##_t() {\
        if constexpr(!std::is_trivially_destructible_v<value_type>) {\
            /* call destructors if necessary */\
            this->for_each_val([](auto &v) {v.~value_type();});\
        }\
        std::free(this->flags); std::free(this->keys); std::free(this->vals);\
    }\
    KH_COPY_DEC(khmap_##name##_t)\
    KH_MOVE_DEC(khmap_##name##_t)\
    /* For each*/ \
    template<typename Func>\
    void for_each(const Func &func) {\
        /* Give up the funk! */ \
        /* We gotta have that funk!! */ \
        for(khiter_t ki = 0; ki < this->n_buckets; ++ki)\
            if(kh_exist(this, ki))\
                func(this->keys[ki], this->vals[ki]);\
    }\
    template<typename Func>\
    void for_each_key(const Func &func) {\
        /* Give up the funk! */ \
        /* We gotta have that funk!! */ \
        for(khiter_t ki = 0; ki < this->n_buckets; ++ki)\
            if(kh_exist(this, ki))\
                func(this->keys[ki]);\
    }\
    void destroy_at(khint_t ki) {throw std::runtime_error("NotImplemented");}\
    void del(khint_t ki) {\
        destroy_at(ki);\
        return kh_del_##name(this, ki);\
    }\
    template<typename Func>\
    void for_each_val(const Func &func) {\
        /* Give up the funk! */ \
        /* We gotta have that funk!! */ \
        for(khiter_t ki = 0; ki < this->n_buckets; ++ki)\
            if(kh_exist(this, ki))\
                func(this->vals[ki]);\
    }\
    void insert(u##nbits key, const VType &val) {\
        int khr;\
        auto ret = kh_put(name, this, key, &khr);\
        switch(khr) {\
            case 0: break; /* already present */ \
            case 1: case 2: break; /* empty or deleted */ \
            case -1: throw std::bad_alloc(); \
            default: __builtin_unreachable();\
        }\
        vals[ret] = val;\
    }\
    \
    operator khash_t(name) &() {return *reinterpret_cast<khash_t(name) *>(this);}\
    operator khash_t(name) *() {return reinterpret_cast<khash_t(name) *>(this);}\
    operator const khash_t(name) &() const {return *reinterpret_cast<const khash_t(name) *>(this);}\
    operator const khash_t(name) *() const {return reinterpret_cast<const khash_t(name) *>(this);}\
    khash_t(name) *operator->() {return static_cast<khash_t(name) *>(*this);}\
    const khash_t(name) *operator->() const {return static_cast<const khash_t(name) *>(*this);}\
    void clear() {kh_clear(name, this);}\
    bool contains(u##nbits x) const {return kh_get(name, this, x) != kh_end(this);}\
    size_t size() const {return kh_size(static_cast<const khash_t(name) *>(this));}\
    size_t capacity() const {return this->n_buckets;}\
    void reserve(size_t sz) {if(kh_resize(name, this, sz) < 0) throw std::bad_alloc();}\
};

#define DECLARE_KHMAP_32(name, VType) DECLARE_KHMAP(name, VType, KHASH_MAP_INIT_INT32, 32)
#define DECLARE_KHMAP_64(name, VType) DECLARE_KHMAP(name, VType, KHASH_MAP_INIT_INT64, 64)

DECLARE_KHMAP_64(64, ::std::uint64_t)


template<typename T>
size_t capacity(const T &a) {return a.capacity();} // Can be specialized later.

template<typename T> T&operator+=(T &a, const T &b) {
   b.for_each([&](auto k){a.insert(k);});
   return a;
}

#undef KH_MOVE_DEC
#undef KH_COPY_DEC

} // namespace kh