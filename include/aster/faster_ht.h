#ifndef FASTER_HT_H
#define FASTER_HT_H

#include "aster/faster_core.h"
#include <stddef.h>
#include <stdlib.h>

typedef faster_base_32_bit_unsigned_t faster_hash_value_t;
#define FASTER_HASH_VALUE_INVALID ((faster_hash_value_t)0xFFFFFFFF)

struct faster_ht_key_data_s {
  faster_value_ptr ptr;
  faster_indexing_t len;
} FASTER_ALIGNED;
typedef struct faster_ht_key_data_s faster_ht_key_data_t;
typedef struct faster_ht_key_data_s *faster_ht_key_data_ptr_t;

struct faster_ht_entry_linked_s {
  faster_hash_value_t hash;
  faster_ht_key_data_t key;
  faster_value_ptr value;
  faster_indexing_t next;
} FASTER_ALIGNED;
typedef struct faster_ht_entry_linked_s faster_ht_entry_linked_t;

typedef faster_indexing_t faster_ht_entry_t;
typedef faster_indexing_t *faster_ht_entry_ptr_t;

DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(faster_ht_entry_linked_t);

// hash function type for supporting custom hash functions
typedef faster_hash_value_t (*faster_ht_hash_func_t)(faster_ht_key_data_ptr_t key);

struct faster_ht_s {
  faster_indexing_t elements;
  faster_indexing_t capacity;
  faster_indexing_t requested_capacity;
  faster_indexing_t next_grow_at;
  faster_indexing_t next_shrink_at;
  faster_ht_hash_func_t hash_func;
  faster_ht_entry_ptr_t entries;
  faster_ht_entry_linked_t_arr_t entries_linked;
};
typedef struct faster_ht_s faster_ht_t;
typedef struct faster_ht_s *faster_ht_ptr_t;

faster_hash_value_t faster_ht_hash(faster_ht_key_data_ptr_t key);

faster_error_code_t faster_ht_init(faster_ht_ptr_t ht, faster_indexing_t initial_capacity, faster_ht_hash_func_t hash_func);
void faster_ht_clear(faster_ht_ptr_t ht);
void faster_ht_free(faster_ht_ptr_t ht);

faster_value_ptr faster_ht_get(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key);
faster_error_code_t faster_ht_set(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key, faster_value_ptr value);
faster_error_code_t faster_ht_remove(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key);

#endif // FASTER_HT_H
