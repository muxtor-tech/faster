#include "aster/faster_aq.h"                                                                                                                                                                                
                                   
static int _aq_init_check(_faster_atomic_fifo_queue_t*q, size_t n_element_size, size_t n_queue_size) {                                                                                         
    if (n_element_size < 1)                                                                                         
        return -1;                                                                                                                    
    if (n_queue_size < 2)                                                                                                 
        return -1;                                                          
    q->element_size = n_element_size;
    q->queue_size = n_queue_size;
    return 0;
}

int _aq_init_preallocated(_faster_atomic_fifo_queue_t*q, void *buf, size_t n_element_size, size_t n_queue_size) {
    if (_aq_init_check(q, n_element_size, n_queue_size))
        return -1;
    if (buf == NULL)
        return -1;                                                   
    q->buffer = (faster_value_ptr)buf;
    q->is_static_buffer = true;
    atomic_store(&q->head_tail, 0);
    return 0;                                                               
}                                                                                                                    
                                                                                                                    
int _aq_init_dynamic(_faster_atomic_fifo_queue_t*q, size_t n_element_size, size_t n_queue_size) {
    if (_aq_init_check(q, n_element_size, n_queue_size))
        return -1;                                                
    q->buffer = aligned_alloc(FASTER_ALIGNMENT_BASE, FASTER_INTERNAL_ADJUST_SIZE_TO_MULTIPLE(q->queue_size * q->element_size, FASTER_ALIGNMENT_BASE));           
    if (q->buffer == NULL)                                                                                             
        return -1;
    q->is_static_buffer = false;
    atomic_store(&q->head_tail, 0);                                                                                                  
    return 0;                                                                                                          
}                                                                                                                    
                                                                                                                    
void _aq_free(_faster_atomic_fifo_queue_t*q) {                                                                 
    if (q->buffer) {
        if (!q->is_static_buffer)                                                                                         
            free(q->buffer);                                                                                                 
        q->buffer = NULL;                                                                                                
    }                                                                                                                  
}                                                                                                                    
                                                                                                                    
int _aq_is_empty(_faster_atomic_fifo_queue_t*q) {                                                              
    faster_internal_head_tail_t ht = atomic_load(&q->head_tail);                                                       
    return FASTER_INTERNAL_GET_HEAD(ht) == FASTER_INTERNAL_GET_TAIL(ht);                                               
}                                                                                                                    
                                                                                                                    
int _aq_is_full(_faster_atomic_fifo_queue_t*q) {                                                               
    faster_internal_head_tail_t ht = atomic_load(&q->head_tail);                                                       
    return ((FASTER_INTERNAL_GET_TAIL(ht) + 1) % q->queue_size) == FASTER_INTERNAL_GET_HEAD(ht);                                
}                                                                                                                    
                                                                                                                    
int _aq_enqueue(_faster_atomic_fifo_queue_t*q, const faster_value_ptr item) {                                                
    faster_internal_head_tail_t ht, new_ht;                                                                            
    size_t head, tail, next_tail;                                                                                      
    do {                                                                                                               
        ht = atomic_load(&q->head_tail);                                                                                 
        head = FASTER_INTERNAL_GET_HEAD(ht);                                                                             
        tail = FASTER_INTERNAL_GET_TAIL(ht);                                                                             
        next_tail = (tail + 1) % q->queue_size;                                                                                   
        if (next_tail == head) {                                                                                         
            return -1; /* Queue is full */                                                                                 
        }                                                                                                                
        new_ht = FASTER_INTERNAL_MAKE_HEAD_TAIL(head, next_tail);                                                        
    } while (!atomic_compare_exchange_weak(&q->head_tail, &ht, new_ht));                                               
    memcpy(FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(q->buffer, tail, q->element_size), item, q->element_size);                                                                         
    return 0;                                                                                                          
}                                                                                                                    
                                                                                                                    
int _aq_dequeue(_faster_atomic_fifo_queue_t*q, const faster_value_ptr item) {                                                      
    faster_internal_head_tail_t ht, new_ht;                                                                            
    size_t head, tail, next_head;                                                                                      
    do {                                                                                                               
        ht = atomic_load(&q->head_tail);                                                                                 
        head = FASTER_INTERNAL_GET_HEAD(ht);                                                                             
        tail = FASTER_INTERNAL_GET_TAIL(ht);                                                                             
        if (head == tail) {                                                                                              
            return -1; /* Queue is empty */                                                                                
        }                                                                                                                
        next_head = (head + 1) % q->queue_size;                                                                                   
        new_ht = FASTER_INTERNAL_MAKE_HEAD_TAIL(next_head, tail);                                                        
    } while (!atomic_compare_exchange_weak(&q->head_tail, &ht, new_ht));                                               
    memcpy(item, FASTER_INCEMENT_POINTER_BY_SIZED_ELEMENT(q->buffer, head, q->element_size), q->element_size);                                                                         
    return 0;                                                                                                          
}
