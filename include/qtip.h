#ifndef QTIP_H
#define QTIP_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Public Enum
 */

/**
 * @brief Queue operation status
 */
typedef enum
{
    QTIP_STATUS_OK,           //!< Operation succeded
    QTIP_STATUS_FULL,         //!< Queue is full
    QTIP_STATUS_EMPTY,        //!< Queue is empty
    QTIP_STATUS_NULL_PTR,     //!< Null pointer encountered
    QTIP_STATUS_INVALID_SIZE, //!< Invalid queue size
    QTIP_STATUS_LOCKED        //!< Queue is locked
} qtipStatus_t;

/*
 * Public Structs
 */

/**
 * @brief Queue context structure
 */
typedef struct
{
    size_t size;     //!< Number of items allowed in the queue
    size_t qty;      //!< Current number of items in the queue
    void* start;     //!< Pointer to the start of the queue
    void* end;       //!< Pointer to the end of the queue
    void* front;     //!< Pointer to the front of the queue
    void* rear;      //!< Pointer to the rear of the queue
    size_t itemSize; //!< Size of each item in the queue
#ifndef DISABLE_LOCK
    bool locked; //!< Lock status
#endif
#ifndef DISABLE_QUEUE_TELEMETRY
    size_t processed; //!< Number of items removed from the queue
    size_t total;     //!< Number of items introduced to the queue
#endif
} qtipContext_t;

/*
 * Public API (Reduced API)
 */

/**
 * @brief     Initialize queue context
 * @details   Initializes the queue context struct with default values.
 * @param[in] pContext Pointer to queue context
 * @param[in] pQueue   Pointer to queue in memory.
 * @param[in] size     Maximum number of items allowed in the queue
 * @param[in] itemSize Size of the item to store in the queue
 * @note      pQueue must be at least size * itemSize bytes
 * @returns   Operation status
 */
qtipStatus_t qtip_init(qtipContext_t* pContext, void* pQueue, size_t size, size_t itemSize);

/**
 * @brief     Put an item in a queue
 * @details   Copies the value of pElement to the back of the queue.
 * @param[in] pContext Pointer to queue context
 * @param[in] pElement Pointer to item to store in the queue
 * @returns   Operation status
 */
qtipStatus_t qtip_put(qtipContext_t* pContext, void* pElement);

/**
 * @brief     Extract the next item from the queue
 * @details   Pulls and removes the the next item in the queue and puts it into pElement.
 * @param[in] pContext Pointer to queue context
 * @param[in] pElement Pointer to item to store in the queue
 * @returns   Operation status
 */
qtipStatus_t qtip_pop(qtipContext_t* pContext, void* pElement);

/**
 * @brief     Copies every element of the queue into a buffer
 * @details   Reads every item in the queue and stores a copy in pBuffer
 * @param[in] pContext Pointer to queue context
 * @param[in] pBuffer  Pointer to buffer to store the copy of the queue
 * @param[in] pSize    Pointer to variable to store the number of items in the queue
 * @note      pBuffer should be size * itemSize bytes
 * @returns   Operation status
 */
qtipStatus_t qtip_peek(qtipContext_t* pContext, void* pBuffer, size_t* pSize);

/**
 * @brief     Removes all the items from the queue
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 */
qtipStatus_t qtip_purge(qtipContext_t* pContext);

/*
 * Extended API
 */

#ifndef REDUCED_API

/**
 * @brief     Checks whether the queue is full
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   True if full
 */
bool qtip_is_full(qtipContext_t* pContext);

/**
 * @brief     Checks whether the queue is empty
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   True if empty
 */
bool qtip_is_empty(qtipContext_t* pContext);

/**
 * @brief     Gets the number of items in the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   Number of items in the queue
 */
size_t qtip_count_items(qtipContext_t* pContext);

#ifndef DISABLE_LOCK
/**
 * @brief     Checks whether the queue is locked
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   Operation status
 */
bool qtip_is_locked(qtipContext_t* pContext);

/**
 * @brief     Locks the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 */
void qtip_lock(qtipContext_t* pContext);

/**
 * @brief     Unlocks the queue
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 */
void qtip_unlock(qtipContext_t* pContext);

#endif
#endif
#endif