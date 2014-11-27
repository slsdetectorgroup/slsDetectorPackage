#ifndef JCTBADCS_H
#define JCTBADCS_H
#include  <TGFrame.h>


#define NADCS 32

class TRootEmbeddedCanvas;
class TGButtonGroup;
class TGVerticalFrame;
class TGHorizontalFrame;
class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TH2F;
class TGComboBox;
class TGCheckButton;
class TGColorSelect;

class   THStack;
class   TGraphErrors;
class  TGTextButton;
class  TGTab;

class TGraph;

class multiSlsDetector;

#include <string>
using namespace std;

class jctbAdc : public TGHorizontalFrame {


 private:
  

   TGLabel *sAdcLabel;
   TGCheckButton *sAdcEnable;
   TGCheckButton *sAdcPlot;

   TGColorSelect  *fColorSel;

   TGraph *gADC;

   int id;
   multiSlsDetector *myDet;
   
 public:
   jctbAdc(TGVerticalFrame *page, int i, multiSlsDetector *det);


   void setAdcAlias(char *tit, int plot, int color);
   string getAdcAlias();
   void ToggledAdcPlot(Int_t b);
   void ToggledPlot(Bool_t b);
   
   void update();

   ClassDef(jctbAdc,0)
     };



class jctbAdcs : public TGGroupFrame {
private:
  
  jctbAdc *sAdc[NADCS];
  
   multiSlsDetector *myDet;
   
public:

   jctbAdcs(TGVerticalFrame *page, multiSlsDetector *det);
   int setAdcAlias(string line);
   string getAdcAlias();

   void update();

   ClassDef(jctbAdcs,0)
};

#endif
