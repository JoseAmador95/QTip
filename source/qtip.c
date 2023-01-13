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

static inline void* absolute_index_to_address(qtipContext_t* pContext, qtipSize_t index)
{
    return pContext->start + index * pContext->itemSize;
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

static inline void write_item_absolute(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    memcpy(absolute_index_to_address(pContext, index), pItem, pContext->itemSize);
}

static inline void read_item_absolute(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    memcpy(pItem, absolute_index_to_address(pContext, index), pContext->itemSize);
}

static inline void delete_item_absolute(qtipContext_t* pContext, qtipSize_t index)
{
    memset(absolute_index_to_address(pContext, index), 0U, pContext->itemSize);
}

static inline qtipSize_t relative_index_to_absolute(qtipContext_t* pContext, qtipSize_t index)
{
    return (pContext->front + index) % pContext->maxItems;
}

static inline void* relative_index_to_address(qtipContext_t* pContext, qtipSize_t index)
{
    return absolute_index_to_address(pContext, relative_index_to_absolute(pContext, index));
}

static inline void write_item_relative(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    write_item_absolute(pContext, relative_index_to_absolute(pContext, index), pItem);
}

static inline void read_item_relative(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    read_item_absolute(pContext, relative_index_to_absolute(pContext, index), pItem);
}

static inline void delete_item_relative(qtipContext_t* pContext, qtipSize_t index)
{
    delete_item_absolute(pContext, relative_index_to_absolute(pContext, index));
}

static inline void reset_queue(qtipContext_t* pContext)
{
    memset(pContext->start, 0U, pContext->itemSize * pContext->maxItems);
}

static inline qtipSize_t next_index_absolute(qtipContext_t* pContext, qtipSize_t index)
{
    return (index + 1U) % pContext->maxItems;
}

static inline qtipSize_t next_index_relative(qtipContext_t* pContext, qtipSize_t index)
{
    return next_index_absolute(pContext, relative_index_to_absolute(pContext, index));
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

static qtipSize_t move_index(qtipContext_t* pContext, qtipSize_t index)
{
    qtipSize_t newHeadIndex = index;
    if (is_empty(pContext))
    {
        newHeadIndex = 0U;
    }
    else
    {
        newHeadIndex = next_index_absolute(pContext, index);
    }

    return newHeadIndex;
}

static void sweep_items(qtipContext_t* pContext, qtipSize_t index)
{
    void* pHead  = relative_index_to_address(pContext, index);
    qtipSize_t i = 0U;

    for (i = index; i < pContext->qty - 2U; i++)
    {
        const void* pNextItem = relative_index_to_address(pContext, i + 1U);
        memcpy(pHead, pNextItem, pContext->itemSize);
        pHead = (void*) pNextItem;
    }

    delete_item_relative(pContext, i + 1U);
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
        pContext->front    = 0U;
        pContext->rear     = 0U;
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
            pContext->rear = move_index(pContext, pContext->rear);

            write_item_absolute(pContext, pContext->rear, pItem);
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
            read_item_absolute(pContext, pContext->front, pItem);
            delete_item_absolute(pContext, pContext->front);
            pContext->qty--;

            pContext->front = move_index(pContext, pContext->front);

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
        *pSize = count_items(pContext);
        for (qtipSize_t i = 0U; i < pContext->qty; i++)
        {
            read_item_relative(pContext, i, pBuffer + i * pContext->itemSize);
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

        reset_queue(pContext);
        pContext->qty   = 0U;
        pContext->front = move_index(pContext, pContext->front);
        pContext->rear  = move_index(pContext, pContext->rear);

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

            read_item_absolute(pContext, pContext->rear, pItem);

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

            read_item_absolute(pContext, pContext->front, pItem);

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

qtipStatus_t qtip_get_item_index(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    status = CHECK_STATUS(status, (index <= pContext->qty) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);

    if (status == QTIP_STATUS_OK)
    {
        read_item_relative(pContext, index, pItem);
    }

    return status;
}

qtipStatus_t qtip_remove_item_index(qtipContext_t* pContext, qtipSize_t index)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    status = CHECK_STATUS(status, (index <= pContext->qty) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);

    if (status == QTIP_STATUS_OK)
    {
        delete_item_relative(pContext, index);
    }

    return status;
}

qtipStatus_t qtip_get_pop_index(qtipContext_t* pContext, qtipSize_t index, void* pItem)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pItem));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    status = CHECK_STATUS(status, (index <= pContext->qty) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);

    if (status == QTIP_STATUS_OK)
    {
        read_item_relative(pContext, index, pItem);
        delete_item_relative(pContext, index);
        sweep_items(pContext, index);
        // delete_item(pContext, index);
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