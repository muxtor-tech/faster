#ifndef FasterConfig_H
#define FasterConfig_H

#include <stddef.h> // For size_t and related types
#include <stdint.h> // For INTPTR_MAX and related macros

#define FASTER_UNICODE_SUPPORT_ONE_BYTE (1)
#define FASTER_UNICODE_SUPPORT_TWO_BYTE (2)
#define FASTER_UNICODE_SUPPORT_FOUR_BYTE (4)
#define FASTER_UNICODE_SUPPORT_NONE (0)

#ifndef FASTER_UNICODE_SUPPORT
// Auto-detect based on platform and C23 capabilities
#if (defined(__STDC_HOSTED__) && __STDC_HOSTED__ == 0) // Embedded-like systems
#if defined(__STDC_UTF_8__) && __STDC_VERSION__ >= 202311L
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_ONE_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#else                       // Hosted systems
#if INTPTR_MAX == INT32_MAX // 32-bit platforms
#if defined(__STDC_UTF_16__)
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_TWO_BYTE
#elif defined(__STDC_UTF_32__) // Secondary fallback
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_FOUR_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#elif INTPTR_MAX == INT64_MAX // 64-bit platforms
#if defined(__STDC_UTF_32__)
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_FOUR_BYTE
#elif defined(__STDC_UTF_16__) // Secondary fallback
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_TWO_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#else // Unknown architecture
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#endif
#endif // FASTER_UNICODE_SUPPORT

#else
#endif