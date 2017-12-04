//#include "ThreadPool.h"

//#include "threadpool.h"

#include <iostream>
#include <string.h>

#include "Multi.h"
using namespace std;

//const int MAX_TASKS = 4;

/*
void hello(void* arg)
{
	string* x = (string*) arg;
  cout << "Hello: " << *x << endl;
//  cout << "\n";
}
*/
int main(int argc, char* argv[])
{

	Multi* m = new Multi();
	cout<<"Answer:"<< m->executeCommand(argc,argv) << endl;
	delete m;
	/*
  ThreadPool tp(2);
  int ret = tp.initialize_threadpool();
  if (ret == -1) {
    cerr << "Failed to initialize thread pool!" << endl;
    return 0;
  }
*/


/*
  for (int i = 0; i < MAX_TASKS; i++) {
	  cout<<"adding task:" <<argv[1]<<":"<< i<< endl;

	   string *x;
	  if(!strcmp(argv[1],"print"))
		  x = new string(argv[2]);
	  else x = new string("foo");

    Task* t = new Task(&hello, (void*) x);
//    cout << "Adding to pool, task " << i+1 << endl;
    tp.add_task(t);
//    cout << "Added to pool, task " << i+1 << endl;
  }

  sleep(2);

  tp.destroy_threadpool();
*/
  // TODO: delete worker objects

  cout << "Exiting app..." << endl;

  return 0;
}
