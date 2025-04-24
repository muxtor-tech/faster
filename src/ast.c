#include <stdbool.h>
#include <string.h>

#include "aster/faster_ast.h"

static faster_error_code_t _faster_reset(faster_ast_ptr_t ast) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_EXECUTING) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    ast->runtime_state.call_stack_depth = 0;
    ast->runtime_state.return_depth = 0;
    ast->runtime_state.loop_depth = 0;
    ast->runtime_state.loop_exit = 0;
    ast->runtime_state.loop_continue = false;
    ast->runtime_state.state = FAST_AST_STATE_READY;
    return FAST_AST_ERROR_NONE;
}

faster_error_code_t faster_ast_init(faster_ast_ptr_t ast) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_NOT_INITIALIZED) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    faster_interned_strings_init(&ast->interned_strings);
    faster_ast_runtime_state_t runtime_state = {
            .call_stack_depth = 0,
            .return_depth = 0,
            .loop_depth = 0,
            .loop_exit = 0,
            .loop_continue = false,
            .state = FAST_AST_STATE_EMPTY,
            .last_node_in_processing_id = FASTER_ARRAY_COUNT_INVALID
    };
    ast->ast_root_id = FASTER_ARRAY_COUNT_INVALID;
    ast->runtime_state = runtime_state;
    return FAST_AST_ERROR_NONE;
}

faster_error_code_t faster_ast_free(faster_ast_ptr_t ast) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_READY) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    faster_interned_strings_free(&ast->interned_strings);
    AVL_reset_and_free(&ast->context_tree);
    faster_token_t_arr_reset_and_free(&ast->token_list, 0);
    faster_ast_node_t_arr_reset_and_free(&ast->ast_list, 0);
    faster_value_float_holder_t_arr_reset_and_free(&ast->float_list, 0);
    ast->runtime_state.state = FAST_AST_STATE_NOT_INITIALIZED;
    return FAST_AST_ERROR_NONE;
}

enum faster_tokenizer_state_e {
    FAST_TOKENIZER_STATE_FLAT = 0x00,
    FAST_TOKENIZER_STATE_SYMBOL,
    FAST_TOKENIZER_STATE_STRING,
    FAST_TOKENIZER_STATE_OPERATOR,
    FAST_TOKENIZER_STATE_NUMBER,
};
typedef enum faster_tokenizer_state_e faster_tokenizer_state_t;

typedef faster_error_code_t (*faster_operator_function_definition_t)(faster_value_ptr lval, faster_value_ptr rval, faster_value_ptr *result);
typedef faster_operator_function_definition_t *faster_operator_function_definition_ptr_t;

enum faster_operator_associativity_e {
    FAST_OPERATOR_ASSOCIATIVITY_UNDEFINED = 0x00,
    FAST_OPERATOR_ASSOCIATIVITY_LEFT = 0x01,
    FAST_OPERATOR_ASSOCIATIVITY_RIGHT,
};
typedef enum faster_operator_associativity_e faster_operator_associativity_t;

enum faster_operator_precedence_e {
    FAST_OPERATOR_PRECEDENCE_UNDEFINED = 0x00,
    FAST_OPERATOR_PRECEDENCE_LOWEST = 0x01,
    FAST_OPERATOR_PRECEDENCE_LOW,
    FAST_OPERATOR_PRECEDENCE_MEDIUM,
    FAST_OPERATOR_PRECEDENCE_HIGH,
    FAST_OPERATOR_PRECEDENCE_HIGHEST = 0xFF,
};
typedef enum faster_operator_precedence_e faster_operator_precedence_t;

enum faster_operator_type_e {
    FAST_OPERATOR_TYPE_UNDEFINED = 0x00,
    FAST_OPERATOR_TYPE_BINARY = 0x01,
    FAST_OPERATOR_TYPE_BINARY_ASSIGNING,
    FAST_OPERATOR_TYPE_UNARY_ASSIGNING_PREFIX,
    FAST_OPERATOR_TYPE_UNARY_ASSIGNING_POSTFIX,
    FAST_OPERATOR_TYPE_UNARY_PREFIX,
    FAST_OPERATOR_TYPE_UNARY_POSTFIX,
};
typedef enum faster_operator_type_e faster_operator_type_t;

