#include "slsDetectorCommand.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <iomanip>

slsDetectorCommand::slsDetectorCommand(slsDetectorUtils *det)  {

  myDet=det;
 
  int i=0;

  cmd=string("none");

  /* Acquisition and status commands */

  descrToFuncMap[i].m_pFuncName="test"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdUnderDevelopment;
  i++;

  descrToFuncMap[i].m_pFuncName="acquire"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAcquire;
  i++;

  descrToFuncMap[i].m_pFuncName="data"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdData;
  i++;

  descrToFuncMap[i].m_pFuncName="frame"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFrame;
  i++;

  descrToFuncMap[i].m_pFuncName="status"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdStatus;
  i++;


  /* Detector structure configuration and debugging commands */

  descrToFuncMap[i].m_pFuncName="free";//OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFree;
  i++;


  descrToFuncMap[i].m_pFuncName="add";//OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAdd;
  i++;

  descrToFuncMap[i].m_pFuncName="remove";//OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRemove;
  i++;


  descrToFuncMap[i].m_pFuncName="type"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdHostname;
  i++;

  descrToFuncMap[i].m_pFuncName="hostname"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdHostname;
  i++;

  descrToFuncMap[i].m_pFuncName="id"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdId;
  i++;

  descrToFuncMap[i].m_pFuncName="master"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdMaster;
  i++; 

  descrToFuncMap[i].m_pFuncName="sync"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSync;
  i++;

  descrToFuncMap[i].m_pFuncName="help";//OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdHelp;
  i++;

  descrToFuncMap[i].m_pFuncName="exitserver";//OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdExitServer;
  i++;

  /* data processing commands */
  descrToFuncMap[i].m_pFuncName="flatfield"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFlatField;
  i++;

  descrToFuncMap[i].m_pFuncName="ffdir"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFlatField;
  i++;

  descrToFuncMap[i].m_pFuncName="ratecorr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRateCorr;
  i++;

  descrToFuncMap[i].m_pFuncName="badchannels"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdBadChannels;
  i++;

  descrToFuncMap[i].m_pFuncName="angconv"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;

  descrToFuncMap[i].m_pFuncName="globaloff"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;

  descrToFuncMap[i].m_pFuncName="fineoff"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;

  descrToFuncMap[i].m_pFuncName="binsize" ;//
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;

  descrToFuncMap[i].m_pFuncName="angdir" ;//
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;


  descrToFuncMap[i].m_pFuncName="moveflag" ;//
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;



  descrToFuncMap[i].m_pFuncName="threaded"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdThreaded;
  i++;

  descrToFuncMap[i].m_pFuncName="darkimage"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdImage;
  i++;

  descrToFuncMap[i].m_pFuncName="gainimage"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdImage;
  i++;

  descrToFuncMap[i].m_pFuncName="readctr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdCounter;
  i++;

  descrToFuncMap[i].m_pFuncName="resetctr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdCounter;
  i++;


  /* trim/cal directories */
  descrToFuncMap[i].m_pFuncName="trimdir"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettingsDir;
  i++;

  descrToFuncMap[i].m_pFuncName="settingsdir"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettingsDir;
  i++;

  descrToFuncMap[i].m_pFuncName="caldir"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdCalDir;
  i++;

  descrToFuncMap[i].m_pFuncName="trimen";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdUnderDevelopment;
  i++;


  /* file name */
  descrToFuncMap[i].m_pFuncName="outdir"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOutDir;
  i++;

  descrToFuncMap[i].m_pFuncName="fname"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFileName;
  i++;

  descrToFuncMap[i].m_pFuncName="index"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFileIndex;
  i++;

  
  descrToFuncMap[i].m_pFuncName="online"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOnline;
  i++;

  descrToFuncMap[i].m_pFuncName="checkonline"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOnline;
  i++;

  descrToFuncMap[i].m_pFuncName="enablefwrite"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdEnablefwrite;
  i++;


  /* Acquisition actions */

  descrToFuncMap[i].m_pFuncName="positions"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPositions;
  i++;

  descrToFuncMap[i].m_pFuncName="startscript"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="startscriptpar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="stopscript"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="stopscriptpar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="scriptbefore"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="scriptbeforepar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="scriptafter"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="scriptafterpar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="headerafter"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="headerafterpar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="encallog"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="angcallog"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;








  descrToFuncMap[i].m_pFuncName="headerbefore"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="headerbeforepar"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScripts;
  i++;

  descrToFuncMap[i].m_pFuncName="scan0script"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan0par"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan0prec"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan0steps"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan0range"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan1script"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan1par"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan1prec"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan1steps"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;

  descrToFuncMap[i].m_pFuncName="scan1range"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdScans;
  i++;


  /* communication configuration */

  descrToFuncMap[i].m_pFuncName="clientip"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="clientmac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="servermac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="configuremac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfigureMac;
  i++;

  descrToFuncMap[i].m_pFuncName="port"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPort;
  i++;

  descrToFuncMap[i].m_pFuncName="stopport"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPort;
  i++;

  descrToFuncMap[i].m_pFuncName="dataport"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPort;
  i++;

  descrToFuncMap[i].m_pFuncName="lock"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdLock;
  i++;

  descrToFuncMap[i].m_pFuncName="lastclient"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdLastClient;
  i++;


  /* detector and data size */
  descrToFuncMap[i].m_pFuncName="nmod"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
  i++;

  descrToFuncMap[i].m_pFuncName="maxmod"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
  i++;

  descrToFuncMap[i].m_pFuncName="dr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
  i++;


  /* flags */

  descrToFuncMap[i].m_pFuncName="flags";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAdvanced;
  i++;

  descrToFuncMap[i].m_pFuncName="extsig"; /* find command! */
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAdvanced;
  i++;

  /* versions/ serial numbers  getId */

  descrToFuncMap[i].m_pFuncName="moduleversion"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;

  descrToFuncMap[i].m_pFuncName="detectornumber"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;

  descrToFuncMap[i].m_pFuncName="modulenumber"; /* find command! */
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;

  descrToFuncMap[i].m_pFuncName="detectorversion"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;

  descrToFuncMap[i].m_pFuncName="softwareversion"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;

  descrToFuncMap[i].m_pFuncName="thisversion"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSN;
  i++;


  /* digital test and debugging */

  descrToFuncMap[i].m_pFuncName="digitest";  // /* find command! */ 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDigiTest;
  i++;

  descrToFuncMap[i].m_pFuncName="bustest"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDigiTest;
  i++;

  descrToFuncMap[i].m_pFuncName="digibittest"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDigiTest;
  i++;

  descrToFuncMap[i].m_pFuncName="acqtest"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDigiTest;
  i++;

  descrToFuncMap[i].m_pFuncName="reg"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRegister;
  i++;

  /* settings, threshold */

  descrToFuncMap[i].m_pFuncName="settings"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettings;
  i++;

  descrToFuncMap[i].m_pFuncName="threshold"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettings;
  i++;

  descrToFuncMap[i].m_pFuncName="trimbits"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettings;
  i++;

  descrToFuncMap[i].m_pFuncName="trim"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettings;
  i++;

  /* pots */

  descrToFuncMap[i].m_pFuncName="vthreshold"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcalibration"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vtrimbit"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vpreamp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vshaper1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vshaper2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vhighvoltage"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vapower"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vddpower"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vshpower"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="viopower"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vref_ds"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcascn_pb"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcascp_pb"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vout_cm"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcasc_out"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vin_cm"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vref_comp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="ib_test_c"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  /* r/w timers */

  descrToFuncMap[i].m_pFuncName="temp_adc"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_fpga"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  /* r/w timers */

  descrToFuncMap[i].m_pFuncName="timing"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTiming;
  i++;

  descrToFuncMap[i].m_pFuncName="exptime"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="period"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="delay"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="gates"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="frames"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="cycles"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="probes"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;


  descrToFuncMap[i].m_pFuncName="measurments"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  /* read only timers */

  descrToFuncMap[i].m_pFuncName="exptimel"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="periodl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="delayl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="gatesl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="framesl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="cyclesl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

//   descrToFuncMap[i].m_pFuncName="progress";
//   descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
//   i++;

  descrToFuncMap[i].m_pFuncName="now"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;

  descrToFuncMap[i].m_pFuncName="timestamp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimeLeft;
  i++;


  /* speed */

  descrToFuncMap[i].m_pFuncName="clkdivider"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="setlength"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="waitstates"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="totdivider"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="totdutycycle"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  /* settings dump/retrieve */
  descrToFuncMap[i].m_pFuncName="config"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  descrToFuncMap[i].m_pFuncName="parameters"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  descrToFuncMap[i].m_pFuncName="setup"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  numberOfCommands=i;
  
// #ifdef VERBOSE
//   cout << "Number of commands is " << numberOfCommands << endl;
// #endif
}






string slsDetectorCommand::executeLine(int narg, char *args[], int action) {
  


  if (action==READOUT_ACTION)
    return cmdAcquire(narg, args, action);


  size_t s=string(args[0]).find(':');
  string key=string(args[0]).substr(0,s); // truncate at :

  if (action==PUT_ACTION && narg<1)
    action=HELP_ACTION;


  for(int i=0; i<numberOfCommands; ++i) {  

    /* this works only if the command completely matches the key */
    /* otherwise one could try if truncated key is unique */


   // size_t p=(descrToFuncMap[i].m_pFuncName).find();
  //  if (p==0) {

    	if(key==descrToFuncMap[i].m_pFuncName){
#ifdef VERBOSE  
	  std::cout<<i << " command="<< descrToFuncMap[i].m_pFuncName<<" key="<<key <<std::endl;
#endif      
      cmd=descrToFuncMap[i].m_pFuncName;

      MemFuncGetter memFunc=descrToFuncMap[i].m_pFuncPtr;
      string dResult=(this->*memFunc)(narg, args, action);  
      
      return dResult;
	}
  }
  return cmdUnknown(narg,args,action);
  
}




