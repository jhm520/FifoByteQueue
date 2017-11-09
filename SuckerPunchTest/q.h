/*!
Public interface for Sucker Punch Programming Problem.
This is a variable number of variable-sized byte queues
operating in a memory-constrained environment.

@author Sam Paul
*/

typedef unsigned int Bit;
struct Q
{
	// At least with the VC2008 compiler, on an x86 machine, this struct
	// gets laid out in memory exactly as I'd like it to. This way I get
	// compact memory usage, clearly defined structure, and easy-to-read
	// access syntax later on without any masking and shifting crap.
	Bit inUse : 1;  // 1 if queue is alloc'd, 0 if not
	Bit headBlock : 7;  // handle to storage block containing deque head
	Bit headOffset : 4;  // offset of deque head within storage block
	Bit tailBlock : 7;  // handle to storage block containing deque tail
	Bit tailOffset : 4;  // offset of deque tail within storage block
	Bit budBlock : 7;  // storage list grows by linking after here,
					   //   needed for const time
	Bit unused : 2;
};

// Must be called before any of the other queue functions
void init_queue_storage();

// Creates a FIFO byte queue, returning a handle to it.
Q * create_queue();

// Destroy an earlier created byte queue.
void destroy_queue(Q * q);

// Adds a new byte to a queue.
void enqueue_byte(Q * q, unsigned char b);

// Pops the next byte off the FIFO queue.
unsigned char dequeue_byte(Q * q);

// These must be defined elsewhere, as Q.cpp does not define them
void on_out_of_memory();
void on_illegal_operation();

void PrintStatusExtern(Q* q);