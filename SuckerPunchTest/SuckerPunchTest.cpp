// SuckerPunchTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SuckerPunchTest.h"
#include <iostream>


namespace
{
	const int MAX_MEMORY = 2048;

	const int MAX_QUEUE_NUM = 64;

	unsigned char data[MAX_MEMORY];

	const int BYTES_PER_BLOCK = 32;

	const int USABLE_BYTES_PER_BLOCK = 28;

	const int Q_BYTES_OFFSET = 4;
}

Q * Create_Queue()
{
	return FindUnallocatedQ();
}

Q* FindUnallocatedQ()
{
	//Find an unallocated Q
	for (int i = 0; i < MAX_QUEUE_NUM; ++i)
	{
		const int QPos = i*BYTES_PER_BLOCK;
		Q* FoundQ = reinterpret_cast<Q*>(data + QPos);

		if (FoundQ && FoundQ->IsAllocated == 0)
		{
			FoundQ->IsAllocated = 1;
			FoundQ->Index = QPos;
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
			Q* FoundQ = reinterpret_cast<Q*>(data + q->NextQIndex);

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
		return;
	}

	Q* NextQ = GetNextQ(q);

	if (NextQ)
	{
		Destroy_Queue(NextQ);
	}

	//Then destroy this Q block
	memset(q, 0, BYTES_PER_BLOCK);

	//set this Q to null
	q = nullptr;
}

void Enqueue_Byte(Q* q, unsigned char b)
{
	if (!q || !q->IsAllocated)
	{
		return;
	}

	//If we haven't used all of the usable bytes of memory on this Q, just add the byte to the end of this Q
	if (q->MemorySize < USABLE_BYTES_PER_BLOCK)
	{
		q->Memory[q->MemorySize] = b;
		q->MemorySize++;
	}
	else
	{
		//See if we have a next Q, enqueue the byte to that one if we do
		Q* NextQ = GetNextQ(q);

		if (NextQ)
		{
			Enqueue_Byte(NextQ, b);
		}
		//We have not yet allocated a new block for more memory, so let's do that, and enqueue the byte on it
		else
		{
			//Find a new block
			Q* NewNextQ = FindUnallocatedQ();

			if (NewNextQ)
			{
				//Enqueue the byte on that one
				q->NextQIndex = NewNextQ->Index;
				Enqueue_Byte(NewNextQ, b);
			}
		}
	}
}

unsigned char Dequeue_Byte(Q * q)
{
	if (!q || !q->IsAllocated)
	{
		return '\0';
	}

	//Pop the byte into the return
	unsigned char ReturnChar = '\0';

	ReturnChar = q->Memory[0];

	//move the other 27 bytes over to the left by 1
	memmove(q->Memory, q->Memory + 1, sizeof(unsigned char)*(USABLE_BYTES_PER_BLOCK - 1));

	//Decrement memory size
	q->MemorySize--;

	Q* NextQ = GetNextQ(q);

	//if we have a next Q block
	if (NextQ)
	{
		//The last byte in this Q's memory should be set to the first one in the next Q
		//And do the memory shift on the next Q as well
		q->Memory[USABLE_BYTES_PER_BLOCK - 1] = Dequeue_Byte(NextQ);
		
		//If the next Q now has an empty memory, destroy it
		if (NextQ->MemorySize)
		{
			Destroy_Queue(NextQ);
			
			//We now no longer have a next Q
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

	//std::printf((char*)q->Memory);

	for (int i = 0; i < q->MemorySize; ++i)
	{
		std::cout << q->Memory[i];
		
		//printf("%c\n", q->Memory[i]);
	}
}

int main()
{

	Q* NewQ = Create_Queue();


	for (int i = 0; i < 70; ++i)
	{
		Enqueue_Byte(NewQ, 1 + i);
	}
	

	Print_Queue(NewQ);

	std::cout << "\n";

	for (int i = 0; i < 64; ++i)
	{
		std::cout << data[i];
	}

	Destroy_Queue(NewQ);

	system("pause");

    return 0;
}
