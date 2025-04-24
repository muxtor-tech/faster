#ifdef FASTER_AVL_INCLUDE
#else

#include "aster/faster_core.h"
#include <stdlib.h>

#define FASTER_AVL_NODE_INDEX_INVALID FASTER_ARRAY_COUNT_INVALID
#define FASTER_AVL_NODE_VALID(node) ((node) != FASTER_AVL_NODE_INDEX_INVALID)
#define FASTER_AVL_NODE_INVALID(node) ((node) == FASTER_AVL_NODE_INDEX_INVALID)

#define FASTER_MAX_AVL_ITERATOR_STACK_SIZE 64
#define FASTER_AVL_TREE_EMPTY_ITERATOR {.top = -1, .initialized = 0}

typedef faster_indexing_t AVLNodeIndex;

// Node structure for AVL Tree
struct AVLNode_t_s {
  faster_str_t key;
  faster_value_ptr value;
  AVLNodeIndex left;
  AVLNodeIndex right;
  int height;
} FASTER_ALIGNED;
typedef struct AVLNode_t_s AVLNode_t;
typedef struct AVLNode_t_s *AVLNodePtr;

// list
DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(AVLNode_t);

struct AVLNodesTree_t_s {
  faster_indexing_t root_node;
  AVLNode_t_arr_t node_list;
};
typedef struct AVLNodesTree_t_s AVLNodesTree_t;
typedef struct AVLNodesTree_t_s *AVLNodesTreePtr;

typedef struct {
  AVLNodeIndex stack[FASTER_MAX_AVL_ITERATOR_STACK_SIZE];
  int top;         // Index of the top element in the stack (-1 when empty)
  int initialized; // Flag to indicate if the iterator has been initialized
} faster_avl_tree_iterator_helper_t;

#define DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(name, initial_capacity)                                                      \
  DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name##_array, AVLNode_t, initial_capacity);                                           \
  AVLNodesTree_t name = {.root_node = FASTER_AVL_NODE_INDEX_INVALID, .node_list = name##_array}

AVLNodeIndex AVL_iterator(AVLNodesTreePtr tree, faster_avl_tree_iterator_helper_t *it);
bool AVL_insert_or_update(const AVLNodesTreePtr tree, const faster_str_ptr_t key, const faster_value_ptr value);
faster_value_ptr AVL_get(const AVLNodesTreePtr tree, const faster_str_ptr_t key);
bool AVL_remove(const AVLNodesTreePtr tree, const faster_str_ptr_t key);
void AVL_reset_and_free(const AVLNodesTreePtr tree);

#define FASTER_AVL_INCLUDE
#endif // FASTER_AVL_INCLUDE