enum faster_operator_function_base_e {
    FAST_OPERATOR_FUNCTION_BASE_UNDEFINED = 0x00,
    FAST_OPERATOR_FUNCTION_BASE_NONE = 0x01,
    FAST_OPERATOR_FUNCTION_BASE_ADDITION,
    FAST_OPERATOR_FUNCTION_BASE_SUBTRACTION,
    FAST_OPERATOR_FUNCTION_BASE_MULTIPLICATION,
    FAST_OPERATOR_FUNCTION_BASE_DIVISION,
    FAST_OPERATOR_FUNCTION_BASE_INTEGER_DIVISION_REMAINDER,
    FAST_OPERATOR_FUNCTION_BASE_POWER,
    FAST_OPERATOR_FUNCTION_BASE_INTEGER_DIVISION,
    FAST_OPERATOR_FUNCTION_BASE_BITWISE_AND,
    FAST_OPERATOR_FUNCTION_BASE_BITWISE_OR,
    FAST_OPERATOR_FUNCTION_BASE_BITWISE_XOR,
    FAST_OPERATOR_FUNCTION_BASE_BITWISE_NOT,
    FAST_OPERATOR_FUNCTION_BASE_LOGICAL_AND,
    FAST_OPERATOR_FUNCTION_BASE_LOGICAL_OR,
    FAST_OPERATOR_FUNCTION_BASE_LOGICAL_NOT,
    FAST_OPERATOR_FUNCTION_BASE_EQUAL,
    FAST_OPERATOR_FUNCTION_BASE_NOT_EQUAL,
    FAST_OPERATOR_FUNCTION_BASE_LESS_THAN,
    FAST_OPERATOR_FUNCTION_BASE_GREATER_THAN,
    FAST_OPERATOR_FUNCTION_BASE_LESS_THAN_OR_EQUAL,
    FAST_OPERATOR_FUNCTION_BASE_GREATER_THAN_OR_EQUAL,
    FAST_OPERATOR_FUNCTION_BASE_ASSIGNMENT,
    FAST_OPERATOR_FUNCTION_BASE_INCREMENT,
    FAST_OPERATOR_FUNCTION_BASE_DECREMENT,
    FAST_OPERATOR_FUNCTION_BASE_CONDITIONAL,
    FAST_OPERATOR_FUNCTION_BASE_NULL,
    FAST_OPERATOR_FUNCTION_BASE_LIST_START,
    FAST_OPERATOR_FUNCTION_BASE_LIST_END,
};
typedef enum faster_operator_function_base_e faster_operator_function_base_t;

struct faster_operator_definition_t_s {
    faster_str_t operator_str;
    faster_operator_type_t operator_type;
    faster_operator_precedence_t operator_precedence;
    faster_operator_associativity_t operator_associativity;
    faster_operator_function_base_t operator_function_base;
    faster_operator_function_definition_t operator_function;
};
typedef struct faster_operator_definition_t_s faster_operator_definition_t;
typedef struct faster_operator_definition_t_s *faster_operator_definition_ptr_t;

static faster_error_code_t _faster_impl_add(faster_value_ptr lval, faster_value_ptr rval, faster_value_ptr *result) {
    assert(lval != NULL && rval != NULL && result != NULL);
    return FAST_AST_ERROR_NONE;
}

static faster_error_code_t _faster_impl_sub(faster_value_ptr lval, faster_value_ptr rval, faster_value_ptr *result) {
    assert(lval != NULL && rval != NULL && result != NULL);
    return FAST_AST_ERROR_NONE;
}

FASTER_DECLARE_RAW_STR(_faster_operator_str_add, "+");
FASTER_DECLARE_RAW_STR(_faster_operator_str_sub, "-");

