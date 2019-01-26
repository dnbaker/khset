# khset
Simple wrappers around khset

### Current Support
khset32_t (uint32_t)
khset64_t (uint64_t)
khset_cstr_t (const char *)

This takes ownership of strings and eliminates them at deconstruction. If you wish to remove these strings from the map earlier, you'll have to remove them from the table to avoid a double-free.
