#include "khset.h"
#include <vector>
#include <cassert>
#include <string>

using namespace kh;

int main(int argc, char* argv[]) {
    khset64_t khis;
    std::string arg = argc > 1 ? static_cast<const char *>(argv[1]): "10000";
    khis.reserve(std::stoi(arg));
    for(size_t i(0); i < std::stoi(arg); ++i)
        khis.insert(i);
 
    std::fprintf(stderr, "size of khis: %zu\n", khis.size());
    khset64_t kh3(khis);
    assert(kh_size(kh3) == kh_size(khis));
    assert(kh3->n_buckets == khis->n_buckets);
    
    //for(khiter_t ki(0); ki < kh_size(kh3); ++k) {
    //}
    assert(std::memcmp(kh3->flags, khis->flags, __ac_fsize(kh3->n_buckets) * sizeof(u32)) == 0);
    assert(std::memcmp(kh3->keys, khis->keys, sizeof(u64) * kh3->n_buckets) == 0);
    std::fprintf(stderr, "size of khis: %zu\n", kh3.size());
    khis += kh3;
    {
        std::vector<u64> vals;
        vals.reserve(khis.size());
        for(khiter_t ki = 0; ki < kh_end(khis); ++ki) {
            
        }
    }
    std::fprintf(stderr, "size of khis: %zu\n", khis.size());
    khset_cstr_t cset;
    cset.insert("Hello");
    cset.insert("World");
    cset.insert("Hi");
    cset.insert("How");
    cset.insert("Are");
    cset.insert("You");
    cset.for_each([](const char *s) {std::fprintf(stderr, "Set contains %s\n", s);});
    khmap_64_t kmap;
    kmap.reserve(10000);
    std::fprintf(stderr, "kmap capacity:%zu\n", kmap.capacity());
    kmap.insert(13, 1337);
    kmap.insert(1337, 13);
    kmap.for_each([](auto x, auto y) {std::fprintf(stderr, "Key %zu has value %zu\n", size_t(x), size_t(y));});
    kh_write(64, kmap, stdout);
    kh_serialize(64, kmap, "Thisfile.txt");
    auto m = kh_deserialize(64, "Thisfile.txt");
    ::std::free(m->keys);
    ::std::free(m->flags);
    ::std::free(m->vals);
}