faster_operator_definition_t faster_operator_list[] = {
        {
                .operator_str = { .str_ptr = _faster_operator_str_add, .str_len = 1 },
                .operator_type = FAST_OPERATOR_TYPE_BINARY,
                .operator_precedence = FAST_OPERATOR_PRECEDENCE_LOW,
                .operator_associativity = FAST_OPERATOR_ASSOCIATIVITY_LEFT,
                .operator_function_base = FAST_OPERATOR_FUNCTION_BASE_ADDITION,
                .operator_function = _faster_impl_add
        },
        {
                .operator_str = { .str_ptr = _faster_operator_str_sub, .str_len = 1 },
                .operator_type = FAST_OPERATOR_TYPE_BINARY,
                .operator_precedence = FAST_OPERATOR_PRECEDENCE_LOW,
                .operator_associativity = FAST_OPERATOR_ASSOCIATIVITY_LEFT,
                .operator_function_base = FAST_OPERATOR_FUNCTION_BASE_SUBTRACTION,
                .operator_function = _faster_impl_sub
        },
        // empty definition to finalize
        {
                .operator_str = { .str_ptr = NULL, .str_len = 0 },
                .operator_type = FAST_OPERATOR_TYPE_UNDEFINED,
                .operator_precedence = FAST_OPERATOR_PRECEDENCE_UNDEFINED,
                .operator_associativity = FAST_OPERATOR_ASSOCIATIVITY_UNDEFINED,
                .operator_function_base = FAST_OPERATOR_FUNCTION_BASE_UNDEFINED,
                .operator_function = NULL
        }
};

static inline faster_operator_definition_t *_get_operator_definition_with_m(const fchar_t *str, const faster_system_indexing_t str_len, bool maybe) {
    for (int i = 0; faster_operator_list[i].operator_str.str_ptr != NULL; i++) {
        if ((maybe ? (faster_operator_list[i].operator_str.str_len >= str_len) : (faster_operator_list[i].operator_str.str_len == str_len)) &&
            memcmp(faster_operator_list[i].operator_str.str_ptr, str, str_len) == 0) {
            return &faster_operator_list[i];
        }
    }
    return NULL;
}

static inline faster_operator_definition_t *_get_operator_definition(const fchar_t *str, const faster_system_indexing_t str_len) {
    return _get_operator_definition_with_m(str, str_len, false);
}

static inline bool _could_it_be_an_operator(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition_with_m(str, str_len, true);
    return op_def != NULL;
}

static inline bool _is_operator(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition(str, str_len);
    return op_def != NULL;
}

static inline faster_operator_precedence_t _get_operator_precedence(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition(str, str_len);
    return op_def ? op_def->operator_precedence : FAST_OPERATOR_PRECEDENCE_UNDEFINED;
}

static inline faster_operator_associativity_t _get_operator_associativity(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition(str, str_len);
    return op_def ? op_def->operator_associativity : FAST_OPERATOR_ASSOCIATIVITY_UNDEFINED;
}

static inline faster_operator_function_base_t _get_operator_function_base(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition(str, str_len);
    return op_def ? op_def->operator_function_base : FAST_OPERATOR_FUNCTION_BASE_UNDEFINED;
}

static inline faster_operator_function_definition_t _get_operator_function(const fchar_t *str, const faster_system_indexing_t str_len) {
    faster_operator_definition_t *op_def = _get_operator_definition(str, str_len);
    return op_def ? op_def->operator_function : NULL;
}

