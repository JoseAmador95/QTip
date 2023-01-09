/**
 * @file test_qtip.c
 * @brief Unit tests for QTip API
 * @author Jose Amador
 * @copyright MIT License
 */

#include "qtip.h"
#include "unity.h"

#include <string.h>

#define QTIP_ASSERT_OK(exp) TEST_ASSERT(QTIP_STATUS_OK == (exp))
#define QUEUE_SIZE          10U

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
    TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element1));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element2));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_pop(&context, &buff));
    TEST_ASSERT_EQUAL_UINT8(element1, buff);
    TEST_ASSERT(QTIP_STATUS_OK == qtip_pop(&context, &buff));
    TEST_ASSERT_EQUAL_UINT8(element2, buff);
}

void test_peek(void)
{
    type_t element1 = 1U;
    type_t element2 = 2U;
    size_t size     = 0U;
    TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element1));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element2));
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
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element));
    }

    for (int i = 0U; i < fisrtPop; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_pop(&context, &element));
    }

    for (int i = 0U; i < secondPut; i++)
    {
        element = i;
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element));
    }
    TEST_ASSERT(QTIP_STATUS_FULL == qtip_put(&context, &element));
}

void test_empty(void)
{
    type_t element = 0U;
    TEST_ASSERT(QTIP_STATUS_EMPTY == qtip_pop(&context, &element));
}

void test_stress(void)
{
    type_t element = 0U;

    for (int i = 0U; i < QUEUE_SIZE; i++)
    {
        element = i;
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &element));
    }
    TEST_ASSERT(QTIP_STATUS_FULL == qtip_put(&context, &element));

    for (int i = 0U; i < QUEUE_SIZE; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_pop(&context, &element));
    }
    TEST_ASSERT(QTIP_STATUS_EMPTY == qtip_pop(&context, &element));
}

void test_purge(void)
{
    type_t item = QUEUE_SIZE;
    size_t qty  = 0U;

    for (int i = 0U; i < item; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &item));
    }

    TEST_ASSERT(QTIP_STATUS_OK == qtip_purge(&context));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_count_items(&context, &qty));
    TEST_ASSERT_EQUAL_size_t(0U, qty);
}

void test_get_front_rear(void)
{
    const type_t itemFront = 1U;
    const type_t itemRear  = 10U;
    type_t item            = 0U;

    for (int i = itemFront; i <= itemRear; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &i));
    }

    TEST_ASSERT(QTIP_STATUS_OK == qtip_get_front(&context, &item));
    TEST_ASSERT_EQUAL_UINT32(itemFront, item);
    item = 0U;
    TEST_ASSERT(QTIP_STATUS_OK == qtip_get_rear(&context, &item));
    TEST_ASSERT_EQUAL_UINT32(itemRear, item);
}

void test_lock(void)
{
    type_t item = 0;
    TEST_ASSERT(QTIP_STATUS_OK == qtip_lock(&context));
    TEST_ASSERT(QTIP_STATUS_LOCKED == qtip_is_locked(&context));
    TEST_ASSERT(QTIP_STATUS_LOCKED == qtip_put(&context, &item));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_unlock(&context));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &item));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_is_locked(&context));
}

void test_telemetry(void)
{
    type_t item      = 0U;
    size_t processed = 0U;
    size_t total     = 0U;

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_put(&context, &item));
    }

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        TEST_ASSERT(QTIP_STATUS_OK == qtip_pop(&context, &item));
    }

    TEST_ASSERT(QTIP_STATUS_OK == qtip_total_processed_items(&context, &processed));
    TEST_ASSERT(QTIP_STATUS_OK == qtip_total_enqueued_items(&context, &total));
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE, total);
    TEST_ASSERT_EQUAL_size_t(QUEUE_SIZE, processed);
}

void test_null_ptr(void) // NOLINT
{
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_init(NULL, NULL, NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_put(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_pop(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_peek(NULL, NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_purge(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_get_front(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_get_rear(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_is_full(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_is_empty(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_count_items(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_is_locked(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_lock(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_unlock(NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_total_enqueued_items(NULL, NULL));
    TEST_ASSERT(QTIP_STATUS_NULL_PTR == qtip_total_processed_items(NULL, NULL));
}

void test_invalid_size(void)
{
    TEST_ASSERT(QTIP_STATUS_INVALID_SIZE == qtip_init(&context, queue, 0U, sizeof(type_t)));
    TEST_ASSERT(QTIP_STATUS_INVALID_SIZE == qtip_init(&context, queue, QUEUE_SIZE, 0U));
}