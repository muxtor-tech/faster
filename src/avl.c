#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aster/faster_avl.h"

// Utility functions
static int max(const int a, const int b) { return (a > b) ? a : b; }
static int height(const AVLNodesTreePtr tree, const AVLNodeIndex node) {
  return FASTER_AVL_NODE_VALID(node) ? tree->node_list.list[node].height : 0;
}
static AVLNodeIndex min_value_node(const AVLNodesTreePtr tree, const AVLNodeIndex node) {
  AVLNodeIndex current = node;
  while (FASTER_AVL_NODE_VALID(tree->node_list.list[current].left)) {
    current = tree->node_list.list[current].left;
  }
  return current;
}

// Iterator for AVL tree
AVLNodeIndex AVL_iterator(AVLNodesTreePtr tree, faster_avl_tree_iterator_helper_t *it) {
  // Initialize the iterator on the first call.
  if (!it->initialized) {
    it->top = -1;
    AVLNodeIndex current = tree->root_node;
    while (FASTER_AVL_NODE_VALID(current)) {
      if (it->top < FASTER_MAX_AVL_ITERATOR_STACK_SIZE - 1) {
        it->stack[++(it->top)] = current;
      }
      // Move to left child
      AVLNodePtr node_ptr = tree->node_list.list + current;
      current = node_ptr->left;
    }
    it->initialized = 1;
  }

  // If the stack is empty, we have traversed all nodes.
  if (it->top < 0) {
    return FASTER_AVL_NODE_INDEX_INVALID;
  }

  // Pop the next node from the stack as the current in-order node.
  AVLNodeIndex node = it->stack[it->top--];

  // If there is a right subtree, push its leftmost nodes onto the stack.
  AVLNodePtr node_ptr = tree->node_list.list + node;
  AVLNodeIndex current = node_ptr->right;
  while (FASTER_AVL_NODE_VALID(current)) {
    if (it->top < FASTER_MAX_AVL_ITERATOR_STACK_SIZE - 1) {
      it->stack[++(it->top)] = current;
    }
    node_ptr = tree->node_list.list + current;
    current = node_ptr->left;
  }

  return node;
}

// Create a new node
static AVLNodeIndex createNode(const AVLNodesTreePtr tree, const faster_str_ptr_t key, faster_value_ptr value) {
  AVLNodeIndex node = AVLNode_t_arr_get_next(&tree->node_list);
  if (FASTER_AVL_NODE_INVALID(node)) {
    return FASTER_AVL_NODE_INDEX_INVALID;
  }
  AVLNodePtr node_ptr = tree->node_list.list + node;
  memcpy((void *)&node_ptr->key, key, sizeof(faster_str_t));
  node_ptr->left = FASTER_AVL_NODE_INDEX_INVALID;
  node_ptr->right = FASTER_AVL_NODE_INDEX_INVALID;
  node_ptr->height = 1;
  node_ptr->value = value;
  return node;
}

// Right rotate subtree rooted with y
static AVLNodeIndex rightRotate(const AVLNodesTreePtr tree, AVLNodeIndex y) {
  AVLNodePtr y_ptr = tree->node_list.list + y;
  AVLNodeIndex x = y_ptr->left;
  AVLNodePtr x_ptr = tree->node_list.list + x;
  AVLNodeIndex T2 = x_ptr->right;

  // Perform rotation
  x_ptr->right = y;
  y_ptr->left = T2;

  // Update heights
  y_ptr->height = max(height(tree, y_ptr->left), height(tree, y_ptr->right)) + 1;
  x_ptr->height = max(height(tree, x_ptr->left), height(tree, x_ptr->right)) + 1;

  return x;
}

// Left rotate subtree rooted with x
static AVLNodeIndex leftRotate(const AVLNodesTreePtr tree, AVLNodeIndex x) {
  AVLNodePtr x_ptr = tree->node_list.list + x;
  AVLNodeIndex y = x_ptr->right;
  AVLNodePtr y_ptr = tree->node_list.list + y;
  AVLNodeIndex T2 = y_ptr->left;

  // Perform rotation
  y_ptr->left = x;
  x_ptr->right = T2;

  // Update heights
  x_ptr->height = max(height(tree, x_ptr->left), height(tree, x_ptr->right)) + 1;
  y_ptr->height = max(height(tree, y_ptr->left), height(tree, y_ptr->right)) + 1;

  return y;
}

// Get balance factor of a node
static int getBalance(const AVLNodesTreePtr tree, const AVLNodeIndex node) {
  AVLNodePtr node_ptr = tree->node_list.list + node;
  return FASTER_AVL_NODE_VALID(node) ? (height(tree, node_ptr->left) - height(tree, node_ptr->right)) : 0;
}

// insert wrapper
static AVLNodeIndex _AVL_insert(const AVLNodesTreePtr tree, const AVLNodeIndex node, const faster_str_ptr_t key,
                                const faster_value_ptr value, bool *found);
bool AVL_insert_or_update(AVLNodesTreePtr tree, const faster_str_ptr_t key, const faster_value_ptr value) {
  bool found = false;
  tree->root_node = _AVL_insert(tree, tree->root_node, key, value, &found);
  return !found;
}

