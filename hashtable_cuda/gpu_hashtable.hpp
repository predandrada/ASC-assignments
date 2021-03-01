#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <string>
#include <cmath>

#include "gpu_hashtable.hpp"


// Auxiliary methods
__device__ unsigned long long hashKey(int key, unsigned long long int capacity) {
	return ((unsigned long long)key * HASH_1) % HASH_2 % (unsigned long long int)capacity;
}

__device__ hashelem_t makeElement(int key, int value) {
   return (((hashelem_t) key) << 8 * sizeof(value)) | value;
}

__device__ int elementKey(hashelem_t e) {
   return e >> 8 * sizeof(int);
}

__device__ int elementValue(hashelem_t e) {
   return (int) (e & ((1ULL << 8 * sizeof(int))-1));
}

/* INIT HASH
 */
__global__ void kernel_init_table(hashtable_t *hashtable, hashelem_t *table, unsigned long long size) {
	hashtable->table = table;
	hashtable->capacity = size;
	hashtable->items = 0;
}

// Create Hashtable
GpuHashTable::GpuHashTable(int size) {
	// Allocate memory
	hashelem_t *table = NULL;
	cudaMalloc((void **) &table, size * sizeof(hashelem_t));
	cudaMemset(table, ZERO_ELEM, size);
	cudaMalloc((void **) &hashtable, sizeof(hashtable_t));

	// Initialize hashtable structure
	kernel_init_table<<<1, 1>>>(hashtable, table, size);
	cudaDeviceSynchronize();

	arraySize = 1;
	allocArrays();
}

/* DESTROY HASH
 */
GpuHashTable::~GpuHashTable() {
	hashtable_t *copy_hashtable = (hashtable_t *)calloc(1, sizeof(hashtable_t));
	cudaMemcpy(copy_hashtable, hashtable, sizeof(hashtable_t), cudaMemcpyDeviceToHost);
	cudaFree(copy_hashtable->table);
	free(copy_hashtable);
	// Destroy arrays for data distribution
	free(hostValues);
	cudaFree(hashtable);
	cudaFree(deviceKeys);
	cudaFree(deviceValues);
}

// Allocate memory for data distribution arrays
void GpuHashTable::allocArrays() {
	hostValues = (int *) calloc(arraySize, sizeof(int));
	cudaMalloc((void **) &deviceKeys, arraySize * sizeof(int));
	cudaMalloc((void **) &deviceValues, arraySize * sizeof(int));
}

/* RESHAPE HASH
 */
void GpuHashTable::reshape(int batchSize) {
	if (batchSize > arraySize) {
		arraySize = batchSize;
		cudaFree(deviceValues);
		cudaFree(deviceKeys);
		free(hostValues);
		allocArrays();		
	}
}

/* INSERT BATCH
 */
__global__ void kernel_rehash(hashtable_t *hashtable, hashelem_t *new_hashtable, int new_capacity) {
	// Rehash old elements from hashtable and insert them to the new_hashtable
	int index = threadIdx.x + blockDim.x * blockIdx.x;
	if (index > hashtable->capacity)
		return;
	
	// Get key and value for this thread
	hashelem_t e = hashtable->table[index];
	if (e == ZERO_ELEM)	// If element is empty, we are done
		return;

	int key = elementKey(e);
	unsigned long long int hashValue = hashKey(key, new_capacity);
	unsigned long long int tableIndex = hashValue;
	// Try to add using quadratic probing:
	for (unsigned long long int i = 0; i < new_capacity; i++) {
		// Use atomicCAS for concurrency management
		hashelem_t currElem = atomicCAS(new_hashtable + tableIndex, ZERO_ELEM, e);
		if (currElem == ZERO_ELEM) // If element was inserted in hashtable, return
			return;

		// If element wasn't inserted, get the next tableIndex using quadratic probing
		tableIndex = (hashValue + i*i) % new_capacity;
	}
}

__global__ void kernel_insert(hashtable_t *hashtable, int *keys, int *values, int numKeys) {
	int index = threadIdx.x + blockDim.x * blockIdx.x;
	if (index > numKeys)
		return;

	// Get key and value for this thread
	int key = keys[index];
	int value = values[index];
	if (key == 0 || value == 0)	// If either key or value is 0, don't do anything
		return;

	unsigned long long int hashValue = hashKey(key, hashtable->capacity);
	unsigned long long int tableIndex = hashValue;
	
	// Make hash element
	hashelem_t e = makeElement(key, value);

	// Try to add using quadratic probing:
	for (unsigned long long int i = 0; i < hashtable->capacity; i++) {
		// Use atomicCAS for concurrency management
		hashelem_t currElem = atomicCAS(hashtable->table + tableIndex, ZERO_ELEM, e);
		if (currElem == ZERO_ELEM) { // If element was inserted in hashtable
			atomicAdd(&hashtable->items, 1);
			return;
		} else if (elementKey(currElem) == key) {	// If the currElem has the same key, then update
			atomicExch(hashtable->table + tableIndex, e);
			return;
		}

		// If element wasn't inserted, get the next tableIndex using quadratic probing
		tableIndex = (hashValue + i*i) % hashtable->capacity;
	}

	// Shouldn't get here unless the hash is full, but we checked for that in checkLoad()
}

