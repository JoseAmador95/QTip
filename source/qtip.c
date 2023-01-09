/**
 * @file qtip.c
 * @brief API for queues
 * @author Jose Amador
 * @copyright MIT License
 */

#include "qtip.h"

#include <string.h>

/*
 * Private defines
 */

/**
 * @brief Check whether the input is a null pointer
 */
#define CHECK_NULL_PRT(ptr) (((ptr) != NULL) ? QTIP_STATUS_OK : QTIP_STATUS_NULL_PTR)

/**
 * @brief Check whether the queue is locked
 */
#define IS_LOCKED(context) ((!is_locked((context))) ? QTIP_STATUS_OK : QTIP_STATUS_LOCKED)

/**
 * @brief Combine the current status with a new expression
 */
#define CHECK_STATUS(status, exp) (((status) == QTIP_STATUS_OK) ? (exp) : (status))

/*
 * Private functions
 */

static inline bool needs_rollover(qtipContext_t* pContext, void* pAddr)
{
    return (void*) (pAddr + pContext->itemSize) > pContext->end;
}

static inline bool is_empty(qtipContext_t* pContext)
{
    return pContext->qty == 0U;
}

static inline bool is_full(qtipContext_t* pContext)
{
    return pContext->qty == pContext->maxItems;
}

static inline qtipSize_t count_items(qtipContext_t* pContext)
{
    return pContext->qty;
}

#ifndef DISABLE_LOCK

static inline bool is_locked(qtipContext_t* pContext)
{
    return pContext->locked;
}

static inline void lock_queue(qtipContext_t* pContext)
{
    pContext->locked = true;
}

static inline void unlock_queue(qtipContext_t* pContext)
{
    pContext->locked = false;
}

#endif

static void advance_front(qtipContext_t* pContext)
{
    if (is_empty(pContext))
    {
        // Move front and rear to default state
        pContext->front = pContext->start;
        pContext->rear  = pContext->start;
    }
    else if (needs_rollover(pContext, pContext->front))
    {
        // Rollover
        pContext->front = pContext->start;
    }
    else
    {
        // Move to next element in the array
        pContext->front += pContext->itemSize;
    }
}

static void advance_rear(qtipContext_t* pContext)
{
    if (is_full(pContext) || is_empty(pContext))
    {
        // Allign rear and front
        pContext->rear = pContext->front;
    }
    else if (needs_rollover(pContext, pContext->rear))
    {
        // Rollover
        pContext->rear = pContext->start;
    }
    else
    {
        pContext->rear += pContext->itemSize;
    }
}

/*
 * Public API
 */

qtipStatus_t qtip_init(qtipContext_t* pContext, void* pBuffer, qtipSize_t maxItems, size_t itemSize)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pBuffer));
    status = CHECK_STATUS(status, (maxItems > 0U) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);
    status = CHECK_STATUS(status, (itemSize > 0U) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        pContext->itemSize = itemSize;
        pContext->maxItems = maxItems;
        pContext->start    = pBuffer;
        pContext->end      = pContext->start + (pContext->maxItems - 1U) * pContext->itemSize;
        pContext->front    = pContext->start;
        pContext->rear     = pContext->start;
        pContext->qty      = 0U;
#ifndef DISABLE_LOCK
        pContext->locked = false;
#endif
#ifndef DISABLE_TELEMETRY
        pContext->total     = 0U;
        pContext->processed = 0U;
#endif
    }

    return status;
}

qtipStatus_t qtip_put(qtipContext_t* pContext, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!is_full(pContext))
        {
#ifndef DISABLE_LOCK
            lock_queue(pContext);
#endif
            advance_rear(pContext);

            memcpy(pContext->rear, pItem, pContext->itemSize);
            pContext->qty++;

#ifndef DISABLE_TELEMETRY
            pContext->total++;
#endif

#ifndef DISABLE_LOCK
            unlock_queue(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_FULL;
        }
    }

    return status;
}