// Insert a key into the AVL tree
static AVLNodeIndex _AVL_insert(const AVLNodesTreePtr tree, const AVLNodeIndex node, const faster_str_ptr_t key,
                                const faster_value_ptr value, bool *found) {
  if (FASTER_AVL_NODE_INVALID(node))
    return createNode(tree, key, value);
  AVLNodeIndex index = node;

  int cmp = faster_str_cmp_binary(key, &tree->node_list.list[node].key);
  if (cmp < 0) {
    index = _AVL_insert(tree, tree->node_list.list[node].left, key, value, found);
    tree->node_list.list[node].left = index;
  } else if (cmp > 0) {
    index = _AVL_insert(tree, tree->node_list.list[node].right, key, value, found);
    tree->node_list.list[node].right = index;
  } else {
    // Update value if key exists
    *found = true;
    tree->node_list.list[node].value = value;
    return node;
  }

  // Update height of this ancestor node
  tree->node_list.list[node].height =
      max(height(tree, tree->node_list.list[node].left), height(tree, tree->node_list.list[node].right)) + 1;

  // Get balance factor to check if this node became unbalanced
  int balance = getBalance(tree, node);

  if (balance > 1) {
    if (faster_str_cmp_binary(key, &tree->node_list.list[tree->node_list.list[node].left].key) < 0) {
      return rightRotate(tree, node);
    } else {
      tree->node_list.list[node].left = leftRotate(tree, tree->node_list.list[node].left);
      return rightRotate(tree, node);
    }
  }

  if (balance < -1) {
    if (faster_str_cmp_binary(key, &tree->node_list.list[tree->node_list.list[node].right].key) > 0) {
      return leftRotate(tree, node);
    } else {
      tree->node_list.list[node].right = rightRotate(tree, tree->node_list.list[node].right);
      return leftRotate(tree, node);
    }
  }

  return node; // Return unchanged pointer
}

faster_value_ptr AVL_get(const AVLNodesTreePtr tree, const faster_str_ptr_t key) {
  // navigate tree using binary search
  AVLNodeIndex node = tree->root_node;
  while (FASTER_AVL_NODE_VALID(node)) {
    AVLNodePtr node_ptr = tree->node_list.list + node;
    int cmp = faster_str_cmp_binary(key, &node_ptr->key);
    if (cmp == 0)
      return node_ptr->value; // key found
    node = (cmp < 0) ? node_ptr->left : node_ptr->right;
  }
  return NULL; // key not found
}

static AVLNodeIndex _AVL_remove(const AVLNodesTreePtr tree, const AVLNodeIndex node, const faster_str_ptr_t key, bool *found);
bool AVL_remove(AVLNodesTreePtr tree, const faster_str_ptr_t key) {
  bool found = false;
  tree->root_node = _AVL_remove(tree, tree->root_node, key, &found);
  return found; // key removed
}

static AVLNodeIndex _AVL_remove(const AVLNodesTreePtr tree, const AVLNodeIndex node, const faster_str_ptr_t key, bool *found) {
  if (FASTER_AVL_NODE_INVALID(node))
    return node;
  AVLNodeIndex index = node;
  AVLNodePtr node_ptr = tree->node_list.list + index;
  if (faster_str_cmp_binary(key, &node_ptr->key) < 0) {
    node_ptr->left = _AVL_remove(tree, node_ptr->left, key, found);
  } else if (faster_str_cmp_binary(key, &node_ptr->key) > 0) {
    node_ptr->right = _AVL_remove(tree, node_ptr->right, key, found);
  } else {
    *found = true; // key found
    // Node with only one child or no child
    if (FASTER_AVL_NODE_INVALID(node_ptr->left)) {
      AVLNodeIndex temp = node_ptr->right;
      AVLNode_t_arr_release(&tree->node_list, index); // Release the node
      return temp;
    } else if (FASTER_AVL_NODE_INVALID(node_ptr->right)) {
      AVLNodeIndex temp = node_ptr->left;
      AVLNode_t_arr_release(&tree->node_list, index); // Release the node
      return temp;
    }
    // Node with two children: Get the inorder successor
    AVLNodeIndex temp = min_value_node(tree, node_ptr->right);
    memcpy((void *)&node_ptr->key, &tree->node_list.list[temp].key, sizeof(faster_str_t));
    node_ptr->value = tree->node_list.list[temp].value;
    node_ptr->right = _AVL_remove(tree, node_ptr->right, (const faster_str_ptr_t)&tree->node_list.list[temp].key, found);
  }

  if (FASTER_AVL_NODE_INVALID(index))
    return index;

  // Update height of this ancestor index
  node_ptr->height = max(height(tree, node_ptr->left), height(tree, node_ptr->right)) + 1;

  // Get balance factor to check if this index became unbalanced
  int balance = getBalance(tree, index);
  if (balance > 1) {
    if (getBalance(tree, node_ptr->left) >= 0) {
      return rightRotate(tree, index);
    } else {
      node_ptr->left = leftRotate(tree, node_ptr->left);
      return rightRotate(tree, index);
    }
  }
  if (balance < -1) {
    if (getBalance(tree, node_ptr->right) <= 0) {
      return leftRotate(tree, index);
    } else {
      node_ptr->right = rightRotate(tree, node_ptr->right);
      return leftRotate(tree, index);
    }
  }
  return index; // Return unchanged pointer
}

void AVL_reset_and_free(const AVLNodesTreePtr tree) {
  tree->node_list.list_header.array_capacity = 0;
  tree->node_list.list_header.array_internal = 0;
  tree->node_list.list_header.next_free_index = FASTER_ARRAY_COUNT_INVALID;
  tree->root_node = FASTER_AVL_NODE_INDEX_INVALID;
  free(tree->node_list.list);
  tree->node_list.list = NULL;
}
