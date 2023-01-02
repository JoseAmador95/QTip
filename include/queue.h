#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#ifndef QUEUE_IGNORE_CONFIG_FILE
#include "queue_config.h"
#else
#warning "Ignoring configuration file."
#endif

typedef enum
{
    QUEUE_OK,
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
#if QUEUE_IGNORE_LOCK == 0
    bool locked;
#endif
#if QUEUE_COUNT_ITEMS == 1
    size_t processed;
    size_t total;
#endif
} queueContext_t;

queueStatus_t queue_init(queueContext_t* pContext, void* pQueue, size_t size, size_t elementSize);
queueStatus_t queue_put(queueContext_t* pContext, void* pElement);
queueStatus_t queue_pop(queueContext_t* pContext, void* pElement);
queueStatus_t queue_peek(queueContext_t* pContext, void* pBuffer, size_t* pSize);
queueStatus_t queue_purge(queueContext_t* pContext);
bool queue_is_full(queueContext_t* pContext);
bool queue_is_empty(queueContext_t* pContext);
size_t queue_count_items(queueContext_t* pContext);

#endif