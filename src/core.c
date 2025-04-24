#include "aster/faster_core.h"

#include <stdlib.h>
#include <unistd.h>

#if FASTER_UNICODE_SUPPORT != FASTER_UNICODE_SUPPORT_NONE
// Convert multibyte string to Unicode (fchar_t)
// return number of bytes written to dest
size_t faster_mb_to_unicode(const char *src, fchar_t *_dest, size_t dest_size) {
  if (!src || !_dest || dest_size == 0)
    return 0;

  mbstate_t state = {0};
  const char *ptr = src;
  const char *end = src + strlen(src);
  size_t rc;
  fchar_t *dest = _dest;
  dest_size--; // leave space for null terminator
  while ((rc = FASTER_UNICODE_MB_TO_UC_FUNC(dest, ptr, ((size_t)(end - ptr)) + 1, &state))) {
    if (rc == (size_t)-3) {
      // earlier surrogate pair
      dest++;
    } else if (rc == (size_t)-2) {
      continue;
    } else if (rc == (size_t)-1) {
// error, store replacement character if possible and reset state
#if FASTER_UNICODE_SUPPORT_ONE_BYTE == FASTER_UNICODE_SUPPORT
      static const size_t _faster_invalid_character_len_runtime = sizeof(FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE) - 1;
      if (dest + _faster_invalid_character_len_runtime < _dest + dest_size) {
        strncpy((char *)dest, FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE, _faster_invalid_character_len_runtime);
        dest += _faster_invalid_character_len_runtime;
      }
#else
      *dest = FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE;
      dest++;
#endif
      ptr++;
      memset(&state, 0, sizeof(state));
    } else {
      // valid conversion
      dest++;
      ptr += rc;
    }
    if (((size_t)(dest - _dest)) >= dest_size) {
      // buffer overflow, null terminate and exit
      break;
    }
  }
  *dest = '\0';
  return (size_t)(dest - _dest);
}

// Convert Unicode (fchar_t) to multibyte string
// return number of bytes written to dest
size_t faster_unicode_to_mb(const fchar_t *src, char *dest, size_t dest_size) {
  if (!src || !dest || dest_size == 0)
    return 0;

  mbstate_t ps = {0};
  const fchar_t *ptr = src;
  const fchar_t *end = src + faster_strlen(src);
  size_t rc;
  char *dest_start = dest;
  dest_size--; // leave space for null terminator
  while (ptr < end) {
    rc = FASTER_UNICODE_UC_TO_MB_FUNC(dest, *(ptr++), &ps);
    if (rc != (size_t)-1) {
      dest += rc;
    }
    if (((size_t)(dest - dest_start)) >= dest_size) {
      // buffer overflow, null terminate and exit
      break;
    }
  }
  *dest = '\0';
  return (size_t)(dest - dest_start);
}

#else
size_t faster_mb_to_unicode(const char *src, fchar_t *dest, size_t dest_size) {
  size_t i = 0;
  if (!dest || !src || dest_size == 0)
    return 0;

  for (; i < dest_size - 1 && src[i]; i++) {
    dest[i] = (unsigned char)src[i]; // Explicit cast for non-ASCII
  }
  dest[i] = '\0';
  return i;
}

size_t faster_unicode_to_mb(const fchar_t *src, char *dest, size_t dest_size) {
  size_t i = 0;
  if (!dest || !src || dest_size == 0)
    return 0;

  for (; i < dest_size - 1 && src[i]; i++) {
    dest[i] = (char)(src[i] & 0xFF); // Truncate to byte
  }
  dest[i] = '\0';
  return i;
}
#endif

int faster_str_cmp_binary(const faster_str_t *str1, const faster_str_t *str2) {
  if (str1->str_len > str2->str_len) {
    return 1;
  } else if (str1->str_len < str2->str_len) {
    return -1;
  }
  return memcmp(str1->str_ptr, str2->str_ptr, FASTER_STRING_MEMORY_SIZE(str1->str_len));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
size_t faster_strlen(const fchar_t *s) {
  const fchar_t *p = s;
  while (*p)
    ++p;
  return p - s; // no worries about overflow, as p is always greater than s
}
#pragma GCC diagnostic pop

fchar_t *faster_strdup(const fchar_t *s) {
  size_t len = faster_strlen(s);
  fchar_t *dup = (fchar_t *)malloc((len + 1) * sizeof(fchar_t));
  if (dup)
    memcpy(dup, s, (len + 1) * sizeof(fchar_t));
  return dup;
}

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