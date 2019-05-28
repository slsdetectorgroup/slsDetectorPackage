#ifndef CTBPOWERS_H
#define CTBPOWERS_H

#include  <TGFrame.h>

#ifndef CTB
#define NPOWERS 0
#else

#define NPOWERS 6
#endif





class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;




class multiSlsDetector;


#include <string>
using namespace std;


class ctbPower : public ctbDac {

  


 public:

  ctbPower(TGGroupFrame* f, int i, multiSlsDetector* d);

    string getLabel();

    int getValue();
    void setValue();
    void setValue(Long_t);

   ClassDef(ctbPower,0)
};


class ctbPowers : public TGGroupFrame 
{
 private:
  
   ctbPower *dacs[NPOWERS]; 

   multiSlsDetector* myDet; 

public:
   //ctbPowers();
    ctbPowers(TGVerticalFrame*, multiSlsDetector*);

    int setPwrAlias(string); 
    string getPwrAlias(); 
    string getPwrParameters(); 

    void update(); 

    ClassDef(ctbPowers,0)
};

#endif
