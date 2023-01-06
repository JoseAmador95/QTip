#include "qtip.h"

#include <stddef.h>
#include <string.h>

/*
 * Macro Magic
 */

#ifndef REDUCED_API
#define REDUCED
#else
#define REDUCED static inline
#endif

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
#define IS_LOCKED(context) ((!qtip_is_locked((context))) ? QTIP_STATUS_OK : QTIP_STATUS_LOCKED)

/**
 * @brief Combine the current status with a new expression
 */
#define CHECK_STATUS(status, exp) (((status) == QTIP_STATUS_OK) ? (exp) : (status))

/*
 * Private functions
 */

static bool queue_needs_rollover(qtipContext_t* pContext, void* pAddr)
{
    return (void*) (pAddr + pContext->itemSize) > pContext->end;
}

static void advance_front(qtipContext_t* pContext)
{
    if (qtip_is_empty(pContext))
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
        pContext->front += pContext->itemSize;
    }
}

static void advance_rear(qtipContext_t* pContext)
{
    if (qtip_is_full(pContext) || qtip_is_empty(pContext))
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
        pContext->rear += pContext->itemSize;
    }
}

/*
 * Extended API
 */

REDUCED bool qtip_is_full(qtipContext_t* pContext)
{
    return pContext->qty == pContext->size;
}

REDUCED bool qtip_is_empty(qtipContext_t* pContext)
{
    return pContext->qty == 0U;
}

REDUCED size_t qtip_count_items(qtipContext_t* pContext)
{
    return pContext->qty;
}

REDUCED bool qtip_is_locked(qtipContext_t* pContext)
{
    return pContext->locked;
}

REDUCED void qtip_lock(qtipContext_t* pContext)
{
    pContext->locked = true;
}

REDUCED void qtip_unlock(qtipContext_t* pContext)
{
    pContext->locked = false;
}

/*
 * Public API (Reduced API)
 */

qtipStatus_t qtip_init(qtipContext_t* pContext, void* pBuffer, size_t size, size_t itemSize)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pBuffer));
    status = CHECK_STATUS(status, (size > 0U) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);
    status = CHECK_STATUS(status, (itemSize > 0U) ? QTIP_STATUS_OK : QTIP_STATUS_INVALID_SIZE);
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        pContext->itemSize = itemSize;
        pContext->size     = size;
        pContext->start    = pBuffer;
        pContext->end      = pContext->start + (pContext->size - 1U) * pContext->itemSize;
        pContext->front    = pContext->start;
        pContext->rear     = pContext->start;
        pContext->qty      = 0U;
#ifndef DISABLE_LOCK
        pContext->locked = false;
#endif
#ifndef DISABLE_QUEUE_TELEMETRY
        pContext->total     = 0U;
        pContext->processed = 0U;
#endif
    }

    return status;
}

qtipStatus_t qtip_put(qtipContext_t* pContext, void* pElement)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pElement));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!qtip_is_full(pContext))
        {
#ifndef DISABLE_LOCK
            qtip_lock(pContext);
#endif
            advance_rear(pContext);

            memcpy(pContext->rear, pElement, pContext->itemSize);
            pContext->qty++;

#ifndef DISABLE_QUEUE_TELEMETRY
            pContext->total++;
#endif

#ifndef DISABLE_LOCK
            qtip_unlock(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_FULL;
        }
    }

    return status;
}

qtipStatus_t qtip_pop(qtipContext_t* pContext, void* pElement)
{
    qtipStatus_t status = QTIP_STATUS_OK;

#ifndef SKIP_ARG_CHECK
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pContext));
    status = CHECK_STATUS(status, CHECK_NULL_PRT(pElement));
#endif

#ifndef DISABLE_LOCK
    status = CHECK_STATUS(status, IS_LOCKED(pContext));
#endif

    if (status == QTIP_STATUS_OK)
    {
        if (!qtip_is_empty(pContext))
        {
#ifndef DISABLE_LOCK
            qtip_lock(pContext);
#endif
            memcpy(pElement, pContext->front, pContext->itemSize);
            memset(pContext->front, 0U, pContext->itemSize);
            pContext->qty--;

            advance_front(pContext);

#ifndef DISABLE_QUEUE_TELEMETRY
            pContext->processed++;
#endif

#ifndef DISABLE_LOCK
            qtip_unlock(pContext);
#endif
        }
        else
        {
            status = QTIP_STATUS_EMPTY;
        }
    }

    return status;
}

qtipStatus_t qtip_peek(qtipContext_t* pContext, void* pBuffer, size_t* pSize)
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
        qtip_lock(pContext);
#endif
        *pSize      = qtip_count_items(pContext);
        void* pHead = pContext->front;

        for (size_t i = 0U; i < pContext->qty; i++)
        {
            memcpy(pBuffer + i * pContext->itemSize, pHead, pContext->itemSize);

            if (queue_needs_rollover(pContext, pHead))
            {
                pHead = pContext->start;
            }
            else
            {
                pHead += pContext->itemSize;
            }
        }

#ifndef DISABLE_LOCK
        qtip_unlock(pContext);
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
        qtip_lock(pContext);
#endif

        pContext->qty = 0;
        memset(pContext->start, 0U, pContext->size * pContext->itemSize);
        advance_rear(pContext);
        advance_front(pContext);

#ifndef DISABLE_LOCK
        qtip_unlock(pContext);
#endif
    }

    return status;
}