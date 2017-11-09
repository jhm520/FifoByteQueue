/*!
Implementation for Sucker Punch Programming Problem.
This is a variable number of variable-sized byte queues
operating in a memory-constrained environment.

@author Sam Paul
*/

#include "stdafx.h"
#include "Q.h"
#include <cstring>  // memset
#include <cstdio>

namespace
{

	/*!
	The low 7 bits of this handle uniquely identify a storage block, which
	can be obtained by calling GetStorageFromHandle.
	*/
	typedef unsigned char StorageBlockHandle;

	/*!
	Chosen so QStorageBlock is exactly 16 bytes. Playing with this number has
	interesting ramifications for memory efficiency, depending on expected
	runtime behavior.
	*/
	const int USABLE_BLOCK_BYTES = 15;

	/*!
	Chunk o' memory doled out by a fixed-width allocator accessed through
	AllocBlock and FreeBlock. The intrusive linked list is used to implement
	the freelist and the ring lists inside the queues.
	*/
	struct QStorageBlock
	{
		StorageBlockHandle nextBlock;
		unsigned char storage[USABLE_BLOCK_BYTES];
	};

	const int TOTAL_MEM_SIZE = 2048;
	const int Q_HEADER_SIZE = 4;
	const int MAX_NUM_QUEUES = 64;
	const int Q_HEADER_STORAGE = 1; // offset at which the Q headers are stored
	const int Q_STORAGE_OFFSET = Q_HEADER_STORAGE + Q_HEADER_SIZE*MAX_NUM_QUEUES;
	const int MAX_NUM_BLOCKS =
		(TOTAL_MEM_SIZE - Q_HEADER_SIZE*MAX_NUM_QUEUES - 1) /
		sizeof(QStorageBlock);

	/*!
	Our entire available memory space.

	Organized like so:

	index       purpose
	--------------------------------------
	0           freelist handle
	1-192       all 64 available Q structs
	193-208     storage block 0
	209-224     storage block 1
	225-240     storage block 2
	...         ...
	2017-2032   storage block 114
	2033-2047   unused
	*/
	unsigned char data[TOTAL_MEM_SIZE];

	/*!
	Helper function allows easy manipulation of freelist while abstracting
	its storage mechanism, location.
	*/
	StorageBlockHandle& Freelist()
	{
		return data[0];
	}

	/*!
	Helper function converts 7-bit storage block handles into actual
	QStorageBlock pointers.
	*/
	QStorageBlock* GetStorageFromHandle(StorageBlockHandle handle)
	{
		if (handle != 0)
		{
			return reinterpret_cast<QStorageBlock*>(
				data + Q_STORAGE_OFFSET + sizeof(QStorageBlock)*(handle - 1)
				);
		}
		else
		{
			return 0;  // Null handle translates to NULL pointer
		}
	}

	/*!
	Returns a usable storage block as a 7-bit handle.

	@see GetStorageFromHandle
	*/
	StorageBlockHandle AllocBlock()
	{
		// Pull front block off of freelist
		StorageBlockHandle allocated = Freelist();

		if (allocated != 0)
		{
			Freelist() = GetStorageFromHandle(allocated)->nextBlock;
			GetStorageFromHandle(allocated)->nextBlock = 0; // Just in case
		}
		else
		{
			on_out_of_memory();
		}

		// Put this down here to silence compiler warnings
		return allocated;
	}

	/*!
	Release a storage block previously acquired with AllocBlock.

	@see AllocBlock
	*/
	void FreeBlock(StorageBlockHandle block)
	{
		// Link the block back onto the front of the freelist
		GetStorageFromHandle(block)->nextBlock = Freelist();
		Freelist() = block;
	}


	// Useful for debugging
	void PrintStatus(Q* q)
	{
		puts("-------------");
		printf("head: %03d,%02d\n", q->headBlock, q->headOffset);
		printf("tail: %03d,%02d\n", q->tailBlock, q->tailOffset);
		printf("bud:  %03d\n", q->budBlock);
		puts("-------------");
	}

}

/*!
This function must be called before any other queue operations so it can
properly initialize the provided memory space.
*/
void init_queue_storage()
{
	// Freelist handle initially points at storage block 0
	data[0] = 1;  // 0 is reserved for 'null'

				  // Q headers initialized to 0
	memset(data + Q_HEADER_STORAGE, 0, Q_HEADER_SIZE*MAX_NUM_QUEUES);

	// Build the freelist, a single-linked list of storage blocks
	for (StorageBlockHandle block = 1; block <= MAX_NUM_BLOCKS; ++block)
	{
		GetStorageFromHandle(block)->nextBlock = block + 1;
	}

	// Last block has 'null' handle
	GetStorageFromHandle(MAX_NUM_BLOCKS)->nextBlock = 0;
}

