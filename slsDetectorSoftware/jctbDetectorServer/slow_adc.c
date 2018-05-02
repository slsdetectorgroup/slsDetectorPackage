#include "firmware_funcs.h"
#include "registers_m.h"
#include "server_defs.h"
#include "blackfin.h"

int prepareSlowADCSeq() {

  //  u_int16_t vv=0x3c40;
  u_int16_t codata=( 1<<13) | (7<<10)  | (7<<7) | (1<<6) | (0<<3) | (2<<1) | 1;

  u_int32_t valw;              
  int  obit, ibit;

  //  int cnv_bit=16, sdi_bit=17, sck_bit=18;
  int cnv_bit=10, sdi_bit=8, sck_bit=9;

  // int oval=0;


  printf("Codata is %04x\n",codata);
  
   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
    
  for (ibit=0; ibit<14; ibit++) {
    obit=((codata >> (13-ibit)) & 1);
    //   printf("%d",obit);
    valw = obit << sdi_bit;
    
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw);

  }
  //   printf("\n");
  


  bus_w(ADC_WRITE_REG,0);

   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
  return 0;

}

int prepareSlowADC(int ichan) {

  // u_int16_t vv=0x3c40;
  // u_int16_t codata=( 1<<13) | (7<<10)  | (7<<7) | (1<<6) | (0<<3) | (2<<1) | 1;

    u_int16_t codata=(1<<13) | (7<<10)  | (ichan<<7) | (1<<6) | (0<<3) | (0<<1) | 1; //read single channel
    if (ichan<0) codata=( 1<<13) | (3<<10)  | (7<7) | (1<<6) | (0<<3) | (0<<1) | 1;

    u_int32_t valw;              
  int obit, ibit;

  //  int cnv_bit=16, sdi_bit=17, sck_bit=18;
  int cnv_bit=10, sdi_bit=8, sck_bit=9;
 

  //  int oval=0;


  printf("Codata is %04x\n",codata);
  
   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
    
  for (ibit=0; ibit<14; ibit++) {
    obit=((codata >> (13-ibit)) & 1);
    //   printf("%d",obit);
    valw = obit << sdi_bit;
    
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw);

  }
  //   printf("\n");
  


  bus_w(ADC_WRITE_REG,0);

   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
  return 0;

}




int readSlowADC(int ichan) {


  // u_int16_t vv=0x3c40;
  //  u_int16_t codata=( 1<<13) | (7<<10)  | (ichan<<7) | (1<<6) | (0<<3) | (0<<1) | 1; //read single channel

  u_int32_t valw;              
  int i, obit;

  

  //  int cnv_bit=16, sdi_bit=17, sck_bit=18;
  int cnv_bit=10, sdi_bit=8, sck_bit=9;

  int oval=0;

  printf("DAC index is %d\n",ichan);

  if (ichan<-1 || ichan>7)
    return -1;


  prepareSlowADC(ichan);


  /* printf("Codata is %04x\n",codata); */
  
  /*  /\* //convert *\/ */
  /* valw=(1<<cnv_bit);  */
  /* bus_w(ADC_WRITE_REG,valw);  */
  
  /* usleep(20); */
  
  /* valw=0;  */
  /* bus_w(ADC_WRITE_REG,(valw));  */
  
  /* usleep(20); */
    
  /* for (ibit=0; ibit<14; ibit++) { */
  /*   obit=((codata >> (13-ibit)) & 1); */
  /*   //   printf("%d",obit); */
  /*   valw = obit << sdi_bit; */
    
  /*   bus_w(ADC_WRITE_REG,valw); */
    
  /*   usleep(20); */
    
  /*   bus_w(ADC_WRITE_REG,valw|(1<<sck_bit)); */
    
  /*   usleep(20); */
    
  /*   bus_w(ADC_WRITE_REG,valw); */

  /* } */
  /* //   printf("\n"); */
  


  /* bus_w(ADC_WRITE_REG,0); */


  


  for (ichan=0; ichan<9; ichan++) {
 
 
   /* //convert */
    valw=(1<<cnv_bit);
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    valw=0;
    bus_w(ADC_WRITE_REG,(valw));
    

    usleep(20);
    //  printf("Channel %d ",ichan);
    //read
    oval=0;
    for (i=0;i<16;i++) {
      obit=bus_r16(SLOW_ADC_REG)&0x1;
      //  printf("%d",obit);
      //write data (i)
      // usleep(0);
      oval|=obit<<(15-i);
      //cldwn
      valw=0;
      bus_w(ADC_WRITE_REG,valw);
      bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
      usleep(20);
      bus_w(ADC_WRITE_REG,valw);
      usleep(20);
      
      
    }
    printf("\t");
    printf("Value %d  is %d (%d mV)\n",ichan, oval,2500*oval/65535);
    
  }
 
    printf("Value %d  is %d\n",ichan, oval);
 
  return 2500*oval/65535;
}

