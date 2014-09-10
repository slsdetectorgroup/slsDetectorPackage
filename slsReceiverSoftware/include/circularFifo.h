/* CircularFifo.h 
* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work
* and I take no responsibility for this code and any problems you
* might get if using it. The code is highly platform dependent!
*
* Code & platform dependent issues with it was originally
* published at http://www.kjellkod.cc/threadsafecircularqueue
* 2009-11-02
* @author Kjell Hedstr�m, hedstrom@kjellkod.cc */

#ifndef CIRCULARFIFO_H_
#define CIRCULARFIFO_H_

//#include "sls_receiver_defs.h"
#include <semaphore.h>
#include <vector>
#include <iostream>
using namespace std;

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
	   sem_init(&free_mutex,0,0);
   }
   virtual ~CircularFifo() {
		sem_destroy(&free_mutex);
   }

   bool push(Element*& item_);
   bool pop(Element*& item_);

   bool isEmpty() const;
   bool isFull() const;

   int getSemValue();

private:
   volatile unsigned int tail; // input index
   vector <Element*> array;
   volatile unsigned int head; // output index
   unsigned int Capacity;
   sem_t free_mutex;

   unsigned int increment(unsigned int idx_) const;
};

template<typename Element>
int CircularFifo<Element>::getSemValue()
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
* \return whether operation was successful or not */
template<typename Element>
bool CircularFifo<Element>::push(Element*& item_)
{

   int nextTail = increment(tail);
   if(nextTail != head)
   {
      array[tail] = item_;
      tail = nextTail;
      sem_post(&free_mutex);
      return true;
   }

   // queue was full
   return false;
}

/** Consumer only: Removes and returns item from the queue
* If queue is empty at 'pop' operation no retrieve will happen
* It is up to the caller to handle this case
*
* \param item_ return by reference the wanted item
* \return whether operation was successful or not */
template<typename Element>
bool CircularFifo<Element>::pop(Element*& item_)
{
  // if(head == tail)
  //    return false;  // empty queue
  sem_wait(&free_mutex);

   item_ = array[head];
   head = increment(head);
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
   return (head == tail);
}

/** Useful for testing and Producer check of status
  * Remember that the 'full' status can change quickly
  * as the Consumer catches up.
  *
  * \return true if circular buffer is full.  */
template<typename Element>
bool CircularFifo<Element>::isFull() const
{
   int tailCheck = (tail+1) % Capacity;
   return (tailCheck == head);
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

#endif /* CIRCULARFIFO_H_ */
