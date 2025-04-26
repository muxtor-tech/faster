#include "aster/faster_core.h"

#include <stdatomic.h>

typedef faster_base_32_bit_unsigned_t faster_internal_head_tail_t;

struct _faster_atomic_fifo_queue_s {
  faster_value_ptr buffer;
  size_t element_size;
  size_t queue_size;
  _Atomic faster_internal_head_tail_t head_tail;
  int is_static_buffer;
} FASTER_ALIGNED_UNPACKED;
typedef struct _faster_atomic_fifo_queue_s _faster_atomic_fifo_queue_t;
typedef struct _faster_atomic_fifo_queue_s *_faster_atomic_fifo_queue_ptr_t;

#define FASTER_INTERNAL_GET_HEAD(ht) ((unsigned int)((ht >> 16) & 0xFFFF))
#define FASTER_INTERNAL_GET_TAIL(ht) ((unsigned int)(ht & 0xFFFF))
#define FASTER_INTERNAL_MAKE_HEAD_TAIL(head, tail) (((head & 0xFFFF) << 16) | (tail & 0xFFFF))
#define FASTER_INTERNAL_ADJUST_SIZE_TO_MULTIPLE(size, align) ((size - 1) + (align - ((size - 1) % align)))

// Macro to define a FIFO queue structure, static buffer type, and functions,
// with static validation of the provided parameters
#define DECLARE_FAST_ATOMIC_FIFO_QUEUE(type, queue_name, queue_capacity)                                                           \
  static_assert(queue_capacity <= 0xFFFF, "queue_capacity MUST BE 2^16 OR LESS");                                                  \
  static_assert(queue_capacity > 1, "queue_capacity MUST BE GREATER THAN 1");                                                      \
  struct queue_name##_s {                                                                                                          \
    type *buffer;                                                                                                                  \
    size_t element_size;                                                                                                           \
    size_t queue_size;                                                                                                             \
    _Atomic faster_internal_head_tail_t head_tail;                                                                                 \
    int is_static_buffer;                                                                                                          \
  } FASTER_ALIGNED_UNPACKED;                                                                                                       \
  typedef struct queue_name##_s queue_name##_t;                                                                                    \
                                                                                                                                   \
  typedef struct {                                                                                                                 \
    type data[queue_capacity] __attribute__((aligned(FASTER_ALIGNMENT_BASE)));                                                     \
  } queue_name##_StaticBuffer;                                                                                                     \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_init_static(queue_name##_t *q, queue_name##_StaticBuffer *buf) {                 \
    return _aq_init_preallocated((_faster_atomic_fifo_queue_ptr_t)q, buf->data, sizeof(type), queue_capacity);                     \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_init_dynamic(queue_name##_t *q) {                                                \
    return _aq_init_dynamic((_faster_atomic_fifo_queue_ptr_t)q, sizeof(type), queue_capacity);                                     \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline void queue_name##_free(queue_name##_t *q) {                                                       \
    return _aq_free((_faster_atomic_fifo_queue_ptr_t)q);                                                                           \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_is_empty(queue_name##_t *q) {                                                    \
    return _aq_is_empty((_faster_atomic_fifo_queue_ptr_t)q);                                                                       \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_is_full(queue_name##_t *q) {                                                     \
    return _aq_is_full((_faster_atomic_fifo_queue_ptr_t)q);                                                                        \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_enqueue(queue_name##_t *q, const type *item) {                                   \
    return _aq_enqueue((_faster_atomic_fifo_queue_ptr_t)q, (const faster_value_ptr)item);                                          \
  }                                                                                                                                \
                                                                                                                                   \
  [[maybe_unused]] static inline int queue_name##_dequeue(queue_name##_t *q, type *item) {                                         \
    return _aq_dequeue((_faster_atomic_fifo_queue_ptr_t)q, (const faster_value_ptr)item);                                          \
  }

int _aq_init_preallocated(_faster_atomic_fifo_queue_t *q, void *buf, size_t n_element_size, size_t n_queue_size);
int _aq_init_dynamic(_faster_atomic_fifo_queue_t *q, size_t n_element_size, size_t n_queue_size);
void _aq_free(_faster_atomic_fifo_queue_t *q);
int _aq_is_empty(_faster_atomic_fifo_queue_t *q);
int _aq_is_full(_faster_atomic_fifo_queue_t *q);
int _aq_enqueue(_faster_atomic_fifo_queue_t *q, const faster_value_ptr item);
int _aq_dequeue(_faster_atomic_fifo_queue_t *q, const faster_value_ptr item);