string slsDetectorCommand::cmdUnknown(int narg, char *args[], int action) {
  return string("Unknown command ")+string(args[0])+string("\n")+helpLine(0, args, action);

}
string slsDetectorCommand::cmdUnderDevelopment(int narg, char *args[], int action) {
  return string("Must still develop ")+string(args[0])+string(" ( ")+cmd+string(" )\n");

}




string slsDetectorCommand::helpLine(int narg, char *args[], int action) {

  ostringstream os;

  if (action==READOUT_ACTION) {
    return helpAcquire(narg,args,HELP_ACTION);
  }

  
  if (narg==0) {
    os << "Command can be: " << endl;
    for(int i=0; i<numberOfCommands; ++i) {  
      os << descrToFuncMap[i].m_pFuncName << "\t" ;
    }
    os << endl;
    return os.str();
  }
  return executeLine(narg,args,HELP_ACTION);




}




string slsDetectorCommand::cmdAcquire(int narg, char *args[], int action) {
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  myDet->setOnline(ONLINE_FLAG);
  myDet->acquire();
  return string("");

}




string slsDetectorCommand::helpAcquire(int narg, char *args[], int action){
  
  
  if (action==PUT_ACTION)
    return string("");
  ostringstream os;
  os << "Usage is "<< std::endl << "sls_detector_acquire  id " << std::endl;
  os << "where id is the id of the detector " << std::endl;
  os << "the detector will be started, the data acquired, processed and written to file according to the preferences configured " << std::endl;
  return os.str();

}
  

string slsDetectorCommand::cmdData(int narg, char *args[], int action) {
  
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  int b;
  if (action==PUT_ACTION) {
    return  string("cannot set");
  } else if (action==HELP_ACTION) {
    return helpData(narg,args,HELP_ACTION);
  } else {
    b=myDet->setThreadedProcessing(-1);
    myDet->setThreadedProcessing(0);
    myDet->setOnline(ONLINE_FLAG);
    myDet->readAll();
    myDet->processData(1);
    myDet->setThreadedProcessing(b);
    return string("");
  } 
}



string slsDetectorCommand::helpData(int narg, char *args[], int action){ 

  if (action==PUT_ACTION)
    return string("");
  else
   return string("data \t gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup\n");

}

string slsDetectorCommand::cmdFrame(int narg, char *args[], int action) {

  int b;
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
    if (action==PUT_ACTION) {
      return  string("cannot set");
    } else if (action==HELP_ACTION) {
      return helpFrame(narg,args,HELP_ACTION);
    } else {
       b=myDet->setThreadedProcessing(-1);
       myDet->setThreadedProcessing(0);
       myDet->setOnline(ONLINE_FLAG);
       myDet->readFrame();
       myDet->processData(1);
       myDet->setThreadedProcessing(b);
       return string("ok");
    } 

}

string slsDetectorCommand::helpFrame(int narg, char *args[], int action) {
  
  if (action==PUT_ACTION)
    return string("");
  return string("frame \t gets a single frame from the detector (if any) processes it and writes it to file according to the preferences already setup\n");
  
}

string slsDetectorCommand::cmdStatus(int narg, char *args[], int action) {

#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    //myDet->setThreadedProcessing(0);
    if (string(args[1])=="start")
      myDet->startAcquisition();
    else if (string(args[1])=="stop")
      myDet->stopAcquisition();
    else
      return string("unknown action");
  } else if (action==HELP_ACTION) {
    return helpStatus(narg,args,HELP_ACTION);
  } 
  runStatus s=myDet->getRunStatus();  
  return myDet->runStatusType(s);

}



string slsDetectorCommand::helpStatus(int narg, char *args[], int action) {
  
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("status \t gets the detector status - can be: running, error, transmitting, finished, waiting or idle\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("status \t controls the detector acquisition - can be start or stop \n");
  return os.str();
}


 string slsDetectorCommand::cmdFree(int narg, char *args[], int action) {
  
#ifdef VERBOSE
   cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  if (action==HELP_ACTION) {
    return helpFree(narg,args,HELP_ACTION);
  } 
  myDet->freeSharedMemory();
  return("freed");
 }


string slsDetectorCommand::helpFree(int narg, char *args[], int action) {

  return string("free \t frees the shared memory\n");

}

   
string slsDetectorCommand::cmdAdd(int narg, char *args[], int action) {
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  int ivar, ival;
  string var=string(args[0]);
  ostringstream os;
  if (action==HELP_ACTION) {
    return helpAdd(narg,args,HELP_ACTION);
  } else    if (action==PUT_ACTION) {
    size_t p=string(args[0]).find(':');
    if (p==string::npos)
      ivar=-1;
    else {
      istringstream vvstr(var.substr(p+1));
      vvstr >> ivar;
      if (vvstr.fail())
	ivar=-1; //append at the end
    }

    if (sscanf(args[1],"%d",&ival)) {
      // add by detector id
      os<< myDet->addSlsDetector(ival, ivar)<< endl;;
    } else {
      //add by hostname
      os<< myDet->addSlsDetector(args[1], ivar)<< endl;
    }
    return os.str();
  } 
  return string("cannot get");

}


string slsDetectorCommand::helpAdd(int narg, char *args[], int action){
  return string("add[:i] det \t adds a detector in position i to the multi detector structure. i is the detector position, default is appended. det can either be the detector hostname or the detector id. Returns -1 if it fails or the total number of detectors in the multidetector structure\n");
}

string slsDetectorCommand::cmdRemove(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
 
    
  ostringstream os;
  int ival;//ivar, 
  string var=string(args[0]);
  
  if (action==HELP_ACTION) {
    return helpRemove(narg,args,HELP_ACTION);
  } else    if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&ival)) {
      // remove detector in position ival
      os << myDet->removeSlsDetector(ival);
    } else {
      // remove detector by hostname
      os<< myDet->removeSlsDetector(args[1]);
    }
    return os.str();
  }
  return string("cannot get");

}

string slsDetectorCommand::helpRemove(int narg, char *args[], int action){ 
  return string("remove det \t removes a detector. det can either be the detector hostname or the detector position. Returns the total number of detectors in the multidetector structure\n");
}

string slsDetectorCommand::cmdHostname(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif

  if (action==HELP_ACTION) {
    return helpHostname(narg,args,HELP_ACTION);
  } 

  ostringstream os;
  int ivar=-1;//, ival;
  string var=string(args[0]);
  char hostname[1000];
  

  size_t p=string(args[0]).find(':');
  if (p==string::npos)
    ivar=-1;
  else {
    istringstream vvstr(var.substr(p+1));
    vvstr >> ivar;
    if (vvstr.fail()) 
      ivar=-1;
  }



  p=string(args[0]).find("hostname");
  
  if (p==string::npos) {
    //type 
    // cout << "should add by type!" << endl;
    
    if (action==PUT_ACTION) {
      //add by type
      if (ivar==-1) {
	strcpy(hostname,"");
	for (int id=1; id<narg; id++) {
	  strcat(hostname,args[id]);
	  if(narg>2)
	    strcat(hostname,"+");
	} 
      }  else
	strcpy(hostname,args[1]);

      myDet->ssetDetectorsType(hostname, ivar);
    }
    return myDet->sgetDetectorsType(ivar);
  } else {
    if (action==PUT_ACTION) {
      //add by hostname
      if (ivar==-1) {
	strcpy(hostname,"");
	for (int id=1; id<narg; id++) {
	  strcat(hostname,args[id]);
	  if(narg>2)
	    strcat(hostname,"+");
	} 
      }  else
	strcpy(hostname,args[1]);
      myDet->setHostname(hostname, ivar);
    }
    
    return string(myDet->getHostname(ivar));
  }

}



string slsDetectorCommand::helpHostname(int narg, char *args[], int action){

  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("hostname[:i] \t returns the hostname(s) of the detector structure. i is the detector position in a multi detector system\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("hostname[:i] name [name name]\t configures the hostnames of the detector structure. i is the detector position in a multi detector system\n");
  return os.str();
}


string slsDetectorCommand::cmdId(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif


  if (action==HELP_ACTION) {
    return helpId(narg,args,HELP_ACTION);
  } 

  ostringstream os;
  int ivar, ival;
  string var=string(args[0]);
  // char answer[1000];
  

  size_t p=string(args[0]).find(':');
  if (p==string::npos)
    ivar=-1;
  else {
    istringstream vvstr(var.substr(p+1));
    vvstr >> ivar;
    if (vvstr.fail()) 
      ivar=-1;
  }
  
  if (action==PUT_ACTION) {
    //add by hostname
    istringstream vvstr(args[1]);
    
    vvstr >> ival;
    if (vvstr.fail()) 
      ival=-1;

    myDet->setDetectorId(ival, ivar);
  }
  os << myDet->getDetectorId(ivar);

  return os.str();

}

string slsDetectorCommand::helpId(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("id[:i] \t returns the id of the detector structure. i is the detector position in a multi detector system\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("id:i l]\t configures the id of the detector structure. i is the detector position in a multi detector system and l is the id of the detector to be added\n");

  return os.str();
}

string slsDetectorCommand::cmdMaster(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");

#endif

  ostringstream os;
  int ival;


  if (action==HELP_ACTION) {
    return helpMaster(narg,args,HELP_ACTION);
  } 
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    istringstream vvstr(args[1]);
    vvstr >> ival;
    if (vvstr.fail()) 
      return helpMaster(narg,args,HELP_ACTION);
    myDet->setMaster(ival);
  } 
  os <<  myDet->setMaster();
  return os.str();
  
}


