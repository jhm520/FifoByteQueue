//Copyright 2017, John Henry Miller, All Rights Reserved.

#include "stdafx.h"
#include "SuckerPunchTest.h"
#include <iostream>


namespace
{
	//Total memory in data
	const int MAX_MEMORY = 2048;

	//Maximum number of Queues
	const int MAX_Q_NUM = 64;

	//the data array
	unsigned char data[MAX_MEMORY];

	//number of bytes per Q
	const int BYTES_PER_Q = 32;

	const int USABLE_BYTES_PER_Q = 28;

	const int Q_BYTES_OFFSET = 4;
}

Q * Create_Queue()
{
	Q* NewQ = FindUnallocatedQ();

	if (!NewQ)
	{
		//Could not allocate a new Q, we're out of memory
		on_out_of_memory();

		return nullptr;
	}

	return NewQ;
}

Q* FindUnallocatedQ()
{
	//Find an unallocated Q somewhere in data
	for (int i = 0; i < MAX_Q_NUM; ++i)
	{
		const int QIndex = i*BYTES_PER_Q;
		Q* FoundQ = reinterpret_cast<Q*>(data + QIndex);

		if (FoundQ && FoundQ->IsAllocated == 0)
		{
			FoundQ->IsAllocated = 1;
			FoundQ->Index = i;
			return FoundQ;
		}
	}

	return nullptr;
}

Q* GetNextQ(Q* q)
{
	if (q)
	{
		//First, destroy this Q's next Q Block, if it has one
		if (q->NextQIndex != 0)
		{
			Q* FoundQ = reinterpret_cast<Q*>(data + q->NextQIndex*32);

			//if this Q is allocated, destroy it as well
			if (FoundQ && FoundQ->IsAllocated)
			{
				return FoundQ;
			}
		}
	}

	return nullptr;
}

void Destroy_Queue(Q* q)
{
	if (!q || !q->IsAllocated)
	{
		on_illegal_operation();
		return;
	}

	//if we have a next queue, destroy that one first
	Q* NextQ = GetNextQ(q);

	if (NextQ)
	{
		Destroy_Queue(NextQ);
	}

	//Then destroy this Q block by zeroing it out
	memset(q, 0, BYTES_PER_Q);

	//set this Q to null
	q = nullptr;
}

void Enqueue_Byte(Q* q, unsigned char b)
{
	if (!q || !q->IsAllocated)
	{
		on_illegal_operation();
		return;
	}

	//If we haven't used all of the usable bytes of memory on this Q, just add the byte to the end of this Q
	if (q->MemorySize < USABLE_BYTES_PER_Q)
	{
		q->Memory[q->MemorySize] = b;
		q->MemorySize++;
	}
	else
	{
		//If we have a next Q, enqueue the byte onto that one
		Q* NextQ = GetNextQ(q);

		if (NextQ)
		{
			Enqueue_Byte(NextQ, b);
		}
		//We have not yet allocated a new block for more memory, so let's try to do that, and enqueue the byte on it
		else
		{
			//Find a new Q
			Q* NewNextQ = FindUnallocatedQ();

			if (NewNextQ)
			{
				//Enqueue the byte on that one, attach the new q to our old Q
				q->NextQIndex = NewNextQ->Index;
				Enqueue_Byte(NewNextQ, b);
			}
			else
			{
				//we failed to allocate a new Q, we have run out of memory
				on_out_of_memory();
			}
		}
	}
}

unsigned char Dequeue_Byte(Q * q)
{
	if (!q || !q->IsAllocated)
	{
		on_illegal_operation();
		return 0;
	}

	//Pop the byte into the return
	unsigned char ReturnChar = q->Memory[0];

	//move the other 27 bytes over to the left by 1
	memmove(q->Memory, q->Memory + 1, sizeof(unsigned char)*(USABLE_BYTES_PER_Q - 1));

	//Decrement memory size
	q->MemorySize--;

	q->Memory[q->MemorySize] = 0;

	Q* NextQ = GetNextQ(q);

	//if we have a next Q block
	if (NextQ)
	{
		//The last byte in this Q's memory should be set to the first one in the next Q
		//And do the memory shift on the next Q as well
		q->Memory[USABLE_BYTES_PER_Q - 1] = Dequeue_Byte(NextQ);
		
		//If the next Q now has an empty memory, destroy it
		if (NextQ->MemorySize == 0)
		{
			Destroy_Queue(NextQ);
			
			//We now no longer have a next Q, set the index to 0
			q->NextQIndex = 0;
		}
	}

	return ReturnChar;
}

void on_out_of_memory()
{

}

void on_illegal_operation()
{

}

void Print_Queue(Q* q)
{
	if (q == nullptr)
	{
		return;
	}

	for (int i = 0; i < q->MemorySize; ++i)
	{
		std::cout << q->Memory[i];
	}
}

int main()
{

	Q* NewQ = Create_Queue();

	for (int i = 0; i < 70; ++i)
	{
		Enqueue_Byte(NewQ, 1 + i);
	}

	Q* NewQ2 = Create_Queue();

	for (int i = 0; i < 70; ++i)
	{
		Enqueue_Byte(NewQ2, 1 + i);
	}
	
	Print_Queue(NewQ);

	std::cout << "\n";

	for (int i = 0; i < 70; ++i)
	{
		std::cout << Dequeue_Byte(NewQ);
	}

	Destroy_Queue(NewQ);

	system("pause");

    return 0;
}
