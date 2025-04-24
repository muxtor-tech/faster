#include <stdio.h>

#include "aster/faster_is.h"

int main() {
    FASTER_DECLARE_FASTER_STR(key1, "key1");
    FASTER_DECLARE_FASTER_STR(key2, "key2");
    FASTER_DECLARE_FASTER_STR(key3, "key3");
    FASTER_DECLARE_FASTER_STR(key4, "key4");
    FASTER_DECLARE_FASTER_STR(key5, "key5");

    faster_interned_strings_t interned_strings;
    faster_interned_strings_init(&interned_strings);

    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key1, FAST_INTERNED_STRING_PURPOSE_TEXT);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key2, FAST_INTERNED_STRING_PURPOSE_VNAME);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key3, FAST_INTERNED_STRING_PURPOSE_FNAME);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key4, FAST_INTERNED_STRING_PURPOSE_OPERATOR);

    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key2, FAST_INTERNED_STRING_PURPOSE_SYMBOL);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key3, FAST_INTERNED_STRING_PURPOSE_SYMBOL);

    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key4, FAST_INTERNED_STRING_PURPOSE_SYMBOL);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key4, FAST_INTERNED_STRING_PURPOSE_SYMBOL);
    faster_interned_strings_intern(&interned_strings, (const faster_str_ptr_t)&key4, FAST_INTERNED_STRING_PURPOSE_SYMBOL);

    faster_interned_string_purpose_t purpose1 = faster_interned_strings_get(&interned_strings, (const faster_str_ptr_t)&key1);
    faster_interned_string_purpose_t purpose2 = faster_interned_strings_get(&interned_strings, (const faster_str_ptr_t)&key2);
    faster_interned_string_purpose_t purpose3 = faster_interned_strings_get(&interned_strings, (const faster_str_ptr_t)&key3);
    faster_interned_string_purpose_t purpose4 = faster_interned_strings_get(&interned_strings, (const faster_str_ptr_t)&key4);
    faster_interned_string_purpose_t purpose5 = faster_interned_strings_get(&interned_strings, (const faster_str_ptr_t)&key5);

    printf("Key1 purpose: %ld\n", purpose1);
    printf("Key2 purpose: %ld\n", purpose2);
    printf("Key3 purpose: %ld\n", purpose3);
    printf("Key4 purpose: %ld\n", purpose4);
    printf("Key5 purpose: %ld\n", purpose5);
    if (purpose1 != FAST_INTERNED_STRING_PURPOSE_TEXT) {
        printf("Key1 purpose mismatch\n");
        return -1;
    }
    if (!FASTER_INTERNED_STRING_PURPOSE_HAS_PURPOSE(purpose2, FAST_INTERNED_STRING_PURPOSE_VNAME)) {
        // original purpose
        printf("Key2 purpose mismatch\n");
        return -1;
    }
    if (!FASTER_INTERNED_STRING_PURPOSE_HAS_PURPOSE(purpose3, FAST_INTERNED_STRING_PURPOSE_SYMBOL)) {
        // added purpose
        printf("Key3 purpose mismatch\n");
        return -1;
    }
    if (!FASTER_INTERNED_STRING_PURPOSE_HAS_PURPOSE(purpose4, (FAST_INTERNED_STRING_PURPOSE_SYMBOL | FAST_INTERNED_STRING_PURPOSE_OPERATOR))) {
        // all expected purposes
        printf("Key4 purpose mismatch\n");
        return -1;
    }
    if (purpose5 != FAST_INTERNED_STRING_PURPOSE_UNDEFINED) {
        printf("Key5 purpose mismatch\n");
        return -1;
    }    
    printf("All keys have correct purposes\n");
    faster_interned_strings_free(&interned_strings);
    return 0;
}
