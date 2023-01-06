#include "queue.h"
#include "unity.h"

#include <string.h>

#define QUEUE_SIZE 10U

typedef uint32_t type_t;

queueContext_t context;
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