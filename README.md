# QTip

QTip is a simple general-purpose queue library for C.

* [Repository](https://github.com/JoseAmador95/QTip).
* [Documentation](https://joseamador95.github.io/QTip/).

## Description

QTip uses a circular buffer to store the queue's items. The memory area used as circular buffer is given by the user, and may exist in either heap or stack. The QTip API does not allocate memory in the heap.

QTip has the basic queue API such as:

* **put**: Put an item into the queue.
* **pop**: Get and remove an item from the queue.
* **peek**: Read the entire queue without.
* **purge**: Delete all the items in the queue.

The locking mechanism prevents multiple threads from interacting with a shared queue.

The integrated telemetry helps keeping track of the number of enqueued items and processed items.

## Configuration

The following preprocessor macros can be defined to disable features in order to save memory.

* **DISABLE_LOCK**: Disables the locking mechanism.
* **DISABLE_TELEMETRY**: Disables the queue telemetry.
* **REDUCED_API**: Reduces the public API to save memory.
* **SKIP_ARG_CHECK**: Skips checking the value of the API's arguments.

## Examples

### Initialise a Queue

A queue can be initialised with a pointer to a memory segment large enough to hold the maximum expected number of items times the size of each item (in bytes).

```c
struct queueItem
{
    uint32_t field1;
    uint32_t field2;
};

const size_t maximumNumberOfItems = 10U;
const size_t sizeOfItem = sizeof(queueItem);
const size_t bufferSize = maximumNumberOfItems * sizeOfItem;
uint8_t circularBuffer[bufferSize];
qtipContext_t qContext;

qtip_init(&qContext, circularBuffer, maximumNumberOfItems, sizeOfItem);
```

### Interacting with the Queue

A queue can receive or give items.

```c
struct queueItem itemToPut = {.field1 = 1U, .field2 =  2U};
struct queueItem *itemToGet;

qtip_put(&qContext, &itemToPut); // Puts an item in the queue
qtip_pop(&qContext, itemToGet); // Gets the previous item from the queue
```
