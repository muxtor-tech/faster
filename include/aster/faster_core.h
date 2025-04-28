#ifdef FASTER_CORE_INCLUDE
#else

#include "faster_config.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "faster_str.h"

#define FASTER_VERSION_MAJOR 0
#define FASTER_VERSION_MINOR 1
#define FASTER_VERSION_PATCH 0
#define FASTER_VERSION_STRING "0.1.0"

#define FAST_LIMIT_INT_MAX (INTPTR_MAX >> 2)
#define FAST_LIMIT_INT_MIN (INTPTR_MIN >> 2)
#define FAST_LIMIT_INDEXING_MAX (UINT_LEAST32_MAX)

typedef uint_least32_t faster_base_32_bit_unsigned_t;
typedef faster_base_32_bit_unsigned_t faster_indexing_t;
typedef size_t faster_system_indexing_t;

typedef uintptr_t faster_value_ptr_handler_t;
typedef intptr_t faster_value_int_holder_t;
typedef faster_indexing_t faster_str_len_t;
typedef double faster_value_float_holder_t;
typedef const fchar_t *faster_value_str_ptr_holder_t;
typedef const fchar_t *const fchar_ptr_t;
typedef void *faster_value_ptr;

#define FASTER_INVALID_VALUE_PTR ((faster_value_ptr)NULL)

#define FASTER_ALIGNMENT_BASE sizeof(faster_indexing_t)
#define FASTER_ALIGNED __attribute__((aligned(FASTER_ALIGNMENT_BASE), packed))
#define FASTER_ALIGNED_UNPACKED __attribute__((aligned(FASTER_ALIGNMENT_BASE)))

#define FASTER_REALLOCATOR(ptr, len, context) realloc(ptr, len)

#define FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(ptr, elements, element_size)                                                      \
  ((faster_value_ptr)((char *)(ptr) + (elements * element_size)))

static_assert(sizeof(faster_value_ptr_handler_t) == sizeof(faster_value_int_holder_t),
              "faster_value_ptr_handler_t and faster_value_int_holder_t must "
              "be the same size");
static_assert(sizeof(faster_value_ptr) == sizeof(faster_value_ptr_handler_t),
              "faster_value_ptr and faster_value_ptr_handler_t must be the same size");
static_assert(sizeof(faster_system_indexing_t) >= sizeof(faster_indexing_t),
              "faster_system_indexing_t must be greater than or equal to "
              "faster_indexing_t");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
typedef struct faster_str_t_s {
  const faster_value_str_ptr_holder_t str_ptr;
  const faster_system_indexing_t str_len;
} FASTER_ALIGNED faster_str_t;
typedef struct faster_str_t_s *faster_str_ptr_t;
#pragma GCC diagnostic pop

#define FASTER_VALUE_MARKER_STR (0b11)
#define FASTER_VALUE_MARKER_INT (0b01)
#define FASTER_VALUE_MARKER_FLO (0b10)
#define FASTER_VALUE_MARKER_OBJ (0b00)
#define FASTER_VALUE_MARKER_MASK                                                                                                   \
  (FASTER_VALUE_MARKER_OBJ | FASTER_VALUE_MARKER_FLO | FASTER_VALUE_MARKER_INT | FASTER_VALUE_MARKER_STR)

#define FASTER_NULL_VALUE ((faster_value_ptr)NULL)

static_assert(FASTER_VALUE_MARKER_MASK < FASTER_ALIGNMENT_BASE,
              "FASTER_VALUE_MARKER_MASK must be less than FASTER_VALUE_ALIGNMENT");

#define FASTER_VALUE_MARKER_READ(value_ptr) (((faster_value_ptr_handler_t)value_ptr) & FASTER_VALUE_MARKER_MASK)
#define FASTER_VALUE_MARKER_IS_STRING(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_STR)
#define FASTER_VALUE_MARKER_IS_INT(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_INT)
#define FASTER_VALUE_MARKER_IS_FLO(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_FLO)
#define FASTER_VALUE_MARKER_IS_OBJ(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_OBJ)

