#ifndef QUEUE_CONFIG_H
#define QUEUE_CONFIG_H

/**
 * @brief   Disable queue lock
 * @details Disable the lock attribute and checks in the queue API.
 */
#define QUEUE_DISABLE_LOCK 0U

/**
 * @brief   Enable item telemetry
 * @details Enable queue telemetry with the total of enqueued items
 *          and processed items.
 */
#define QUEUE_COUNT_ITEMS 1U

/**
 * @brief   Enable cheking function arguments
 * @details Check the validity of the input arguments.
 */
#define QUEUE_CHECK_ARGS 1U

/**
 * @brief   Reduce the available API
 * @details Reduce the public API to save memory.
 * @note    Limiting the API does not remove features
 */
#define QUEUE_REDUCED_API 0U

#endif