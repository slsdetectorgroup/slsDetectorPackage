

#ifndef CTBSLOWADCS_H
#define CTBSLOWADCS_H
#include  <TGFrame.h>


//#define NDACS 16
#define NSLOWADCS 8




class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;
class TGTextButton;



namespace sls
{
   class Detector;
};

#include <string>
using namespace std;

class ctbSlowAdc : public TGHorizontalFrame {


 protected:
  // TGLabel *dacsLabel;
  // TGNumberEntry *dacsEntry;
  // TGCheckButton *dacsUnit;
   TGLabel *dacsLabel;
   TGLabel *dacsValue;
   int id;

   sls::Detector* myDet;
 public:
   ctbSlowAdc(TGGroupFrame*, int , sls::Detector*);
   int getValue();
   
   int setLabel(char *tit);
   string getLabel();



   ClassDef(ctbSlowAdc,0)
};


class ctbSlowAdcs : public TGGroupFrame {
private:
  


   ctbSlowAdc *adcs[NSLOWADCS+1];

   sls::Detector* myDet;

public:
   ctbSlowAdcs(TGVerticalFrame *page, sls::Detector*);

   int setSlowAdcAlias(string line);
   // int setDacAlias(string line);
   string getSlowAdcAlias();
   string getAdcParameters();

   void update();

   ClassDef(ctbSlowAdcs,0)
};

#endif

