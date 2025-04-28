#include "aster/faster_ht.h"
#include <stddef.h>

static const faster_ht_key_data_t FASTER_HT_KEY_NULL = {FASTER_INVALID_VALUE_PTR, 0};

static inline bool _faster_ht_keys_equal(faster_ht_key_data_ptr_t key1, faster_ht_key_data_ptr_t key2) {
  if (key1->len != key2->len) {
    return false;
  }
  if (key1->ptr == key2->ptr) {
    return true;
  }
  return memcmp(key1->ptr, key2->ptr, key1->len) == 0;
}

faster_indexing_t _fht_create_list_element(faster_ht_ptr_t ht, faster_hash_value_t hv, faster_ht_key_data_ptr_t key,
                                           faster_value_ptr value) {
  faster_indexing_t new_item = faster_ht_entry_linked_t_arr_get_next(&ht->entries_linked);
  if (new_item == FASTER_ARRAY_INDEX_INVALID) {
    return FASTER_ARRAY_INDEX_INVALID;
  }
  faster_ht_entry_linked_t *list_ref = ht->entries_linked.list + new_item;
  list_ref->hash = hv;
  list_ref->key = *key;
  list_ref->value = value;
  list_ref->next = FASTER_ARRAY_INDEX_INVALID;
  return new_item;
}

static bool _faster_ht_resize_and_rehash(faster_ht_ptr_t ht, faster_indexing_t requested_capacity) {
  volatile faster_ht_entry_linked_t_arr_ptr_t linked_entries_table_ref = &ht->entries_linked;
  faster_ht_entry_ptr_t new_entries = NULL;
  size_t new_capacity = requested_capacity;
  if (new_capacity < ht->requested_capacity) {
    new_capacity = ht->requested_capacity;
  }
  size_t new_size = faster_get_optimal_block_size(sizeof(faster_ht_entry_t), new_capacity);
  new_entries = (faster_ht_entry_ptr_t)malloc(new_size);
  if (new_entries == NULL) {
    return false;
  }
  new_capacity = new_size / sizeof(faster_ht_entry_t);
  assert(FASTER_ARRAY_INDEX_INVALID == 0xffffffff);
  memset(new_entries, 0xff, new_size);
  // rehashing
  for (size_t i = 0; i < ht->capacity; i++) {
    if (ht->entries[i].list_ref != FASTER_ARRAY_INDEX_INVALID) {
      faster_indexing_t list_index = ht->entries[i].list_ref;
      // for all elements in the list, place them in the new table
      while (list_index != FASTER_ARRAY_INDEX_INVALID) {
        // read element data
        faster_hash_value_t hv = linked_entries_table_ref->list[list_index].hash;
        faster_ht_key_data_t key = linked_entries_table_ref->list[list_index].key;
        faster_value_ptr value = linked_entries_table_ref->list[list_index].value;
        faster_indexing_t list_index_next = linked_entries_table_ref->list[list_index].next;
        // release the old list element
        faster_ht_entry_linked_t_arr_release(linked_entries_table_ref, list_index);
        // emplace the new element in the new table
        size_t new_index = hv % new_capacity;
        faster_indexing_t new_list_index = _fht_create_list_element(ht, hv, &key, value);
        if (new_list_index == FASTER_ARRAY_INDEX_INVALID) {
          free(new_entries);
          return false;
        }
        if (new_entries[new_index].list_ref == FASTER_ARRAY_INDEX_INVALID) {
          new_entries[new_index].list_ref = new_list_index;
        } else {
          // add to the list (at head)
          faster_indexing_t list_head = new_entries[new_index].list_ref;
          new_entries[new_index].list_ref = new_list_index;
          linked_entries_table_ref->list[new_list_index].next = list_head;
        }
        // next element
        list_index = list_index_next;
      }
    }
  }

  free(ht->entries);
  ht->entries = new_entries;
  ht->capacity = new_capacity;

  ht->next_grow_at = (ht->capacity * 3) / 4;
  ht->next_shrink_at = (ht->capacity / 4);

  return true;
}

faster_error_code_t faster_ht_init(faster_ht_ptr_t ht, faster_indexing_t initial_capacity, faster_ht_hash_func_t hash_func) {
  DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(_new_ht_list_table, faster_ht_entry_linked_t, initial_capacity);
  ht->requested_capacity = initial_capacity;
  ht->entries_linked = _new_ht_list_table;
  ht->hash_func = hash_func;
  ht->next_shrink_at = 0;
  ht->next_grow_at = 0;
  ht->entries = NULL;
  ht->elements = 0;
  ht->capacity = 0;
  return FAST_ERROR_NONE;
}

void faster_ht_free(faster_ht_ptr_t ht) {
  faster_ht_entry_linked_t_arr_reset_and_free(&ht->entries_linked, ht->requested_capacity);
  free(ht->entries);
  ht->entries = NULL;
  ht->capacity = 0;
  ht->elements = 0;
  ht->next_grow_at = 0;
  ht->next_shrink_at = 0;
}

enum _fth_helper_values {
  _FTH_NOT_FOUND_AT_ALL_POINTING_AT_FREE = 0,
  _FTH_FOUND_MATCH_POINTING_DIRECTLY = 1,
  _FTH_FOUND_LAST_WITH_SAME_HASH_POINTING_PREV_INDEX_EMPTY = 2,
};
typedef enum _fth_helper_values _fth_helper_values_t;

