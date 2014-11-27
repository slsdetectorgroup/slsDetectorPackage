#ifndef JCTBDACS_H
#define JCTBDACS_H
#include  <TGFrame.h>


#define NDACS 16





class multiSlsDetector;
class jctbDac;

#include <string>
using namespace std;


class jctbDacs : public TGGroupFrame {
private:
  


   jctbDac *dacs[NDACS];

   multiSlsDetector* myDet;

public:
   jctbDacs(TGVerticalFrame *page, multiSlsDetector*);

   int setDacAlias(string line);
   string getDacAlias();
   string getDacParameters();

   void update();

   ClassDef(jctbDacs,0)
};

#endif
