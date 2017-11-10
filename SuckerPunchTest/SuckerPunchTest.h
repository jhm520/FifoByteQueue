//Copyright 2017, John Henry Miller, All Rights Reserved.

#pragma once

//Sucker Punch Productions programming challenge response

struct Q
{
	//Whether this Q is allocated
	unsigned char IsAllocated = 0;	//1 byte

	//What position in data the Q is located, when multiplied by 32
	unsigned char Index = 0;
	
	//The index of the next Q, when multiplied by 32, 0 if no next block
	unsigned char NextQIndex = 0;	//1 byte

	//How many chars of memory on this block are in use
	unsigned char MemorySize = 0;	//1 byte

	//the data itself
	unsigned char Memory[28];		//28 bytes
};

// Creates a FIFO byte queue, returning a handle to it.
Q* Create_Queue();

//Find a space in the data array for a new Queue, allocate and return it
Q* FindUnallocatedQ();

//Given a Queue, get it's next Queue (if it has allocated one)
Q* GetNextQ(Q* q);

// Destroy an earlier created byte queue.
void Destroy_Queue(Q * q);

// Adds a new byte to a queue.
void Enqueue_Byte(Q * q, unsigned char b);

// Pops the next byte off the FIFO queue
unsigned char Dequeue_Byte(Q * q);

//Called when data is out of memory
void on_out_of_memory();

//Called when an illegal operation occurs
void on_illegal_operation();