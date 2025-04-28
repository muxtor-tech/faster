#include <stdio.h>
#include <string.h>

#include "aster/faster_avl.h"
#include <time.h>

int main(int argc, char *argv[]) {
  DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(avl_tree, 256);

  srand(0);
  int generation = RAND_MAX / 1000;

  if (argc > 1) {
    generation = atoi(argv[1]);
    if (generation <= 0) {
      printf("Invalid argument, using default generation of %d\n", generation);
      generation = RAND_MAX / 1000;
    }
  }

  faster_indexing_t insertions = 0;
  clock_t start_time = clock();
  for (int i = 0; i < (generation / 10); i++) {
    intptr_t random_number = rand() % generation;
    char str_ptr[64];
    fchar_t aster_text[64];
    sprintf(str_ptr, "key%ld", random_number + 1);
    faster_mb_to_unicode(str_ptr, aster_text, 64);
    faster_str_t keyp = {faster_strdup(aster_text), faster_strlen(aster_text)};
    if (AVL_insert_or_update(&avl_tree, &keyp, (faster_value_ptr)random_number + 1))
      insertions++;
  }
  clock_t end_time = clock();
  double avg_insertion_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) / (generation / 10);
  printf("Average insertion time: %f useconds\n", avg_insertion_time * 1000000);

  printf("AVL Tree node count: %u\n", avl_tree.node_list.list_header.array_internal);
  printf("AVL Tree node capacity: %u\n", avl_tree.node_list.list_header.array_capacity);
  printf("AVL Tree node root index: %u\n", avl_tree.root_node);
  printf("AVL Tree height: %u\n", avl_tree.node_list.list[avl_tree.root_node].height);

  fchar_t aster_textx[] = ASTER_TEXT("key-notfound");
  faster_str_t keyp = {aster_textx, faster_strlen(aster_textx)};
  if (AVL_remove(&avl_tree, &keyp) == true) {
    printf("ERROR: Key notfound removed from AVL Tree\n");
    return -1;
  } else {
    printf("OK: Key notfound not found in AVL Tree\n");
  }

  clock_t rem_start_time = clock();
  faster_indexing_t removal = 0;
  for (int i = 0; i < generation / 2; i++) {
    intptr_t random_number = rand() % generation;
    char str_ptr[64];
    fchar_t aster_text[64];
    sprintf(str_ptr, "key%ld", random_number + 1);
    faster_mb_to_unicode(str_ptr, aster_text, 64);
    faster_str_t keyp = {aster_text, faster_strlen(aster_text)};
    if (AVL_remove(&avl_tree, &keyp) == true) {
      removal++;
    }
  }
  clock_t rem_end_time = clock();
  double avg_rem_time = ((double)(rem_end_time - rem_start_time) / CLOCKS_PER_SEC) / (generation / 2);
  printf("Average removal time of %u items over %u deletion attempts: %f "
         "useconds\n",
         removal, generation / 2, avg_rem_time * 1000000);

  faster_indexing_t found_counter = 0;
  faster_avl_tree_iterator_helper_t it = FASTER_AVL_TREE_EMPTY_ITERATOR;
  clock_t seek_start_time = clock();
  AVLNodeIndex node;
  while ((node = AVL_iterator(&avl_tree, &it)) != FASTER_AVL_NODE_INDEX_INVALID) {
    faster_value_ptr tmp = AVL_get(&avl_tree, (const faster_str_ptr_t)&avl_tree.node_list.list[node].key);
    if (tmp != NULL) {
      found_counter++;
    }
  }
  clock_t seek_end_time = clock();
  double avg_seek_time = ((double)(seek_end_time - seek_start_time) / CLOCKS_PER_SEC) / found_counter;
  printf("Average seek time: %f useconds\n", avg_seek_time * 1000000);

  if ((found_counter + removal) == insertions) {
    printf("Structure OK for AVL Tree\n");
  } else {
    printf("Questionable structure of AVL Tree (%u inserted, %u found, %u "
           "removed, %u nodes)\n",
           insertions, found_counter, removal, avl_tree.node_list.list_header.array_internal);
    return -1;
  }

  faster_indexing_t capacity_before_adding = avl_tree.node_list.list_header.array_capacity;
  for (int i = 0; i < removal; i++) {
    intptr_t random_number = rand() % generation;
    char str_ptr[64];
    fchar_t aster_text[64];
    sprintf(str_ptr, "key%ld", random_number + 1);
    faster_mb_to_unicode(str_ptr, aster_text, 64);
    faster_str_t keyp = {aster_text, faster_strlen(aster_text)};
    AVL_insert_or_update(&avl_tree, &keyp, (faster_value_ptr)random_number + 1);
  }
  if (avl_tree.node_list.list_header.array_capacity > capacity_before_adding) {
    printf("AVL Tree resized unnecesarly\n");
    return -1;
  } else {
    printf("AVL Tree did not resize, capacity was reused\n");
  }

  AVL_reset_and_free(&avl_tree);

  return 0;
}
