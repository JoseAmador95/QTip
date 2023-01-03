#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Queue operation status
 */
typedef enum
{
    QUEUE_OK,           //!< Operation succeded
    QUEUE_FULL,         //!< Queue is full
    QUEUE_EMPTY,        //!< Queue is empty
    QUEUE_NULL_PTR,     //!< Null pointer encountered
    QUEUE_INVALID_SIZE, //!< Invalid queue size
    QUEUE_LOCKED        //!< Queue is locked
} queueStatus_t;

/**
 * @brief Queue context structure
 */
typedef struct
{
    size_t size;        //!< Number of items allowed in the queue
    size_t qty;         //!< Current number of items in the queue
    void* start;        //!< Pointer to the start of the queue
    void* end;          //!< Pointer to the end of the queue
    void* front;        //!< Pointer to the front of the queue
    void* rear;         //!< Pointer to the rear of the queue
    size_t elementSize; //!< Size of each item in the queue
#ifndef DISABLE_LOCK
    bool locked; //!< Lock status
#endif
#ifndef DISABLE_QUEUE_TELEMETRY
    size_t processed; //!< Number of items removed from the queue
    size_t total;     //!< Number of items introduced to the queue
#endif
} queueContext_t;

/**
 * @brief     Initialize queue context
 * @details   Initializes the queue context struct with default values.
 * @param[in] pContext Pointer to queue context
 * @param[in] pQueue   Pointer to queue in memory.
 * @param[in] size     Maximum number of items allowed in the queue
 * @param[in] elementSize Size of the item to store in the queue
 * @note      pQueue must be at least size * elementSize bytes
 * @returns   Operation status
 */
queueStatus_t queue_init(queueContext_t* pContext, void* pQueue, size_t size, size_t elementSize);

/**
 * @brief     Put an item in a queue
 * @details   Copies the value of pElement to the back of the queue.
 * @param[in] pContext Pointer to queue context
 * @param[in] pElement Pointer to item to store in the queue
 * @returns   Operation status
 */
queueStatus_t queue_put(queueContext_t* pContext, void* pElement);

/**
 * @brief     Extract the next item from the queue
 * @details   Pulls and removes the the next item in the queue and puts it into pElement.
 * @param[in] pContext Pointer to queue context
 * @param[in] pElement Pointer to item to store in the queue
 * @returns   Operation status
 */
queueStatus_t queue_pop(queueContext_t* pContext, void* pElement);

/**
 * @brief     Copies every element of the queue into a buffer
 * @details   Reads every item in the queue and stores a copy in pBuffer
 * @param[in] pContext Pointer to queue context
 * @param[in] pBuffer  Pointer to buffer to store the copy of the queue
 * @param[in] pSize    Pointer to variable to store the number of items in the queue
 * @note      pBuffer should be size * elementSize bytes
 * @returns   Operation status
 */
queueStatus_t queue_peek(queueContext_t* pContext, void* pBuffer, size_t* pSize);

/**
 * @brief     Removes all the items from the queue
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 */
queueStatus_t queue_purge(queueContext_t* pContext);

#ifndef REDUCED_API

/**
 * @brief     Checks whether the queue is full
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   True if full
 */
bool queue_is_full(queueContext_t* pContext);

/**
 * @brief     Checks whether the queue is empty
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   True if empty
 */
bool queue_is_empty(queueContext_t* pContext);

/**
 * @brief     Gets the number of items in the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   Number of items in the queue
 */
size_t queue_count_items(queueContext_t* pContext);

#ifndef DISABLE_LOCK
/**
 * @brief     Checks whether the queue is locked
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   Operation status
 */
bool queue_is_locked(queueContext_t* pContext);

/**
 * @brief     Locks the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 */
void queue_lock(queueContext_t* pContext);

/**
 * @brief     Unlocks the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 */
void queue_unlock(queueContext_t* pContext);

#endif
#endif
#endif