#define FASTER_VALUE_GET_PTR(value_ptr) ((faster_value_ptr)(((faster_value_ptr_handler_t)value_ptr) & ~FASTER_VALUE_MARKER_MASK))

#define FASTER_VALUE_GET_STRING(value_ptr) ((faster_value_str_ptr_holder_t)FASTER_VALUE_GET_PTR(value_ptr))
#define FASTER_VALUE_GET_INT(value_ptr) (((faster_value_int_holder_t)FASTER_VALUE_GET_PTR(value_ptr)) >> 2)
#define FASTER_VALUE_GET_FLO(value_ptr) (*((faster_value_float_holder_t)FASTER_VALUE_GET_PTR(value_ptr)))
#define FASTER_VALUE_GET_OBJ(value_ptr) ((void *)FASTER_VALUE_GET_PTR(value_ptr))

#define FASTER_VALUE_MAKE_STRING(str_ptr) (faster_value_ptr)(((faster_value_ptr_handler_t)str_ptr) | FASTER_VALUE_MARKER_STR)
#define FASTER_VALUE_MAKE_INT(int_value)                                                                                           \
  (faster_value_ptr)(((faster_value_ptr_handler_t)(int_value << 2)) | FASTER_VALUE_MARKER_INT)
#define FASTER_VALUE_MAKE_OBJ(ptr_value) (faster_value_ptr)(((faster_value_ptr_handler_t)ptr_value) | FASTER_VALUE_MARKER_OBJ)

#define FASTER_VALUE_GET_PTR_DIRECT(value_ptr) ((faster_value_ptr)value_ptr)
#define FASTER_VALUE_GET_INT_DIRECT(value_ptr) (((faster_value_int_holder_t)FASTER_VALUE_GET_PTR_DIRECT(value_ptr)))
#define FASTER_VALUE_MAKE_INT_DIRECT(int_value) (faster_value_ptr)(((faster_value_ptr_handler_t)(int_value)))

struct faster_array_header_t_s {
  faster_indexing_t array_internal;
  faster_indexing_t array_capacity;
  faster_indexing_t next_free_index;
};
typedef struct faster_array_header_t_s faster_array_header_t;

#define FASTER_ARRAY_COUNT_INVALID (FAST_LIMIT_INDEXING_MAX)
#define FASTER_ARRAY_INDEX_INVALID (FASTER_ARRAY_COUNT_INVALID)

#define _SUB_DECLARE_ARRAY_HEADER(initcap)                                                                                         \
  {.array_internal = initcap, .array_capacity = 0, .next_free_index = FASTER_ARRAY_COUNT_INVALID}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
static inline faster_indexing_t _assume_within_range(const size_t value) {
  assert(value <= FAST_LIMIT_INDEXING_MAX);
  return (faster_indexing_t)value;
}
#pragma GCC diagnostic pop

