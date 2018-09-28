#pragma once
/* CircularFifo.h 
* Code & platform dependent issues with it was originally
* published at http://www.kjellkod.cc/threadsafecircularqueue
* 2009-11-02
* @author Kjell Hedstr�m, hedstrom@kjellkod.cc
* modified by the sls detetor group
* */

//#include "sls_receiver_defs.h"
#include <semaphore.h>
#include <vector>
#include <iostream>

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;



/** Circular Fifo (a.k.a. Circular Buffer)
* Thread safe for one reader, and one writer */
template<typename Element>
class CircularFifo {
public:

   CircularFifo(unsigned int Size) : tail(0), head(0){
	   Capacity = Size + 1;
	   array.resize(Capacity);
	   sem_init(&data_mutex,0,0);
	   sem_init(&free_mutex,0,Size);
   }
   virtual ~CircularFifo() {
       sem_destroy(&data_mutex);
	   sem_destroy(&free_mutex);
   }

   bool push(Element*& item_, bool no_block=false);
   bool pop(Element*& item_, bool no_block=false);

   bool isEmpty() const;
   bool isFull() const;

   int getDataValue() const;
   int getFreeValue() const;

private:
   std::vector <Element*> array;
   unsigned int tail; // input index
   unsigned int head; // output index
   unsigned int Capacity;
   mutable sem_t data_mutex;
   mutable sem_t free_mutex;
   unsigned int increment(unsigned int idx_) const;
};

template<typename Element>
int CircularFifo<Element>::getDataValue() const
{
   int value;
   sem_getvalue(&data_mutex, &value);
   return value;
}

template<typename Element>
int CircularFifo<Element>::getFreeValue() const
{
    int value;
    sem_getvalue(&free_mutex, &value);
    return value;
}


/** Producer only: Adds item to the circular queue.
* If queue is full at 'push' operation no update/overwrite
* will happen, it is up to the caller to handle this case
*
* \param item_ copy by reference the input item
* \param no_block if true, return immediately if fifo is full
* \return whether operation was successful or not */
template<typename Element>
bool CircularFifo<Element>::push(Element*& item_, bool no_block)
{
    // check for fifo full
    if (no_block && isFull())
        return false;

    sem_wait(&free_mutex);
    array[tail] = item_;
    tail = increment(tail);
    sem_post(&data_mutex);
    return true;
}

/** Consumer only: Removes and returns item from the queue
* If queue is empty at 'pop' operation no retrieve will happen
* It is up to the caller to handle this case
*
* \param item_ return by reference the wanted item
* \param no_block if true, return immediately if fifo is full
* \return whether operation was successful or not */
template<typename Element>
bool CircularFifo<Element>::pop(Element*& item_, bool no_block)
{
    // check for fifo empty
    if (no_block && isEmpty())
        return false;

   sem_wait(&data_mutex);
   item_ = array[head];
   head = increment(head);
   sem_post(&free_mutex);
   return true;
}

/** Useful for testinng and Consumer check of status
  * Remember that the 'empty' status can change quickly
  * as the Procuder adds more items.
  *
  * \return true if circular buffer is empty */
template<typename Element>
bool CircularFifo<Element>::isEmpty() const
{
    return (getDataValue() == 0);
}

/** Useful for testing and Producer check of status
  * Remember that the 'full' status can change quickly
  * as the Consumer catches up.
  *
  * \return true if circular buffer is full.  */
template<typename Element>
bool CircularFifo<Element>::isFull() const
{
    return (getFreeValue() == 0);
}

/** Increment helper function for index of the circular queue
* index is inremented or wrapped
*
*  \param idx_ the index to the incremented/wrapped
*  \return new value for the index */
template<typename Element>
unsigned int CircularFifo<Element>::increment(unsigned int idx_) const
{
   // increment or wrap
   // =================
   //    index++;
   //    if(index == array.lenght) -> index = 0;
   //
   //or as written below:
   //    index = (index+1) % array.length
   idx_ = (idx_+1) % Capacity;
   return idx_;
}

