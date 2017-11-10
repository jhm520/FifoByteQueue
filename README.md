# SuckerPunchTest


This project was written as a response to pre-interview programming challenge from Sucker Punch Productions.

I tried to make my solution a blend of readability and efficiency. The key parameter of the test was that there would be no more than 64 Queues allocated. So 2048 bytes/64 = 32 bytes per Queue. So I daisy chained the queues together (in the event that we need more than 32 bytes in a queue) to somewhat resemble a linked list, although instead of storing a pointer to another Queue (4 bytes), I just had it store an integer index between 0 and 63 (1 byte), which when multiplied by 32, would give us the index of the Queue's next Queue in the 2048 byte data array. The linked list style allowed me to use recursion when enqueuing, dequeuing, and destroying queues.