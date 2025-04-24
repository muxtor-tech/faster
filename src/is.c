#include "aster/faster_is.h"

void faster_interned_strings_init(faster_interned_strings_ptr_t interned_strings) {
  DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(avl_tree, 256);
  interned_strings->avl_tree = avl_tree;
}

void faster_interned_strings_free(faster_interned_strings_ptr_t interned_strings) {
  AVL_reset_and_free(&interned_strings->avl_tree);
}

faster_interned_string_purpose_t faster_interned_strings_get(faster_interned_strings_ptr_t interned_strings,
                                                             const faster_str_ptr_t str) {
  return (faster_interned_string_purpose_t)(FASTER_VALUE_GET_INT_DIRECT(AVL_get(&interned_strings->avl_tree, str)));
}

void faster_interned_strings_intern(const faster_interned_strings_ptr_t interned_strings, const faster_str_ptr_t str,
                                    const faster_interned_string_purpose_t purpose) {
  faster_value_int_holder_t value = FASTER_VALUE_GET_INT_DIRECT(AVL_get(&interned_strings->avl_tree, str));
  if (!FASTER_INTERNED_STRING_PURPOSE_HAS_PURPOSE(value, purpose)) {
    AVL_insert_or_update(&interned_strings->avl_tree, str, FASTER_VALUE_MAKE_INT_DIRECT(purpose | value));
  }
}