string slsDetectorCommand::helpMaster(int narg, char *args[], int action){

  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("master \t gets the master of the detector structure (-1 if none)\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("master pos \t sets position of the master of the detector structure (-1 if none) \n");
  return os.str();
 
}

string slsDetectorCommand::cmdSync(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif

  if (action==HELP_ACTION) {
    return helpSync(narg,args,HELP_ACTION);
  } 
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    if (myDet->getSyncType(string(args[1]))==GET_SYNCHRONIZATION_MODE) return helpSync(narg,args, action);
    myDet->setSynchronization(myDet->getSyncType(string(args[1])));
  }
  return myDet->getSyncType(myDet->setSynchronization());

}
string slsDetectorCommand::helpSync(int narg, char *args[], int action){
  
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("sync \t gets the synchronization mode of the structure\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("sync mode \t sets synchronization mode of the structure. Cane be none, gating, trigger, complementary \n");
  return os.str();
}

string slsDetectorCommand::cmdHelp(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif

  cout << narg << endl;

  if (narg>=1)
    return helpLine(narg-1, args+1, action);
  else
    return helpLine(0, args, action);
    


}

string slsDetectorCommand::cmdExitServer(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  if (action==HELP_ACTION) {
    return helpExitServer(narg, args, action);
  } 
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    if (myDet->exitServer()!=OK)
      return string("Server shut down.");
    else 
      return string("Error closing server\n");
  } else
    return ("cannot get");

}

string slsDetectorCommand::helpExitServer(int narg, char *args[], int action){
  return string("exitserver \t shuts down all the detector servers. Don't use it!!!!");
}


string slsDetectorCommand::cmdSettingsDir(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif
  if (action==HELP_ACTION) {
    return helpSettingsDir(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    myDet->setSettingsDir(string(args[1]));
  }
  if (myDet->getSettingsDir()==NULL)
    return string("undefined");
  return string(myDet->getSettingsDir());
}



string slsDetectorCommand::helpSettingsDir(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("settingsdir \t  gets the directory where the settings files are located\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("settingsdir dir \t  sets the directory where the settings files are located\n");
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("trimdir \t  obsolete for settingsdir\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("trimdir dir \t  obsolete for settingsdir\n");
  return os.str();
}



string slsDetectorCommand::cmdCalDir(int narg, char *args[], int action){

  if (action==HELP_ACTION) {
    return helpCalDir(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    myDet->setCalDir(string(args[1]));
  }
  if (myDet->getCalDir()==NULL)
    return string("undefined");
  return string(myDet->getCalDir());
}



string slsDetectorCommand::helpCalDir(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("caldir \t  gets the directory where the calibration files are located\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("caldir dir \t  sets the directory where the calibration files are located\n");
  return os.str();
}



string slsDetectorCommand::cmdTrimEn(int narg, char *args[], int action){
  int ival;
  int ip;

  char answer[1000];

  if (action==HELP_ACTION) return helpTrimEn(narg,args,action);

  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&ival)) {
      int pos[ival];
      for (ip=0; ip<ival;ip++) {  
	if ((2+ip)<narg) {
	  if (sscanf(args[2+ip],"%d",pos+ip)) {
	  } else
	    break;
	}
      }
      myDet->setTrimEn(ip,pos); 
    }
  }
  int npos=myDet->getTrimEn();
  sprintf(answer,"%d",npos);
  int opos[npos];
  myDet->getTrimEn(opos);
  for (int ip=0; ip<npos;ip++) {
    sprintf(answer,"%s %d",answer,opos[ip]);
  }
  return string(answer);
    
}

string slsDetectorCommand::helpTrimEn(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) 
    os << "trimen ne [e0 e1...ene] \t sets the number of energies at which the detector has default trim files"<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION) 
    os << "trimen  \t returns the number of energies at which the detector has default trim files and their values"<< std::endl;
  return os.str();
}


string slsDetectorCommand::cmdOutDir(int narg, char *args[], int action){

  if (action==HELP_ACTION) {
    return helpOutDir(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    myDet->setFilePath(string(args[1]));
  }
  return string(myDet->getFilePath());
}



string slsDetectorCommand::helpOutDir(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("outdir \t  gets the directory where the output files will be written\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("outdir dir \t  sets the directory where the output files will be written\n");
  return os.str();
}




string slsDetectorCommand::cmdFileName(int narg, char *args[], int action){

  if (action==HELP_ACTION) {
    return helpFileName(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    myDet->setFileName(string(args[1]));
  }
  return string(myDet->getFileName());
}



string slsDetectorCommand::helpFileName(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("fname \t  gets the filename for the data without index and extension\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("fname s \t  sets the filename for the data (index and extension will be automatically appended)\n");
  return os.str();
}


//enable file write
string slsDetectorCommand::cmdEnablefwrite(int narg, char *args[], int action){

  int i;
  char ans[100];
  
  if (action==HELP_ACTION) {
    return helpFileName(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&i))
      myDet->enableWriteToFile(i);
    else
      return string("could not decode enable file write");
    

  }
  sprintf(ans,"%d",myDet->enableWriteToFile());
  return string(ans);
}



string slsDetectorCommand::helpEnablefwrite(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("When Enabled writes the data into the file\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string(" i \t  should be 1 or 0 or -1\n");
  return os.str();
}

string slsDetectorCommand::cmdFileIndex(int narg, char *args[], int action){

  int i;
  char ans[100];
  if (action==HELP_ACTION) {
    return helpFileName(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&i))
      myDet->setFileIndex(i);
  }
  sprintf(ans,"%d", myDet->getFileIndex());
  return string(ans);
}



string slsDetectorCommand::helpFileIndex(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("index \t  gets the file index for the next the data file\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("fname i \t  sets the fileindex for the next data file\n");
  return os.str();
}


string slsDetectorCommand::cmdFlatField(int narg, char *args[], int action){

  if (action==HELP_ACTION) {
    return helpFlatField(narg, args, action);
  } 
  string sval;

  if (string(args[0])==string("ffdir")) {
    if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      myDet->setFlatFieldCorrectionDir(sval); 
    }
    return string(myDet->getFlatFieldCorrectionDir());

  } else if  (string(args[0])==string("flatfield")) {

    if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      myDet->setFlatFieldCorrection(sval);
      return string(myDet->getFlatFieldCorrectionFile());
      
    }  else {// if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      double corr[24*1280], ecorr[24*1280];
      if (myDet->getFlatFieldCorrection(corr,ecorr)) {
	if (sval!="none") {
	  myDet->writeDataFile(sval,corr,ecorr,NULL,'i');
	  return sval;
	} 
	return string(myDet->getFlatFieldCorrectionFile());
      } else {
	return string("none");
      }
    }
  }
  return string("could not decode flat field action ")+cmd;
  
}
 

string slsDetectorCommand::helpFlatField(int narg, char *args[], int action){

  int t=0;
  ostringstream os;
  if (string(args[0])==string("ffdir")) {
    t=1;
  } else if  (string(args[0])==string("flatfield")) {
    t=2;
  }
  if (t!=1) {
    
    if (action==GET_ACTION || action==HELP_ACTION) {
      os << string("flatfield [fn]\t  gets the flat field file name. the coorection values and errors can be dumped to fn if specified. \n");
    } if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("flatfield s \t  sets the flat field file name\n");
  }  
  if (t!=2) {
    
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("ffdir \t  gets the path for the flat field files \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("ffdir s \t  sets the path for flat field files\n");
  }  
  return os.str();

}
 




string slsDetectorCommand::cmdRateCorr(int narg, char *args[], int action){

  if (action==HELP_ACTION) {
    return helpRateCorr(narg, args, action);
  } 
  double fval;
  char answer[1000];

  if (action==PUT_ACTION) {
    sscanf(args[1],"%lf",&fval);
    myDet->setRateCorrection(fval);
  } 
  double t;
  if (myDet->getRateCorrection(t)) {
    sprintf(answer,"%f",t);
  } else {
    sprintf(answer,"%f",0.);
  }
  return string(answer);
}
 

string slsDetectorCommand::helpRateCorr(int narg, char *args[], int action){ 
  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("ratecorr \t  returns the dead time used for rate correections in ns \n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("ratecorr  ns \t  sets the deadtime correction constant in ns\n");
  return os.str();
  
}




string slsDetectorCommand::cmdBadChannels(int narg, char *args[], int action){ 
  
  string sval;

  if (action==HELP_ACTION) {
    return helpBadChannels(narg, args, action);
  } 
  if (action==PUT_ACTION) {
      sval=string(args[1]);
      if (sval=="none")
	sval="";
      myDet->setBadChannelCorrection(sval);
    } else if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      int bch[24*1280], nbch;
      if ((nbch=myDet->getBadChannelCorrection(bch))) {
	if (sval!="none") {  
	  ofstream outfile;
	  outfile.open (sval.c_str(),ios_base::out);
	  if (outfile.is_open()) {
	    for (int ich=0; ich<nbch; ich++) {
	      outfile << bch[ich] << std::endl;
	    }
	    outfile.close();	
	    return sval;
	  } else 
	    std::cout<< "Could not open file " << sval << " for writing " << std::endl;
	}
      } 
    }
    return string(myDet->getBadChannelCorrectionFile());

}
 

string slsDetectorCommand::helpBadChannels(int narg, char *args[], int action){
 ostringstream os;  
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("badchannels [fn]\t  returns the badchannels file. Prints the list of bad channels in fn, if specified. \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("badchannels \t  sets the bad channels list\n");
   
  return os.str();
}


 
string slsDetectorCommand::cmdAngConv(int narg, char *args[], int action){ 

  if (action==HELP_ACTION) {
    return helpAngConv(narg, args, action);
  } 
  string sval;
  char answer[1000];
  double fval;
  angleConversionParameter c;

  if (string(args[0])==string("angconv")) {
    if (action==PUT_ACTION) {
      sval=string(args[1]);

      if (sval=="none")
	sval="";

      myDet->setAngularConversionFile(sval);

      return string(myDet->getAngularConversionFile());
    } else if (action==GET_ACTION) {
      if (narg>1)
	sval=string(args[1]);
      else
	sval="none";
      int dir;
      if (myDet->getAngularConversion(dir)) {
	if (sval!="none") {  
	  myDet->writeAngularConversion(sval.c_str());
	  return sval;
	}
	return string(myDet->getAngularConversionFile());
      } else {
	return string("none");
      }
    }
  } else if  (string(args[0])==string("globaloff")) {
    c=GLOBAL_OFFSET;


  } else if  (string(args[0])==string("fineoff")) {
    c=FINE_OFFSET;


  } else if  (string(args[0])==string("binsize")) {
    c=BIN_SIZE;

  } else if  (string(args[0])==string("angdir")) {
    c=ANGULAR_DIRECTION;

  } else if  (string(args[0])==string("moveflag")) {
    c=MOVE_FLAG;
  } else
    return string("could not decode angular conversion parameter ")+cmd;



    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%lf",&fval))
	myDet->setAngularConversionParameter(c,fval);
    } 
    sprintf(answer,"%f",myDet->getAngularConversionParameter(c));
    return string(answer); 


}
 

string slsDetectorCommand::helpAngConv(int narg, char *args[], int action){


  int t=0xffff;
  ostringstream os;
  
  if (string(args[0])==string("angconv")) {
    t=1;
  } else  if (string(args[0])==string("globaloff")) {
    t=2;
  } else  if (string(args[0])==string("fineoff")) {
    t=4;
  } else  if (string(args[0])==string("binsize")) {
    t=8;
  }
  if (t&1) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("angconv [fn]\t  returns the constants used for angular conversion prints them to the file fn if specified \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("angconv fn\t  sets the angualr conversion constants (none unsets) \n");
  } if (t&2) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("globaloff\t  returns the global offset used for angular conversion \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("globaloff f\t  sets the global offset used for the angular conversion \n");


  } if (t&4) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("fineoff\t  returns the fine offset used for angular conversion \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("fineoff f\t  sets the fine offset used for the angular conversion \n");



  } if (t&8) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("binsize\t  returns the bin size used for the angular conversion \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("binsize f\t  sets the bin size used for the angular conversion \n");




  }


  return os.str();
}


string slsDetectorCommand::cmdThreaded(int narg, char *args[], int action){
  int ival;
  char answer[1000];

  if (action==HELP_ACTION)
    return helpThreaded(narg,args,action);

  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&ival))
      myDet->setThreadedProcessing(ival);
  } 
  sprintf(answer,"%d",myDet->setThreadedProcessing());
  return string(answer); 
    
} 
   

