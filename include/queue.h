#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    QUEUE_GOOD,
    QUEUE_FULL,
    QUEUE_EMPTY,
    QUEUE_GENERIC,
    QUEUE_NULL_PTR,
    QUEUE_INVALID_SIZE,
    QUEUE_LOCKED
} queueStatus_t;

typedef struct
{
    size_t size;
    size_t qty;
    void* start;
    void* end;
    void* front;
    void* rear;
    size_t elementSize;
    bool locked;
} queueContext_t;

queueStatus_t queue_init(queueContext_t* pContext, void* pQueue, size_t size, size_t elementSize);
queueStatus_t queue_put(queueContext_t* pContext, void* pElement);
queueStatus_t queue_pop(queueContext_t* pContext, void* pElement);
queueStatus_t queue_peek(queueContext_t* pContext, void* pBuffer, size_t* pSize);
bool queue_is_full(queueContext_t* pContext);
bool queue_is_empty(queueContext_t* pContext);
size_t queue_count_items(queueContext_t* pContext);

#endif