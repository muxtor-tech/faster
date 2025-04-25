#include "aster/faster.h"
#include <locale.h>
#include <stdio.h>

void test_stub_conversion() {
// only in non-unicode mode
#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_NONE
  const char *test = "abc";
  fchar_t buf[4];
  assert(faster_mb_to_unicode(test, buf, 4) == 3);
  assert(memcmp(test, buf, 3) == 0);
#endif
}

void test_buffer_overflow() {
  const char *long_str = "1234567890";
  fchar_t small_buf[5];

  size_t conv = faster_mb_to_unicode(long_str, small_buf, 5);
  assert(conv == 4); // Fits 4 code points + null
  assert(small_buf[4] == 0);
}

void test_conversion_roundtrip() {
  const char *test_cases[] = {
      "Hello",  // ASCII
      "Привет", // Cyrillic
      "你好",   // Chinese
      "🚀",     // Emoji
      "abc\xFF\xFE"
      "def",             // Invalid bytes
      "\xF0\x9D\x84\x9E" // Musical G-clef (U+1D11E)
  };

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
    fchar_t unicode[64] = {0};
    char roundtrip[64] = {0};

    printf("Testing: %s\n", test_cases[i]);
    size_t unicode_len = faster_mb_to_unicode(test_cases[i], unicode, 64);
    size_t roundtrip_len = faster_unicode_to_mb(unicode, roundtrip, 64);

    printf("Unicode length: %zu, Roundtrip length: %zu\n", unicode_len, roundtrip_len);
    printf("Roundtrip result: %s\n\n", roundtrip);

    // Verify round-trip for valid sequences
    if (strspn(test_cases[i], "\x00-\x7F") == strlen(test_cases[i])) {
      assert(strcmp(test_cases[i], roundtrip) == 0);
    }
  }
}

void test_surrogate_handling() {
  const char *surrogate = "\xF0\x9D\x84\x9E"; // U+1D11E (MUSICAL SYMBOL G CLEF)
  fchar_t unicode_buf[64] = {0};

  size_t conv = faster_mb_to_unicode(surrogate, unicode_buf, 64);

#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_TWO_BYTE
  // UTF-16 should use surrogate pair (two char16_t values)
  assert(conv == 2);
  assert(unicode_buf[0] == 0xD834);
  assert(unicode_buf[1] == 0xDD1E);
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_FOUR_BYTE
  // UTF-32 should use a single value
  assert(conv == 1);
  assert(unicode_buf[0] == 0x1D11E);
#endif

  // Round-trip test
  char roundtrip[64] = {0};
  faster_unicode_to_mb(unicode_buf, roundtrip, 64);

  // Compare bytes directly since printf may not render it correctly
  assert(memcmp(surrogate, roundtrip, 4) == 0);
}

void test_invalid_sequences() {
  const char *invalid = "abc\xFF\xFE";
  fchar_t unicode_buf[64] = {0};

  size_t conv = faster_mb_to_unicode(invalid, unicode_buf, 64);

  // Basic verification for all modes
  assert(unicode_buf[0] == 'a');
  assert(unicode_buf[1] == 'b');
  assert(unicode_buf[2] == 'c');

#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_NONE
  // In non-Unicode mode, all bytes pass through unchanged
  assert(conv == 5);
  assert((unsigned char)unicode_buf[3] == 0xFF);
  assert((unsigned char)unicode_buf[4] == 0xFE);
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_ONE_BYTE
  // In one-byte mode, invalid bytes become - EF BF BD - � (replacement
  // character)
  size_t tmp = strlen(FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE);
  assert(conv == (3 + 2 * tmp));
  assert(memcmp(unicode_buf + 3 + 0, FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE, tmp) == 0);
  assert(memcmp(unicode_buf + 3 + tmp, FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE, tmp) == 0);
#else
  // In two-byte and four-byte modes, invalid bytes become U+FFFD - �
  // (replacement character)
  assert(conv == 5);
  assert(unicode_buf[3] == FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE);
  assert(unicode_buf[4] == FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE);
#endif
}

#if FASTER_UNICODE_SUPPORT != FASTER_UNICODE_SUPPORT_NONE
#define _TEST_LOCALE "C.UTF-8"
#else
#define _TEST_LOCALE "C"
#endif

int main(void) {
  char *act_locale = setlocale(LC_ALL, _TEST_LOCALE);
  assert(act_locale != NULL);
  assert(strcmp(act_locale, _TEST_LOCALE) == 0);
  printf("Locale set to: %s\n", act_locale);
  test_buffer_overflow();
  test_stub_conversion();
  test_invalid_sequences();
  test_surrogate_handling();
  test_conversion_roundtrip();
  return 0;
}
