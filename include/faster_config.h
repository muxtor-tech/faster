#ifndef FasterConfig_H
#define FasterConfig_H

#include <stddef.h> // For size_t and related types
#include <stdint.h> // For INTPTR_MAX and related macros

#define FASTER_UNICODE_SUPPORT_AUTODETECT (-1)
#define FASTER_UNICODE_SUPPORT_ONE_BYTE (1)
#define FASTER_UNICODE_SUPPORT_TWO_BYTE (2)
#define FASTER_UNICODE_SUPPORT_FOUR_BYTE (4)
#define FASTER_UNICODE_SUPPORT_NONE (0)

#ifndef FASTER_UNICODE_SUPPORT
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_AUTODETECT
#endif

#else
#endif