string slsDetectorCommand::helpThreaded(int narg, char *args[], int action){
   ostringstream os;  
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("threaded \t  returns wether the data processing is threaded. \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("threaded t \t  sets the threading flag ( 1sets, 0 unsets).\n");
   
  return os.str();
  
} 
   

string slsDetectorCommand::cmdImage(int narg, char *args[], int action){
  string sval;
  int retval;
  if (action==HELP_ACTION)
    return helpImage(narg,args,HELP_ACTION);
  else if (action==GET_ACTION)
	  return string("Cannot get");

  sval=string(args[1]);
  myDet->setOnline(ONLINE_FLAG);


  if (string(args[0])==string("darkimage"))
	  retval=myDet->loadImageToDetector(DARK_IMAGE,sval);
  else if (string(args[0])==string("gainimage"))
	  retval=myDet->loadImageToDetector(GAIN_IMAGE,sval);

  
  if(retval==OK)
    return string("Image loaded succesfully");
  else
    return string("Image load failed");
}


string slsDetectorCommand::helpImage(int narg, char *args[], int action){
	ostringstream os;
	if (action==PUT_ACTION || action==HELP_ACTION){
		os << "darkimage f \t  loads the image to detector from file f"<< std::endl;
		os << "gainimage f \t  loads the image to detector from file f"<< std::endl;
	}
	if (action==GET_ACTION || action==HELP_ACTION){
		os << "darkimage \t  Cannot get"<< std::endl;
		os << "gainimage \t  Cannot get"<< std::endl;
	}
	return os.str();
}


string slsDetectorCommand::cmdCounter(int narg, char *args[], int action){
  int ival;
  string sval;
  int retval;
  if (action==HELP_ACTION)
    return helpCounter(narg,args,HELP_ACTION);
  else if (action==PUT_ACTION)
	  ival=atoi(args[1]);

  myDet->setOnline(ONLINE_FLAG);

 if (string(args[0])==string("readctr")){
	  if (action==PUT_ACTION)
		  return string("Cannot put");
	  else{
		  if (narg<3)
		    return string("should specify I/O file");
		  sval=string(args[2]);
		  retval=myDet->writeCounterBlockFile(sval,ival);
	  }
  }
  else if (string(args[0])==string("resetctr")){
	  if (action==GET_ACTION)
		  return string("Cannot get");
	  else
		  retval=myDet->resetCounterBlock(ival);
  }


  if(retval==OK)
    return string("Counter read/reset succesfully");
  else
    return string("Counter read/reset failed");
}


string slsDetectorCommand::helpCounter(int narg, char *args[], int action){
	ostringstream os;
	os << std::endl;
	if (action==PUT_ACTION || action==HELP_ACTION){
		os << "readctr \t  Cannot put"<< std::endl;
		os << "resetctr i \t  resets counter in detector, restarts acquisition if i=1"<< std::endl;
	}
	if (action==GET_ACTION || action==HELP_ACTION)
		os << "readctr i fname\t  reads counter in detector to file fname, restarts acquisition if i=1"<< std::endl;
		os << "resetctr \t  Cannot get"<< std::endl;
	return os.str();
}




string slsDetectorCommand::cmdPositions(int narg, char *args[], int action){
  int ival;
  int ip;

  char answer[1000];

  if (action==HELP_ACTION) return helpPositions(narg,args,action);

  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&ival)) {
      double pos[ival];
      for (ip=0; ip<ival;ip++) {  
	if ((2+ip)<narg) {
	  if (sscanf(args[2+ip],"%lf",pos+ip)) {
#ifdef VERBOSE
	    std::cout<< "Setting position " << ip <<" to " << pos[ip] <<  std::endl; 
#endif
	  } else
	    break;
	}
      }
      myDet->setPositions(ip,pos); 
    }
  }
  int npos=myDet->getPositions();
  sprintf(answer,"%d",npos);
  double opos[npos];
  myDet->getPositions(opos);
  for (int ip=0; ip<npos;ip++) {
    sprintf(answer,"%s %f",answer,opos[ip]);
  }
  return string(answer);
    
}

string slsDetectorCommand::helpPositions(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) 
    os << "positions np [pos0 pos1...posnp] \t sets the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION) 
    os << "positions  \t returns the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  return os.str();
}

string slsDetectorCommand::cmdScripts(int narg, char *args[], int action) {
  
  int ia=-1;
  char answer[100];
  if (action==HELP_ACTION)
    return helpScripts(narg,args,action);

  if (cmd.find("start")!=string::npos) ia=startScript;
  if (cmd.find("stop")!=string::npos) ia=stopScript;
  if (cmd.find("scriptbefore")!=string::npos) ia=scriptBefore;
  if (cmd.find("scriptafter")!=string::npos) ia=scriptAfter;
  if (cmd.find("headerafter")!=string::npos) ia=headerAfter;
  if (cmd.find("headerbefore")!=string::npos) ia=headerBefore;
  
  if (cmd.find("encallog")!=string::npos) ia=enCalLog;
  if (cmd.find("angcallog")!=string::npos) ia=angCalLog;



  if (ia==-1) return string("cannot define action ")+cmd;
  
  if (cmd.find("par")!=string::npos) {
    
    if (action==PUT_ACTION) {
      myDet->setActionParameter(ia, args[1]);
    }
    return string(myDet->getActionParameter(ia));
		  
  } else {

    if (ia==enCalLog || ia==angCalLog) {
      

      if (action==PUT_ACTION) {

	int arg=-1;


	sscanf(args[1],"%d",&arg);

	if (arg==0)
	  myDet->setActionScript(ia,"none");
	else
	  myDet->setActionScript(ia,args[1]);
	
      }

      sprintf(answer,"%d",myDet->getActionMode(ia));
      return string(answer);

    }


    if (action==PUT_ACTION) {
      myDet->setActionScript(ia, args[1]);
    }
    return string(myDet->getActionScript(ia));
    
  }
  return string("could not decode command")+cmd;




}

string slsDetectorCommand::helpScripts(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) 
    os << "positions np [pos0 pos1...posnp] \t sets the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION) 
    os << "positions  \t returns the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  return os.str();

}