/*!
Allocate and return an empty byte queue. It is an illegal operation
to create more than 64 queues at a time.
*/
Q* create_queue()
{
	Q* newQueue = nullptr;

	// Search for a queue header not currently in use
	for (int queueIndex = 0; queueIndex < MAX_NUM_QUEUES; ++queueIndex)
	{
		Q* queueHeader = reinterpret_cast<Q*>(data + Q_HEADER_STORAGE + queueIndex*Q_HEADER_SIZE);

		if (!queueHeader->inUse)
		{
			newQueue = queueHeader;
			break;
		}
	}

	if (newQueue != 0)
	{
		// Mark this queue as 'in use'
		newQueue->inUse = 1;

		// Allocate a block of storage for the queue to start with
		newQueue->headBlock = AllocBlock();
		newQueue->tailBlock = newQueue->headBlock;
		newQueue->budBlock = newQueue->headBlock;

		// Form the beginnings of our ring list by linking to ourselves
		GetStorageFromHandle(newQueue->headBlock)->nextBlock =
			newQueue->headBlock;

		// Conceptually, the 'head' and 'tail' pointers of our circular buffer
		// are both set to the first element.
		newQueue->headOffset = newQueue->tailOffset = 0;
	}
	else
	{
		// Tried to create more than 64 queues
		on_illegal_operation();
	}

	return newQueue;
}

/*!
Deallocate a byte queue previously allocated with create_queue.
*/
void destroy_queue(Q* deadQueue)
{
	// Splice our ring list onto the freelist
	StorageBlockHandle blockAfterBudBlock =
		GetStorageFromHandle(deadQueue->budBlock)->nextBlock;
	GetStorageFromHandle(deadQueue->budBlock)->nextBlock = Freelist();
	Freelist() = blockAfterBudBlock;

	// Mark this queue as not 'in use'
	deadQueue->inUse = 0;
}

/*!
Add a byte to the back of a queue.

@param[in] q
A byte queue returned by create_queue

@param[in] b
A byte of data. Will be added to the FIFO queue.
*/
void enqueue_byte(Q* q, unsigned char b)
{
	// We always enqueue at the TAIL, so it must always be valid to write
	// to the tail block/offset location.
	QStorageBlock* tailBlock = GetStorageFromHandle(q->tailBlock);
	tailBlock->storage[q->tailOffset] = b;

	// If the tail wraps to the head block but doesn't collide,
	// move the bud block to where the tail block used to be
	StorageBlockHandle oldTailBlock = q->tailBlock;
	bool updateBudBlock = false;

	unsigned int newOffset = (q->tailOffset + 1) % USABLE_BLOCK_BYTES;
	if (newOffset < q->tailOffset)
	{
		// Wrap around to head block, if we're not there already
		q->tailBlock = q->headBlock;
		updateBudBlock = true;  // Might happen
	}

	q->tailOffset = newOffset;

	if (q->tailBlock == q->headBlock)
	{
		if (newOffset == q->headOffset)
		{
			// No place to put the next byte, so we need to grow
			StorageBlockHandle newBlockHandle = AllocBlock();
			QStorageBlock* newBlock = GetStorageFromHandle(newBlockHandle);

			// Copy trailing elements, if any, to new block
			for (unsigned int copyOffset = 0; copyOffset < newOffset; ++copyOffset)
			{
				newBlock->storage[copyOffset] =
					GetStorageFromHandle(q->headBlock)->storage[copyOffset];
			}

			// If we just went off the end of a block, that block is the new 
			// bud block
			if (updateBudBlock)
			{
				q->budBlock = oldTailBlock;
			}

			// Link up with new block
			newBlock->nextBlock = q->headBlock;
			GetStorageFromHandle(q->budBlock)->nextBlock = newBlockHandle;
			q->tailBlock = newBlockHandle;

			updateBudBlock = false; // Not needed
		}
	}

	// 'Bud' block (as in new growth) must be known for constant time insertion
	if (updateBudBlock)
	{
		q->budBlock = oldTailBlock;
	}

	// For debugging
	//PrintStatus(q);
}

/*!
Pop a byte off the front of a queue. The byte is removed from the queue.

@param[in] q
A byte queue previously returned by create_queue

@return
A byte of data previously pushed on with enqueue_byte, according
to FIFO ordering.
*/
unsigned char dequeue_byte(Q* q)
{
	if (q->headBlock == q->tailBlock && q->headOffset == q->tailOffset)
	{
		// Queue is empty!
		on_illegal_operation();
	}

	// We always dequeue at the HEAD
	unsigned char poppedByte = GetStorageFromHandle(q->headBlock)->storage[q->headOffset];

	unsigned int newOffset = (q->headOffset + 1) % USABLE_BLOCK_BYTES;

	// If we wrapped around, we might be on a whole new block
	if (newOffset < q->headOffset)
	{
		StorageBlockHandle newHeadBlock =
			GetStorageFromHandle(q->headBlock)->nextBlock;

		// If the block we just left wasn't the tail block...
		if (q->headBlock != q->tailBlock)
		{
			// The block we just left is not being used at all.
			// Furthermore, it MUST (by induction) be immediately after
			// the tail block. So we can unlink it, and free it.
			GetStorageFromHandle(q->tailBlock)->nextBlock = newHeadBlock;
			FreeBlock(q->headBlock);
		}

		q->headBlock = newHeadBlock;
	}

	q->headOffset = newOffset;

	// For debugging
	//PrintStatus(q);

	return poppedByte;
}

void on_out_of_memory()
{

}

void on_illegal_operation()
{

}

void PrintStatusExtern(Q* q)
{
	PrintStatus(q);
}
