
/**
 * @author Ian Johnson
 * @version 1.0
 */

#ifndef SLSQTNUMBERENTRY_H
#define SLSQTNUMBERENTRY_H

#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif


#include <qwidget.h>
#include <qgroupbox.h>

class QGridLayout;

class QLabel;
class QLineEdit;
class QIntValidator;
class QDoubleValidator;
class QCheckBox;
class QComboBox;
class QSpinBox;

#include "SlsQtValidators.h"

class SlsQtNumberEntry:public QWidget{
  Q_OBJECT
     
 public:
  //type=0->units only, type=1->int,type=2->double, type=3->spinbox
  //adding middle text will automatically add a second number field
  SlsQtNumberEntry(QWidget *parent,int with_checkbox=0, char *start_string=0, int num_type=0, char *middle_string=0, int num2_type=0, int n_units=0, char** units=0, double* unit_factors=0, char* end_string=0);
  //without unit box
  SlsQtNumberEntry(QWidget *parent,int with_checkbox, char *start_strin, int num_type, char *middle_string, int num2_type, char* end_string);


  ~SlsQtNumberEntry();

  void Enable(bool en_flag=1);
  void Disable();
  void EnableNumberField(int which_number_field,bool en_flag=1); //which_number_field is 0 or 1
  void DisableNumberField(int which_number_field);

  void AddCheckBox();
  void SetFrontText(char* s);
  void SetMiddleText(char* s);
  void SetBackText(char* s);
  void SetupNumberField(int type,int which_number_field=0);
  void SetUnits(int n_units,char** units,double* unit_factors);

  void SetMinimumNumberWidth(int nchar_width,int which_number_field=0);
  void SetNDecimalsOfDoubleValidators(int ndecimals);
  void SetNDecimalsOfDoubleValidator(int ndecimals,int which_number_field=0);
  void SetMinimumUnitWidth(int nchar_width);
  
  bool   Enabled();  
  bool   CheckBoxState();
  bool   IsValueOk(int which_number_field=0);

  const char* GetFrontText();
  const char* GetMiddleText();
  const char* GetBackText();

  int    GetNumberInt(int which_number_field=0,bool* ok=0);
  double GetNumber(int which_number_field=0,bool *ok=0);

  int    GetValueInt(int which_number_field=0,bool* ok=0);
  double GetValue(int which_number_field=0,bool *ok=0);

  int     GetComboBoxIndex();
  double  GetComboBoxValue();

 private:
  void SetupNumberEntry(int with_checkbox=0, char *start_string=0, int num_type=0, char *middle_string=0, int num2_type=0, int n_units=0, char** units=0, double* unit_factors=0, char* end_string=0);

  QGridLayout*       layout;  //default layout

  QCheckBox*          check_box;
  QLabel*             front_text;
  QLabel*             middle_text;
  QLineEdit*          num_field[2];
  QSpinBox*           spin_box[2];
  bool                num_field_enabled[2];

  SlsQtIntValidator*     validator_int[2];
  SlsQtDoubleValidator*  validator_double[2];
  QComboBox*          unit_cbb;
  double*             factors;
  QLabel*             back_text;

  void SetText(char* s, QLabel** pp);
  void SetLayout();
  

  public slots:
    void FirstValueEntered();
    void SecondValueEntered();
    void UnitSelected();

    void SetRange(int min, int max, int which_number_field=0);
    void SetRange(double min, double max, int which_number_field=0);
    void SetFirstRange(int min, int max)  {SetRange(min,max,0);}
    void SetSecondRange(int min, int max) {SetRange(min,max,1);}

    double SetValue(double v, int which_number_field=0);
    void SetFirstValue(int v)     {SetValue(v,0);}
    void SetSecondValue(int v)    {SetValue(v,1);}
    void SetFirstValue(double v)  {SetValue(v,0);}
    void SetSecondValue(double v) {SetValue(v,1);}

    //  double SetNumber(int v, int which_number_field=0);
    double SetNumber(double v, int which_number_field=0);
    void SetFirstNumber(int v)     {SetNumber(v,0);}
    void SetSecondNumber(int v)    {SetNumber(v,1);}
    void SetFirstNumber(double v)  {SetNumber(v,0);}
    void SetSecondNumber(double v) {SetNumber(v,1);}

    int  SetComboBoxIndex(int index);

    void CheckBoxClicked();
    void PrintTheValue();

    void RefreshFirstNumberEntry();
    void RefreshSecondNumberEntry();
    void RefreshNumberEntery(int number_field=0);

  signals:
    void CheckBoxChanged(bool state);
    void CheckBoxChanged(SlsQtNumberEntry* ptr);
    void AValueChanged(SlsQtNumberEntry* ptr);
    void FirstValueChanged(int value);
    void FirstValueChanged(double value);
    void FirstValueChanged(SlsQtNumberEntry* ptr);
    void SecondValueChanged(int value);
    void SecondValueChanged(double value);
    void SecondValueChanged(SlsQtNumberEntry* ptr);
    void UnitChanged(double);
    void UnitChanged(SlsQtNumberEntry* ptr);
};

#endif
