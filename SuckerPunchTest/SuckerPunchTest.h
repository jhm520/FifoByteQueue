#pragma once


struct Q
{
	//Whether this Q is allocated
	unsigned char IsAllocated = 0;	//1 byte

	//What position in data it is located
	unsigned char Index = 0;
	
	//The index of the next block, 0 if no next block
	unsigned char NextQIndex = 0;	//1 byte

	//How many chars of memory on this block are in use
	unsigned char MemorySize = 0;	//1 byte

	//the data itself
	unsigned char Memory[28];		//28 bytes
};

// Creates a FIFO byte queue, returning a handle to it.
Q* Create_Queue();

Q* FindUnallocatedQ();

Q* GetNextQ(Q* q);

// Destroy an earlier created byte queue.
void Destroy_Queue(Q * q);

// Adds a new byte to a queue.
void Enqueue_Byte(Q * q, unsigned char b);

// Pops the next byte off the FIFO queue
unsigned char Dequeue_Byte(Q * q);

void on_out_of_memory();

void on_illegal_operation();