/**
 * @file qtip.h
 * @brief API for queues
 * @author Jose Amador
 * @copyright MIT License
 *
 * @defgroup API Public API
 * @addtogroup API
 * @{
 * @brief Public API
 */

#ifndef QTIP_H
#define QTIP_H

#ifdef __cplusplus
// clang-format off
#define QTIP_CPP_SUPPORT_START extern "C" {
#define QTIP_CPP_SUPPORT_END }
#define QTIP_IS_CPP (1U)
#else
#define QTIP_CPP_SUPPORT_START
#define QTIP_CPP_SUPPORT_END
#define QTIP_IS_CPP (0U)
// clang-format on
#endif

QTIP_CPP_SUPPORT_START

#include <stdbool.h>
#include <stddef.h>

/*
 * Public defines
 */
#ifndef SIZE_TYPE
#define SIZE_TYPE size_t //!< Type for holding the number of items in queue
#else
#include <stdint.h>
#endif

/*
 * Public typedefs
 */
typedef SIZE_TYPE qtipSize_t; //!< Number of items in queue

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
    qtipSize_t maxItems; //!< Number of items allowed in the queue
    qtipSize_t qty;      //!< Current number of items in the queue
    void* start;         //!< Pointer to the start of the queue
    qtipSize_t front;    //!< Pointer to the front of the queue
    qtipSize_t rear;     //!< Pointer to the rear of the queue
    size_t itemSize;     //!< Size of each item in the queue
#ifndef DISABLE_LOCK
    bool locked; //!< Lock status
#endif
#ifndef DISABLE_TELEMETRY
    size_t processed; //!< Number of items removed from the queue
    size_t total;     //!< Number of items introduced to the queue
#endif
} qtipContext_t;

/*
 * Public API
 */

/**
 * @brief     Initialize queue context
 * @details   Initializes the queue context struct with default values.
 * @param[in] pContext Pointer to queue context
 * @param[in] pQueue   Pointer to queue in memory.
 * @param[in] maxItems Maximum number of items allowed in the queue
 * @param[in] itemSize Size of the item to store in the queue
 * @note      pQueue must be at least maxItems * itemSize bytes
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | `itemSize` or `maxItems` is `0` |
 */
qtipStatus_t qtip_init(qtipContext_t* pContext, void* pQueue, qtipSize_t maxItems, size_t itemSize);

/**
 * @brief     Put an item in a queue
 * @details   Copies the value of pItem to the back of the queue.
 * @param[in] pContext Pointer to queue context
 * @param[in] pItem Pointer to item to store in the queue
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL   |
 *    | @ref QTIP_STATUS_FULL         | Queue is full                   |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_put(qtipContext_t* pContext, void* pItem);

/**
 * @brief      Extract the next item from the queue
 * @details    Pulls and removes the the next item in the queue and puts it into pItem.
 * @param[in]  pContext Pointer to queue context
 * @param[out] pItem Pointer to item to store in the queue
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL   |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | Queue is empty                  |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_pop(qtipContext_t* pContext, void* pItem);

/**
 * @brief      Copies every element of the queue into a buffer
 * @details    Reads every item in the queue and stores a copy in pBuffer
 * @param[in]  pContext Pointer to queue context
 * @param[in]  pBuffer  Pointer to buffer to store the copy of the queue
 * @param[out] pSize    Pointer to variable to store the number of items in the queue
 * @note       pBuffer should be maxItems * itemSize bytes
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                                  |
 *    | ----------------------------- | --------------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful                    |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                         |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext`, `pItem`, or `pSIze` is NULL |
 *    | @ref QTIP_STATUS_FULL         | NA                                      |
 *    | @ref QTIP_STATUS_EMPTY        | NA                                      |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                                      |
 */
qtipStatus_t qtip_peek(qtipContext_t* pContext, void* pBuffer, qtipSize_t* pSize);

/**
 * @brief     Removes all the items from the queue
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                                  |
 *    | ----------------------------- | --------------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful                    |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                         |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL                      |
 *    | @ref QTIP_STATUS_FULL         | NA                                      |
 *    | @ref QTIP_STATUS_EMPTY        | NA                                      |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                                      |
 */
qtipStatus_t qtip_purge(qtipContext_t* pContext);

/**
 * @brief      Gets the item at the front of the queue
 * @details    The item at the front of the queue is fetched, but not removed.
 * @param[in]  pContext Pointer to queue context
 * @param[out] pItem Pointer to the item at the front of the queue
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                                  |
 *    | ----------------------------- | --------------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful                    |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                         |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL           |
 *    | @ref QTIP_STATUS_FULL         | NA                                      |
 *    | @ref QTIP_STATUS_EMPTY        | Queue is empty                          |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                                      |
 */
qtipStatus_t qtip_get_front(qtipContext_t* pContext, void* pItem);

