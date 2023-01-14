/**
 * @file test_qtip.c
 * @brief Unit tests for QTip API
 * @author Jose Amador
 * @copyright MIT License
 */

#include "qtip.h"
#include "unity.h"

#include <string.h>

#define QTIP_ASSERT_OK(exp)           TEST_ASSERT(QTIP_STATUS_OK == (exp))
#define QTIP_ASSERT_NULL_PTR(exp)     TEST_ASSERT(QTIP_STATUS_NULL_PTR == (exp))
#define QTIP_ASSERT_EMPTY(exp)        TEST_ASSERT(QTIP_STATUS_EMPTY == (exp))
#define QTIP_ASSERT_FULL(exp)         TEST_ASSERT(QTIP_STATUS_FULL == (exp))
#define QTIP_ASSERT_LOCKED(exp)       TEST_ASSERT(QTIP_STATUS_LOCKED == (exp))
#define QTIP_ASSERT_INVALID_SIZE(exp) TEST_ASSERT(QTIP_STATUS_INVALID_SIZE == (exp))

#define QTIP_ASSERT_ITEM(expected, actual) TEST_ASSERT_EQUAL_size_t((expected), (actual))

#define QUEUE_SIZE 10U

typedef uint32_t type_t;

qtipContext_t context;
type_t queue[QUEUE_SIZE];
type_t buffer[QUEUE_SIZE];

void setUp(void)
{
    qtip_init(&context, queue, QUEUE_SIZE, sizeof(type_t));
}

void tearDown(void)
{
    memset(queue, 0U, sizeof(queue));
}

void test_put_pop(void)
{
    type_t element1 = 1U;
    type_t element2 = 2U;
    type_t buff     = 0U;
    size_t size     = 0U;
    QTIP_ASSERT_OK(qtip_put(&context, &element1));
    QTIP_ASSERT_OK(qtip_put(&context, &element2));
    QTIP_ASSERT_OK(qtip_pop(&context, &buff));
    TEST_ASSERT_EQUAL_UINT8(element1, buff);
    QTIP_ASSERT_OK(qtip_pop(&context, &buff));
    TEST_ASSERT_EQUAL_UINT8(element2, buff);
}

void test_peek(void)
{
    type_t element1 = 1U;
    type_t element2 = 2U;
    size_t size     = 0U;
    QTIP_ASSERT_OK(qtip_put(&context, &element1));
    QTIP_ASSERT_OK(qtip_put(&context, &element2));
    qtip_peek(&context, buffer, &size);
    TEST_ASSERT_EQUAL_size_t(2U, size);
    TEST_ASSERT_EQUAL_UINT8(element1, buffer[0U]);
    TEST_ASSERT_EQUAL_UINT8(element2, buffer[1U]);
}

void test_rollover(void)
{
    type_t element = 0U;

    const size_t firstPut  = QUEUE_SIZE - 2U;
    const size_t fisrtPop  = 4U;
    const size_t secondPut = 6U;

    for (int i = 0U; i < firstPut; i++)
    {
        element = i;
        QTIP_ASSERT_OK(qtip_put(&context, &element));
    }

    for (int i = 0U; i < fisrtPop; i++)
    {
        QTIP_ASSERT_OK(qtip_pop(&context, &element));
    }

    for (int i = 0U; i < secondPut; i++)
    {
        element = i;
        QTIP_ASSERT_OK(qtip_put(&context, &element));
    }
    QTIP_ASSERT_FULL(qtip_put(&context, &element));
}

void test_empty(void)
{
    type_t element = 0U;
    QTIP_ASSERT_EMPTY(qtip_pop(&context, &element));
}

void test_stress(void)
{
    type_t element = 0U;

    for (int i = 0U; i < QUEUE_SIZE; i++)
    {
        element = i;
        QTIP_ASSERT_OK(qtip_put(&context, &element));
    }
    QTIP_ASSERT_FULL(qtip_put(&context, &element));

    for (int i = 0U; i < QUEUE_SIZE; i++)
    {
        QTIP_ASSERT_OK(qtip_pop(&context, &element));
    }
    QTIP_ASSERT_EMPTY(qtip_pop(&context, &element));
}

void test_purge(void)
{
    type_t item = QUEUE_SIZE;
    size_t qty  = 0U;

    for (int i = 0U; i < item; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &item));
    }

    QTIP_ASSERT_OK(qtip_purge(&context));
    QTIP_ASSERT_OK(qtip_count_items(&context, &qty));
    TEST_ASSERT_EQUAL_size_t(0U, qty);
}