string slsDetectorCommand::cmdScans(int narg, char *args[], int action) {

  int is=-1, ival, ns=0;
  char answer[MAX_SCAN_STEPS*10];
  double *values;
  if (action==HELP_ACTION)
    return helpScans(narg,args,action);


  if (cmd.find("0")!=string::npos) is=0;
  else if  (cmd.find("1")!=string::npos) is=1;
  else return string("cannot define scan level ")+cmd;

  if (cmd.find("par")!=string::npos) {
    if (action==PUT_ACTION) {
      myDet->setScanParameter(is, args[1]);
    }
    return myDet->getScanParameter(is);
  }
  if (cmd.find("script")!=string::npos) {
    if (action==PUT_ACTION) {
      myDet->setScanScript(is, args[1]);
    }
    return myDet->getScanScript(is);
  }
  if (cmd.find("prec")!=string::npos) {
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival)) {
	myDet->setScanPrecision(is, ival);
      } else
	return string("invalid precision ")+cmd;
    }
    sprintf(answer,"%d", myDet->getScanPrecision(is));
    return string(answer);
  }
  if (cmd.find("steps")!=string::npos) {

    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival)) {
      
	if (ival>MAX_SCAN_STEPS)
	  return string("too many steps required!");
      
	values=new double[ival];
	for (int i=0; i<ival; i++) {
	  if (narg>=(i+2)) {
	    if (sscanf(args[i+2],"%lf",values+i))
	      ns++;
	    else
	      break;
	  } else
	    break;
	}
	myDet->setScanSteps(is, ns, values);
	delete [] values;
      } else {
	return string("invalid number of steps ")+string(args[1]);
      }
    }
    ns=myDet->getScanSteps(is);
    values=new double[ns];
    ns=myDet->getScanSteps(is, values);
    int p=myDet->getScanPrecision(is);
    char format[1000];
    sprintf(format, "%%s %%0.%df",p);
    sprintf(answer,"%d ",ns);
    for (int i=0; i<ns; i++) {
      sprintf(answer,format,answer,values[i]);
    }
    delete [] values;
    return string(answer);
  }
  if (cmd.find("range")!=string::npos) {


    if (action==PUT_ACTION) {
      double fmin, fmax, fstep;
      if (narg<4)
	return string("wrong number of arguments ")+helpScans(narg,args,action);
      
      if (sscanf(args[1],"%lf",&fmin))
	;
      else
	return string("invalid scan minimum")+string(args[1]);
      if (sscanf(args[2],"%lf",&fmax))
	;
      else
	return string("invalid scan maximum")+string(args[2]);
      if (sscanf(args[3],"%lf",&fstep))
	;
      else
	return string("invalid scan step")+string(args[3]);


      if (fstep==0)
	return string("scan step cannot be 0!");

#ifdef VERBOSE
      cout << fmin << " " << fmax << " " << fstep << endl;
#endif
      
      ns=(int)((fmax-fmin)/fstep);
      if (ns<0)
	ns=-1*ns;
      ns++;
      
      if (ns>MAX_SCAN_STEPS)
	return string("too many steps required!");
      
      if (fmax>fmin)
	if (fstep<0)
	  fstep=-1*fstep;
      
      if (fmax<fmin)
	if (fstep>0)
	  fstep=-1*fstep;

      
      values=new double[ns];
      for (int i=0; i<ns; i++) {
	values[i]=fmin+i*fstep;
      }
      myDet->setScanSteps(is, ns, values);
      delete [] values;
    } 
    
    ns=myDet->getScanSteps(is);
    values=new double[ns];
    ns=myDet->getScanSteps(is, values);
    int p=myDet->getScanPrecision(is);
    char format[1000];
    sprintf(format, "%%s %%0.%df",p);
    sprintf(answer,"%d ",ns);
    for (int i=0; i<ns; i++) {
      sprintf(answer,format,answer,values[i]);
    }
    delete [] values;
    return string(answer);
  }
  return string("could not decode command")+cmd;
  
}

string slsDetectorCommand::helpScans(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) 
    os << "positions np [pos0 pos1...posnp] \t sets the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION) 
    os << "positions  \t returns the number of positions at which the detector is moved during the acquisition and their values"<< std::endl;
  return os.str();


}






string slsDetectorCommand::cmdNetworkParameter(int narg, char *args[], int action) {

  networkParameter t;

  if (action==HELP_ACTION)
    return helpNetworkParameter(narg,args,action);

  if (cmd=="clientip") {
    t=CLIENT_IP;
  } else if (cmd=="clientmac") {
    t=CLIENT_MAC;
  } else if (cmd=="servermac") {
    t=SERVER_MAC;
  } else return ("unknown network parameter")+cmd;
  
  if (action==PUT_ACTION)
    myDet->setNetworkParameter(t, args[1]);
  return myDet->getNetworkParameter(t);

}

string slsDetectorCommand::helpNetworkParameter(int narg, char *args[], int action) {



  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "clientip ip \n sets client ip to ip"<< std::endl;
    os << "clientmac mac \n sets client mac to mac"<< std::endl;
    os << "servermac mac \n sets server mac to mac"<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "clientip \n gets client ip "<< std::endl;
    os << "clientmac \n gets client mac "<< std::endl;
    os << "servermac \n gets server mac "<< std::endl;
  } 
  return os.str();


}

string slsDetectorCommand::cmdPort(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpPort(narg,args,action);
  int val; //ret,
  char ans[1000];
  portType index;

  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&val))
      ;
    else
      return string("could not scan port number")+string(args[1]);
  }

  if (cmd=="port") {
    index=CONTROL_PORT;
  } else if (cmd=="dataport") {
    index=DATA_PORT;
  } else if (cmd=="stopport") {
    index=STOP_PORT;
  } else
    return string("unknown port type ")+cmd;
  
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION)
    myDet->setPort(index,val);
  
  sprintf(ans,"%d",myDet->setPort(index));
  return string(ans);

}



string slsDetectorCommand::helpPort(int narg, char *args[], int action) {


  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "port i \n sets the communication control port"<< std::endl;
    os << "dataport i \n sets the communication data port"<< std::endl;
    os << "stopport i \n sets the communication stop port "<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "port  \n gets the communication control port"<< std::endl;
    os << "dataport  \n gets the communication data port"<< std::endl;
    os << "stopport \n gets the communication stop port "<< std::endl;
  } 
  return os.str();


}


string slsDetectorCommand::cmdLock(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpLock(narg,args,action);
  
  int val;//, ret;
  char ans[1000];
  
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&val))
      myDet->lockServer(val);
    else
      return string("could not lock status")+string(args[1]);
  }
  
  sprintf(ans,"%d",myDet->lockServer());
  return string(ans);
}



string slsDetectorCommand::helpLock(int narg, char *args[], int action) {


  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "lock i \n locks (1) or unlocks (0) the detector to communicate to this client"<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "lock \n returns the detector lock status"<< std::endl;
  } 
  return os.str();


}


string slsDetectorCommand::cmdLastClient(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpLastClient(narg,args,action);
  
  myDet->setOnline(ONLINE_FLAG);
 
  if (action==PUT_ACTION)
    return string("cannot set");
  
  return myDet->getLastClientIP();

}

string slsDetectorCommand::helpLastClient(int narg, char *args[], int action) {

  
  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "lastclient \n returns the last client communicating with the detector"<< std::endl;
  } 
  return os.str();

}


string slsDetectorCommand::cmdOnline(int narg, char *args[], int action) {

  if (action==HELP_ACTION) {
    return helpOnline(narg,args,action);
  }
  int ival;
  char ans[1000];

  if(cmd=="online"){
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival))
	myDet->setOnline(ival);
      else
	return string("Could not scan online mode ")+string(args[1]);
    }
    sprintf(ans,"%d",myDet->setOnline());
  }//"checkonline"
  else{
	  if (action==PUT_ACTION)
		  return string("cannot set");
	  strcpy(ans,myDet->checkOnline().c_str());
	  if(!strlen(ans))
		strcpy(ans,"All online");
	  else
		strcat(ans," :Not online");
  }
  return ans;
}

string slsDetectorCommand::helpOnline(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "online i \n sets the detector in online (1) or offline (0) mode"<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "online \n gets the detector online (1) or offline (0) mode"<< std::endl;
    os << "checkonline \n returns the hostnames of all detectors in offline mode"<< std::endl;
  } 
  return os.str();


}




string slsDetectorCommand::cmdConfigureMac(int narg, char *args[], int action) {

  if (action==HELP_ACTION) {
    return helpConfigureMac(narg,args,action);
  }
  int ival;
  int ret;
  char ans[1000];
  
  if (action==PUT_ACTION){
    if (sscanf(args[1],"%d",&ival))
      if(ival==1){
	myDet->setOnline(ONLINE_FLAG);
	ret=myDet->configureMAC();
      }
      else
	return string("Not yet implemented with arguments other than 1");
  }
  else
    return string("Cannot get ")+cmd;

  sprintf(ans,"%d",ret);
  return ans;
}

string slsDetectorCommand::helpConfigureMac(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << "configuremac i \n configures the MAC of the detector. i=1 for configure; i=0 for unconfigure(not implemented yet)"<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << "configuremac " << "Cannot get " << std::endl;
  
  return os.str();
}


string slsDetectorCommand::cmdDetectorSize(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpDetectorSize(narg,args,action);
  int ret, val=-1;
  char ans[1000];
  //  portType index;

  if (action==PUT_ACTION) {
    if (cmd=="maxmod")
      return string("cannot put!");

    if (sscanf(args[1],"%d",&val))
      ;
    else
      return string("could not scan ")+string(args[0])+string(" ")+string(args[1]);
  } 

  myDet->setOnline(ONLINE_FLAG);

  if (cmd=="nmod") {
    ret=myDet->setNumberOfModules(val);
  } else if (cmd=="maxmod") {
    ret=myDet->getMaxNumberOfModules();
  } else if (cmd=="dr") {
    ret=myDet->setDynamicRange(val);
  } else
    return string("unknown detector size ")+cmd;
  

  sprintf(ans,"%d",ret);
  return string(ans);

}


string slsDetectorCommand::helpDetectorSize(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "nmod i \n sets the number of modules of the detector"<< std::endl;
    os << "dr i \n sets the dynamic range of the detector"<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "nmod \n gets the number of modules of the detector"<< std::endl;
    os << "maxmod \n gets the maximum number of modules of the detector"<< std::endl;
    os << "dr \n gets the dynamic range of the detector"<< std::endl;
  } 
  return os.str();


}



