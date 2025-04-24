#ifdef FASTER_IS_INCLUDE
#else

#include "faster_avl.h"

// interned strings implementation
enum faster_interned_string_purpose_e {
    FAST_INTERNED_STRING_PURPOSE_UNDEFINED = 0x00,
    FAST_INTERNED_STRING_PURPOSE_TEXT = 0x10,
    FAST_INTERNED_STRING_PURPOSE_VNAME = 0x01,
    FAST_INTERNED_STRING_PURPOSE_FNAME = 0x02,
    FAST_INTERNED_STRING_PURPOSE_OPERATOR = 0x04,
    FAST_INTERNED_STRING_PURPOSE_SYMBOL = 0x08
};

typedef faster_value_int_holder_t faster_interned_string_purpose_t;

#define FASTER_INTERNED_STRING_PURPOSE_HAS_PURPOSE(purpose, check_purpose) \
    (((purpose) & (check_purpose)) != 0)

struct faster_interned_strings_t_s {
    AVLNodesTree_t avl_tree;
};
typedef struct faster_interned_strings_t_s faster_interned_strings_t;
typedef struct faster_interned_strings_t_s *faster_interned_strings_ptr_t;

void faster_interned_strings_init(faster_interned_strings_ptr_t interned_strings);
void faster_interned_strings_free(faster_interned_strings_ptr_t interned_strings);
faster_interned_string_purpose_t faster_interned_strings_get(faster_interned_strings_ptr_t interned_strings, const faster_str_ptr_t str);
void faster_interned_strings_intern(const faster_interned_strings_ptr_t interned_strings, const faster_str_ptr_t str, const faster_interned_string_purpose_t purpose);

#define FASTER_IS_INCLUDE
#endif // FASTER_IS_INCLUDE
