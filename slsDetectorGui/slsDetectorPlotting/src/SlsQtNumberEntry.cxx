
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>
#include <math.h>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

#include <qlayout.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qgroupbox.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include <qvalidator.h>

#include "SlsQtNumberEntry.h"

using namespace std;

SlsQtNumberEntry::SlsQtNumberEntry(QWidget *parent, int with_checkbox, char *start_string, int num_type, char* middle_string, int num2_type, int n_units, char** units, double* unit_factors,char* end_string):QWidget(parent){
  SetupNumberEntry(with_checkbox,start_string,num_type,middle_string,num2_type,n_units,units,unit_factors,end_string);
}

//without unit drop box
SlsQtNumberEntry::SlsQtNumberEntry(QWidget *parent,int with_checkbox, char *start_string, int num_type, char* middle_string, int num2_type,char* end_string):QWidget(parent){
  SetupNumberEntry(with_checkbox,start_string,num_type,middle_string,num2_type,0,0,0,end_string);
}

void SlsQtNumberEntry::SetupNumberEntry(int with_checkbox, char *start_string, int num_type, char* middle_string, int num2_type, int n_units, char** units, double* unit_factors,char* end_string){

  layout = 0;

  check_box  = 0;
  front_text = 0;
  num_field[0] = 0;
  spin_box[0] = 0;
  validator_int[0] = 0;
  validator_double[0] = 0;
  middle_text = 0;
  num_field[1] = 0;
  spin_box[1] = 0;
  validator_int[1] = 0;
  validator_double[1] = 0;
  unit_cbb = 0;
  factors = 0;
  back_text  = 0;
  if(with_checkbox) AddCheckBox();
  SetFrontText(start_string);

  SetupNumberField(num_type,0);
  SetMiddleText(middle_string);
  SetupNumberField(num2_type,1);
  SetUnits(n_units,units,unit_factors);
  SetBackText(end_string);

  SetLayout();

}


SlsQtNumberEntry::~SlsQtNumberEntry(){

  if(check_box)           delete check_box;
  if(front_text)          delete front_text;
  if(middle_text)         delete middle_text;
  if(validator_int[0])    delete validator_int[0];
  if(validator_double[0]) delete validator_double[0];
  if(num_field[0])        delete num_field[0];
  if(spin_box[0])         delete spin_box[0];
  if(validator_int[1])    delete validator_int[1];
  if(validator_double[1]) delete validator_double[1];
  if(num_field[1])        delete num_field[1];
  if(spin_box[1])         delete spin_box[1];
  if(unit_cbb)            delete unit_cbb;
  if(factors)             delete factors;
  if(back_text)           delete back_text;

}

void SlsQtNumberEntry::AddCheckBox(){
  if(check_box) delete check_box;
  check_box = new QCheckBox(this);
  connect(check_box,SIGNAL(clicked()),this,SLOT(CheckBoxClicked()));  
  SetLayout();
}

void SlsQtNumberEntry::SetFrontText(char* s)   {SetText(s,&front_text);  SetLayout();}
void SlsQtNumberEntry::SetMiddleText(char* s)  {SetText(s,&middle_text); SetLayout();}
void SlsQtNumberEntry::SetBackText(char* s)    {SetText(s,&back_text);   SetLayout();};
void SlsQtNumberEntry::SetText(char* s, QLabel** pp){
  if(*pp){delete *pp; *pp=0;}
  if(s){
    *pp = new QLabel(this);
    (*pp)->setText(s);
  }
  SetLayout();
}


