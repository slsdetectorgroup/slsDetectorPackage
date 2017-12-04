
/**
 * @author Ian Johnson
 * @version 1.0
 */

#ifndef SLSQTVALIDATORS_H
#define SLSQTVALIDATORS_H

#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif

#include <iostream>
#include <math.h>

#include <qwidget.h>
#include <qvalidator.h>

using std::cout;
using std::endl;

class SlsQtIntValidator:public QIntValidator{

 public:
  SlsQtIntValidator(QWidget *parent):QIntValidator(parent){}

  virtual void fixup (QString& text) const {
    bool ok = 1;
    int v = text.toInt(&ok);

    if(!ok){
      v = text.toDouble(&ok);      
      if(ok) text = QString::number(v);
      else   text = QString::number(0);
      fixup(text);
    }

    if(v<bottom())   text = QString::number(bottom());
    else if(v>top()) text = QString::number(top());
  }

};

class SlsQtDoubleValidator:public QDoubleValidator{

 public:
  SlsQtDoubleValidator(QWidget *parent):QDoubleValidator(parent){}

  virtual void fixup (QString& text) const {

    bool ok = 1;
    double v = text.toDouble(&ok);

    if(!ok){
      text = QString::number(0);
      fixup(text);
    }    
   
    int nd = this->decimals();  //ndigest behind zero
    if(v<bottom()){
      text = QString::number(bottom(),'g',nd);
    }else{
      if(nd<0) nd=0;
      if(v>top()){
	v = floor(top()*pow(10,nd))/pow(10,nd);
	text = QString::number(v,'g');
      }else{
	v = round(v*pow(10,nd))/pow(10,nd);
	text = QString::number(v,'g');
      }
    }
  }
  
};

#endif
