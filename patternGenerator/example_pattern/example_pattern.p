//define signals and directions (Input, outputs, clocks)


#define output0 0
setoutput(output0);

#define output1 1
setoutput(output1);

#define output2 2
setoutput(output2);

#define output3 3
setoutput(output3);

#define input0 4
setinput(input0);

#define input1 5
setinput(input1);

#define input2 6
setinput(input2);

#define input3 7
setinput(input3);

  
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

			     
#define INST0 CB(output3);CB(output2);CB(output1);CB(output0);PW;
#define INST1 CB(output3);CB(output2);CB(output1);SB(output0);PW;
#define INST2 CB(output3);CB(output2);SB(output1);CB(output0);PW;
#define INST3 CB(output3);CB(output2);SB(output1);SB(output0);PW;
#define INST4 CB(output3);SB(output2);CB(output1);CB(output0);PW;
#define INST5 CB(output3);SB(output2);CB(output1);SB(output0);PW;
#define INST6 CB(output3);SB(output2);SB(output1);CB(output0);PW;
#define INST7 CB(output3);SB(output2);SB(output1);SB(output0);PW;
#define INST8 SB(output3);CB(output2);CB(output1);CB(output0);PW;
#define INST9 SB(output3);CB(output2);CB(output1);SB(output0);PW;
#define INST10 SB(output3);CB(output2);SB(output1);CB(output0);PW;
#define INST11 SB(output3);CB(output2);SB(output1);SB(output0);PW;
#define INST12 SB(output3);SB(output2);CB(output1);CB(output0);PW;

START;
INST0;

setwaitpoint(0);
setwaittime(0,5);
INST1;

setstartloop(5);
setnloop(5,2);
INST2;

setstartloop(0);
setnloop(0,2);
INST3;

INST4;

setstoploop(0);
setstoploop(5);
INST5;

INST6;

setwaitpoint(4);
setwaittime(1,0);
INST7;

INST8;

setstartloop(2);
setnloop(2,0);
INST9;

INST10;

setstoploop(2);
INST11;

STOP;
INST12;