void SlsQtNumberEntry::SetupNumberField(int type, int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  num_field_enabled[i]=1;
  if(spin_box[i])         { delete spin_box[i];         spin_box[i]=0;}
  if(validator_int[i])    { delete validator_int[i];    validator_int[i]=0;}
  if(validator_double[i]) { delete validator_double[i]; validator_double[i]=0;}
  if(num_field[i])        { delete num_field[i];        num_field[i]=0;}

  if(type>0&&type<3){
    num_field[i] = new QLineEdit(this);
    num_field[i]->setAlignment(Qt::AlignRight);
    SetMinimumNumberWidth(3,i);
    if(type==1){
      validator_int[i] = new SlsQtIntValidator(num_field[i]);
      num_field[i]->setValidator(validator_int[i]);  
      SetNumber(0,i);
    }else{
      validator_double[i] = new SlsQtDoubleValidator(num_field[i]);
      num_field[i]->setValidator(validator_double[i]); 
      //default settings
      SetNDecimalsOfDoubleValidator(3,i);  //defalut value	
      SetNumber(0,i);
    }

    num_field[i]->setAlignment(Qt::AlignRight);
    
    if(i==0){
      connect(num_field[i],SIGNAL(lostFocus()),this,SLOT(RefreshFirstNumberEntry()));
      connect(num_field[i],SIGNAL(returnPressed()),this,SLOT(FirstValueEntered()));
      connect(num_field[i],SIGNAL(lostFocus()),this,SLOT(FirstValueEntered()));      
    }else{
      connect(num_field[i],SIGNAL(lostFocus()),this,SLOT(RefreshSecondNumberEntry()));
      connect(num_field[i],SIGNAL(returnPressed()),this,SLOT(SecondValueEntered()));
      connect(num_field[i],SIGNAL(lostFocus()),this,SLOT(SecondValueEntered()));
    }
  }else if(type==3){
    spin_box[i] = new QSpinBox();
    if(i==0) connect(spin_box[i],SIGNAL(editingFinished()),this,SLOT(FirstValueEntered()));
    else     connect(spin_box[i],SIGNAL(editingFinished()),this,SLOT(SecondValueEntered()));
    spin_box[i]->setAlignment(Qt::AlignRight);
  }

  SetLayout();
}

void SlsQtNumberEntry::SetUnits(int n_units, char** units, double* unit_factors){
  if(unit_cbb){ delete unit_cbb; unit_cbb=0;}
  if(factors) { delete factors; factors=0;}

  if(n_units>0&&units&&unit_factors){
    unit_cbb = new QComboBox(this);
    factors  = new double [n_units];

    for(int i=0;i<n_units;i++){
      unit_cbb->insertItem(i,units[i]);
      factors[i] = unit_factors[i];
    }

    connect(unit_cbb,SIGNAL(activated(int)),this,SLOT(UnitSelected()));
  }

  SetLayout();
}

void SlsQtNumberEntry::SetLayout(){
  if(layout) delete layout;
  layout =  new QGridLayout(this);

  int i = 0;
  if(check_box)    layout->addWidget(check_box,1,i++);
  if(front_text)   layout->addWidget(front_text,1,i++);
  if(num_field[0]) layout->addWidget(num_field[0],1,i++);
  if(spin_box[0])  layout->addWidget(spin_box[0],1,i++);
  if(middle_text)  layout->addWidget(middle_text,1,i++);
  if(num_field[1]) layout->addWidget(num_field[1],1,i++);
  if(spin_box[1])  layout->addWidget(spin_box[1],1,i++);
  if(unit_cbb)     layout->addWidget(unit_cbb,1,i++);
  if(back_text)    layout->addWidget(back_text,1,i++);

  CheckBoxClicked();
}


void SlsQtNumberEntry::SetMinimumNumberWidth(int nchar_width,int which_number_field){
  if(num_field[which_number_field]) num_field[which_number_field]
				      ->setMinimumWidth(nchar_width*num_field[which_number_field]->minimumSizeHint().width());
  if(spin_box[which_number_field])  spin_box[which_number_field]
				      ->setMinimumWidth(nchar_width*spin_box[which_number_field]->minimumSizeHint().width());
}

void SlsQtNumberEntry::SetNDecimalsOfDoubleValidators(int ndecimals){
  SetNDecimalsOfDoubleValidator(ndecimals,0);
  SetNDecimalsOfDoubleValidator(ndecimals,1);
}

void SlsQtNumberEntry::SetNDecimalsOfDoubleValidator(int ndecimals, int which_number_field){
  //0 -> standard, 1->scientific
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;
  if(validator_double[i]){
    validator_double[i]->setDecimals(ndecimals);
    SetNumber(GetNumber(i),i);
  }
}

void SlsQtNumberEntry::SetMinimumUnitWidth(int nchar_width){
  if(unit_cbb) unit_cbb->setMinimumWidth(nchar_width*unit_cbb->minimumSizeHint().width());
}


/*
double SlsQtNumberEntry::SetNumber(int v,int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  if(num_field[i]){
    if(validator_int[i]){
      QString s = QString::number(v);
      validator_int[i]->fixup(s);            
      num_field[i]->setText(s);
    }
    if(validator_double[i]){
      QString s = QString::number(v);
      validator_double[i]->fixup(s);            
      num_field[i]->setText(s);
    }
  }else if(spin_box[i]){
    spin_box[i]->setValue(v);
  }else return 0;

  return GetNumber(i);
}
*/

