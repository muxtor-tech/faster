#ifdef FASTER_CORE_INCLUDE
#else

#include "faster_config.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define FASTER_VERSION_MAJOR 0
#define FASTER_VERSION_MINOR 1
#define FASTER_VERSION_PATCH 0
#define FASTER_VERSION_STRING "0.1.0"

// we don't need the c++ compatibility warning
// the complaint about charx_t is a false positive


#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_ONE_BYTE
# include <uchar.h>
# define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc8
# define FASTER_UNICODE_UC_TO_MB_FUNC c8rtomb
# define FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE u8"�"
# define ASTER_TEXT(s) u8 ## s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
  typedef char8_t fchar_t;
#pragma GCC diagnostic pop
  static_assert(sizeof(fchar_t) == 1, "unicode support not possible, fchar_t must be at least 1 byte");
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_TWO_BYTE
# include <uchar.h>
# define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc16
# define FASTER_UNICODE_UC_TO_MB_FUNC c16rtomb
# define FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE 0xFFFD
# define ASTER_TEXT(s) u ## s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
  typedef char16_t fchar_t;
#pragma GCC diagnostic pop
  static_assert(sizeof(fchar_t) >= 2, "unicode support not possible, fchar_t must be at least 2 bytes");
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_FOUR_BYTE
# include <uchar.h>
# define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc32
# define FASTER_UNICODE_UC_TO_MB_FUNC c32rtomb
# define FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE 0xFFFD
# define ASTER_TEXT(s) U ## s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
  typedef char32_t fchar_t;
#pragma GCC diagnostic pop
  static_assert(sizeof(fchar_t) >= 4, "unicode support not possible, fchar_t must be at least 2 bytes");
#else   // no unicode support
# define ASTER_TEXT(s) s
# define faster_sprintf(s, mem, f, ...) sprintf(s, ASTER_TEXT(f), __VA_ARGS__)
  typedef char fchar_t;
  static_assert(sizeof(fchar_t) == 1, "wider char available, consider a switch to unicode support");
#endif



#define FASTER_STRING_MEMORY_SIZE(strlen) (sizeof(fchar_t) * strlen)

#define FASTER_DECLARE_RAW_STR(name, str) \
    static const fchar_t name[] = ASTER_TEXT(str) \