bool GpuHashTable::insertBatch(int *keys, int* values, int numKeys) {

	// checking whether the hashtable is full using the current_load_factor
	// hashtable_t *copy_hashtable = (hashtable_t *)calloc(1, sizeof(hashtable_t));	
	// cudaMemcpy(copy_hashtable, hashtable, sizeof(hashtable_t), cudaMemcpyDeviceToHost);

	// double current_load_factor = ((double) copy_hashtable->items + batchSize) / ((double) copy_hashtable->capacity);
	double current_load_factor = ((double) hashtable->items + numKeys) / ((double) hashtable->capacity);
	
	if (MAX_LOAD <= current_load_factor) {
		// Compute new capacity:
		// unsigned long long int new_capacity = ((double)copy_hashtable->items + (double)batchSize) * 5 / 4;
		unsigned long long int new_capacity = ((double)hashtable->items + (double)numKeys) * 5 / 4;

		// Create new table
		hashelem_t *new_hashtable = NULL;
		cudaMalloc((void **) &new_hashtable, new_capacity * sizeof(hashelem_t));
		cudaMemset(new_hashtable, 0, new_capacity);

		// Rehash elements in hashtable
		int numBlocks = hashtable->capacity / THREADS_ON_BLOCK + 1;
		kernel_rehash<<<numBlocks, THREADS_ON_BLOCK>>>(hashtable, new_hashtable, new_capacity);
		cudaDeviceSynchronize();

		// Exchange the old hashtable with the new table
		cudaFree(hashtable->table);
		hashtable->table = new_hashtable;
		hashtable->capacity = new_capacity;
		
		// Copy hashtable info from host to device
	}

	reshape(numKeys);
	int numBlocks = numKeys / 512 + 1;

	// Copy data from host to device
	cudaMemcpy(deviceKeys, keys, numKeys * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(deviceValues, values, numKeys * sizeof(int), cudaMemcpyHostToDevice);

	kernel_insert<<<numBlocks, 512>>>(hashtable, deviceKeys, deviceValues, numKeys);
	cudaDeviceSynchronize();

	return true;
}

/* GET BATCH
 */
__global__ void kernel_get_batch(hashtable_t *hashtable, int *keys, int *values, int numKeys) {
	int index = threadIdx.x + blockDim.x * blockIdx.x;
	if (index > numKeys)
		return;

	// Get key for this thread and try to find it's value in the hashtable
	int key = keys[index];
	unsigned long long int hashcode = hashKey(key, hashtable->capacity);
	unsigned long long int tableIndex = hashcode;
	hashelem_t elem;

	// Get hash element using quadratic probing:
	for(unsigned long long int i = 0; i < hashtable->capacity; i++) {
		elem = hashtable->table[tableIndex];
		if (elementKey(elem) == key) { // If element has the same key, save the value
			values[index] = elementValue(elem);
			return;
		}

		// If element wasn't found, get the next tableIndex using quadratic probing
		tableIndex = (hashcode + i*i) % hashtable->capacity;
	}
}

int* GpuHashTable::getBatch(int* keys, int numKeys) {
	int numBlocks = numKeys / 512 + 1;

	// Copy keys array from host to device and start kernel
	cudaMemcpy(deviceKeys, keys, numKeys * sizeof(int), cudaMemcpyHostToDevice);
	kernel_get_batch<<<numBlocks, 512>>>(hashtable, deviceKeys, deviceValues, numKeys);
	cudaDeviceSynchronize();

	// Copy back the values from device to host and return them
	cudaMemcpy(hostValues, deviceValues, numKeys * sizeof(int), cudaMemcpyDeviceToHost);

	return hostValues;
}

/* GET LOAD FACTOR
 * num elements / hash total slots elements
 */
float GpuHashTable::loadFactor() {
	// Get hashtable info from the device to host and compute load
	hashtable_t *copy_hashtable = (hashtable_t *)calloc(1, sizeof(hashtable_t));
	
	cudaMemcpy(copy_hashtable, hashtable, sizeof(hashtable_t), cudaMemcpyDeviceToHost);
	float current_load_factor = ((float) copy_hashtable->items) / ((float) copy_hashtable->capacity);

	free(copy_hashtable);
	return current_load_factor;
}

/*********************************************************/

#define HASH_INIT GpuHashTable GpuHashTable(100);
#define HASH_RESERVE(size) GpuHashTable.reshape(size);

#define HASH_BATCH_INSERT(keys, values, numKeys) GpuHashTable.insertBatch(keys, values, numKeys)
#define HASH_BATCH_GET(keys, numKeys) GpuHashTable.getBatch(keys, numKeys)

#define HASH_LOAD_FACTOR GpuHashTable.loadFactor()
#include "test_map.cpp"