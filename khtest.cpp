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
#define ASSERT_STUFF(hash_set)\
    assert(hash_set->n_buckets == khis->n_buckets);\
    assert(std::memcmp(hash_set->flags, khis->flags, __ac_fsize(hash_set->n_buckets) * sizeof(u32)) == 0);\
    assert(std::memcmp(hash_set->keys, khis->keys, sizeof(u64) * hash_set->n_buckets) == 0);\
    assert(hash_set->vals == nullptr);
    ASSERT_STUFF(kh3);
    std::fprintf(stderr, "size of khis: %zu\n", kh3.size());
    khis += kh3;
    khis.write("tmp.khs");
    khset64_t read_from_file;
    gzFile t = gzopen("tmp.khs", "rb");
    read_from_file.read(t);
    gzclose(t);
    ASSERT_STUFF(read_from_file);
    {
        std::vector<u64> vals;
        vals.reserve(read_from_file.size());
        size_t n_in_map = 0;
        for(khiter_t ki = 0; ki < kh_end(read_from_file); ++ki) {
            if(!kh_exist(read_from_file, ki)) continue;
            ++n_in_map;
            assert(khis.contains(kh_key(read_from_file, ki)));
        }
        assert(n_in_map == kh_size(read_from_file));
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
    kmap.write("Thisfile.txt");
    khset64_t m("Thisfile.txt");
    assert(m.size() == kmap.size());
    assert(m.capacity() == kmap.capacity());
    //m = khis;
}
