#ifndef INTMAP_H
#define INTMAP_H
#define N 60
#include <stdio.h>
#include <math.h>




class IntMap{


 public:

  IntMap(){};

  ~IntMap(){};

  void Init(){
    //lookup table of the output intensity for IR laser
    //measurements performed by Dominic april 2014
    //intensity[59]=intensity with 0 Optical Density
    //intensity[0]=intensity with 5.9 Optical Density
    intensity[0]=29;//5.9
    intensity[1]=21;//5.8
    intensity[2]=31;//5.7
    intensity[3]=43;//5.6
    intensity[4]=60;//5.5
    intensity[5]=91;//5.4
    intensity[6]=69;//5.3
    intensity[7]=102;//5.2
    intensity[8]=136;//5.1
    intensity[9]=196;//5.0
    intensity[10]=425;//4.9
    intensity[11]=311;//4.8
    intensity[12]=462;//4.7
    intensity[13]=653;//4.6
    intensity[14]=926;//4.5
    intensity[15]=1423;//4.4
    intensity[16]=1072;//4.3
    intensity[17]=1592;//4.2
    intensity[18]=2142;//4.1
    intensity[19]=3085;//4.0
    intensity[20]=729;//3.9
    intensity[21]=533;//3.8
    intensity[22]=793;//3.7
    intensity[23]=1121;//3.6
    intensity[24]=1588;//3.5
    intensity[25]=2439;//3.4
    intensity[26]=1842;//3.3
    intensity[27]=2730;//3.2
    intensity[28]=3663;//3.1
    intensity[29]=5271;//3.0
    intensity[30]=8102;//2.9
    intensity[31]=5933;//2.8
    intensity[32]=8789;//2.7
    intensity[33]=12350;//2.6
    intensity[34]=17358;//2.5
    intensity[35]=26300;//2.4
    intensity[36]=20029;//2.3
    intensity[37]=29414;//2.2
    intensity[38]=39202;//2.1
    intensity[39]=55724;//2.0
    intensity[40]=15697;//1.9
    intensity[41]=11541;//1.8
    intensity[42]=16976;//1.7
    intensity[43]=23866;//1.6
    intensity[44]=33478;//1.5
    intensity[45]=50567;//1.4
    intensity[46]=38552;//1.3
    intensity[47]=56394;//1.2
    intensity[48]=74897;//1.1
    intensity[49]=106023;//1.0
    intensity[50]=157384;//0.9
    intensity[51]=117677;//0.8
    intensity[52]=171101;//0.7
    intensity[53]=236386;//0.6
    intensity[54]=327248;//0.5
    intensity[55]=492781;//0.4
    intensity[56]=379641;//0.3
    intensity[57]=546927;//0.2
    intensity[58]=717203;//0.1
    intensity[59]=1000000;//0.
    return;
  };
    //_od is the total Optical Density
  int getIntensity(float _od){
    int _int(-1);

    //these lines are to take into account rounding errors with floats
    float hun_od = 100.*_od;
    int Ihun_od = (int)round(hun_od); 
    float R_od =(float) Ihun_od/10.;
    int I_od = (int)R_od;

    if(I_od >-1 && I_od <60){
      int index=59-I_od;
      cerr<<index<<endl;
      _int=intensity[index];
    }else{
      cerr<<"Optical density out of range!"<<endl;
    }
    return _int;
  };


 private:

  int intensity[N];

};

#endif
