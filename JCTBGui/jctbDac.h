#ifndef JCTBDAC_H
#define JCTBDAC_H
#include  <TGFrame.h>



class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;


class multiSlsDetector;

#include <string>
using namespace std;

class jctbDac : public TGHorizontalFrame {


 private:
   TGLabel *dacsLabel;
   TGNumberEntry *dacsEntry;
   TGCheckButton *dacsUnit;
   TGLabel *dacsValue;
   int id;

   multiSlsDetector* myDet;
 public:
   jctbDac(TGGroupFrame*, int , multiSlsDetector*);
   void setValue();
   int getValue();
  
   
   int setLabel(char *tit, int mv);
   string getLabel();



   ClassDef(jctbDac,0)
};


#endif
