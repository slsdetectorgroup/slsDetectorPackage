#include "mythenDetector.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>



using namespace std;



mythenDetector::mythenDetector(int id): slsDetector(MYTHEN,id)
 {
   ;
}