#define FASTER_DECLARE_FASTER_STR(name, str) \
    FASTER_DECLARE_RAW_STR(_##name##_rawstr, str); \
    static const faster_str_t name = {_##name##_rawstr, sizeof(_##name##_rawstr) / sizeof(fchar_t)} \

#define FAST_LIMIT_INT_MAX (INTPTR_MAX>>2)
#define FAST_LIMIT_INT_MIN (INTPTR_MIN>>2)
#define FAST_LIMIT_INDEXING_MAX (UINT_LEAST32_MAX)

typedef uint_least32_t faster_base_32_bit_unsigned_t;
typedef faster_base_32_bit_unsigned_t faster_indexing_t;
typedef size_t faster_system_indexing_t;

typedef uintptr_t faster_value_ptr_handler_t;
typedef intptr_t faster_value_int_holder_t;
typedef faster_indexing_t faster_str_len_t;
typedef double faster_value_float_holder_t;
typedef const fchar_t *faster_value_str_ptr_holder_t;
typedef const fchar_t * const fchar_ptr_t;
typedef void *faster_value_ptr;

#define FASTER_ALIGNMENT_BASE sizeof(faster_indexing_t)

#define FASTER_ALIGNED __attribute__ ((aligned (FASTER_ALIGNMENT_BASE), packed))
#define FASTER_ALIGNED_UNPACKED __attribute__ ((aligned (FASTER_ALIGNMENT_BASE)))

#define FASTER_REALLOCATOR(ptr, len, context) realloc(ptr, len)

static_assert(sizeof(faster_value_ptr_handler_t) == sizeof(faster_value_int_holder_t), "faster_value_ptr_handler_t and faster_value_int_holder_t must be the same size");
static_assert(sizeof(faster_value_ptr) == sizeof(faster_value_ptr_handler_t), "faster_value_ptr and faster_value_ptr_handler_t must be the same size");
static_assert(sizeof(faster_system_indexing_t) >= sizeof(faster_indexing_t), "faster_system_indexing_t must be greater than or equal to faster_indexing_t");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
typedef struct faster_str_t_s {
    const faster_value_str_ptr_holder_t str_ptr;
    const faster_system_indexing_t str_len;
} FASTER_ALIGNED faster_str_t;
typedef struct faster_str_t_s *faster_str_ptr_t;
#pragma GCC diagnostic pop

#define FASTER_VALUE_MARKER_STR (0b00)
#define FASTER_VALUE_MARKER_INT (0b01)
#define FASTER_VALUE_MARKER_FLO (0b10)
#define FASTER_VALUE_MARKER_OBJ (0b11)
#define FASTER_VALUE_MARKER_MASK (FASTER_VALUE_MARKER_OBJ|FASTER_VALUE_MARKER_FLO|FASTER_VALUE_MARKER_INT|FASTER_VALUE_MARKER_STR)

static_assert(FASTER_VALUE_MARKER_MASK < FASTER_ALIGNMENT_BASE, "FASTER_VALUE_MARKER_MASK must be less than FASTER_VALUE_ALIGNMENT");

#define FASTER_VALUE_MARKER_READ(value_ptr) (((faster_value_ptr_handler_t)value_ptr) & FASTER_VALUE_MARKER_MASK)
#define FASTER_VALUE_MARKER_IS_STRING(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_STR)
#define FASTER_VALUE_MARKER_IS_INT(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_INT)
#define FASTER_VALUE_MARKER_IS_FLO(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_FLO)
#define FASTER_VALUE_MARKER_IS_OBJ(value_ptr) (FASTER_VALUE_MARKER_READ(value_ptr) == FASTER_VALUE_MARKER_OBJ)

#define FASTER_VALUE_GET_PTR(value_ptr) ((faster_value_ptr)(((faster_value_ptr_handler_t)value_ptr) & ~FASTER_VALUE_MARKER_MASK))

#define FASTER_VALUE_GET_STRING(value_ptr) ((faster_value_str_ptr_holder_t)FASTER_VALUE_GET_PTR(value_ptr))
#define FASTER_VALUE_GET_INT(value_ptr) (((faster_value_int_holder_t)FASTER_VALUE_GET_PTR(value_ptr))>>2)
#define FASTER_VALUE_GET_FLO(value_ptr) (*((faster_value_float_holder_t)FASTER_VALUE_GET_PTR(value_ptr)))
#define FASTER_VALUE_GET_OBJ(value_ptr) ((void *)FASTER_VALUE_GET_PTR(value_ptr))

#define FASTER_VALUE_MAKE_STRING(str_ptr) (faster_value_ptr)(((faster_value_ptr_handler_t)str_ptr) | FASTER_VALUE_MARKER_STR)
#define FASTER_VALUE_MAKE_INT(int_value) (faster_value_ptr)(((faster_value_ptr_handler_t)(int_value<<2)) | FASTER_VALUE_MARKER_INT)
#define FASTER_VALUE_MAKE_OBJ(ptr_value) (faster_value_ptr)(((faster_value_ptr_handler_t)ptr_value) | FASTER_VALUE_MARKER_MASK)

#define FASTER_VALUE_GET_PTR_DIRECT(value_ptr) ((faster_value_ptr)value_ptr)
#define FASTER_VALUE_GET_INT_DIRECT(value_ptr) (((faster_value_int_holder_t)FASTER_VALUE_GET_PTR_DIRECT(value_ptr)))

#define FASTER_VALUE_MAKE_INT_DIRECT(int_value) (faster_value_ptr)(((faster_value_ptr_handler_t)(int_value)))

struct faster_array_header_t_s{
    faster_indexing_t array_internal;
    faster_indexing_t array_capacity;
    faster_indexing_t next_free_index;
};
typedef struct faster_array_header_t_s faster_array_header_t;

#define FASTER_ARRAY_COUNT_INVALID (FAST_LIMIT_INDEXING_MAX)
#define _SUB_DECLARE_ARRAY_HEADER(initcap) \
    { \
        .array_internal = initcap, \
        .array_capacity = 0, \
        .next_free_index = FASTER_ARRAY_COUNT_INVALID \
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
static inline faster_indexing_t _assume_within_range(const size_t value) {
    assert(value <= FAST_LIMIT_INDEXING_MAX);
    return (faster_indexing_t)value;
}
#pragma GCC diagnostic pop

#define DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(type) \
    static_assert(sizeof(type) % FASTER_ALIGNMENT_BASE == 0, "Type must be aligned to FASTER_ALIGNMENT_BASE"); \
    static_assert(sizeof(type) >= sizeof(faster_indexing_t)); \
    struct faster_array_for_##type##_t_s { \
        faster_array_header_t list_header; \
        type *list; \
    } FASTER_ALIGNED; \
    typedef struct faster_array_for_##type##_t_s type##_arr_t; \
    typedef struct faster_array_for_##type##_t_s *type##_arr_ptr_t; \
    [[maybe_unused]] static void type##_arr_reset_and_free(type##_arr_ptr_t v, faster_indexing_t initial_capacity) { \
        free(v->list); v->list = NULL; \
        faster_array_header_t tmp = _SUB_DECLARE_ARRAY_HEADER(initial_capacity); v->list_header = tmp; \
    } \
    [[maybe_unused]] static faster_indexing_t type##_arr_count(type##_arr_ptr_t v) { \
        return (v->list == NULL) ? 0 : v->list_header.array_internal; \
    } \
    [[maybe_unused]] static faster_indexing_t type##_arr_get_next(type##_arr_ptr_t v) { \
        if (v->list_header.next_free_index == FASTER_ARRAY_COUNT_INVALID) {  \
            size_t new_size;  \
            new_size = faster_get_optimal_block_size(sizeof(type), v->list_header.array_capacity  \
                + ((v->list == NULL) ? v->list_header.array_internal : faster_get_optimal_growth_increment(v->list_header.array_capacity)));  \
            type *tmp = (type*)FASTER_REALLOCATOR(v->list, new_size, NULL);  \
            if (tmp == NULL) {  \
                return FASTER_ARRAY_COUNT_INVALID; \
            } \
            faster_indexing_t new_capacity = _assume_within_range(new_size / sizeof(type)); \
            if (new_capacity == 0) { return FASTER_ARRAY_COUNT_INVALID; } \
            if (new_capacity > v->list_header.array_capacity) { \
                for (faster_indexing_t i = v->list_header.array_capacity; i < new_capacity; i++) { \
                    *(faster_indexing_t *)(tmp + i) = i + 1; \
                } \
                *(faster_indexing_t *)(tmp + (new_capacity - 1)) = FASTER_ARRAY_COUNT_INVALID; \
            } \
            v->list_header.next_free_index = (v->list == NULL) ? 0 : v->list_header.array_capacity; \
            v->list_header.array_capacity = new_capacity; \
            v->list_header.array_internal = (v->list == NULL) ? 0 : v->list_header.array_internal; \
            v->list = tmp; \
        } \
        faster_indexing_t idx = v->list_header.next_free_index; \
        v->list_header.next_free_index = *(faster_indexing_t *)(v->list + idx); \
        v->list_header.array_internal++; \
        return idx; \
    } \
    [[maybe_unused]] static void type##_arr_release(type##_arr_ptr_t v, const faster_indexing_t idx) { \
        *(faster_indexing_t *)(v->list + idx) = v->list_header.next_free_index; \
        v->list_header.next_free_index = idx;\
        v->list_header.array_internal--; \
    } \
    static_assert(0==0)

#define DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name, type, initial_capacity) \
    struct faster_array_for_##type##_t_s name = { \
        _SUB_DECLARE_ARRAY_HEADER(initial_capacity), \
        NULL \
    }

// allocation helpers
size_t faster_get_optimal_block_size(const size_t element_size, const size_t initial_count);
size_t faster_get_optimal_growth_increment(const size_t current_size);

// local string comparison function dedicated for faster_str_t
int faster_str_cmp_binary(const faster_str_t *str1, const faster_str_t *str2);
faster_str_t faster_str_create(const fchar_t *null_terminated_str);

// dedicated implemenations for string
size_t faster_strlen(const fchar_t* s);
fchar_t* faster_strdup(const fchar_t* s);

// these are implemented as simple copy functions in case of non-unicode support 
size_t faster_mb_to_unicode(const char* src, fchar_t* dest, size_t dest_size);
size_t faster_unicode_to_mb(const fchar_t* src, char* dest, size_t dest_size);

#ifdef DEBUG
#warning "Debug mode enabled"
#endif

#define FASTER_CORE_INCLUDE
#endif // FASTER_CORE_INCLUDE
