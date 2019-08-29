#ifndef CTBPOWERS_H
#define CTBPOWERS_H

#include  <TGFrame.h>

#define NPOWERS 6



class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;




namespace sls
{
   class Detector;
};

#include <string>
using namespace std;


class ctbPower : public ctbDac {

  


 public:

  ctbPower(TGGroupFrame* f, int i, sls::Detector* d);

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

   sls::Detector* myDet; 

public:
   //ctbPowers();
    ctbPowers(TGVerticalFrame*, sls::Detector*);

    int setPwrAlias(string); 
    string getPwrAlias(); 
    string getPwrParameters(); 

    void update(); 

    ClassDef(ctbPowers,0)
};

#endif