void test_get_front_rear(void)
{
    const type_t itemFront = 1U;
    const type_t itemRear  = QUEUE_SIZE;
    type_t item            = 0U;

    for (int i = itemFront; i <= itemRear; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &i));
    }

    QTIP_ASSERT_OK(qtip_get_front(&context, &item));
    TEST_ASSERT_EQUAL_UINT32(itemFront, item);
    item = 0U;
    QTIP_ASSERT_OK(qtip_get_rear(&context, &item));
    TEST_ASSERT_EQUAL_UINT32(itemRear, item);
}

void test_get_index(void)
{
    const qtipSize_t firstPut  = QUEUE_SIZE - 1U;
    const qtipSize_t firstPop  = firstPut - 1U;
    const qtipSize_t secondPut = 3U;
    const qtipSize_t index     = 3;
    const type_t expectedItem  = firstPut + secondPut - 1U;
    type_t item                = 0U;

    for (int i = 0; i < firstPut; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &i));
    }
    for (int i = 0; i < firstPop; i++)
    {
        QTIP_ASSERT_OK(qtip_pop(&context, &i));
    }
    for (int i = 0; i < secondPut; i++)
    {
        int item = i + firstPut;
        QTIP_ASSERT_OK(qtip_put(&context, &item));
    }

    QTIP_ASSERT_OK(qtip_get_item_index(&context, index, &item));
    TEST_ASSERT_EQUAL_size_t(expectedItem, item);
}

void test_remove_index(void) // NOLINT(readability-function-cognitive-complexity)
{
    const qtipSize_t index = 2;
    qtipSize_t size        = 0U;
    type_t item            = 0U;

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &i));
    }

    QTIP_ASSERT_OK(qtip_remove_item_index(&context, index));
    QTIP_ASSERT_OK(qtip_count_items(&context, &size));
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE - 1U, size);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(0U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(1U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(3U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(4U, item);
}

void test_pop_index(void) // NOLINT(readability-function-cognitive-complexity)
{
    const qtipSize_t index = 2;
    qtipSize_t size        = 0U;
    type_t item            = 0U;

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &i));
    }

    QTIP_ASSERT_OK(qtip_get_pop_index(&context, index, &item));
    QTIP_ASSERT_OK(qtip_count_items(&context, &size));
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE - 1U, size);
    QTIP_ASSERT_ITEM(index, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(0U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(1U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(3U, item);

    QTIP_ASSERT_OK(qtip_pop(&context, &item));
    QTIP_ASSERT_ITEM(4U, item);
}

void test_lock(void)
{
    type_t item = 0;
    QTIP_ASSERT_OK(qtip_lock(&context));
    QTIP_ASSERT_LOCKED(qtip_is_locked(&context));
    QTIP_ASSERT_LOCKED(qtip_put(&context, &item));
    QTIP_ASSERT_OK(qtip_unlock(&context));
    QTIP_ASSERT_OK(qtip_put(&context, &item));
    QTIP_ASSERT_OK(qtip_is_locked(&context));
}

void test_telemetry(void)
{
    type_t item      = 0U;
    size_t processed = 0U;
    size_t total     = 0U;

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        QTIP_ASSERT_OK(qtip_put(&context, &item));
    }

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        QTIP_ASSERT_OK(qtip_pop(&context, &item));
    }

    QTIP_ASSERT_OK(qtip_total_processed_items(&context, &processed));
    QTIP_ASSERT_OK(qtip_total_enqueued_items(&context, &total));
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE, total);
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE, processed);
}

void test_null_ptr(void) // NOLINT
{
    QTIP_ASSERT_NULL_PTR(qtip_init(NULL, NULL, 0U, 0U));
    QTIP_ASSERT_NULL_PTR(qtip_put(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_pop(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_peek(NULL, NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_purge(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_get_front(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_get_rear(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_is_full(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_is_empty(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_count_items(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_is_locked(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_lock(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_unlock(NULL));
    QTIP_ASSERT_NULL_PTR(qtip_total_enqueued_items(NULL, NULL));
    QTIP_ASSERT_NULL_PTR(qtip_total_processed_items(NULL, NULL));
}

void test_invalid_size(void)
{
    QTIP_ASSERT_INVALID_SIZE(qtip_init(&context, queue, 0U, sizeof(type_t)));
    QTIP_ASSERT_INVALID_SIZE(qtip_init(&context, queue, QUEUE_SIZE, 0U));
}