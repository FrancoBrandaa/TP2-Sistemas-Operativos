#include <stdint.h>
#include <stdlib.h>

#include "CuTest.h"
#include "MemoryManager.h"
#include "MemoryManagerTest.h"

#define MANAGED_MEMORY_SIZE 20480
#define ALLOCATION_SIZE 1024
#define WRITTEN_VALUE 'a'
#define TEST_MIN_ORDER 6   // 2^6 = 64 bytes mínimo
#define TEST_MAX_ORDER 15  // 2^15 = 32KB máximo

void testAllocMemory(CuTest *const cuTest);
void testTwoAllocations(CuTest *const cuTest);
void testWriteMemory(CuTest *const cuTest);
void testFreeMemory(CuTest *const cuTest);
void testFreeAndRealloc(CuTest *const cuTest);
void testMemoryAlignment(CuTest *const cuTest);

static const size_t TestQuantity = 6;
static const Test MemoryManagerTests[] = {
    testAllocMemory, 
    testTwoAllocations, 
    testWriteMemory,
    testFreeMemory,
    testFreeAndRealloc,
    testMemoryAlignment
};

static inline void givenAMemoryManager(CuTest *const cuTest);
static inline void givenAMemoryAmount(void);
static inline void givenAnAllocation(void);

static inline void whenMemoryIsAllocated(void);
static inline void whenMemoryIsWritten(void);

static inline void thenSomeMemoryIsReturned(CuTest *const cuTest);
static inline void thenTheTwoAdressesAreDifferent(CuTest *const cuTest);
static inline void thenBothDoNotOverlap(CuTest *const cuTest);
static inline void thenMemorySuccessfullyWritten(CuTest *const cuTest);

static MemoryManagerADT memoryManager;

static size_t memoryToAllocate;

static void *allocatedMemory = NULL;
static void *anAllocation = NULL;

CuSuite *getMemoryManagerTestSuite(void) {
	CuSuite *const suite = CuSuiteNew();

	for (size_t i = 0; i < TestQuantity; i++)
		SUITE_ADD_TEST(suite, MemoryManagerTests[i]);

	return suite;
}

void testAllocMemory(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	givenAMemoryAmount();

	whenMemoryIsAllocated();

	thenSomeMemoryIsReturned(cuTest);
}

void testTwoAllocations(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	givenAMemoryAmount();
	givenAnAllocation();

	whenMemoryIsAllocated();

	thenSomeMemoryIsReturned(cuTest);
	thenTheTwoAdressesAreDifferent(cuTest);
	thenBothDoNotOverlap(cuTest);
}

void testWriteMemory(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	givenAMemoryAmount();
	givenAnAllocation();

	whenMemoryIsWritten();

	thenMemorySuccessfullyWritten(cuTest);
}

inline void givenAMemoryManager(CuTest *const cuTest) {
	// Allocate memory for the entire heap (including space for the memory manager structure)
	void *managedMemory = malloc(MANAGED_MEMORY_SIZE);
	if (managedMemory == NULL) {
		CuFail(cuTest, "[givenAMemoryManager] Managed Memory cannot be null");
	}

	// Calculate end of heap
	char *heapStart = (char *)managedMemory;
	char *heapEnd = heapStart + MANAGED_MEMORY_SIZE;
	
	// Create memory manager with your interface: (heapStart, heapEnd, minOrder, maxOrder)
	memoryManager = createMemoryManager(heapStart, heapEnd, TEST_MIN_ORDER, TEST_MAX_ORDER);
	
	if (memoryManager == NULL) {
		CuFail(cuTest, "[givenAMemoryManager] Memory Manager creation failed");
	}
}

inline void givenAMemoryAmount(void) {
	memoryToAllocate = ALLOCATION_SIZE;
}

inline void givenAnAllocation(void) {
	anAllocation = allocMemory(memoryManager, memoryToAllocate);
}

inline void whenMemoryIsAllocated(void) {
	allocatedMemory = allocMemory(memoryManager, memoryToAllocate);
}

inline void whenMemoryIsWritten(void) {
	*((char *) anAllocation) = WRITTEN_VALUE;
}

inline void thenSomeMemoryIsReturned(CuTest *const cuTest) {
	CuAssertPtrNotNull(cuTest, allocatedMemory);
}

inline void thenTheTwoAdressesAreDifferent(CuTest *const cuTest) {
	CuAssertTrue(cuTest, anAllocation != allocatedMemory);
}

inline void thenBothDoNotOverlap(CuTest *const cuTest) {
	int distance = (char *) anAllocation - (char *) allocatedMemory;
	distance = abs(distance);

	CuAssertTrue(cuTest, distance >= ALLOCATION_SIZE);
}

inline void thenMemorySuccessfullyWritten(CuTest *const cuTest) {
	uint8_t writtenValue = WRITTEN_VALUE;
	uint8_t readValue = *((uint8_t *) anAllocation);

	CuAssertIntEquals(cuTest, writtenValue, readValue);
}

void testFreeMemory(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	givenAMemoryAmount();

	// Allocate some memory
	void *ptr = allocMemory(memoryManager, memoryToAllocate);
	CuAssertPtrNotNull(cuTest, ptr);
	
	size_t usedBefore = getUsedMemory(memoryManager);
	size_t freeBefore = getFreeMemory(memoryManager);
	
	// Free the memory
	freeMemory(memoryManager, ptr);
	
	size_t usedAfter = getUsedMemory(memoryManager);
	size_t freeAfter = getFreeMemory(memoryManager);
	
	// Memory should be freed
	CuAssertTrue(cuTest, usedAfter < usedBefore);
	CuAssertTrue(cuTest, freeAfter > freeBefore);
}

void testFreeAndRealloc(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	givenAMemoryAmount();

	// Allocate, free, then allocate again
	void *ptr1 = allocMemory(memoryManager, memoryToAllocate);
	CuAssertPtrNotNull(cuTest, ptr1);
	
	freeMemory(memoryManager, ptr1);
	
	void *ptr2 = allocMemory(memoryManager, memoryToAllocate);
	CuAssertPtrNotNull(cuTest, ptr2);
	
	// Should be able to write to the new allocation
	*((char *)ptr2) = WRITTEN_VALUE;
	CuAssertIntEquals(cuTest, WRITTEN_VALUE, *((char *)ptr2));
}

void testMemoryAlignment(CuTest *const cuTest) {
	givenAMemoryManager(cuTest);
	
	// Test various allocation sizes
	void *ptr1 = allocMemory(memoryManager, 1);     // Very small
	void *ptr2 = allocMemory(memoryManager, 64);    // Minimum block size
	void *ptr3 = allocMemory(memoryManager, 1000);  // Large
	
	CuAssertPtrNotNull(cuTest, ptr1);
	CuAssertPtrNotNull(cuTest, ptr2);
	CuAssertPtrNotNull(cuTest, ptr3);
	
	// All pointers should be different
	CuAssertTrue(cuTest, ptr1 != ptr2);
	CuAssertTrue(cuTest, ptr2 != ptr3);
	CuAssertTrue(cuTest, ptr1 != ptr3);
	
	// Test that we can write to all of them
	*((char *)ptr1) = 'A';
	*((char *)ptr2) = 'B';
	*((char *)ptr3) = 'C';
	
	CuAssertIntEquals(cuTest, 'A', *((char *)ptr1));
	CuAssertIntEquals(cuTest, 'B', *((char *)ptr2));
	CuAssertIntEquals(cuTest, 'C', *((char *)ptr3));
}