FASTER_DECLARE_RAW_STR(_space_chars, " \n\r\t");
FASTER_DECLARE_RAW_STR(_string_chars, "\"'");
FASTER_DECLARE_RAW_STR(_operator_chars, "+-*/%&|^!~<>=?:XORAND");
FASTER_DECLARE_RAW_STR(_number_chars, "-.0123456789e");
FASTER_DECLARE_RAW_STR(_symbol_chars, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
FASTER_DECLARE_RAW_STR(_orp_chars, "()");
FASTER_DECLARE_RAW_STR(_osp_chars, "[]");
FASTER_DECLARE_RAW_STR(_ocp_chars, "{}");
FASTER_DECLARE_RAW_STR(_comment_chars, "#");
FASTER_DECLARE_RAW_STR(_comment_end_chars, "\n\r");
static const int _space_chars_len = 4;
static const int _string_chars_len = 2;
static const int _operator_chars_len = 16;
static const int _number_chars_start_len = 12;
static const int _number_chars_len = 13;
static const int _symbol_chars_start_len = 53;
static const int _symbol_chars_len = 63;
static const int _orp_chars_len = 2;
static const int _osp_chars_len = 2;
static const int _ocp_chars_len = 2;
static const int _comment_chars_len = 1;
static const int _comment_end_chars_len = 2;

static inline bool _is_any_of(const fchar_t item, const fchar_t *list, const int clist_len) {
    int list_len = clist_len;
    while(list_len) {
        if (item == list[list_len--]) {
            return true;
        }
    }
    return false;
}

static inline bool _are_all_in(const fchar_t *tested, const int ctest_len, const fchar_t *list, const int clist_len) {
    int test_len = ctest_len;
    while(test_len) {
        if (!_is_any_of(tested[test_len--], list, clist_len)) {
            return false;
        }
    }
    return true;
}

faster_error_code_t faster_tokenize(faster_ast_ptr_t ast, const faster_str_ptr_t str) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_EMPTY) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    if (str == NULL || str->str_len == 0) {
        return FAST_AST_ERROR_INVALID_STRING;
    }
    faster_tokenizer_state_t state = FAST_TOKENIZER_STATE_FLAT;
    faster_indexing_t token_id = 0;
    faster_system_indexing_t token_starting_pos = 0;
    faster_system_indexing_t token_current_pos = 0;
    faster_system_indexing_t str_len = str->str_len;
    faster_value_str_ptr_holder_t str_ptr = str->str_ptr;
    while (token_current_pos < str_len) {
        fchar_t current_char = str_ptr[token_current_pos];
        switch (state) {
            case FAST_TOKENIZER_STATE_FLAT:
                if (_is_any_of(current_char, _space_chars, _space_chars_len)) {
                    token_current_pos++;
                    continue;
                } else if (_is_any_of(current_char, _string_chars, _string_chars_len)) {
                    state = FAST_TOKENIZER_STATE_STRING;
                    token_starting_pos = token_current_pos + 1;
                } else if (current_char == '(') {
                    faster_token_t new_token = {  
                        .token_type = FAST_TOKEN_TYPE_ORP,
                        .token_str = {
                            .str_len = 1,
                            .str_ptr = str_ptr + token_current_pos
                        }
                    };
                    memcpy(&ast->token_list.list[faster_token_t_arr_get_next(&ast->token_list)], &new_token, sizeof(faster_token_t));
                } else if (current_char == ')') {
                    faster_token_t new_token = {  
                        .token_type = FAST_TOKEN_TYPE_CRP,
                        .token_str = {
                            .str_len = 1,
                            .str_ptr = str_ptr + token_current_pos
                        }
                    };
                    memcpy(&ast->token_list.list[faster_token_t_arr_get_next(&ast->token_list)], &new_token, sizeof(faster_token_t));
                } else if (current_char >= '0' && current_char <= '9') {
                    state = FAST_TOKENIZER_STATE_NUMBER;
                    token_starting_pos = token_current_pos;
                } else {
                    state = FAST_TOKENIZER_STATE_SYMBOL;
                    token_starting_pos = token_current_pos;
                }
                break;

            case FAST_TOKENIZER_STATE_STRING:
                if (current_char == '"') {
                    faster_token_t new_token = {  
                        .token_type = FAST_TOKEN_TYPE_STRING,
                        .token_str = {
                            .str_len = token_current_pos - token_starting_pos,
                            .str_ptr = &str_ptr[token_current_pos]
                        }
                    };
                    memcpy(&ast->token_list.list[faster_token_t_arr_get_next(&ast->token_list)], &new_token, sizeof(faster_token_t));
                    state = FAST_TOKENIZER_STATE_FLAT;
                }
                break;

            case FAST_TOKENIZER_STATE_SYMBOL:
                // if (current_char == '\'') {
                //     new_token.token_type = FAST_TOKEN_TYPE_SYMBOL;
                //     new_token.token_str.str_ptr = &str_ptr[token_starting_pos];
                //     new_token.token_str.str_len = token_current_pos - token_starting_pos;
                //     faster_token_t_arr_set(&ast->token_list, token_id++, &new_token);
                //     state = FAST_TOKENIZER_STATE_FLAT;
                // }
                break;

            case FAST_TOKENIZER_STATE_OPERATOR:
                // if (current_char == ' ' || current_char == '\n' || current_char == '\t' || current_char == '('
                //     || current_char == ')') {
                //     new_token.token_type = FAST_TOKEN_TYPE_OPERATOR;
                //     new_token.token_str.str_ptr = &str_ptr[token_starting_pos];
                //     new_token.token_str.str_len = token_current_pos - token_starting_pos;
                //     faster_token_t_arr_set(&ast->token_list, token_id++, &new_token);
                //     state = FAST_TOKENIZER_STATE_FLAT;
                //     token_current_pos--;
                // }
                break;  
            case FAST_TOKENIZER_STATE_NUMBER:
                // if (current_char < '0' || current_char > '9') {
                //     new_token.token_type = FAST_TOKEN_TYPE_NUMBER;
                //     new_token.token_str.str_ptr = &str_ptr[token_starting_pos];
                //     new_token.token_str.str_len = token_current_pos - token_starting_pos;
                //     faster_token_t_arr_set(&ast->token_list, token_id++, &new_token);
                //     state = FAST_TOKENIZER_STATE_FLAT;
                //     token_current_pos--;
                // }
                break;
            default:
                return FAST_AST_ERROR_INVALID_TOKEN;
        }
        token_current_pos++;
    }
    if (state == FAST_TOKENIZER_STATE_STRING || state == FAST_TOKENIZER_STATE_SYMBOL) {
        return FAST_AST_ERROR_INVALID_TOKEN;
    }
    if (state == FAST_TOKENIZER_STATE_OPERATOR || state == FAST_TOKENIZER_STATE_NUMBER) {
        // new_token.token_type = (state == FAST_TOKENIZER_STATE_OPERATOR) ? FAST_TOKEN_TYPE_OPERATOR : FAST_TOKEN_TYPE_NUMBER;
        // new_token.token_str.str_ptr = &str_ptr[token_starting_pos];
        // new_token.token_str.str_len = token_current_pos - token_starting_pos;
        // faster_token_t_arr_set(&ast->token_list, token_id++, &new_token);
    }
    if (state == FAST_TOKENIZER_STATE_FLAT) {
        // new_token.token_type = FAST_TOKEN_TYPE_END;
        // new_token.token_str.str_ptr = NULL;
        // new_token.token_str.str_len = 0;
        // faster_token_t_arr_set(&ast->token_list, token_id++, &new_token);
    }
    if (token_id == 0) {
        return FAST_AST_ERROR_INVALID_LIST;
    }
    if (token_id > ast->token_list.list_header.array_capacity) {
        return FAST_AST_ERROR_INVALID_LIST;
    }
    return FAST_AST_ERROR_NONE;
}