double SlsQtNumberEntry::SetNumber(double v,int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  if(num_field[i]){
    if(validator_int[i]){
      QString s = QString::number(v);
      validator_int[i]->fixup(s);            
      num_field[i]->setText(s);
    }
    if(validator_double[i]){
      QString s = QString::number(v);
      validator_double[i]->fixup(s);
      num_field[i]->setText(s);
    }
  }else if(spin_box[i]){
    spin_box[i]->setValue(round(v));
  }else return 0;

  return GetNumber(i);
}


void SlsQtNumberEntry::SetRange(int min, int max,int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;
  if(min>max){
    cout<<"Warning: SetRange(int,int) no effect min > max"<<endl;
  }else{
    if(validator_int[i])    validator_int[i]->setRange(min,max);
    if(validator_double[i]) validator_double[i]->setRange(min,max,validator_double[i]->decimals());
    if(spin_box[i])         spin_box[i]->setRange(min,max);
    SetNumber(GetNumber(i),i);
  }
}


void SlsQtNumberEntry::SetRange(double min, double max,int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;
  if(min>max){
    cout<<"Warning: SetRange(double,double) no effect min >= max"<<endl;
  }else{
    if(validator_int[i])    cout<<"Waring can not call SetRange(double,double) with \"int\" type Validator"<<endl;
    if(validator_double[i]) validator_double[i]->setRange(min,max,validator_double[i]->decimals());
    if(spin_box[i])         spin_box[i]->setRange(min,max);
    SetNumber(GetNumber(i),i);
  }
  
}

bool SlsQtNumberEntry::CheckBoxState(){
  if(check_box&&check_box->checkState()) return 1;
  return 0;
}

void SlsQtNumberEntry::CheckBoxClicked(){
  if(check_box){
    if(check_box->checkState()) Enable();
    else                        Disable();
    
    emit CheckBoxChanged(check_box->checkState());
    emit CheckBoxChanged(this);
  }
  
}

void SlsQtNumberEntry::Disable(){ Enable(0); }
void SlsQtNumberEntry::Enable(bool en_flag){
  if(check_box)     check_box->setChecked(en_flag);
  if(front_text)    front_text->setEnabled(en_flag);
  if(num_field[0])  num_field[0]->setEnabled(en_flag&&num_field_enabled[0]);
  if(spin_box[0])   spin_box[0]->setEnabled(en_flag&&num_field_enabled[0]);
  if(middle_text)   middle_text->setEnabled(en_flag);
  if(num_field[1])  num_field[1]->setEnabled(en_flag&&num_field_enabled[1]);
  if(spin_box[1])   spin_box[1]->setEnabled(en_flag&&num_field_enabled[1]);
  if(unit_cbb)      unit_cbb->setEnabled(en_flag);
  if(back_text)     back_text->setEnabled(en_flag);
}

void SlsQtNumberEntry::DisableNumberField(int which_number_field){ EnableNumberField(which_number_field,0); }
void SlsQtNumberEntry::EnableNumberField(int which_number_field,bool en_flag){
  if(which_number_field>=0||which_number_field<=1){
    num_field_enabled[which_number_field]=en_flag;
    if(num_field[which_number_field])  num_field[which_number_field]->setEnabled(num_field_enabled[which_number_field]);
    if(spin_box[which_number_field])   spin_box[which_number_field]->setEnabled(num_field_enabled[which_number_field]);
  }
}
    
void SlsQtNumberEntry::UnitSelected(){ 
  emit  UnitChanged(GetComboBoxValue());
  emit  UnitChanged(this);
  emit  AValueChanged(this);
  emit  FirstValueChanged(GetValueInt(0));
  emit  FirstValueChanged(GetValue(0));
  emit  FirstValueChanged(this);
  emit  SecondValueChanged(GetValueInt(1));
  emit  SecondValueChanged(GetValue(1));
  emit  SecondValueChanged(this);
}

void SlsQtNumberEntry::RefreshFirstNumberEntry()  { RefreshNumberEntery(0);}
void SlsQtNumberEntry::RefreshSecondNumberEntry() { RefreshNumberEntery(1);}
void SlsQtNumberEntry::RefreshNumberEntery(int which_number_field){
  //does not apply to spin boxes
  if(num_field[which_number_field]){
    SetNumber(GetNumber(which_number_field),which_number_field);
  }
  //refreshes the entery in a general format for ints and fixes doubles, 
  //for example removes leading zeros   
  //However, also moves curser position
}

void SlsQtNumberEntry::FirstValueEntered(){
  emit  AValueChanged(this);
  emit  FirstValueChanged(GetValue(0));
  emit  FirstValueChanged(GetValueInt(0));
  emit  FirstValueChanged(this);
}

