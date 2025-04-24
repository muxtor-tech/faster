#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "aster/faster_avl.h"

int main() {
    DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(avl_tree, 0);
    faster_str_t key1 = {ASTER_TEXT("key1"), 4};
    faster_str_t key2 = {ASTER_TEXT("key2"), 4};
    faster_str_t key3 = {ASTER_TEXT("key3"), 4};
    faster_str_t key4 = {ASTER_TEXT("key4"), 4};
    faster_str_t key5 = {ASTER_TEXT("key5"), 4};
    
    AVL_insert_or_update(&avl_tree, &key1, (faster_value_ptr)1);
    AVL_insert_or_update(&avl_tree, &key2, (faster_value_ptr)2);
    AVL_insert_or_update(&avl_tree, &key3, (faster_value_ptr)3);
    AVL_insert_or_update(&avl_tree, &key5, (faster_value_ptr)5);

    printf("AVL Tree node count: %u\n", avl_tree.node_list.list_header.array_internal);
    printf("AVL Tree node capacity: %u\n", avl_tree.node_list.list_header.array_capacity);
    printf("AVL Tree node root index: %u\n", avl_tree.root_node);
    printf("AVL Tree height: %u\n", avl_tree.node_list.list[avl_tree.root_node].height);

    for (faster_indexing_t i = 0; i < avl_tree.node_list.list_header.array_internal; i++) {
        char str_ptr[128];
        faster_unicode_to_mb(avl_tree.node_list.list[i].key.str_ptr, str_ptr, avl_tree.node_list.list[i].key.str_len + 1);
        printf("Node %u: key=%.*s, l:%u r:%u value=%p\n", i, (int)avl_tree.node_list.list[i].key.str_len, str_ptr,
                avl_tree.node_list.list[i].left, avl_tree.node_list.list[i].right, 
                avl_tree.node_list.list[i].value);
        if (AVL_get(&avl_tree, (const faster_str_ptr_t)&avl_tree.node_list.list[i].key) != NULL) {
            printf("Key found in AVL Tree\n");
        } else {
            printf("Key not found in AVL Tree\n");
            return -1;
        }
    }

    if (AVL_get(&avl_tree, &key4) != NULL) {
        printf("Key4 found in AVL Tree\n");
        return -1;
    } else {
        printf("Key4 not found in AVL Tree\n");
    }
    
    AVL_reset_and_free(&avl_tree);

    return 0;
}