faster_error_code_t faster_parse(faster_ast_ptr_t ast) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_TOKENIZED) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    if (faster_token_t_arr_count(&ast->token_list) == 0) {
        return FAST_AST_ERROR_INVALID_LIST;
    }
    return FAST_AST_ERROR_NONE;
}

static faster_error_code_t _faster_execute_node(faster_ast_ptr_t ast, faster_indexing_t node_id, faster_value_ptr *result) {
    return FAST_AST_ERROR_NONE;
}

faster_error_code_t faster_execute(faster_ast_ptr_t ast, faster_value_ptr *result) {
    if (ast == NULL || ast->runtime_state.state != FAST_AST_STATE_READY) {
        return FAST_AST_ERROR_INVALID_STATE;
    }
    if (faster_ast_node_t_arr_count(&ast->ast_list) == 0) {
        return FAST_AST_ERROR_INVALID_LIST;
    }
    if (result == NULL) {
        return FAST_AST_ERROR_INVALID_PARAMETER;
    }
    if (ast->runtime_state.state != FAST_AST_STATE_EXECUTING) {
        ast->runtime_state.state = FAST_AST_STATE_EXECUTING;
    }
    if (ast->ast_root_id == FASTER_ARRAY_COUNT_INVALID) {
        return FAST_AST_ERROR_INVALID_LIST;
    }
    faster_indexing_t node_id = ast->ast_root_id;
    faster_value_ptr tmp_result = NULL;
    faster_error_code_t error_code = _faster_execute_node(ast, node_id, &tmp_result);
    assert(_faster_reset(ast) == FAST_AST_ERROR_NONE);
    if (error_code != FAST_AST_ERROR_NONE) {
        return error_code;
    }
    if (tmp_result != NULL) {
        *result = tmp_result;
    } else {
        *result = NULL;
    }
    return FAST_AST_ERROR_NONE;
}

