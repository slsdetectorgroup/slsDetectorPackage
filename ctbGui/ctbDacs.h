

#ifndef CTBDACS_H
#define CTBDACS_H
#include  <TGFrame.h>


#define NDACS 18
//#define NDACS 16


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


class ctbDac : public TGHorizontalFrame {


 protected:
  // TGLabel *dacsLabel;
   TGNumberEntry *dacsEntry;
   TGCheckButton *dacsUnit;
   TGCheckButton *dacsLabel;
   TGLabel *dacsValue;
   int id;

   sls::Detector* myDet;
 public:
   ctbDac(TGGroupFrame*, int , sls::Detector*);
   void setValue();
   void setValue(Long_t);
   int getValue();
   void setOn(Bool_t);
   
   int setLabel(char *tit, int mv);
   string getLabel();
int getMoenchDacId();


   ClassDef(ctbDac,0)
};

class ctbDacs : public TGGroupFrame {
private:
  
   ctbDac *dacs[NDACS+2];

   sls::Detector* myDet;

public:
   ctbDacs(TGVerticalFrame *page, sls::Detector*);

   int setDacAlias(string line);
   // int setDacAlias(string line);
   string getDacAlias();
   string getDacParameters();

   void update();

   ClassDef(ctbDacs,0)
};

#endif

