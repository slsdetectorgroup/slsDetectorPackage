
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>
#include <stdio.h>

#include "xparameters.h"

#include "Feb.h"


using namespace std;

int main(){

  cout<<"\n\n\n\n\n\n\n\n\n\n"<<endl;


  char s[2000];
  sprintf(s,"papamama");

  Feb* feb = new Feb();

    unsigned int v=22;
      feb->ReadRegister(0,0,v);
      feb->ReadRegister(0,0xffffffff,v);
      cout<<endl<<endl;
      feb->ReadRegister(1,0,v);
      feb->ReadRegister(1,0xffffffff,v);

  delete feb;

  return 1;
}