void SlsQtNumberEntry::SecondValueEntered(){
  emit  AValueChanged(this);
  emit  SecondValueChanged(GetValue(1));
  emit  SecondValueChanged(GetValueInt(1));
  emit  SecondValueChanged(this);
}


bool SlsQtNumberEntry::Enabled(){
  if(check_box && !check_box->checkState()) return 0;
  return 1;
}

bool SlsQtNumberEntry::IsValueOk(int which_number_field){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  bool ok=0;
  if(validator_int[i])     num_field[i]->text().toInt(&ok);
  if(validator_double[i])  num_field[i]->text().toDouble(&ok);
  if(spin_box[i])          ok=true;
  
  return ok;
}

const char* SlsQtNumberEntry::GetFrontText(){
  if(front_text) return front_text->text().toStdString().c_str();
  return 0;
}
const char* SlsQtNumberEntry::GetMiddleText(){
  if(middle_text) return middle_text->text().toStdString().c_str();
  return 0;
}
const char* SlsQtNumberEntry::GetBackText(){
  if(back_text) back_text->text().toStdString().c_str();
  return 0;
}

int SlsQtNumberEntry::GetNumberInt(int which_number_field,bool* ok){
  return round(GetNumber(which_number_field,ok));
}

double SlsQtNumberEntry::GetNumber(int which_number_field,bool* ok){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  if(ok) *ok = 1;

  if(num_field[i]){
    if(validator_int[i]) return num_field[i]->text().toInt(ok);
    else                 return num_field[i]->text().toDouble(ok);
  }
  else if(spin_box[i])   return spin_box[i]->value();
  else                   {if(ok) *ok=0;}

  return 0;
}



double SlsQtNumberEntry::SetValue(double v,int which_number_field){
  if(unit_cbb) SetNumber(round(v/GetComboBoxValue()),which_number_field);
  else         SetNumber(v,which_number_field);
  
  return GetValue(which_number_field);
}


int SlsQtNumberEntry::GetValueInt(int which_number_field,bool* ok){
  return round(GetValue(which_number_field,ok));
}

double SlsQtNumberEntry::GetValue(int which_number_field,bool* ok){
  int i = (which_number_field<0||which_number_field>1) ? 0:which_number_field;

  double v;
  if(ok) *ok = 1;

  if(num_field[i]){
    if(validator_int[i]) v = num_field[i]->text().toInt(ok);
    else                 v = num_field[i]->text().toDouble(ok);
  }
  else if(spin_box[i])   v = spin_box[i]->value();
  else                   v = 1; //incase there is only a unit pulldown


  if(unit_cbb)     v *= GetComboBoxValue();

  if(!num_field[i]&&!spin_box[i]&&!unit_cbb){v=0; if(ok) *ok=0;}

  return v;
}


int SlsQtNumberEntry::SetComboBoxIndex(int index){
  if(unit_cbb){
    if(index<0||index>=unit_cbb->count()){
      cout<<"usage error : can not set combo box index, index out of range."<<endl;
    }else{
      unit_cbb->setCurrentIndex(index);
    }
  }
  else cout<<"usage error : can not set combo box index, no combo box."<<endl;

  return GetComboBoxIndex();
}

int SlsQtNumberEntry::GetComboBoxIndex(){
  if(unit_cbb)  return unit_cbb->currentIndex();

  cout<<"usage error : can not get combo box index, no combo box."<<endl;
  return 0;
}

double SlsQtNumberEntry::GetComboBoxValue(){
  if(unit_cbb)  return factors[unit_cbb->currentIndex()];

  cout<<"usage error : can not get combo box value, no combo box."<<endl;
  return 0;
}


void SlsQtNumberEntry::PrintTheValue(){
  
  cout<<endl<<endl<<"Printing value:"<<endl;
  
  int n_not_printed = 0;
  if(validator_int[0]) 
    cout<<"The interger value has been changed to: "<<GetValue(0)<<endl;
  else if(validator_double[0]) 
    cout<<"The double value has been changed to: "<<GetValue(0)<<endl;
  else n_not_printed++;

  if(validator_int[1]) 
    cout<<"and the integer value of the second field is: "<<GetValue(1)<<endl;
  else if(validator_double[1]) 
    cout<<"and the double value of the second field is: "<<GetValue(1)<<endl;
  else n_not_printed++;

  if(unit_cbb){
    cout<<"ComboBox Status: "<<"Index: index = "<<GetComboBoxIndex()<<",    v = "<<GetComboBoxValue()<<endl;
  }


  if(n_not_printed==2) cout<<"The value of the unit box is: "<<GetValue()<<endl;
  
}