qtipStatus_t qtip_pop(qtipContext_t* pContext, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!is_empty(pContext))
        {
#ifndef DISABLE_LOCK
            lock_queue(pContext);
#endif
            memcpy(pItem, pContext->front, pContext->itemSize);
            memset(pContext->front, 0U, pContext->itemSize);
            pContext->qty--;

            advance_front(pContext);

#ifndef DISABLE_TELEMETRY
            pContext->processed++;
#endif

#ifndef DISABLE_LOCK
            unlock_queue(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_EMPTY;
        }
    }

    return status;
}

qtipStatus_t qtip_peek(qtipContext_t* pContext, void* pBuffer, qtipSize_t* pSize)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pBuffer));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pSize));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
#ifndef DISABLE_LOCK
        lock_queue(pContext);
#endif
        *pSize      = count_items(pContext);
        void* pHead = pContext->front;

        for (qtipSize_t i = 0U; i < pContext->qty; i++)
        {
            memcpy(pBuffer + i * pContext->itemSize, pHead, pContext->itemSize);

            if (needs_rollover(pContext, pHead))
            {
                pHead = pContext->start;
            }
            else
            {
                pHead += pContext->itemSize;
            }
        }

#ifndef DISABLE_LOCK
        unlock_queue(pContext);
#endif
    }

    return status;
}

qtipStatus_t qtip_purge(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
#ifndef DISABLE_LOCK
        lock_queue(pContext);
#endif

        pContext->qty = 0;
        memset(pContext->start, 0U, pContext->maxItems * pContext->itemSize);
        advance_rear(pContext);
        advance_front(pContext);

#ifndef DISABLE_LOCK
        unlock_queue(pContext);
#endif
    }

    return status;
}

qtipStatus_t qtip_get_rear(qtipContext_t* pContext, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!is_empty(pContext))
        {
#ifndef DISABLE_LOCK
            lock_queue(pContext);
#endif

            memcpy(pItem, pContext->rear, pContext->itemSize);

#ifndef DISABLE_LOCK
            unlock_queue(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_EMPTY;
        }
    }

    return status;
}

qtipStatus_t qtip_get_front(qtipContext_t* pContext, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!is_empty(pContext))
        {
#ifndef DISABLE_LOCK
            lock_queue(pContext);
#endif

            memcpy(pItem, pContext->front, pContext->itemSize);

#ifndef DISABLE_LOCK
            unlock_queue(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_EMPTY;
        }
    }

    return status;
}

#ifndef DISABLE_LOCK

qtipStatus_t qtip_is_locked(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        status = IS_LOCKED(pContext);
    }

    return status;
}

qtipStatus_t qtip_lock(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        lock_queue(pContext);
    }

    return status;
}

qtipStatus_t qtip_unlock(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        unlock_queue(pContext);
    }

    return status;
}

#endif // DISABLE_LOCK

#ifndef REDUCED_API
qtipStatus_t qtip_is_full(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        status = (is_full(pContext)) ? QTIP_STATUS_FULL : QTIP_STATUS_OK;
    }

    return status;
}

qtipStatus_t qtip_is_empty(qtipContext_t* pContext)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        status = (is_empty(pContext)) ? QTIP_STATUS_FULL : QTIP_STATUS_OK;
    }

    return status;
}

qtipStatus_t qtip_count_items(qtipContext_t* pContext, qtipSize_t* pResult)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        *pResult = count_items(pContext);
    }

    return status;
}

#endif // REDUCED_API

#ifndef DISABLE_TELEMETRY

qtipStatus_t qtip_total_enqueued_items(qtipContext_t* pContext, size_t* pResult)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pResult));
#endif

    if (status == QTIP_STATUS_OK)
    {
        *pResult = pContext->total;
    }

    return status;
}

qtipStatus_t qtip_total_processed_items(qtipContext_t* pContext, size_t* pResult)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pResult));
#endif

    if (status == QTIP_STATUS_OK)
    {
        *pResult = pContext->processed;
    }

    return status;
}

#endif // DISABLE_TELEMETRY