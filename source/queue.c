#include "queue.h"

#include <stddef.h>
#include <string.h>

#if QUEUE_DISABLE_LOCK == 0
#define QUEUE_LOCK(context)   ((queueContext_t*) (context))->locked = true
#define QUEUE_UNLOCK(context) ((queueContext_t*) (context))->locked = false
#else
#define QUEUE_LOCK(context)
#define QUEUE_UNLOCK(context)
#endif

#if QUEUE_REDUCED_API == 0
#define QUEUE_API
#else
#define QUEUE_API static inline
#endif

#define CHECK_NULL_PRT(ptr)       (((ptr) != NULL) ? QUEUE_OK : QUEUE_NULL_PTR)
#define IS_LOCKED(context)        ((!(context)->locked) ? QUEUE_OK : QUEUE_LOCKED)
#define CHECK_STATUS(status, exp) (((status) == QUEUE_OK) ? (exp) : (status))

static bool queue_needs_rollover(queueContext_t* pContext, void* pAddr)
{
    return (void*) (pAddr + pContext->elementSize) > pContext->end;
}

static void advance_front(queueContext_t* pContext)
{
    if (queue_is_empty(pContext))
    {
        // Move front and rear to default state
        pContext->front = pContext->start;
        pContext->rear  = pContext->start;
    }
    else if (queue_needs_rollover(pContext, pContext->front))
    {
        // Rollover
        pContext->front = pContext->start;
    }
    else
    {
        // Move to next element in the array
        pContext->front++;
    }
}

static void advance_rear(queueContext_t* pContext)
{
    if (queue_is_full(pContext) || queue_is_empty(pContext))
    {
        // Allign rear and front
        pContext->rear = pContext->front;
    }
    else if (queue_needs_rollover(pContext, pContext->rear))
    {
        // Rollover
        pContext->rear = pContext->start;
    }
    else
    {
        pContext->rear++;
    }
}

QUEUE_API bool queue_is_full(queueContext_t* pContext)
{
    bool isFull = false;

    if (pContext != NULL)
    {
        isFull = pContext->qty == pContext->size;
    }

    return isFull;
}

QUEUE_API bool queue_is_empty(queueContext_t* pContext)
{
    return pContext->qty == 0U;
}

QUEUE_API size_t queue_count_items(queueContext_t* pContext)
{
    return pContext->qty;
}

QUEUE_API bool queue_is_locked(queueContext_t* pContext)
{
    return pContext->locked;
}

QUEUE_API void queue_lock(queueContext_t* pContext)
{
    pContext->locked = true;
}

QUEUE_API void queue_unlock(queueContext_t* pContext)
{
    pContext->locked = false;
}

queueStatus_t queue_init(queueContext_t* pContext, void* pBuffer, size_t size, size_t elementSize)
{
    queueStatus_t status = QUEUE_OK;

#if QUEUE_CHECK_ARGS == 1U
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pBuffer));
    status = CHECK_STATUS(status, (size > 0U) ? QUEUE_OK : QUEUE_INVALID_SIZE);
    status = CHECK_STATUS(status, (elementSize > 0U) ? QUEUE_OK : QUEUE_INVALID_SIZE);
#endif

#if QUEUE_DISABLE_LOCK == 0U
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QUEUE_OK)
    {
        pContext->elementSize = elementSize;
        pContext->size        = size;
        pContext->start       = pBuffer;
        pContext->end         = pContext->start + (pContext->size - 1U) * pContext->elementSize;
        pContext->front       = pContext->start;
        pContext->rear        = pContext->start;
        pContext->qty         = 0U;
#if QUEUE_DISABLE_LOCK == 0U
        pContext->locked = false;
#endif
#if QUEUE_COUNT_ITEMS == 1U
        pContext->total     = 0U;
        pContext->processed = 0U;
#endif
    }

    return status;
}

queueStatus_t queue_put(queueContext_t* pContext, void* pElement)
{
    queueStatus_t status = QUEUE_OK;

#if QUEUE_CHECK_ARGS == 1U
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pElement));
#endif

#if QUEUE_DISABLE_LOCK == 0U
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QUEUE_OK)
    {
        if (!queue_is_full(pContext))
        {
#if QUEUE_DISABLE_LOCK == 0U
            queue_lock(pContext);
#endif
            advance_rear(pContext);

            memcpy(pContext->rear, pElement, pContext->elementSize);
            pContext->qty++;

#if QUEUE_COUNT_ITEMS == 1U
            pContext->total++;
#endif

#if QUEUE_DISABLE_LOCK == 0U
            queue_unlock(pContext);
#endif
        }
        else
        {
            status = QUEUE_FULL;
        }
    }

    return status;
}

queueStatus_t queue_pop(queueContext_t* pContext, void* pElement)
{
    queueStatus_t status = QUEUE_OK;

#if QUEUE_CHECK_ARGS == 1U
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pElement));
#endif

#if QUEUE_DISABLE_LOCK == 0U
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QUEUE_OK)
    {
        if (!queue_is_empty(pContext))
        {
#if QUEUE_DISABLE_LOCK == 0U
            queue_lock(pContext);
#endif
            memcpy(pElement, pContext->front, pContext->elementSize);
            memset(pContext->front, 0U, pContext->elementSize);
            pContext->qty--;

            advance_front(pContext);

#if QUEUE_COUNT_ITEMS == 1U
            pContext->processed++;
#endif

#if QUEUE_DISABLE_LOCK == 0U
            queue_unlock(pContext);
#endif
        }
        else
        {
            status = QUEUE_EMPTY;
        }
    }

    return status;
}

queueStatus_t queue_peek(queueContext_t* pContext, void* pBuffer, size_t* pSize)
{
    queueStatus_t status = QUEUE_OK;

#if QUEUE_CHECK_ARGS == 1U
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pBuffer));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pSize));
#endif

#if QUEUE_DISABLE_LOCK == 0U
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QUEUE_OK)
    {
#if QUEUE_DISABLE_LOCK == 0U
        queue_lock(pContext);
#endif
        *pSize      = queue_count_items(pContext);
        void* pHead = pContext->front;

        for (size_t i = 0U; i < pContext->qty; i++)
        {
            memcpy(pBuffer + i, pHead, pContext->elementSize);

            if (queue_needs_rollover(pContext, pHead))
            {
                pHead = pContext->start;
            }
            else
            {
                pHead += pContext->elementSize;
            }
        }

#if QUEUE_DISABLE_LOCK == 0U
        queue_unlock(pContext);
#endif
    }

    return status;
}

queueStatus_t queue_purge(queueContext_t* pContext)
{
    queueStatus_t status = QUEUE_OK;

#if QUEUE_CHECK_ARGS == 1U
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

#if QUEUE_DISABLE_LOCK == 0U
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QUEUE_OK)
    {
#if QUEUE_DISABLE_LOCK == 0U
        queue_lock(pContext);
#endif

        pContext->qty = 0;
        memset(pContext->start, 0U, pContext->size * pContext->elementSize);
        advance_rear(pContext);
        advance_front(pContext);

#if QUEUE_DISABLE_LOCK == 0U
        queue_unlock(pContext);
#endif
    }

    return status;
}