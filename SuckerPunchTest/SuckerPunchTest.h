#pragma once

struct Q;

// Creates a FIFO byte queue, returning a handle to it.
Q * Create_Queue();

// Destroy an earlier created byte queue.
void Destroy_Queue(Q * q);

// Adds a new byte to a queue.
void Enqueue_Byte(Q * q, unsigned char b);

// Pops the next byte off the FIFO queue
unsigned char Dequeue_Byte(Q * q);

void on_out_of_memory();

void on_illegal_operation();