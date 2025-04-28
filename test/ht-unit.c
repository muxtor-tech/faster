#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "aster/faster_ht.h"

int main(int argc, char *argv[]) {
  faster_ht_t ht;
  char str_ptr[64];
  fchar_t aster_text[64];
  fchar_t **aster_text_ptr;

  int generation = RAND_MAX / 1000;

  // check if there was an argument passed, and if it is a number, use it to update the generation
  if (argc > 1) {
    generation = atoi(argv[1]);
    if (generation <= 0) {
      printf("Invalid argument, using default generation of %d\n", generation);
      generation = RAND_MAX / 1000;
    }
  }

  aster_text_ptr = (fchar_t **)malloc(sizeof(fchar_t *) * generation * 5);
  if (aster_text_ptr == NULL) {
    printf("Failed to allocate memory for aster_text_ptr\n");
    return -1;
  }
  size_t aster_text_ctr = 0;

#define managed_strdup(str) ((aster_text_ptr[aster_text_ctr++] = faster_strdup(str)))

  // simple test for the colllisions of the _simple_hash_function
  // this is a simple test to check the hash function
  // it is not a real test for the hash table

  int collision_count = 0;
  uint8_t test_hash[generation];
  int test_fill_percent = 75;
  int test_fill_size = sizeof(test_hash) * test_fill_percent / 100;
  memset(test_hash, 0, generation);

  clock_t start_time = clock();
  for (int i = 0; i < test_fill_size; i++) {
    sprintf(str_ptr, "%ukey", i);
    faster_mb_to_unicode(str_ptr, aster_text, 64);
    faster_ht_key_data_t key = {aster_text, faster_str_bytelen(aster_text)};
    faster_hash_value_t hash = faster_ht_hash(&key);
    if (test_hash[hash % sizeof(test_hash)] == 0) {
      test_hash[hash % sizeof(test_hash)] = 1;
    } else {
      collision_count++;
    }
  }
  clock_t end_time = clock();
  double avg_hash_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) / (test_fill_size);
  printf("Average hash calculation time %f useconds for %u insertions \n", avg_hash_time * 1000000, test_fill_size);

  printf("Collisions: %d\n", collision_count);
  double collision_rate = (double)collision_count / sizeof(test_hash);
  printf("Collision rate: %f\n", collision_rate);
  if (collision_rate > 0.25) {
    printf("Hash function has more than 25%% collisions\n");
    // return -1;
  } else {
    printf("Hash function has acceptable collisions\n");
  }

  if (faster_ht_init(&ht, 1000, faster_ht_hash) != FAST_ERROR_NONE) {
    printf("Failed to initialize hash table\n");
    return -1;
  }
  faster_ht_key_data_t key1 = {managed_strdup(ASTER_TEXT("1234")), faster_str_bytelen(ASTER_TEXT("1234"))};
  faster_ht_key_data_t key2 = {managed_strdup(ASTER_TEXT("7890")), faster_str_bytelen(ASTER_TEXT("7890"))};
  if (faster_ht_set(&ht, &key1, (faster_value_ptr)5678) != FAST_ERROR_NONE) {
    printf("Failed to set initial value\n");
    faster_ht_free(&ht);
    return -1;
  }
  if (faster_ht_set(&ht, &key2, (faster_value_ptr)2345) != FAST_ERROR_NONE) {
    printf("Failed to set another value\n");
    faster_ht_free(&ht);
    return -1;
  }
  if (faster_ht_get(&ht, &key1) != (faster_value_ptr)5678) {
    printf("Failed to get initial value\n");
    faster_ht_free(&ht);
    return -1;
  }
  if (faster_ht_get(&ht, &key2) != (faster_value_ptr)2345) {
    printf("Failed to get another value\n");
    faster_ht_free(&ht);
    return -1;
  }
  if (faster_ht_remove(&ht, &key1) != FAST_ERROR_NONE) {
    printf("Failed to remove initial value\n");
    faster_ht_free(&ht);
    return -1;
  }
  if (faster_ht_get(&ht, &key1) != FASTER_INVALID_VALUE_PTR) {
    printf("Initial value still exists after removal\n");
    faster_ht_free(&ht);
    return -1;
  }
  // try freeing and recreating
  faster_ht_free(&ht);
  if (faster_ht_init(&ht, 1000, faster_ht_hash) != FAST_ERROR_NONE) {
    printf("Failed to REinitialize hash table\n");
    return -1;
  }

  srand(0);

  // fill and rinse several times
  for (int i = 0; i < 3; i++) {
    // insertion
    int insertions = 0;
    start_time = clock();
    for (int i = 0; i < (generation / 10); i++) {
      intptr_t random_number = i + 1; // rand() % generation;
      sprintf(str_ptr, "key%ld", random_number);
      faster_mb_to_unicode(str_ptr, aster_text, 64);
      faster_ht_key_data_t key = {managed_strdup(aster_text), faster_str_bytelen(aster_text)};
      if (faster_ht_set(&ht, &key, (faster_value_ptr)random_number) != FAST_ERROR_NONE) {
        printf("Failed to insert key: %s\n", str_ptr);
        faster_ht_free(&ht);
        return -1;
      } else {
        insertions++;
      }
    }
    end_time = clock();
    double avg_insertion_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) / (generation / 10);
    printf("Average insertion time: %f useconds for %u insertions \n", avg_insertion_time * 1000000, insertions);

    // lookup
    faster_indexing_t lookup = 0;
    clock_t seek_start_time = clock();
    for (int i = 0; i < generation / 10; i++) {
      intptr_t random_number = i + 1; // rand() % generation;
      sprintf(str_ptr, "key%ld", random_number);
      faster_mb_to_unicode(str_ptr, aster_text, 64);
      faster_ht_key_data_t key = {managed_strdup(aster_text), faster_str_bytelen(aster_text)};
      faster_value_ptr value = faster_ht_get(&ht, &key);
      if (value == (faster_value_ptr)random_number) {
        lookup++;
      } else if (value != NULL) {
        printf("Found wrong data: %s %ld\n", str_ptr, (long)value);
        return -1;
      }
    }
    clock_t seek_end_time = clock();
    double avg_seek_time = ((double)(seek_end_time - seek_start_time) / CLOCKS_PER_SEC) / (generation / 10);
    printf("Average seek time: %f useconds\n", avg_seek_time * 1000000);

    if (lookup == insertions) {
      printf("(needed %u) At least %u keys found in hash\n", insertions, lookup);
    } else {
      printf("Not enough (only %u vs %u) keys found in hash\n", lookup, insertions);
      return -1;
    }

    // negative lookup (non-existing keys)
    faster_indexing_t neg_lookup = 0;
    clock_t nseek_start_time = clock();
    for (int i = 0; i < generation / 10; i++) {
      intptr_t random_number = i + 1; // rand() % generation;
      sprintf(str_ptr, "ne-key%ld", random_number);
      faster_mb_to_unicode(str_ptr, aster_text, 64);
      faster_ht_key_data_t key = {managed_strdup(aster_text), faster_str_bytelen(aster_text)};
      faster_value_ptr value = faster_ht_get(&ht, &key);
      if (value == FASTER_INVALID_VALUE_PTR) {
        neg_lookup++;
      } else {
        printf("Found non-existing key: %s %ld\n", str_ptr, (long)value);
        return -1;
      }
    }
    clock_t nseek_end_time = clock();
    double avg_nseek_time = ((double)(nseek_end_time - nseek_start_time) / CLOCKS_PER_SEC) / (generation / 10);
    printf("Average negative seek time: %f useconds\n", avg_nseek_time * 1000000);

    if (neg_lookup == (generation / 10)) {
      printf("(needed %u) All %u non-keys not-found in hash\n", (generation / 10), neg_lookup);
    }

    // removal with pre and post checks
    faster_indexing_t removal = 0;
    faster_indexing_t found_pre_removal = 0;
    faster_indexing_t found_post_removal = 0;
    clock_t rem_start_time = clock();
    char test[64];
    for (int i = 0; i < generation / 10; i++) {
      intptr_t random_number = i + 1; // rand() % generation;
      sprintf(str_ptr, "key%ld", random_number);
      faster_mb_to_unicode(str_ptr, aster_text, 64);
      faster_ht_key_data_t key = {managed_strdup(aster_text), faster_str_bytelen(aster_text)};
      faster_unicode_to_mb(key.ptr, test, 64);
      if (strcmp(str_ptr, test) != 0) {
        printf("Failed to convert key: %s %s\n", str_ptr, test);
        return -1;
      }
      faster_value_ptr pvalue = faster_ht_get(&ht, &key);
      if (pvalue != FASTER_INVALID_VALUE_PTR) {
        found_pre_removal++;
      } else {
        printf("Not found an expected key: %s %ld\n", str_ptr, (long)pvalue);
        return -1;
      }
      faster_error_code_t updateec = faster_ht_set(&ht, &key, (faster_value_ptr)pvalue + 1);
      if (updateec != FAST_ERROR_NONE) {
        printf("Failed to update key: %s %ld\n", str_ptr, (long)pvalue);
        return -1;
      }
      faster_value_ptr pvalue2 = faster_ht_get(&ht, &key);
      if (pvalue2 != (faster_value_ptr)pvalue + 1) {
        printf("Failed to update key: %s %ld\n", str_ptr, (long)pvalue);
        return -1;
      }
      faster_error_code_t value = faster_ht_remove(&ht, &key);
      if (value == FAST_ERROR_NONE) {
        removal++;
      } else {
        printf("Failed to remove key: %s %ls %ld\n", str_ptr, (wchar_t *)aster_text, (long)pvalue);
        return -1;
      }
      faster_value_ptr povalue = faster_ht_get(&ht, &key);
      if (povalue != FASTER_INVALID_VALUE_PTR) {
        printf("Found non-existing key: %s %ld\n", str_ptr, (long)povalue);
        found_post_removal++;
        return -1;
      }
    }
    clock_t rem_end_time = clock();
    double avg_rem_time = ((double)(rem_end_time - rem_start_time) / CLOCKS_PER_SEC) / (generation / 10);
    printf("Average find and removal time: %f useconds\n", avg_rem_time * 1000000);
    printf("Found %u keys pre-removal\n", found_pre_removal);
    if (removal == insertions) {
      printf("(needed %u) At least %u keys removed from hash\n", insertions, removal);
    } else {
      printf("Not enough (only %u vs %u) keys removed from hash\n", removal, insertions);
      printf("Hash table is not empty\n");
      printf("Elements: %u\n", ht.elements);
      return -1;
    }
  }

  // check if empty
  if (ht.elements != 0) {
    printf("Hash table is not empty\n");
    printf("Elements: %u\n", ht.elements);
    return -1;
  }

  faster_ht_free(&ht);

  // free all the strings
  for (size_t i = 0; i < aster_text_ctr; i++) {
    free(aster_text_ptr[i]);
  }
  free(aster_text_ptr);

  return 0;
}
