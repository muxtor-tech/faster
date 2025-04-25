#ifdef FASTER_AST_INCLUDE
#else

#include "faster_core.h"
#include "faster_avl.h"
#include "faster_is.h"

enum faster_token_type_e {
  FAST_TOKEN_TYPE_NONE = 0x00,
  FAST_TOKEN_TYPE_INVALID,
  FAST_TOKEN_TYPE_STRING,
  FAST_TOKEN_TYPE_NUMBER,
  FAST_TOKEN_TYPE_OPERATOR,
  FAST_TOKEN_TYPE_ORP,
  FAST_TOKEN_TYPE_OSP,
  FAST_TOKEN_TYPE_OCP,
  FAST_TOKEN_TYPE_CRP,
  FAST_TOKEN_TYPE_CSP,
  FAST_TOKEN_TYPE_CCP,
  FAST_TOKEN_TYPE_NULL,
  FAST_TOKEN_TYPE_TREE_NODE_NONE = 0x100,
  FAST_TOKEN_TYPE_TREE_NODE_INVALID,
  FAST_TOKEN_TYPE_TREE_NODE_OPERATOR,
  FAST_TOKEN_TYPE_TREE_NODE_STRING,
  FAST_TOKEN_TYPE_TREE_NODE_NUMBER,
  FAST_TOKEN_TYPE_TREE_NODE_NULL,
  FAST_TOKEN_TYPE_TREE_NODE_LIST,
  FAST_TOKEN_TYPE_TREE_NODE_LIST_END,
  FAST_TOKEN_TYPE_TREE_NODE_LIST_START
};
typedef enum faster_token_type_e faster_token_type_t;

enum faster_error_codes_e {
  FAST_AST_ERROR_NONE = 0x00,
  FAST_AST_ERROR_INVALID_TOKEN,
  FAST_AST_ERROR_INVALID_NODE,
  FAST_AST_ERROR_INVALID_CONTEXT,
  FAST_AST_ERROR_INVALID_STATE,
  FAST_AST_ERROR_INVALID_VALUE,
  FAST_AST_ERROR_INVALID_STRING,
  FAST_AST_ERROR_INVALID_OPERATOR,
  FAST_AST_ERROR_INVALID_LIST,
  FAST_AST_ERROR_INVALID_PARAMETER,
};
typedef enum faster_error_codes_e faster_error_code_t;

enum faster_ast_state_e {
  FAST_AST_STATE_NOT_INITIALIZED = 0x00,
  FAST_AST_STATE_EMPTY,
  FAST_AST_STATE_TOKENIZED,
  FAST_AST_STATE_READY,
  FAST_AST_STATE_EXECUTING
};
typedef enum faster_ast_state_e faster_ast_state_t;

struct faster_token_t_s {
  faster_token_type_t token_type;
  faster_str_t token_str;
} FASTER_ALIGNED_UNPACKED;
typedef struct faster_token_t_s faster_token_t;
typedef struct faster_token_t_s *faster_token_ptr_t;

// typedef a function pointer to a function that takes an index of a ast_node
// and the ast structure and returns a pointer
struct faster_ast_t_s;
typedef faster_value_ptr (*faster_ast_node_func_t)(faster_indexing_t node_id, const struct faster_ast_t_s *ast);

struct faster_ast_node_t_s {
  faster_indexing_t token_id;
  faster_indexing_t left_id;
  faster_indexing_t right_id;
  faster_ast_node_func_t execution_func;
} FASTER_ALIGNED;
typedef struct faster_ast_node_t_s faster_ast_node_t;
typedef struct faster_ast_node_t_s *faster_ast_node_ptr_t;

DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(faster_token_t);
DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(faster_ast_node_t);
DEFINE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(faster_value_float_holder_t);

struct faster_ast_runtime_state_t_s {
  int call_stack_depth;
  int return_depth;
  int loop_depth;
  int loop_exit;
  int loop_continue;
  faster_ast_state_t state;
  faster_indexing_t last_node_in_processing_id;
};
typedef struct faster_ast_runtime_state_t_s faster_ast_runtime_state_t;
typedef struct faster_ast_runtime_state_t_s *faster_ast_runtime_state_ptr_t;

struct faster_ast_t_s {
  // declared
  AVLNodesTree_t context_tree;
  faster_token_t_arr_t token_list;
  faster_indexing_t ast_root_id;
  faster_ast_node_t_arr_t ast_list;
  faster_value_float_holder_t_arr_t float_list;
  // initialized
  faster_ast_runtime_state_t runtime_state;
  faster_interned_strings_t interned_strings;
};
typedef struct faster_ast_t_s faster_ast_t;
typedef struct faster_ast_t_s *faster_ast_ptr_t;

// we take node capacity to be the same as token capacity
// we will have less nodes than tokens, but we will need more nodes during
// actual execution we use the AVL tree structure to store the context of the
// AST but runtime status will be kept directly for speed ast is actually a tree
// of nodes, but we use a flat array to store the nodes

#define DECLARE_AST_WITH_DYNAMIC_ALLOCATION(name, token_capacity)                                                                  \
  DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name##_token_array, faster_token_t, token_capacity);                                  \
  DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name##_ast_array, faster_ast_node_t, token_capacity);                                 \
  DECLARE_FAST_ARRAY_WITH_DYNAMIC_ALLOCATION(name##_float_array, faster_value_float_holder_t, token_capacity);                     \
  DECLARE_AVL_NODE_TREE_WITH_DYNAMIC_ALLOCATION(name##faster_ast_context_tree, 16);                                                \
  faster_ast_t name = {.token_list = name##_token_array,                                                                           \
                       .ast_list = name##_ast_array,                                                                               \
                       .context_tree = name##_faster_ast_context_tree,                                                             \
                       .ast_root_id = FASTER_ARRAY_COUNT_INVALID.float_list = name##_float_array.runtime_state = {0}}

faster_error_code_t faster_ast_init(faster_ast_ptr_t ast);
faster_error_code_t faster_ast_free(faster_ast_ptr_t ast);
faster_error_code_t faster_tokenize(faster_ast_ptr_t ast, const faster_str_ptr_t str);
faster_error_code_t faster_parse(faster_ast_ptr_t ast);
faster_error_code_t faster_execute(faster_ast_ptr_t ast, faster_value_ptr *result);

#define FASTER_VALUE_STORE_FLOAT(ast, float_value)                                                                                 \
  (faster_value_ptr)(                                                                                                              \
      ((faster_value_ptr_handler_t)ast->float_list.list + faster_value_float_holder_t_arr_get_next(&ast->float_list)) |            \
      FASTER_VALUE_MARKER_FLO)

#define FASTER_AST_INCLUDE
#endif // FASTER_AST_INCLUDE
