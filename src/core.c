#include "aster/faster_core.h"

#include <stdlib.h>
#include <unistd.h>

size_t faster_get_optimal_block_size(const size_t element_size, const size_t initial_count) {
  // Get system page size
  long ipage_size = sysconf(_SC_PAGESIZE);
  if (ipage_size < 0) {
    ipage_size = 4096; // Fallback to a common page size
  }
// ignoring warning as we're checking the numbers first
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
  size_t page_size = ipage_size;
#pragma GCC diagnostic pop

  // Calculate minimum size needed
  size_t min_size = element_size * initial_count;

  // Round up to the nearest multiple of 16 bytes (minimum allocation on many
  // systems)
  size_t aligned_size = (min_size + 15) & ~((size_t)15);

  // For very small allocations, use minimum efficient size
  if (aligned_size < 64) {
    return 64; // Arbitrary small but efficient size
  }

  // For larger allocations, consider rounding to a fraction of page size
  if (aligned_size < page_size / 4) {
    return aligned_size;
  } else {
    // Round up to the nearest 1/4, 1/2, or full page
    return ((aligned_size + page_size - 1) / page_size) * page_size;
  }
}

size_t faster_get_optimal_growth_increment(const size_t current_size) {
  // Growth factor between 1.5 and 2 is generally efficient
  // Using 1.5 as a compromise between memory usage and reallocation frequency
  return (current_size / 2) + 1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
faster_str_t faster_str_create(const fchar_t *null_terminated_str) {
  const faster_str_t str = {.str_ptr = (fchar_t *const)null_terminated_str, .str_len = faster_strlen(null_terminated_str)};
  return str;
}
#pragma GCC diagnostic pop

void _arr_reset_and_free(_faster_default_array_ptr_t v, faster_indexing_t initial_capacity) {
  free(v->list);
  v->list = NULL;
  faster_array_header_t tmp = _SUB_DECLARE_ARRAY_HEADER(initial_capacity);
  v->list_header = tmp;
}

faster_indexing_t _arr_count(_faster_default_array_ptr_t v) { return (v->list == NULL) ? 0 : v->list_header.array_internal; }

faster_indexing_t _arr_get_next(_faster_default_array_ptr_t v, size_t element_size) {
  if (v->list_header.next_free_index == FASTER_ARRAY_COUNT_INVALID) {
    size_t new_size;
    new_size = faster_get_optimal_block_size(
        element_size,
        v->list_header.array_capacity + ((v->list == NULL) ? v->list_header.array_internal
                                                           : faster_get_optimal_growth_increment(v->list_header.array_capacity)));
    void *tmp = (void *)FASTER_REALLOCATOR(v->list, new_size, NULL);
    if (tmp == NULL) {
      return FASTER_ARRAY_COUNT_INVALID;
    }
    faster_indexing_t new_capacity = _assume_within_range(new_size / element_size);
    if (new_capacity == 0) {
      return FASTER_ARRAY_COUNT_INVALID;
    }
    if (new_capacity > v->list_header.array_capacity) {
      for (faster_indexing_t i = v->list_header.array_capacity; i < new_capacity; i++) {
        *(faster_indexing_t *)(FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(tmp, i, element_size)) = i + 1;
      }
      *(faster_indexing_t *)(FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(tmp, (new_capacity - 1), element_size)) =
          FASTER_ARRAY_COUNT_INVALID;
    }
    v->list_header.next_free_index = (v->list == NULL) ? 0 : v->list_header.array_capacity;
    v->list_header.array_capacity = new_capacity;
    v->list_header.array_internal = (v->list == NULL) ? 0 : v->list_header.array_internal;
    v->list = (faster_value_ptr)tmp;
  }
  faster_indexing_t idx = v->list_header.next_free_index;
  v->list_header.next_free_index = *(faster_indexing_t *)(FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(v->list, idx, element_size));
  v->list_header.array_internal++;
  return idx;
}

void _arr_release(_faster_default_array_ptr_t v, const faster_indexing_t idx, size_t element_size) {
  *(faster_indexing_t *)(FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(v->list, idx, element_size)) = v->list_header.next_free_index;
  v->list_header.next_free_index = idx;
  v->list_header.array_internal--;
}