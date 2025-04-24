#include <stdio.h>
#include <string.h>
#include <time.h>

#include "aster/faster_avl.h"

int main() {
    DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(avl_tree, 256);

    srand(0);
    int generation = RAND_MAX / 400;
    
    clock_t start_time = clock();
    for(int i = 0; i<(generation/10); i++) {
        intptr_t random_number = rand() % generation;
        char str_ptr[64];
        fchar_t aster_text[64];
        sprintf(str_ptr, "key%ld", random_number+1);
        faster_mb_to_unicode(str_ptr, aster_text, 64);
        faster_str_t keyp = {faster_strdup(aster_text), faster_strlen(aster_text)};
        AVL_insert_or_update(&avl_tree, &keyp, (faster_value_ptr)random_number+1);
    }
    clock_t end_time = clock();
    double avg_insertion_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) / (generation / 10);
    printf("Average insertion time: %f useconds\n", avg_insertion_time * 1000000);

    printf("AVL Tree node count: %u\n", avl_tree.node_list.list_header.array_internal);
    printf("AVL Tree node capacity: %u\n", avl_tree.node_list.list_header.array_capacity);
    printf("AVL Tree node root index: %u\n", avl_tree.root_node);
    printf("AVL Tree height: %u\n", avl_tree.node_list.list[avl_tree.root_node].height);

    faster_indexing_t found_counter = 0;
    clock_t seek_start_time = clock();
    for(faster_indexing_t i = 0; i<avl_tree.node_list.list_header.array_internal; i++) {
        faster_value_ptr tmp = AVL_get(&avl_tree, (const faster_str_ptr_t)&avl_tree.node_list.list[i].key);
        if (tmp != NULL) {
            found_counter++;
        } else {
            printf("Key not found in AVL Tree\n");
            return -1;
        }
    }
    clock_t seek_end_time = clock();
    double avg_seek_time = ((double)(seek_end_time - seek_start_time) / CLOCKS_PER_SEC) / avl_tree.node_list.list_header.array_internal;
    printf("Average seek time: %f useconds\n", avg_seek_time * 1000000);

    faster_indexing_t random_lookup = 0;
    for(int i = 0; i<generation/10; i++) {
        intptr_t random_number = rand() % generation;
        char str_ptr[64];
        fchar_t aster_text[64];
        sprintf(str_ptr, "key%ld", random_number+1);
        faster_mb_to_unicode(str_ptr, aster_text, 64);
        faster_str_t keyp = {aster_text, faster_strlen(aster_text)};
        if (AVL_get(&avl_tree, &keyp) == (faster_value_ptr)random_number+1) {
            random_lookup++;
        }
    }

    if (found_counter == avl_tree.node_list.list_header.array_internal) {
        printf("All keys found in AVL Tree\n");
    } else {
        printf("Not all keys found in AVL Tree\n");
        return -1;
    }

    generation /= 1000;
    if (random_lookup > generation) {
        printf("(%u) At least %u random keys found in AVL Tree\n", random_lookup, generation);
    } else {
        printf("Not enough random keys found in AVL Tree\n");
        return -1;
    }

    AVL_reset_and_free(&avl_tree);

    return 0;
}