string slsDetectorCommand::cmdSettings(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpSettings(narg,args,action);
  int val=-1;//ret, 
  char ans[1000];
  //  portType index;

 


//     if (sscanf(args[1],"%d",&val))
//       ;
//     else
//       return string("could not scan port number")+string(args[1]);
//   } 

  myDet->setOnline(ONLINE_FLAG);

  if (cmd=="settings") {
    if (action==PUT_ACTION) 
      myDet->setSettings(myDet->getDetectorSettings(string(args[1])));
    return myDet->getDetectorSettings(myDet->getSettings());
  } else if (cmd=="threshold") {
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&val))
	myDet->setThresholdEnergy(val);
      else
	return string("invalid threshold value ")+cmd;
    }
    sprintf(ans,"%d",myDet->getThresholdEnergy());
    return string(ans);
  } else if (cmd=="trimbits") {
    if (narg>=2) {
      string sval=string(args[1]);
#ifdef VERBOSE
      std::cout<< " trimfile " << sval << std::endl;
#endif 
      if (action==GET_ACTION) {
	//create file names
	myDet->saveSettingsFile(sval, -1);
      } else if (action==PUT_ACTION) {
	myDet->loadSettingsFile(sval,-1);
      }
    }
    return string(myDet->getSettingsFile());
  } else if (cmd=="trim") { 
    if (action==GET_ACTION) 
      return string("cannot get!");

      trimMode mode=NOISE_TRIMMING;
      int par1=0, par2=0;
      if (string(args[0]).find("trim:")==string::npos)
	return helpSettings(narg,args,action);
      else if  (string(args[0]).find("noise")!=string::npos) {
	// par1 is countlim; par2 is nsigma
	mode=NOISE_TRIMMING;
	par1=500;
	par2=4;
      } else if  (string(args[0]).find("beam")!=string::npos){
	// par1 is countlim; par2 is nsigma
	mode=BEAM_TRIMMING;
	par1=1000;
	par2=4;
      } else if  (string(args[0]).find("improve")!=string::npos) {
	// par1 is maxit; if par2!=0 vthresh will be optimized
	mode=IMPROVE_TRIMMING;
	par1=5;
	par2=0;
      } else if  (string(args[0]).find("fix")!=string::npos)  {
	// par1 is countlim; if par2<0 then trimwithlevel else trim with median 
	mode=FIXEDSETTINGS_TRIMMING;
	par1=1000;
	par2=1;
	// }else if  (string(args[0]).find("fix")!=string::npos) {
	//mode=OFFLINE_TRIMMING;
      } else {
	return string("Unknown trim mode ")+cmd;
      } 
      myDet->executeTrimming(mode, par1, par2);
      string sval=string(args[1]);
      myDet->saveSettingsFile(sval, -1);
      return string("done");
    
  }
  return string("unknown settings command ")+cmd;
  
}


string slsDetectorCommand::helpSettings(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "settings s \n sets the settings of the detector - can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain"<< std::endl;
    os << "threshold eV\n sets the detector threshold in eV"<< std::endl;
    os << "trimbits fname\n loads the trimfile fname to the detector. If no extension is specified, the serial number of each module will be attached."<< std::endl;
    os << "trim:mode fname\n trims the detector according to mode (can be noise, beam, improve, fix) and saves the resulting trimbits to file fname."<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "settings \n gets the settings of the detector"<< std::endl;
    os << "threshold V\n gets the detector threshold"<< std::endl;
    os << "trimbits [fname]\n returns the trimfile loaded on the detector. If fname is specified the trimbits are saved to file. If no extension is specified, the serial number of each module will be attached."<< std::endl;
  } 
  return os.str();


}










string slsDetectorCommand::cmdSN(int narg, char *args[], int action) {

  char answer[1000];


  if (action==PUT_ACTION)
    return string("cannot set");


  if (action==HELP_ACTION)
    return helpSN(narg, args, action);


  if (cmd=="thisversion"){
    sprintf(answer,"%llx",myDet->getId(THIS_SOFTWARE_VERSION));
    return string(answer);
  }

  myDet->setOnline(ONLINE_FLAG);


  if (cmd=="moduleversion") {
    int ival=-1;
    if (sscanf(args[0],"moduleversion:%d",&ival)) {
      sprintf(answer,"%llx",myDet->getId(MODULE_FIRMWARE_VERSION,ival));
      return string(answer);
    } else
      return string("undefined module number");
  }
  if (cmd=="detectornumber") {
    sprintf(answer,"%llx",myDet->getId(DETECTOR_SERIAL_NUMBER));
    return string(answer);
  }
  if (cmd.find("modulenumber")!=string::npos) {
    int ival=-1;
    if (sscanf(args[0],"modulenumber:%d",&ival)) {
      sprintf(answer,"%llx",myDet->getId(MODULE_SERIAL_NUMBER,ival));
      return string(answer);
    } else
      return string("undefined module number");
  }

  if (cmd=="detectorversion") {
    sprintf(answer,"%llx",myDet->getId(DETECTOR_FIRMWARE_VERSION));
    return string(answer);
  }
  
  if (cmd=="softwareversion") {
    sprintf(answer,"%llx",myDet->getId(DETECTOR_SOFTWARE_VERSION));
    return string(answer);
  }

  return string("unknown id mode ")+cmd;

}

string slsDetectorCommand::helpSN(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "moduleversion:i \n gets the firmwareversion of the module i"<< std::endl;
    os << "modulenumber:i \n gets the serial number of the module i"<< std::endl;
    os << "detectornumber \n gets the serial number of the detector (MAC)"<< std::endl;
    os << "detectorversion \n gets the firmware version of the detector"<< std::endl;
    os << "softwareversion \n gets the software version of the detector"<< std::endl;
    os << "thisversion \n gets the version of this software"<< std::endl;
  } 
  return os.str();



}


string slsDetectorCommand::cmdDigiTest(int narg, char *args[], int action) {

  char answer[1000];

  if (action==HELP_ACTION)
	  return helpSN(narg, args, action);


  myDet->setOnline(ONLINE_FLAG);

  if (cmd=="bustest"){
    if (action==PUT_ACTION)
      return string("cannot set ")+cmd;
    sprintf(answer,"%x",myDet->digitalTest(DETECTOR_BUS_TEST));
    return string(answer);
  } 

  if (cmd=="digitest") {
    if (action==PUT_ACTION)
      return string("cannot set ")+cmd;
    int ival=-1;
    if (sscanf(args[0],"digitest:%d",&ival)) {
      sprintf(answer,"%x",myDet->digitalTest(CHIP_TEST, ival));
      return string(answer);
    } else
      return string("undefined module number");
  }

  if (cmd=="digibittest") {
    if (action==GET_ACTION)
      return string("cannot get ")+cmd;
    int ival=-1;
    if (sscanf(args[1],"%d",&ival)) {
      if((ival==0)||(ival==1)){
	sprintf(answer,"%x",myDet->digitalTest(DIGITAL_BIT_TEST,ival));
	return string(answer);
      }
      else
	return string("Use only 0 or 1 to set/clear digital test bit\n");
    } else
      return string("undefined number");
  }

  if (cmd=="acqtest") {
    if (action==GET_ACTION)
      return string("cannot get ")+cmd;
    int ival=-1;
    if (sscanf(args[1],"%d",&ival)) {
      if(ival<1)
	return helpDigiTest(narg, args, action);
      else {
	sprintf(answer,"%x",myDet->testFunction(ival));
	return string(answer);
      }
    } else
      return string("undefined number");
  }
  
  return string("unknown digital test mode ")+cmd;

}

string slsDetectorCommand::helpDigiTest(int narg, char *args[], int action) {


  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "digitaltest:i \t performs digital test of the module i. Returns 0 if succeeded, otherwise error mask."<< std::endl;
    os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes."<< std::endl;
  }
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "digibittest i\t sets a variable in the server to be used in configuremac function. i sets/clears the digital test bit."<< std::endl;
    os << "acqtest i\t runs start acquisition i number of times."<< std::endl;
  } 
  return os.str();
}

string slsDetectorCommand::cmdRegister(int narg, char *args[], int action) {
  

  if (action==HELP_ACTION)
    return helpRegister(narg, args, action);

  int addr, val;
  char answer[1000];

  myDet->setOnline(ONLINE_FLAG);
  
  if (action==PUT_ACTION) {
    if(narg<3) 
      return string("wrong usage: should specify both address and value (hexadecimal fomat) ");
    if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan address  (hexadecimal fomat) ")+string(args[1]);
    
    if (sscanf(args[2],"%x",&val))
      ;
    else
      return string("Could not scan value  (hexadecimal fomat) ")+string(args[2]);
    
    
    sprintf(answer,"%x",myDet->writeRegister(addr,val));
    
  } else {
      if (narg<2) 
	return string("wrong usage: should specify address  (hexadecimal fomat) ");
      if (sscanf(args[1],"%x",&addr))
	;
      else
	return string("Could not scan address  (hexadecimal fomat) ")+string(args[1]);
	
	
      sprintf(answer,"%x",myDet->readRegister(addr));
      
  }

  return string(answer);


}

string slsDetectorCommand::helpRegister(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "reg addr val \n writes the register addr with the value val"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "reg addr \n reads the register addr"<< std::endl;
  } 
  return os.str();



}


