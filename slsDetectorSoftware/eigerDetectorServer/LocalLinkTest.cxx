
#include <stdio.h>

#include "xparameters.h"

#include "LocalLinkInterface.h"

int main(){

  char s[2000];
  sprintf(s,"papamama");

  LocalLinkInterface* l0 = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_LEFT_BASEADDR);
    l0->Test(8,s);
  LocalLinkInterface* l1 = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_RX4_TX1_LEFT_BASEADDR);
    l1->Test(8,s);
  LocalLinkInterface* l2 = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_RIGHT_BASEADDR);
    l2->Test(8,s);
  LocalLinkInterface* l3 = new LocalLinkInterface(XPAR_PLB_LL_FIFO_AURORA_RX4_TX1_RIGHT_BASEADDR);
    l3->Test(8,s);
    
  delete l0;
  delete l1;
  delete l2;
  delete l3;

  return 1;
}
