// SuckerPunchTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;


namespace SuckerPunch
{
	const int MAX_MEMORY = 2048;

	const int MAX_QUEUE_NUM = 64;

	unsigned char data[MAX_MEMORY];

	const int BYTES_PER_BLOCK = 32;

	const int USABLE_BYTES_PER_BLOCK = 30;
}


struct Q
{
	//32 bytes per block
	unsigned char IsAllocated = 0;
	unsigned char TailIndex = 0;
	unsigned char Memory[30];
};

Q * Create_Queue()
{
	Q* CreatedQ = nullptr;
	//Look for a Memory block that hasn't been allocated yet
	for (int i = 0; i < SuckerPunch::MAX_QUEUE_NUM; i++)
	{
		Q* IsAQ = reinterpret_cast<Q*>(SuckerPunch::data + i*SuckerPunch::BYTES_PER_BLOCK);

		//if this block hasn't been allocated
		if (IsAQ && !IsAQ->IsAllocated)
		{
			CreatedQ = IsAQ;
			CreatedQ->IsAllocated = 1;
			return CreatedQ;
		}
	}

	return nullptr;
}

void Destroy_Queue(Q* q)
{
	memset(q, 0, sizeof(Q));
}

void Enqueue_Byte(Q* q, unsigned char b)
{
	//if the Q is not allocated, return
	if (!q || !q->IsAllocated)
	{
		return;
	}

	q->Memory[q->TailIndex] = b;
	q->TailIndex++;
}

unsigned char Dequeue_Byte(Q * q)
{
	//if the Q is not allocated, return
	if (!q || !q->IsAllocated)
	{
		return 0;
	}

	unsigned char PoppedChar = q->Memory[0];

	memmove(q->Memory, q->Memory+1, sizeof(unsigned char) * (q->TailIndex));

	q->TailIndex--;

	return PoppedChar;
}

void on_out_of_memory()
{

}

void on_illegal_operation()
{

}

void Print_Queue(Q* q)
{
	for (int i = 0; i < SuckerPunch::USABLE_BYTES_PER_BLOCK; ++i)
	{
		if (q->Memory[i])
		{
			printf("%c\n", q->Memory[i]);
		}
		else
		{
			break;
		}
	}
}

int main()
{
	printf("%d %d", sizeof(Q*), sizeof(unsigned char));

	Q* NewQ = Create_Queue();

	Enqueue_Byte(NewQ, 'B');
	Enqueue_Byte(NewQ, 'O');
	Enqueue_Byte(NewQ, 'O');
	Enqueue_Byte(NewQ, 'B');
	Enqueue_Byte(NewQ, 'S');


	Q* NewQu = Create_Queue();

	Enqueue_Byte(NewQu, 'F');
	Enqueue_Byte(NewQu, 'U');
	Enqueue_Byte(NewQu, 'C');
	Enqueue_Byte(NewQu, 'K');
	Enqueue_Byte(NewQu, ' ');
	Enqueue_Byte(NewQu, 'Y');
	Enqueue_Byte(NewQu, 'O');
	Enqueue_Byte(NewQu, 'U');

	unsigned char F = Dequeue_Byte(NewQu);
	unsigned char U = Dequeue_Byte(NewQu);
	unsigned char C = Dequeue_Byte(NewQu);
	unsigned char K = Dequeue_Byte(NewQu);

	Destroy_Queue(NewQu);

	printf("%c%c%c%c\n", F, U, C, K);

	char i;

	cin >> i;

	cout << "The value you entered is " << i;
    return 0;
}
