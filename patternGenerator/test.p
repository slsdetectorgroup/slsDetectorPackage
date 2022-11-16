//define signals and directions (Input, outputs, clocks)


#define compTestIN 1
setoutput(compTestIN);

#define curON 32
setoutput(curON);

#define side_clk 2
setclk(side_clk);

#define side_din 3
setoutput(side_din);

#define clear_shr 4
setoutput(clear_shr);

#define bottom_din 5
setoutput(bottom_din);

#define bottom_clk 6
setclk(bottom_clk);

#define gHG 7
setoutput(gHG);

#define bypassCDS 31
setoutput(bypassCDS);


#define ENprechPRE 8
setoutput(ENprechPRE);


#define res 9
setoutput(res);

#define pulseOFF 30
setoutput(pulseOFF);

#define connCDS 27
setoutput(connCDS);

#define Dsg_1 24
setoutput(Dsg_1);


#define Dsg_2 25
setoutput(Dsg_2);


#define Dsg_3 23
setoutput(Dsg_3);

#define sto0 10
setoutput(sto0);

#define sto1 11
setoutput(sto1);

#define sto2 12
setoutput(sto2);

#define resCDS 13
setoutput(resCDS);

#define prechargeConnect 14
setoutput(prechargeConnect);

#define pulse 15
setoutput(pulse);

#define PCT_mode 21
setoutput(PCT_mode);

#define res_DGS 16
setoutput(res_DGS);

#define adc_ena 17
setoutput(adc_ena);


#define CLKBIT 18
setclk(CLKBIT);


#define adc_sync 63
setoutput(adc_sync);





	  
#define PW pw()
#define SB(x) setbit(x) 
#define CB(x) clearbit(x)
#define CLOCK  clearbit(CLKBIT); pw();setbit(CLKBIT);pw()
#define LCLOCK  clearbit(CLKBIT); pw();setbit(CLKBIT);pw();clearbit(CLKBIT); pw()
#define CLOCKS(x) for (i=0;i<x;i++) {clearbit(CLKBIT);pw(); setbit(CLKBIT); pw();}
#define STOP setstop();
#define START setstart();
#define REPEAT(x) for (i=0;i<(x);i++) {pw();}
#define DOFOR(x) for (j=0;j<(x);j++) {
// }				     
#define STARTUP1 CB(compTestIN);SB(clear_shr);CB(side_clk);CB(side_din);CB(bottom_din);CB(bottom_clk);
#define STARTUP2 CB(pulse);SB(PCT_mode);SB(pulseOFF);CB(curON);
#define STARTUP3 SB(res);SB(gHG);SB(ENprechPRE);
#define STARTUP4 SB(bypassCDS); CB(connCDS);CB(sto0);SB(sto1);SB(sto2);
#define STARTUP5 SB(resCDS);CB(Dsg_1);CB(Dsg_2);SB(Dsg_3);CB(prechargeConnect);SB(res_DGS);
#define STARTUP STARTUP1 STARTUP2 STARTUP3 STARTUP4 STARTUP5 PW;





//****NOTES****//
//FUNCTIONS
//Declare functions at the beginning 
void load_pix(int nx, int ny)
{//SELECT PIXEL 1,1 for readout
SB(clear_shr);PW;PW;
CB(clear_shr);PW;PW;PW;PW;

SB(side_din);PW;
SB(side_clk);PW;
CB(side_din);
setstartloop(0); //loop on the rows
SB(side_clk);PW;
setstoploop(0);  //finish loop on the rows
setnloop(0,ny); //set number row selected -can be changed dynamically
CB(side_clk);PW;
SB(bottom_din);PW;
SB(bottom_clk);PW;
CB(bottom_din);
setstartloop(1); //loop on the columns
SB(bottom_clk);PW;
setstoploop(1); //loop on the columns
setnloop(1,ny); //set number columns selected -can be changed dynamically
}
   
void load_col(void)
{//SELECT COLUMN 1 for readout
SB(clear_shr);PW;PW;
CB(clear_shr);PW;PW;PW;PW;
SB(bottom_din);PW;
SB(bottom_clk);PW;
CB(bottom_clk);PW;
CB(bottom_din);PW;
}
//END of FUNCTIONS
////////////////////////////////////////////////////////
//LET BYPASS PREAMP AND CDS and write on preamp out.//
//THIS ALLOWS CHECKING SOURCE FOLLOWERS               //
////////////////////////////////////////////////////////

   
PW;
   
SB(5); PW;

CB(5); PW;

START; //pattern starts from here
STARTUP;
setwaitpoint(0); //set wait points
PW;
setwaittime(0,20); //wait time - can be changed dynamically
SB(adc_ena);PW;
   printf("ADC sync %x %d %llx\n",iaddr,adc_sync, pat);
SB(adc_sync);PW;
   printf("ADC sync %x %d  %llx\n",iaddr, adc_sync, pat);
CB(gHG);
setwaitpoint(1); //set wait points
setwaittime(1,16); //wait time - can be changed dynamically
CB(adc_sync);PW;
load_pix(10, 20);

CB(res);
//CB(Dsg_3);PW;
CB(res_DGS);
setwaitpoint(2); //set wait points
setwaittime(2,1000); //wait time - can be changed dynamically

//SB(res_DGS);
//PW;
//SB(Dsg_3);
//
//CB(connCDS);
//TEST SIGNALS END
//
REPEAT(20)

//****************//
//*FINAL COMMANDS*//
//****************//
CB(adc_ena);PW;
//STARTUP;
STOP; PW; //stops here
//REPEAT(4);