faster_error_code_t faster_ht_set(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key, faster_value_ptr value) {
  faster_ht_entry_linked_t_arr_ptr_t linked_entries_table_ref = &ht->entries_linked;
  if (ht->capacity == 0) {
    if (!_faster_ht_resize_and_rehash(ht, ht->requested_capacity)) {
      return FAST_ERROR_MEMORY_ALLOCATION_FAILED;
    }
  }
  if (ht->elements >= ht->next_grow_at) {
    if (!_faster_ht_resize_and_rehash(ht, ht->capacity + faster_get_optimal_growth_increment(ht->capacity))) {
      return FAST_ERROR_MEMORY_ALLOCATION_FAILED;
    }
  }
  faster_hash_value_t hash = ht->hash_func(key);
  faster_indexing_t hash_index = hash % ht->capacity;
  // no list
  if (ht->entries[hash_index].list_ref == FASTER_ARRAY_INDEX_INVALID) {
    ht->entries[hash_index].list_ref = _fht_create_list_element(ht, hash, key, value);
    ht->elements++;
    return FAST_ERROR_NONE;
  }
  // list exists
  faster_indexing_t list_index = ht->entries[hash_index].list_ref;
  while (list_index != FASTER_ARRAY_INDEX_INVALID) {
    if (_faster_ht_keys_equal(&linked_entries_table_ref->list[list_index].key, key)) {
      // element already exists, update the value
      linked_entries_table_ref->list[list_index].value = value;
      return FAST_ERROR_NONE;
    }
    list_index = linked_entries_table_ref->list[list_index].next;
  }
  // add new element to the list
  faster_indexing_t new_list_index = _fht_create_list_element(ht, hash, key, value);
  if (new_list_index == FASTER_ARRAY_INDEX_INVALID) {
    return FAST_ERROR_MEMORY_ALLOCATION_FAILED;
  }
  list_index = ht->entries[hash_index].list_ref;
  linked_entries_table_ref->list[new_list_index].next = list_index;
  ht->entries[hash_index].list_ref = new_list_index;
  ht->elements++;
  return FAST_ERROR_NONE;
}

faster_value_ptr faster_ht_get(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key) {
  faster_hash_value_t hash = ht->hash_func(key);
  faster_indexing_t hash_index = hash % ht->capacity;
  // list "exists" - empty or otherwise
  faster_indexing_t list_index = ht->entries[hash_index].list_ref;
  while (list_index != FASTER_ARRAY_INDEX_INVALID) {
    if (_faster_ht_keys_equal(&ht->entries_linked.list[list_index].key, key)) {
      return ht->entries_linked.list[list_index].value;
    }
    list_index = ht->entries_linked.list[list_index].next;
  }
  return FASTER_INVALID_VALUE_PTR;
}

faster_error_code_t faster_ht_remove(faster_ht_ptr_t ht, faster_ht_key_data_ptr_t key) {
  faster_ht_entry_linked_t_arr_ptr_t linked_entries_table_ref = &ht->entries_linked;
  faster_hash_value_t hash = ht->hash_func(key);
  faster_indexing_t hash_index = hash % ht->capacity;
  faster_indexing_t list_head = ht->entries[hash_index].list_ref;
  // no list
  if (list_head == FASTER_ARRAY_INDEX_INVALID) {
    return FAST_ERROR_HT_KEY_NOT_FOUND;
  }
  // list exists
  faster_indexing_t list_index = list_head;
  faster_indexing_t prev_index = FASTER_ARRAY_INDEX_INVALID;
  while (list_index != FASTER_ARRAY_INDEX_INVALID) {
    if (_faster_ht_keys_equal(&ht->entries_linked.list[list_index].key, key)) {
      // found the key, apply the removal
      if (list_index == list_head) {
        // first element in the list
        ht->entries[hash_index].list_ref = linked_entries_table_ref->list[list_index].next;
      } else {
        // not first element in the list
        linked_entries_table_ref->list[prev_index].next = linked_entries_table_ref->list[list_index].next;
      }
      faster_ht_entry_linked_t_arr_release(linked_entries_table_ref, list_index);
      ht->elements--;
      return FAST_ERROR_NONE;
    }
    prev_index = list_index;
    list_index = ht->entries_linked.list[list_index].next;
  }
  return FAST_ERROR_HT_KEY_NOT_FOUND;
}

void faster_ht_clear(faster_ht_ptr_t ht) {
  for (size_t i = 0; i < ht->capacity; i++) {
    ht->entries[i].list_ref = FASTER_ARRAY_INDEX_INVALID;
  }
  faster_ht_entry_linked_t_arr_reset_and_free(&ht->entries_linked, 0);
  ht->elements = 0;
}

// MurmurHash2, by Austin Appleby, taken from
// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp

// According to the file preamble:
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

faster_hash_value_t faster_ht_hash(faster_ht_key_data_ptr_t key) {
  uint32_t seed = 0x9747b28c;

  const uint32_t m = 0x5bd1e995;
  const int r = 24;

  size_t len = key->len;
  const unsigned char *data = (const unsigned char *)key->ptr;

  // Initialize the hash to a 'random' value

  uint32_t h = seed ^ len;

  // Mix 4 bytes at a time into the hash

  while (len >= 4) {
    uint32_t k = *(uint32_t *)data;

    k *= m;
    k ^= k >> r;
    k *= m;

    h *= m;
    h ^= k;

    data += 4;
    len -= 4;
  }

  // Handle the last few bytes of the input array

  switch (len) {
  case 3:
    h ^= data[2] << 16;
  case 2:
    h ^= data[1] << 8;
  case 1:
    h ^= data[0];
    h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  if (h == FASTER_HASH_VALUE_INVALID) {
    h++;
  }
  return h;
}