#define DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(type)                                                                            \
  static_assert(sizeof(type) % FASTER_ALIGNMENT_BASE == 0, "Type must be aligned to FASTER_ALIGNMENT_BASE");                       \
  static_assert(sizeof(type) >= sizeof(faster_indexing_t));                                                                        \
  struct faster_array_for_##type##_t_s {                                                                                           \
    faster_array_header_t list_header;                                                                                             \
    type *list;                                                                                                                    \
  } FASTER_ALIGNED;                                                                                                                \
  static_assert(sizeof(type) % FASTER_ALIGNMENT_BASE == 0, "Type must be aligned to FASTER_ALIGNMENT_BASE");                       \
  static_assert(sizeof(struct faster_array_for_##type##_t_s) == sizeof(_faster_default_array_t));                                  \
  typedef struct faster_array_for_##type##_t_s type##_arr_t;                                                                       \
  typedef struct faster_array_for_##type##_t_s *type##_arr_ptr_t;                                                                  \
  [[maybe_unused]] static inline void type##_arr_reset_and_free(type##_arr_ptr_t v, faster_indexing_t initial_capacity) {          \
    _arr_reset_and_free((_faster_default_array_ptr_t)v, initial_capacity);                                                         \
  }                                                                                                                                \
  [[maybe_unused]] static inline faster_indexing_t type##_arr_count(type##_arr_ptr_t v) {                                          \
    return _arr_count((_faster_default_array_ptr_t)v);                                                                             \
  }                                                                                                                                \
  [[maybe_unused]] static inline faster_indexing_t type##_arr_get_next(type##_arr_ptr_t v) {                                       \
    return _arr_get_next((_faster_default_array_ptr_t)v, sizeof(type));                                                            \
  }                                                                                                                                \
  [[maybe_unused]] static inline void type##_arr_release(type##_arr_ptr_t v, const faster_indexing_t idx) {                        \
    _arr_release((_faster_default_array_ptr_t)v, idx, sizeof(type));                                                               \
  }                                                                                                                                \
  static_assert(0 == 0)

#define DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name, type, initial_capacity)                                                   \
  struct faster_array_for_##type##_t_s name = {_SUB_DECLARE_ARRAY_HEADER(initial_capacity), NULL}

// array implementations
struct _default_array_struct {
  faster_array_header_t list_header;
  faster_value_ptr list;
} FASTER_ALIGNED;
typedef struct _default_array_struct _faster_default_array_t;
typedef struct _default_array_struct *_faster_default_array_ptr_t;
void _arr_reset_and_free(_faster_default_array_ptr_t v, faster_indexing_t initial_capacity);
faster_indexing_t _arr_count(_faster_default_array_ptr_t v);
faster_indexing_t _arr_get_next(_faster_default_array_ptr_t v, size_t element_size);
void _arr_release(_faster_default_array_ptr_t v, const faster_indexing_t idx, size_t element_size);

// allocation helpers
size_t faster_get_optimal_block_size(const size_t element_size, const size_t initial_count);
size_t faster_get_optimal_growth_increment(const size_t current_size);

// local string comparison function dedicated for faster_str_t
int faster_str_cmp_binary(const faster_str_t *str1, const faster_str_t *str2);
faster_str_t faster_str_create(const fchar_t *null_terminated_str);

// dedicated implemenations for string
size_t faster_strlen(const fchar_t *s);
fchar_t *faster_strdup(const fchar_t *s);
size_t faster_str_bytelen(const fchar_t *s);

// these are implemented as simple copy functions in case of non-unicode support
size_t faster_mb_to_unicode(const char *src, fchar_t *dest, size_t dest_size);
size_t faster_unicode_to_mb(const fchar_t *src, char *dest, size_t dest_size);

#ifdef NDEBUG
#warning "Debug mode enabled"
#endif

enum faster_error_codes_e {
  FAST_ERROR_NONE = 0x0,
  FAST_AST_ERROR_NONE = 0x0,
  FAST_AST_ERROR_GENERAL = 0x100,
  FAST_AST_ERROR_INVALID_TOKEN,
  FAST_AST_ERROR_INVALID_NODE,
  FAST_AST_ERROR_INVALID_CONTEXT,
  FAST_AST_ERROR_INVALID_STATE,
  FAST_AST_ERROR_INVALID_VALUE,
  FAST_AST_ERROR_INVALID_STRING,
  FAST_AST_ERROR_INVALID_OPERATOR,
  FAST_AST_ERROR_INVALID_LIST,
  FAST_AST_ERROR_INVALID_PARAMETER,
  FAST_ERROR_GENERAL = 0x200,
  FAST_ERROR_MEMORY_ALLOCATION_FAILED,
  FAST_ERROR_HT_KEY_NOT_FOUND,
};
typedef enum faster_error_codes_e faster_error_code_t;

#define FASTER_CORE_INCLUDE
#endif // FASTER_CORE_INCLUDE