string slsDetectorCommand::cmdDAC(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpDAC(narg, args, action);
  
  dacIndex dac;
  dacs_t val=-1;
  char answer[1000];
  
  if (cmd=="vthreshold")
    dac=THRESHOLD;
  else if (cmd=="vcalibration")
    dac=CALIBRATION_PULSE;
  else if (cmd=="vtrimbit")
    dac=TRIMBIT_SIZE;
  else if (cmd=="vpreamp")
    dac=PREAMP;
  else if (cmd=="vshaper1")
    dac=SHAPER1;
  else if (cmd=="vshaper2")
    dac=SHAPER2;
  else if (cmd=="vhighvoltage")
    dac=HV_POT;
  else if (cmd=="vapower")
    dac=VA_POT;
  else if (cmd=="vddpower")
    dac=VDD_POT;
  else if (cmd=="vshpower")
    dac=VSH_POT;
  else if (cmd=="viopower")
    dac=VIO_POT;
  else if (cmd=="vref_ds")
    dac=G_VREF_DS;
  else if (cmd=="vcascn_pb")
    dac=G_VCASCN_PB;
  else if (cmd=="vcascp_pb")
    dac=G_VCASCP_PB;
  else if (cmd=="vout_cm")
    dac=G_VOUT_CM;
  else if (cmd=="vcasc_out")
    dac=G_VCASC_OUT;
  else if (cmd=="vin_cm")
    dac=G_VIN_CM;
  else if (cmd=="vref_comp")
    dac=G_VREF_COMP;
  else if (cmd=="ib_test_c")
    dac=G_IB_TESTC;
  else if (cmd=="temp_adc") {
    dac=TEMPERATURE_ADC;
    if (action==PUT_ACTION)
      return string("cannot set ")+cmd;
  } else if (cmd=="temp_fpga") {
    dac=TEMPERATURE_FPGA;
    if (action==PUT_ACTION)
      return string("cannot set ")+cmd;
  } else
    return string("cannot decode dac ")+cmd;
  
  if (action==PUT_ACTION) {
#ifdef DACS_INT
    if (sscanf(args[1],"%d", &val))
#else
    if (sscanf(args[1],"%f", &val))
#endif
      ;
    else
      return string("cannot scan DAC value ")+string(args[1]);
  }
  
  myDet->setOnline(ONLINE_FLAG);
#ifdef DACS_INT
  sprintf(answer,"%d",myDet->setDAC(val,dac));
#else
  sprintf(answer,"%f",myDet->setDAC(val,dac));
#endif
  return string(answer);

}


string slsDetectorCommand::helpDAC(int narg, char *args[], int action) {

	ostringstream os;
	if (action==PUT_ACTION || action==HELP_ACTION) {
		os << "vthreshold dacu\t sets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
		os << std::endl;

		os << "vcalibration " << "dacu\t sets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vtrimbit " << "dacu\t sets the trimbit amplitude in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vpreamp " << "dacu\t sets the preamp feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshaper1 " << "dacu\t sets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshaper2 " << "dacu\t sets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vhighvoltage " << "dacu\t CHIPTEST BOARD ONLY - sets the detector HV in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vapower " << "dacu\t CHIPTEST BOARD ONLY - sets the analog power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vddpower " << "dacu\t CHIPTEST BOARD ONLY - sets the digital power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshpower " << "dacu\t CHIPTEST BOARD ONLY - sets the comparator power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "viopower " << "dacu\t CHIPTEST BOARD ONLY - sets the FPGA I/O power supply in dac units (0-1024)." << std::endl;



		os << "vrefds " << "dacu\t sets vrefds" << std::endl;
		os << "vcascn_pb " << "dacu\t sets vcascn_pb" << std::endl;
		os << "vcascp_pb " << "dacu\t sets vcascp_pb" << std::endl;
		os << "vout_cm " << "dacu\t sets vout_cm" << std::endl;
		os << "vin_cm " << "dacu\t sets vin_cm" << std::endl;
		os << "vcasc_out " << "dacu\t sets vcasc_out" << std::endl;
		os << "vref_comp " << "dacu\t sets vref_comp" << std::endl;
		os << "ib_test_c " << "dacu\t sets ib_test_c" << std::endl;

	}
	if (action==GET_ACTION || action==HELP_ACTION) {

		os << "vthreshold \t Gets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
		os << std::endl;

		os << "vcalibration " << "dacu\t gets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vtrimbit " << "dacu\t gets the trimbit amplitude in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vpreamp " << "dacu\t gets the preamp feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshaper1 " << "dacu\t gets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshaper2 " << "dacu\t gets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vhighvoltage " << "dacu\t CHIPTEST BOARD ONLY - gets the detector HV in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vapower " << "dacu\t CHIPTEST BOARD ONLY - gets the analog power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vddpower " << "dacu\t CHIPTEST BOARD ONLY - gets the digital power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "vshpower " << "dacu\t CHIPTEST BOARD ONLY - gets the comparator power supply in dac units (0-1024)." << std::endl;
		os << std::endl;
		os << "viopower " << "dacu\t CHIPTEST BOARD ONLY - gets the FPGA I/O power supply in dac units (0-1024)." << std::endl;
		os << std::endl;


		os << "vrefds " << "\t gets vrefds" << std::endl;
		os << "vcascn_pb " << "\t gets vcascn_pb" << std::endl;
		os << "vcascp_pb " << "\t gets vcascp_pb" << std::endl;
		os << "vout_cm " << "\t gets vout_cm" << std::endl;
		os << "vin_cm " << "\t gets vin_cm" << std::endl;
		os << "vcasc_out " << "\t gets vcasc_out" << std::endl;
		os << "vref_comp " << "\t gets vref_comp" << std::endl;
		os << "ib_test_c " << "\t gets ib_test_c" << std::endl;
	}
	return os.str();
}



string slsDetectorCommand::cmdADC(int narg, char *args[], int action) {

  dacIndex adc;
  //  double val=-1;
  char answer[1000];
  
  if (action==HELP_ACTION)
    return helpADC(narg, args, action);
  else if (action==PUT_ACTION)
    return string("cannot set ")+cmd;
  
  if (cmd=="temp_adc")
    adc=TEMPERATURE_ADC;
  else if (cmd=="temp_fpga")
    adc=TEMPERATURE_FPGA;
  else
    return string("cannot decode adc ")+cmd;

  myDet->setOnline(ONLINE_FLAG);
#ifdef DACS_INT
  sprintf(answer,"%d",myDet->getADC(adc));
#else
  sprintf(answer,"%f",myDet->getADC(adc));
#endif
  return string(answer);

}

string slsDetectorCommand::helpADC(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "temp_adc " << "Cannot be set" << std::endl;
    os << "temp_fpga " << "Cannot be set" << std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "temp_adc " << "\t gets the temperature of the adc" << std::endl;
    os << "temp_fpga " << "\t gets the temperature of the fpga" << std::endl;
  }
  return os.str();
}

string slsDetectorCommand::cmdTiming(int narg, char *args[], int action){
#ifdef VERBOSE
  cout << string("Executing command ")+string(args[0])+string(" ( ")+cmd+string(" )\n");
#endif

  if (action==HELP_ACTION) {
    return helpTiming(narg,args,HELP_ACTION);
  } 
  myDet->setOnline(ONLINE_FLAG);
  if (action==PUT_ACTION) {
    if (myDet->externalCommunicationType(string(args[1]))== GET_EXTERNAL_COMMUNICATION_MODE) return helpTiming(narg,args, action);
    myDet->setExternalCommunicationMode(myDet->externalCommunicationType(string(args[1])));
  }
  return myDet->externalCommunicationType(myDet->setExternalCommunicationMode());

}
string slsDetectorCommand::helpTiming(int narg, char *args[], int action){
  
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("sync \t gets the synchronization mode of the structure\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("sync mode \t sets synchronization mode of the structure. Cane be none, gating, trigger, complementary \n");
  return os.str();
}



string slsDetectorCommand::cmdTimer(int narg, char *args[], int action) {
  timerIndex index; 
  int64_t t=-1, ret;
  double val, rval;
  
  char answer[1000];

  
  if (action==HELP_ACTION)
    return helpTimer(narg, args, action);
  
  if (cmd=="exptime")
    index=ACQUISITION_TIME;
  else if (cmd=="period")
    index=FRAME_PERIOD;
  else if (cmd=="delay")
    index=DELAY_AFTER_TRIGGER;
  else if (cmd=="gates")
    index=GATES_NUMBER;
  else if (cmd=="frames")
    index=FRAME_NUMBER;
  else if (cmd=="cycles")
    index=CYCLES_NUMBER;
  else if (cmd=="probes")
    index=PROBES_NUMBER;
  else if (cmd=="measurments")
    index=MEASUREMENTS_NUMBER;
  else
    return string("could not decode timer ")+cmd;

  
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%lf", &val))
      ;
    else
      return string("cannot scan timer value ")+string(args[1]);
    if (index==ACQUISITION_TIME || index==FRAME_PERIOD || index==DELAY_AFTER_TRIGGER)
      t=(int64_t)(val*1E+9);
    else t=(int64_t)val;
  }
  
  
  myDet->setOnline(ONLINE_FLAG);

  ret=myDet->setTimer(index,t);
  if (index==ACQUISITION_TIME || index==FRAME_PERIOD || index==DELAY_AFTER_TRIGGER)
    rval=(double)ret*1E-9;
  else rval=ret;
  

  sprintf(answer,"%0.9f",rval);
  return string(answer);
  



}


string slsDetectorCommand::helpTimer(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "exptime t \t sets the exposure time in s" << std::endl;
    os << "period t \t sets the frame period in s" << std::endl;
    os << "delay t \t sets the delay after trigger in s" << std::endl;
    os << "frames t \t sets the number of frames per cycle (e.g. after each trigger)" << std::endl;
    os << "cycles t \t sets the number of cycles (e.g. number of triggers)" << std::endl;
    os << "probes t \t sets the number of probes to accumulate (max 3! cycles should be set to 1, frames to the number of pump-probe events)" << std::endl;
    os << std::endl;


  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    
    os << "exptime  \t gets the exposure time in s" << std::endl;
    os << "period  \t gets the frame period in s" << std::endl;
    os << "delay  \t gets the delay after trigger in s" << std::endl;
    os << "frames  \t gets the number of frames per cycle (e.g. after each trigger)" << std::endl;
    os << "cycles  \t gets the number of cycles (e.g. number of triggers)" << std::endl;
    os << "probes  \t gets the number of probes to accumulate" << std::endl;
    os << std::endl;

  } 
  return os.str();








}


