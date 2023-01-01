#include "queue.h"

#include <stddef.h>
#include <string.h>

static bool queue_needs_rollover(queueContext_t* pContext, void* pAddr)
{
    return pAddr + 1U > pContext->end;
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

queueStatus_t queue_init(queueContext_t* pContext, void* pBuffer, size_t size, size_t elementSize)
{
    queueStatus_t status = QUEUE_GENERIC;

    if (pBuffer == NULL || pContext == NULL)
    {
        status = QUEUE_NULL_PTR;
    }
    else if (size == 0U)
    {
        status = QUEUE_INVALID_SIZE;
    }
    else
    {
        pContext->elementSize = elementSize;
        pContext->size        = size;
        pContext->start       = pBuffer;
        pContext->end         = pContext->start + (pContext->size - 1U) * pContext->elementSize;
        pContext->front       = (void**) pContext->start;
        pContext->rear        = (void**) pContext->start;
        pContext->qty         = 0U;
        pContext->locked      = false;
        status                = QUEUE_GOOD;
    }

    return status;
}

bool queue_is_full(queueContext_t* pContext)
{
    return pContext->qty == pContext->size;
}

bool queue_is_empty(queueContext_t* pContext)
{
    return pContext->qty == 0U;
}

size_t queue_count_items(queueContext_t* pContext)
{
    return pContext->qty;
}

queueStatus_t queue_put(queueContext_t* pContext, void* pElement)
{
    queueStatus_t status = QUEUE_GENERIC;

    if (pContext == NULL || pElement == NULL)
    {
        status = QUEUE_NULL_PTR;
    }
    else if (pContext->locked)
    {
        status = QUEUE_LOCKED;
    }
    else
    {
        if (!queue_is_full(pContext))
        {
            pContext->locked = true;
            advance_rear(pContext);

            memcpy(pContext->rear, &pElement, pContext->elementSize);
            pContext->qty++;

            pContext->locked = false;
            status           = QUEUE_GOOD;
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
    queueStatus_t status = QUEUE_GENERIC;

    if (pContext == NULL)
    {
        status = QUEUE_NULL_PTR;
    }
    else if (pContext->locked)
    {
        status = QUEUE_LOCKED;
    }
    else
    {
        if (!queue_is_empty(pContext))
        {
            // not empty
            pContext->locked = true;
            memcpy(pElement, *pContext->front, pContext->elementSize);
            memset(pContext->front, 0U, pContext->elementSize);
            pContext->qty--;

            advance_front(pContext);

            pContext->locked = false;
            status           = QUEUE_GOOD;
        }
        else
        {
            // empty
            status = QUEUE_EMPTY;
        }
    }

    (void) pElement;
    return status;
}

queueStatus_t queue_peek(queueContext_t* pContext, void** pBuffer, size_t* pSize)
{
    queueStatus_t status = QUEUE_GENERIC;

    if (pContext == NULL || pBuffer == NULL || pSize == NULL)
    {
        status = QUEUE_NULL_PTR;
    }
    else if (pContext->locked)
    {
        status = QUEUE_LOCKED;
    }
    else
    {
        pContext->locked = true;
        *pSize           = pContext->qty;
        void** pHead     = pContext->front;

        for (size_t i = 0U; i < pContext->qty; i++)
        {
            memcpy(pBuffer + i, pHead, pContext->elementSize);

            if (queue_needs_rollover(pContext, pHead))
            {
                pHead = pContext->start;
            }
            else
            {
                pHead++;
            }
        }

        pContext->locked = false;
    }

    return status;
}