/**
 * @brief      Gets the item at the rear of the queue
 * @details    The item at the rear of the queue is fetched, but not removed.
 * @param[in]  pContext Pointer to queue context
 * @param[out] pItem Pointer to the item at the rear of the queue
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                                  |
 *    | ----------------------------- | --------------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful                    |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                         |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL           |
 *    | @ref QTIP_STATUS_FULL         | NA                                      |
 *    | @ref QTIP_STATUS_EMPTY        | Queue is empty                          |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                                      |
 */
qtipStatus_t qtip_get_rear(qtipContext_t* pContext, void* pItem);

#ifndef REDUCED_API

/**
 * @brief     Checks whether the queue is full
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Queue is not full               |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | Queue is full                   |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_is_full(qtipContext_t* pContext);

/**
 * @brief     Checks whether the queue is empty
 * @param[in] pContext Pointer to queue context
 * @note      This function is part of the extended API
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Queue is not empty              |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | Queue is empty                  |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_is_empty(qtipContext_t* pContext);

/**
 * @brief      Gets the number of items in the queue
 * @param[in]  pContext Pointer to queue context
 * @param[out] pResult Pointer to the variable to hold the result
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_count_items(qtipContext_t* pContext, qtipSize_t* pResult);

/**
 * @brief      Gets the item from an intex in the queue
 * @details    Returns the item in the requested position, where `index = 0`
 *             results in the item at the front of the queue.
 * @param[in]  pContext Pointer to queue context
 * @param[in]  index    Item index
 * @param[out] pItem    Pointer to the variable to hold the result
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL   |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | Index unavailable               |
 */
qtipStatus_t qtip_get_item_index(qtipContext_t* pContext, qtipSize_t index, void* pItem);

/**
 * @brief      Removes the item from an intex in the queue
 * @details    Removes the item in the requested position, where `index = 0`
 *             is the item at the front of the queue.
 * @param[in]  pContext Pointer to queue context
 * @param[in]  index    Item index
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | Index unavailable               |
 */
qtipStatus_t qtip_remove_item_index(qtipContext_t* pContext, qtipSize_t index);

/**
 * @brief      Pops the item from an intex in the queue
 * @details    Returns and removes the item in the requested position,
 *             where `index = 0` is the item at the front of the queue.
 * @param[in]  pContext Pointer to queue context
 * @param[in]  index    Item index
 * @param[out] pItem    Pointer to the variable to hold the result
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pItem` is NULL   |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | Index unavailable               |
 */
qtipStatus_t qtip_get_pop_index(qtipContext_t* pContext, qtipSize_t index, void* pItem);

#endif // REDUCED_API

#ifndef DISABLE_LOCK

/**
 * @brief     Checks whether the queue is locked
 * @details   Deletes the items from the queue and sets the queue in a known state.
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Queue is not locked             |
 *    | @ref QTIP_STATUS_LOCKED       | Queue is locked                 |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_is_locked(qtipContext_t* pContext);

/**
 * @brief     Locks the queue
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_lock(qtipContext_t* pContext);

/**
 * @brief     Unlocks the queue
 * @param[in] pContext Pointer to queue context
 * @returns   Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` is NULL              |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_unlock(qtipContext_t* pContext);

#endif // DISABLE_LOCK

#ifndef DISABLE_TELEMETRY

/**
 * @brief      Get number of items inserted in the queue
 * @details    The result considers the all-time number of inserted items.
 * @param[in]  pContext Pointer to queue context
 * @param[out] pResult  Pointer to variable to hold the result
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pResult` is NULL |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_total_enqueued_items(qtipContext_t* pContext, size_t* pResult);

/**
 * @brief      Get number of processed items in the queue
 * @details    The result considers the all-time number of popped items.
 * @param[in]  pContext Pointer to queue context
 * @param[out] pResult  Pointer to variable to hold the result
 * @returns    Operation status
 *
 * @details
 *    | Returned @ref qtipStatus_t    | Reason                          |
 *    | ----------------------------- | ------------------------------- |
 *    | @ref QTIP_STATUS_OK           | Operation successful            |
 *    | @ref QTIP_STATUS_LOCKED       | NA                              |
 *    | @ref QTIP_STATUS_NULL_PTR     | `pContext` or `pResult` is NULL |
 *    | @ref QTIP_STATUS_FULL         | NA                              |
 *    | @ref QTIP_STATUS_EMPTY        | NA                              |
 *    | @ref QTIP_STATUS_INVALID_SIZE | NA                              |
 */
qtipStatus_t qtip_total_processed_items(qtipContext_t* pContext, size_t* pResult);

#endif // DISABLE_TELEMETRY

#endif // QTIP_H

QTIP_CPP_SUPPORT_END

/**
 * @}
 */