string slsDetectorCommand::cmdTimeLeft(int narg, char *args[], int action) {
  timerIndex index; 
  int64_t ret;
  double rval;
  
  char answer[1000];

  
  if (action==HELP_ACTION)
    return helpTimeLeft(narg, args, action);
  
  if (cmd=="exptimel")
    index=ACQUISITION_TIME;
  else if (cmd=="periodl")
    index=FRAME_PERIOD;
  else if (cmd=="delayl")
    index=DELAY_AFTER_TRIGGER;
  else if (cmd=="gatesl")
    index=GATES_NUMBER;
  else if (cmd=="framesl")
    index=FRAME_NUMBER;
  else if (cmd=="cyclesl")
    index=CYCLES_NUMBER;
  else if (cmd=="probesl")
    index=PROBES_NUMBER;
  else if (cmd=="now")
    index=ACTUAL_TIME;
  else if (cmd=="timestamp")
    index=MEASUREMENT_TIME;
  else
    return string("could not decode timer ")+cmd;

  
  if (action==PUT_ACTION) {
      return string("cannot set ")+string(args[1]);
  }
  
  

  
  myDet->setOnline(ONLINE_FLAG);

  ret=myDet->getTimeLeft(index);

  if (index==ACQUISITION_TIME || index==FRAME_PERIOD || index==DELAY_AFTER_TRIGGER || index==ACTUAL_TIME || index==MEASUREMENT_TIME)
    rval=(double)ret*1E-9;
  else rval=ret;
  

  sprintf(answer,"%0.9f",rval);
  return string(answer);
  
  



}

  
string slsDetectorCommand::helpTimeLeft(int narg, char *args[], int action) {
  

  
  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    
    os << "exptimel  \t gets the exposure time left" << std::endl;
    os << "periodl \t gets the frame period left" << std::endl;
    os << "delayl  \t gets the delay left" << std::endl;
    os << "framesl  \t gets the number of frames left" << std::endl;
    os << "cyclesl  \t gets the number of cycles left" << std::endl;
    os << "probesl  \t gets the number of probes left" << std::endl;
    os << std::endl;

  } 
  return os.str();




}







string slsDetectorCommand::cmdSpeed(int narg, char *args[], int action) {

  speedVariable index; 
  int t=-1, ret;
  
  char answer[1000];

  
  if (action==HELP_ACTION)
    return helpSpeed(narg, args, action);
  
  if (cmd=="clkdivider")
    index=CLOCK_DIVIDER;
  else if (cmd=="setlength")
    index=SET_SIGNAL_LENGTH;
  else if (cmd=="waitstates")
    index=WAIT_STATES;
  else if (cmd=="totdivider")
    index=TOT_CLOCK_DIVIDER;
  else if (cmd=="totdutycycle")
    index=TOT_DUTY_CYCLE;
  else
    return string("could not decode speed variable ")+cmd;

  
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d", &t))
      ;
    else
      return string("cannot scan speed value ")+string(args[1]);
  }
  
  
  myDet->setOnline(ONLINE_FLAG);

  ret=myDet->setSpeed(index,t);
  

  sprintf(answer,"%d",ret);
  return string(answer);
  



}


string slsDetectorCommand::helpSpeed(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {

    os << "clkdivider c  \t sets readout clock divider" << std::endl;
    os << "setlength  c\t sets the length of the set/reset signals (in clock cycles)" << std::endl;
    os << "waitstates c \t sets the waitstates of the bus interface" << std::endl;
    os << "totdivider  c\t sets the clock divider in tot mode" << std::endl;
    os << "totdutycycle  c\t sets the duty cycle of the tot clock" << std::endl;
    os << std::endl;

  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    
    os << "clkdivider  \t gets readout clock divider" << std::endl;
    os << "setlength  \t gets the length of the set/reset signals (in clock cycles)" << std::endl;
    os << "waitstates  \t gets the waitstates of the bus interface" << std::endl;
    os << "totdivider \t gets the clock divider in tot mode" << std::endl;
    os << "totdutycycle \t gets the duty cycle of the tot clock" << std::endl;
    os << std::endl;

  } 
  return os.str();








}




string slsDetectorCommand::cmdAdvanced(int narg, char *args[], int action) {


  
  if (action==HELP_ACTION)
    return helpAdvanced(narg, args, action);
  
  if (cmd=="flags") {

    readOutFlags flag=GET_READOUT_FLAGS;

    if (action==PUT_ACTION) {
      string sval=string(args[1]);
      if (sval=="none")
	flag=NORMAL_READOUT;
      else if (sval=="storeinram")
	flag=STORE_IN_RAM;
      else if (sval=="tot")
	flag=TOT_MODE;
      else if (sval=="continous")
	flag=CONTINOUS_RO;
      else
	return string("could not scan flag ")+string(args[1]);
    }
  

    myDet->setOnline(ONLINE_FLAG);

    switch (myDet->setReadOutFlags(flag)) {
    case NORMAL_READOUT:
      return string("none");
    case STORE_IN_RAM:
      return string("storeinram");
    case TOT_MODE:
      return string("tot");
    case CONTINOUS_RO:
      return string("continous");
    default:
      return string("unknown");
    }  
  
  }  else if (cmd=="extsig") {
    externalSignalFlag flag=GET_EXTERNAL_SIGNAL_FLAG;
    int is=-1;
    if (sscanf(args[0],"extsig:%d",&is))
      ;
    else
      return string("could not scan signal number ")+string(args[0]);
	
    if (action==PUT_ACTION) {
      flag=myDet->externalSignalType(args[1]);
      if (flag==GET_EXTERNAL_SIGNAL_FLAG)
	return string("could not scan external signal mode ")+string(args[1]);
    }
    myDet->setOnline(ONLINE_FLAG);
    
    return myDet->externalSignalType(myDet->setExternalSignalFlags(flag,is));

  } else
    return string("could not decode flag ")+cmd;

}


string slsDetectorCommand::helpAdvanced(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {

    os << "extsig:i mode \t sets the mode of the external signal i. can be  \n \t \t \t off, \n \t \t \t gate_in_active_high, \n \t \t \t gate_in_active_low, \n \t \t \t trigger_in_rising_edge, \n \t \t \t trigger_in_falling_edge, \n \t \t \t ro_trigger_in_rising_edge, \n \t \t \t ro_trigger_in_falling_edge, \n \t \t \t gate_out_active_high, \n \t \t \t gate_out_active_low, \n \t \t \t trigger_out_rising_edge, \n \t \t \t trigger_out_falling_edge, \n \t \t \t ro_trigger_out_rising_edge, \n \t \t \t ro_trigger_out_falling_edge" << std::endl;
    os << "flags mode \t sets the readout flags to mode. can be none, storeinram, tot, continous, unknown" << std::endl;
    
  }
  if (action==GET_ACTION || action==HELP_ACTION) {

    os << "extsig:i \t gets the mode of the external signal i. can be  \n \t \t \t off, \n \t \t \t gate_in_active_high, \n \t \t \t gate_in_active_low, \n \t \t \t trigger_in_rising_edge, \n \t \t \t trigger_in_falling_edge, \n \t \t \t ro_trigger_in_rising_edge, \n \t \t \t ro_trigger_in_falling_edge, \n \t \t \t gate_out_active_high, \n \t \t \t gate_out_active_low, \n \t \t \t trigger_out_rising_edge, \n \t \t \t trigger_out_falling_edge, \n \t \t \t ro_trigger_out_rising_edge, \n \t \t \t ro_trigger_out_falling_edge" << std::endl;
    os << "flags \t gets the readout flags. can be none, storeinram, tot, continous, unknown" << std::endl;

  } 
  return os.str();








}




string slsDetectorCommand::cmdConfiguration(int narg, char *args[], int action) {


  
  if (action==HELP_ACTION)
    return helpConfiguration(narg, args, action);
  
  string sval;
  
  if (narg<2)
    return string("should specify I/O file");

  myDet->setOnline(ONLINE_FLAG);

  if (cmd=="config") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
     myDet->readConfigurationFile(sval);
    } else if (action==GET_ACTION) {
      sval=string(args[1]);
      myDet->writeConfigurationFile(sval);
    }  
    return sval;
  } else if (cmd=="parameters") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
     myDet->retrieveDetectorSetup(sval);
    } else if (action==GET_ACTION) {
     sval=string(args[1]);
     myDet->dumpDetectorSetup(sval);
    }  
    return sval;
  } else if (cmd=="setup") {
    if (action==PUT_ACTION) {
     sval=string(args[1]);
     myDet->retrieveDetectorSetup(sval,2);
    } else if (action==GET_ACTION) {
     sval=string(args[1]);
     myDet->dumpDetectorSetup(sval,2);
    }  
    return sval;
  }
  return string("could not decode conf mode");


}


string slsDetectorCommand::helpConfiguration(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {

    os << "config fname \t sets the detector to the configuration contained in fname" << std::endl;
    os << "parameters fname \t sets the detector parameters to those contained in fname" << std::endl;
    os << "setup fname \t sets the detector complete detector setup to that contained in fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    
  }
  if (action==GET_ACTION || action==HELP_ACTION) {

    os << "config fname \t saves the detector to the configuration to fname" << std::endl;
    os << "parameters fname \t saves the detector parameters to  fname" << std::endl;
    os << "setup fname \t saves the detector complete detector setup to  fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    
  } 

  return os.str();








}


