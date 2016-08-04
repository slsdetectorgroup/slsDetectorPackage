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

  descrToFuncMap[i].m_pFuncName="exitreceiver";//OK
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
 
  descrToFuncMap[i].m_pFuncName="samplex" ;//
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAngConv;
  i++;


  descrToFuncMap[i].m_pFuncName="sampley" ;//
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

  descrToFuncMap[i].m_pFuncName="resmat"; //
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

  descrToFuncMap[i].m_pFuncName="activate"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOnline;
  i++;

  descrToFuncMap[i].m_pFuncName="enablefwrite"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdEnablefwrite;
  i++;

  descrToFuncMap[i].m_pFuncName="overwrite"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOverwrite;
  i++;

  descrToFuncMap[i].m_pFuncName="currentfname"; //OK
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdFileName;
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

  descrToFuncMap[i].m_pFuncName="rx_hostname"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_udpip"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_udpmac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_udpport"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_udpport2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="detectormac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="detectorip"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="txndelay_left"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="txndelay_right"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="txndelay_frame"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="flowcontrol_10g"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdNetworkParameter;
  i++;

  descrToFuncMap[i].m_pFuncName="configuremac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfigureMac;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_tcpport"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPort;
  i++;

  descrToFuncMap[i].m_pFuncName="port"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPort;
  i++;

  descrToFuncMap[i].m_pFuncName="stopport"; //
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

  descrToFuncMap[i].m_pFuncName="roi"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
  i++;

  descrToFuncMap[i].m_pFuncName="detsizechan"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
  i++;

  descrToFuncMap[i].m_pFuncName="roimask"; //
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

  descrToFuncMap[i].m_pFuncName="receiverversion"; //
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

  descrToFuncMap[i].m_pFuncName="reg"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRegister;
  i++;

  descrToFuncMap[i].m_pFuncName="adcreg"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRegister;
  i++;

  descrToFuncMap[i].m_pFuncName="setbit"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRegister;
  i++;

  descrToFuncMap[i].m_pFuncName="clearbit"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdRegister;
  i++;

  descrToFuncMap[i].m_pFuncName="getbit"; //
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

  descrToFuncMap[i].m_pFuncName="trimval"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSettings;
  i++;

  descrToFuncMap[i].m_pFuncName="pedestal"; //
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

  descrToFuncMap[i].m_pFuncName="ib_test_c"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac0"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac3"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac4"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac5"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac6"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="dac7"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vsvp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vsvn"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vtr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vrf"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vrs"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vtgstv"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcmp_ll"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcmp_lr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcall"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcmp_rl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcmp_rr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="rxb_rb"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="rxb_lb"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vcn"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="vis"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;

  descrToFuncMap[i].m_pFuncName="iodelay"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;


  descrToFuncMap[i].m_pFuncName="dac"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;



  descrToFuncMap[i].m_pFuncName="adcvpp"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDAC;
  i++;


  /* r/w timers */

  descrToFuncMap[i].m_pFuncName="temp_adc"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_fpga"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_fpgaext"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_10ge"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_dcdc"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_sodl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  descrToFuncMap[i].m_pFuncName="temp_sodr"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdADC;
  i++;

  /* r/w timers */

  descrToFuncMap[i].m_pFuncName="timing"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTiming;
  i++;

  descrToFuncMap[i].m_pFuncName="exptime"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdTimer;
  i++;

  descrToFuncMap[i].m_pFuncName="subexptime"; //
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


  descrToFuncMap[i].m_pFuncName="measurements"; //
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

  descrToFuncMap[i].m_pFuncName="nframes"; //
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

  descrToFuncMap[i].m_pFuncName="phasestep"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="oversampling"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="adcclk"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;

  descrToFuncMap[i].m_pFuncName="adcphase"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;


  descrToFuncMap[i].m_pFuncName="adcpipeline"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdSpeed;
  i++;



  /* settings dump/retrieve */
  descrToFuncMap[i].m_pFuncName="config"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_printconfig";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  descrToFuncMap[i].m_pFuncName="parameters"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;

  descrToFuncMap[i].m_pFuncName="setup"; 
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdConfiguration;
  i++;


  /* receiver functions */
  descrToFuncMap[i].m_pFuncName="receiver";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="r_online";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOnline;
  i++;

  descrToFuncMap[i].m_pFuncName="r_checkonline";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdOnline;
  i++;

  descrToFuncMap[i].m_pFuncName="framescaught";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="resetframescaught";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="frameindex";
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="r_lock"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdLock;
  i++;

  descrToFuncMap[i].m_pFuncName="r_lastclient"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdLastClient;
  i++;

  descrToFuncMap[i].m_pFuncName="r_readfreq"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="r_compression"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="tengiga"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  descrToFuncMap[i].m_pFuncName="rx_fifodepth"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdReceiver;
  i++;

  /* pattern generator */

 
  descrToFuncMap[i].m_pFuncName="adcinvert"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;
 
  descrToFuncMap[i].m_pFuncName="adcdisable"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;
 
  descrToFuncMap[i].m_pFuncName="pattern"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;
 

  descrToFuncMap[i].m_pFuncName="patword"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patioctrl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  descrToFuncMap[i].m_pFuncName="patclkctrl"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  descrToFuncMap[i].m_pFuncName="patlimits"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  descrToFuncMap[i].m_pFuncName="patloop0"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patnloop0"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  descrToFuncMap[i].m_pFuncName="patwait0"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

 
  descrToFuncMap[i].m_pFuncName="patwaittime0"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patloop1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patnloop1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  descrToFuncMap[i].m_pFuncName="patwait1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

 
  descrToFuncMap[i].m_pFuncName="patwaittime1"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patloop2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;


  descrToFuncMap[i].m_pFuncName="patnloop2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  descrToFuncMap[i].m_pFuncName="patwait2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

 
  descrToFuncMap[i].m_pFuncName="patwaittime2"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPattern;
  i++;

  
  /* pulse */
  
  descrToFuncMap[i].m_pFuncName="pulse"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPulse;
  i++;

  descrToFuncMap[i].m_pFuncName="pulsenmove"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPulse;
  i++;

  descrToFuncMap[i].m_pFuncName="pulsechip"; //
  descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdPulse;
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
      os << descrToFuncMap[i].m_pFuncName << "\n" ;
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

	if(myDet->acquire() == FAIL)
		return string("acquire unsuccessful");
	if(myDet->setReceiverOnline()==ONLINE_FLAG){
		char answer[100];
	      sprintf(answer,"\n%d",myDet->getFramesCaughtByReceiver());
	      return string(answer);
	}

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
    //processdata in receiver is useful only for gui purposes
    if(myDet->setReceiverOnline()==OFFLINE_FLAG)
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
    //processdata in receiver is useful only for gui purposes
    if(myDet->setReceiverOnline()==OFFLINE_FLAG)
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

  if (action==PUT_ACTION) {
	  if (cmd=="exitserver"){
		  myDet->setOnline(ONLINE_FLAG);
		  if (myDet->exitServer()!=OK)
			  return string("Server shut down.");
		  else
			  return string("Error closing server\n");
	  }
	  else if (cmd=="exitreceiver"){
		  if(myDet->exitReceiver()!=OK)
			  return string("Receiver shut down\n");
		  else
			  return string("Error closing receiver\n");
	  }
	  else return("cannot decode command\n");
  } else
    return ("cannot get");

}

string slsDetectorCommand::helpExitServer(int narg, char *args[], int action){
	 ostringstream os;
	 os << string("exitserver \t shuts down all the detector servers. Don't use it!!!!\n");
	 os << string("exitreceiver \t shuts down all the receiver servers.\n");
	 return os.str();
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
	if (action==HELP_ACTION)
		return helpOutDir(narg, args, action);

	else if(action==PUT_ACTION)
		myDet->setFilePath(string(args[1]));

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
	if (action==HELP_ACTION)
		return helpFileName(narg, args, action);
	if (cmd=="fname") {
	if (action==PUT_ACTION)
		myDet->setFileName(string(args[1]));

	return string(myDet->getFileName());
	} else
	  return string(myDet->getCurrentFileName());
	  
}



string slsDetectorCommand::helpFileName(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("fname \t  gets the filename for the data without index and extension\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("fname s \t  sets the filename for the data (index and extension will be automatically appended)\n");
  return os.str();
}



string slsDetectorCommand::cmdEnablefwrite(int narg, char *args[], int action){

  int i;
  char ans[100];
  
  if (action==HELP_ACTION) {
    return helpEnablefwrite(narg, args, action);
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


string slsDetectorCommand::cmdOverwrite(int narg, char *args[], int action){

  int i;
  char ans[100];

  if (action==HELP_ACTION) {
    return helpOverwrite(narg, args, action);
  } 
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%d",&i))
      myDet->overwriteFile(i);
    else
      return string("could not decode overwrite");


  }
  sprintf(ans,"%d",myDet->overwriteFile());
  return string(ans);
}



string slsDetectorCommand::helpOverwrite(int narg, char *args[], int action){
  ostringstream os;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << string("When Enabled overwrites files\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string(" i \t  should be 1 or 0 or -1\n");
  return os.str();
}



string slsDetectorCommand::cmdFileIndex(int narg, char *args[], int action){
	char ans[100];
	int i;

	if (action==HELP_ACTION) {
		return helpFileName(narg, args, action);
	}
	else if (action==PUT_ACTION){
		if(!sscanf(args[1],"%d",&i))
			return string("cannot parse file index");
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
      //  cout << myDet->getMaxNumberOfChannels() << endl;
      double corr[ myDet->getMaxNumberOfChannels()], ecorr[myDet->getMaxNumberOfChannels()];
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
    sprintf(answer,"%0.9f",t);
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
    os << string("ratecorr  ns \t  sets the deadtime correction constant in ns, -1 in Eiger will set it to default tau of settings\n");
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
    int bch[myDet->getMaxNumberOfChannels()], nbch;
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
  } else if  (string(args[0])==string("samplex")) {
    c=SAMPLE_X;
  } else if  (string(args[0])==string("sampley")) {
    c=SAMPLE_Y;
  } 


  else
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
  } else  if (string(args[0])==string("samplex")) {
    t=16;
  } else  if (string(args[0])==string("sampley")) {
    t=32;
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
  if (t&16) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("samplex \t   gets the sample displacement in th direction parallel to the beam  \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("samplex f\t  sets the sample displacement in th direction parallel to the beam \n");
  }
  if (t&32) {
    if (action==GET_ACTION || action==HELP_ACTION)
      os << string("sampley \t   gets the sample displacement in the direction orthogonal to the beam  \n");
    if (action==PUT_ACTION || action==HELP_ACTION)
      os << string("sampley f\t  sets the sample displacement in the direction orthogonal to the beam \n");
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
  char answer[100];
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

  else if (string(args[0])==string("resmat")){
	  if (action==PUT_ACTION){
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan resmat input ")+string(args[1]);
		  if(ival>=0)
			  sprintf(answer,"%d",myDet->setCounterBit(ival));
	  }else
		  sprintf(answer,"%d",myDet->setCounterBit());
	  return string(answer);
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
    os << "resmat i \t  sets/resets counter bit in detector"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION){
	  os << "readctr i fname\t  reads counter in detector to file fname, restarts acquisition if i=1"<< std::endl;
	  os << "resetctr \t  Cannot get"<< std::endl;
	  os << "resmat i \t  gets the counter bit in detector"<< std::endl;
  }
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
  
  if (narg>0) {
    if ((string(args[0]).find("start")!=string::npos) || (string(args[0]).find("stop")!=string::npos) || (string(args[0]).find("scriptbefore")!=string::npos) || \
	(string(args[0]).find("scriptafter")!=string::npos)  ||  (string(args[0]).find("headerafter")!=string::npos) ||  (string(args[0]).find("headerbefore")!=string::npos)) {
      

      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " script \t sets the script to execute for the corresponding action"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t returns the script to execute for the corresponding action"<< std::endl;
      
    }
      
      
    if ((string(args[0]).find("encallog")!=string::npos) ||  (string(args[0]).find("angcallog")!=string::npos)) {
      


      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " i \t enables (1) or disables (0) the logging for the calibration"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t returns the calibration log mode"<< std::endl;
    }
  }
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
    sprintf(answer,"%d ",ns);
    if (ns>0) {
      values=new double[ns];
      ns=myDet->getScanSteps(is, values);
      int p=myDet->getScanPrecision(is);
      char format[1000];
      sprintf(format, "%%s %%0.%df",p);
      for (int i=0; i<ns; i++) {
	sprintf(answer,format,answer,values[i]);
      }
      delete [] values;
    }
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

  if ((string(args[0])).find("script")!=string::npos) {


      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " script \t sets the script to execute for the corresponding scan level (threshold, energy, trimbits or positions are internally defined)"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t returns the script to execute for the corresponding scan level (threshold, energy, trimbits or positions are internally defined)"<< std::endl;
      
    }
      

  if ((string(args[0])).find("prec")!=string::npos) {
      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " i \t sets the number of decimals for the scan variable in the file name"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t returns the number of decimals for the scan variable in the file name"<< std::endl;
      
    }
      
  if ((string(args[0])).find("steps")!=string::npos) {
      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " n [st0...stn] \t sets the number of steps and steps value for the corresponding scan level"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t returns the the number of steps and steps value for the corresponding scan level"<< std::endl;
      
    }
      
  if ((string(args[0])).find("range")!=string::npos) {
      if (action==PUT_ACTION || action==HELP_ACTION) 
	os << args[0] << " min max step \t sets the minimum, maximum and step size for the corresponding scan level"<< std::endl;
      if (action==GET_ACTION || action==HELP_ACTION) 
	os << args[0] << " \t  returns the the number of steps and steps value for the corresponding scan level"<< std::endl;
      
    }
      




  return os.str();


}






string slsDetectorCommand::cmdNetworkParameter(int narg, char *args[], int action) {

	networkParameter t;
	int i;
	if (action==HELP_ACTION)
		return helpNetworkParameter(narg,args,action);

	myDet->setOnline(ONLINE_FLAG);

	if (cmd=="detectormac") {
		t=DETECTOR_MAC;
	}	else if (cmd=="detectorip") {
		t=DETECTOR_IP;
	} else if (cmd=="rx_hostname") {
		t=RECEIVER_HOSTNAME;
	} else if (cmd=="rx_udpip") {
		t=RECEIVER_UDP_IP;
	} else if (cmd=="rx_udpmac") {
		t=RECEIVER_UDP_MAC;
	} else if (cmd=="rx_udpport") {
		t=RECEIVER_UDP_PORT;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	} else if (cmd=="rx_udpport2") {
		t=RECEIVER_UDP_PORT2;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	} else if (cmd=="txndelay_left") {
		t=DETECTOR_TXN_DELAY_LEFT;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	} else if (cmd=="txndelay_right") {
		t=DETECTOR_TXN_DELAY_RIGHT;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	} else if (cmd=="txndelay_frame") {
		t=DETECTOR_TXN_DELAY_FRAME;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	} else if (cmd=="flowcontrol_10g") {
		t=FLOW_CONTROL_10G;
		if (action==PUT_ACTION){
			if (!(sscanf(args[1],"%d",&i)))
				return ("cannot parse argument") + string(args[1]);
		}
	}else return ("unknown network parameter")+cmd;

	if (action==PUT_ACTION)
		myDet->setNetworkParameter(t, args[1]);

	return myDet->getNetworkParameter(t);

}

string slsDetectorCommand::helpNetworkParameter(int narg, char *args[], int action) {



  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
	os << "detectormac mac \n sets detector mac to mac"<< std::endl;
	os << "detectorip ip \n sets detector ip to ip"<< std::endl;
    os << "rx_hostname name \n sets receiver ip/hostname to name"<< std::endl;
    os << "rx_udpip ip \n sets receiver udp ip to ip"<< std::endl;
    os << "rx_udpmac mac \n sets receiver udp mac to mac"<< std::endl;
    os << "rx_udpport port \n sets receiver udp port to port"<< std::endl;
    os << "rx_udpport2 port \n sets receiver udp port to port. For Eiger, it is the second half module and for other detectors, same as rx_udpport"<< std::endl;
    os << "txndelay_left port \n sets detector transmission delay of the left port"<< std::endl;
    os << "txndelay_right port \n sets detector transmission delay of the right port"<< std::endl;
    os << "txndelay_frame port \n sets detector transmission delay of the entire frame"<< std::endl;
    os << "flowcontrol_10g port \n sets flow control for 10g for eiger"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
	os << "detectormac \n gets detector mac "<< std::endl;
	os << "detectorip \n gets detector ip "<< std::endl;
    os << "rx_hostname \n gets receiver ip "<< std::endl;
    os << "rx_udpmac \n gets receiver udp mac "<< std::endl;
    os << "rx_udpport \n gets receiver udp port "<< std::endl;
    os << "rx_udpport2 \n gets receiver udp port. For Eiger, it is the second half module and for other detectors, same as rx_udpport"<< std::endl;
    os << "txndelay_left \n gets detector transmission delay of the left port"<< std::endl;
    os << "txndelay_right \n gets detector transmission delay of the right port"<< std::endl;
    os << "txndelay_frame \n gets detector transmission delay of the entire frame"<< std::endl;
    os << "flowcontrol_10g \n sets flow control for 10g for eiger"<< std::endl;
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
  } else if (cmd=="rx_tcpport") {
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
    os << "rx_tcpport i \n sets the communication receiver port"<< std::endl;
    os << "stopport i \n sets the communication stop port "<< std::endl;
  
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "port  \n gets the communication control port"<< std::endl;
    os << "rx_tcpport  \n gets the communication receiver port"<< std::endl;
    os << "stopport \n gets the communication stop port "<< std::endl;
  } 
  return os.str();


}


string slsDetectorCommand::cmdLock(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpLock(narg,args,action);
  
  int val;//, ret;
  char ans[1000];
  
  if(cmd=="lock"){
    myDet->setOnline(ONLINE_FLAG);
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&val))
	myDet->lockServer(val);
      else
	return string("could not lock status")+string(args[1]);
    }

    sprintf(ans,"%d",myDet->lockServer());
  }


  else  if(cmd=="r_lock"){
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&val))
    	 myDet->lockReceiver(val);
      else
    	  return string("could not decode lock status")+string(args[1]);
    }
    sprintf(ans,"%d",myDet->lockReceiver());
  }


  else
    return string("could not decode command");

  return string(ans);
}



string slsDetectorCommand::helpLock(int narg, char *args[], int action) {


  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "lock i \n locks (1) or unlocks (0) the detector to communicate to this client"<< std::endl;
    os << "r_lock i \n locks (1) or unlocks (0) the receiver to communicate to this client"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "lock \n returns the detector lock status"<< std::endl;
    os << "r_lock \n returns the receiver lock status"<< std::endl;
  } 
  return os.str();


}


string slsDetectorCommand::cmdLastClient(int narg, char *args[], int action) {

  if (action==HELP_ACTION)
    return helpLastClient(narg,args,action);
  
  if (action==PUT_ACTION)
    return string("cannot set");


  if(cmd=="lastclient"){
    myDet->setOnline(ONLINE_FLAG);
    return myDet->getLastClientIP();
  }

  else if(cmd=="r_lastclient")
    return myDet->getReceiverLastClientIP();

  return string("cannot decode command");
}

string slsDetectorCommand::helpLastClient(int narg, char *args[], int action) {

  
  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "lastclient \n returns the last client communicating with the detector"<< std::endl;
    os << "r_lastclient \n returns the last client communicating with the receiver"<< std::endl;
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
  }
  else if(cmd=="checkonline"){
    if (action==PUT_ACTION)
      return string("cannot set");
    strcpy(ans,myDet->checkOnline().c_str());
    if(!strlen(ans))
      strcpy(ans,"All online");
    else
      strcat(ans," :Not online");
  }
  else if(cmd=="activate"){
	  myDet->setOnline(ONLINE_FLAG);
	  if (action==PUT_ACTION) {
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan activate mode ")+string(args[1]);
		/*  if(dynamic_cast<slsDetector*>(myDet) != NULL)
			  return string("Can only set it from the multiDetector mode");*/
		  myDet->activate(ival);
	  }
	  sprintf(ans,"%d",myDet->activate());
  }
  else if(cmd=="r_online"){
    if (action==PUT_ACTION) {
      if (sscanf(args[1],"%d",&ival))
	myDet->setReceiverOnline(ival);
      else
	return string("Could not scan online mode ")+string(args[1]);
    }
    sprintf(ans,"%d",myDet->setReceiverOnline());
  }
  else{
    if (action==PUT_ACTION)
      return string("cannot set");
    strcpy(ans,myDet->checkReceiverOnline().c_str());
    if(!strlen(ans))
      strcpy(ans,"All receiver online");
    else
      strcat(ans," :Not receiver online");
  }

  return ans;
}

string slsDetectorCommand::helpOnline(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "online i \n sets the detector in online (1) or offline (0) mode"<< std::endl;
    os << "r_online i \n sets the receiver in online (1) or offline (0) mode"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "online \n gets the detector online (1) or offline (0) mode"<< std::endl;
    os << "checkonline \n returns the hostnames of all detectors in offline mode"<< std::endl;
    os << "r_online \n gets the receiver online (1) or offline (0) mode"<< std::endl;
    os << "r_checkonline \n returns the hostnames of all receiver in offline mode"<< std::endl;
  } 
  return os.str();


}




string slsDetectorCommand::cmdConfigureMac(int narg, char *args[], int action) {

  if (action==HELP_ACTION) {
    return helpConfigureMac(narg,args,action);
  }
  int ret;
  char ans[1000];
  
  if (action==PUT_ACTION){
    myDet->setOnline(ONLINE_FLAG);
    ret=myDet->configureMAC();
  }
  else
    return string("Cannot get ")+cmd;

  sprintf(ans,"%d",ret);
  return ans;
}

string slsDetectorCommand::helpConfigureMac(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << "configuremac i \n configures the MAC of the detector."<< std::endl;
  if (action==GET_ACTION || action==HELP_ACTION)
    os << "configuremac " << "Cannot get " << std::endl;
  
  return os.str();
}


string slsDetectorCommand::cmdDetectorSize(int narg, char *args[], int action) {

	if (action==HELP_ACTION)
		return helpDetectorSize(narg,args,action);
	int ret, val=-1, pos=-1,i;
	char ans[1000], temp[100];

	myDet->setOnline(ONLINE_FLAG);

	if (action==PUT_ACTION) {
		if (cmd=="maxmod")
			return string("cannot put!");
		else if (cmd=="roimask"){
		  if (!sscanf(args[1],"%d",&val))
		    return string("could not scan ")+string(args[0])+string(" ")+string(args[1]);
		}
		else if (!sscanf(args[1],"%d",&val))
		  return string("could not scan ")+string(args[0])+string(" ")+string(args[1]);

		if (cmd=="roi"){
		  //debug number of arguments
		  if ((val<0)	|| (narg!=((val*4)+2)) )
		    return helpDetectorSize(narg,args,action);
			ROI allroi[val];
			pos=2;
			for(i=0;i<val;i++){
				if ((!sscanf(args[pos++],"%d",&allroi[i].xmin)) ||
					(!sscanf(args[pos++],"%d",&allroi[i].xmax)) ||
					(!sscanf(args[pos++],"%d",&allroi[i].ymin)) ||
					(!sscanf(args[pos++],"%d",&allroi[i].ymax)) )
					return string("cannot parse arguments for roi");
			}
			myDet->setROI(val,allroi);
		}

		if(cmd=="detsizechan"){
			if ((sscanf(args[1],"%d",&val)) && (val>0))
				myDet->setMaxNumberOfChannelsPerDetector(X,val);
			if ((narg > 2) && (sscanf(args[2],"%d",&val)) && (val>0))
				myDet->setMaxNumberOfChannelsPerDetector(Y,val);
		}

	}

	if (cmd=="nmod" || cmd=="roimask") {
		ret=myDet->setNumberOfModules(val);
	} else if (cmd=="maxmod") {
		ret=myDet->getMaxNumberOfModules();
	} else if (cmd=="dr") {
		ret=myDet->setDynamicRange(val);
	} else if (cmd=="roi") {
		myDet->getROI(ret);
	} else if (cmd=="detsizechan") {
		sprintf(ans,"%d",myDet->getMaxNumberOfChannelsPerDetector(X));
		sprintf(temp,"%d",myDet->getMaxNumberOfChannelsPerDetector(Y));
		strcat(ans," ");
		strcat(ans,temp);
		return string(ans);
	}
	else
		return string("unknown detector size ")+cmd;

	if (cmd=="roimask")
	  sprintf(ans,"%x",ret);
	else
	  sprintf(ans,"%d",ret);
	  
	return string(ans);

}


string slsDetectorCommand::helpDetectorSize(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "nmod i \n sets the number of modules of the detector"<< std::endl;
    os << "dr i \n sets the dynamic range of the detector"<< std::endl;
    os << "roi i xmin xmax ymin ymax \n sets region of interest where i is number of rois;i=0 to clear rois"<< std::endl;
    os << "detsizechan x y \n sets the maximum number of channels for complete detector set in both directions; -1 is no limit"<< std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "nmod \n gets the number of modules of the detector"<< std::endl;
    os << "maxmod \n gets the maximum number of modules of the detector"<< std::endl;
    os << "dr \n gets the dynamic range of the detector"<< std::endl;
    os << "roi \n gets region of interest"<< std::endl;
    os << "detsizechan \n gets the maximum number of channels for complete detector set in both directions; -1 is no limit"<< std::endl;

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
    
  } else if (cmd=="trimval") {
	  if (action==PUT_ACTION){
		  if (sscanf(args[1],"%d",&val))
			  myDet->setAllTrimbits(val);
		  else
			  return string("invalid trimbit value ")+cmd;
	  }
	  sprintf(ans,"%d",myDet->setAllTrimbits(-1));
	  return ans;
  } else if (cmd=="pedestal") {
	  if (action==GET_ACTION)
		  return string("cannot get");
	  if (sscanf(args[1],"%d",&val)){
		  sprintf(ans,"%d",myDet->calibratePedestal(val));
		  return string(ans);
	  }else
    	  return string("cannot parse frame number")+cmd;

  }
  return string("unknown settings command ")+cmd;
  
}


string slsDetectorCommand::helpSettings(int narg, char *args[], int action) {

  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "settings s \n sets the settings of the detector - can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain"
    		"lownoise, dynamichg0,fixgain1,fixgain2,forceswitchg1, forceswitchg2"<< std::endl;
    os << "threshold eV\n sets the detector threshold in eV"<< std::endl;
    os << "trimbits fname\n loads the trimfile fname to the detector. If no extension is specified, the serial number of each module will be attached."<< std::endl;
    os << "trim:mode fname\n trims the detector according to mode (can be noise, beam, improve, fix) and saves the resulting trimbits to file fname."<< std::endl;
    os << "trimval i \n sets all the trimbits to i" << std::endl;
    os << "pedestal i \n starts acquisition for i frames, calculates pedestal and writes back to fpga."<< std::endl;

  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "settings \n gets the settings of the detector"<< std::endl;
    os << "threshold V\n gets the detector threshold"<< std::endl;
    os << "trimbits [fname]\n returns the trimfile loaded on the detector. If fname is specified the trimbits are saved to file. If no extension is specified, the serial number of each module will be attached."<< std::endl;
    os << "trimval \n returns the value all trimbits are set to. If they are different, returns -1." << std::endl;
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

  if (cmd=="receiverversion") {
    sprintf(answer,"%llx",myDet->getId(RECEIVER_VERSION));
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
    os << "receiverversion \n gets the version of the receiver"<< std::endl;
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

  
  return string("unknown digital test mode ")+cmd;

}

string slsDetectorCommand::helpDigiTest(int narg, char *args[], int action) {


  ostringstream os;  
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "digitaltest:i \t performs digital test of the module i. Returns 0 if succeeded, otherwise error mask."<< std::endl;
    os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes."<< std::endl;
  }
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "digibittest i\t will perform test which will plot the unique channel identifier, instead of data."<< std::endl;
  } 
  return os.str();
}

string slsDetectorCommand::cmdRegister(int narg, char *args[], int action) {
  

  if (action==HELP_ACTION)
    return helpRegister(narg, args, action);

  int addr, val,n;
  char answer[1000];

  myDet->setOnline(ONLINE_FLAG);
  

 // "reg" //
 
  // "setbit" //
 
    // "clearbit" //
 
    // "getbit" //
 


  if (action==PUT_ACTION) {
      if (cmd=="getbit")	
	return string("Cannot put");
	
    if(narg<3) {
      if (cmd=="reg")
	return string("wrong usage: should specify both address and value (hexadecimal fomat) ");
      else
	return string("wrong usage: should specify both address (hexadecimal fomat) and bit number");
	
    }

    if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan address  (hexadecimal fomat) ")+string(args[1]);

    
    if (cmd=="reg") {
      if (sscanf(args[2],"%x",&val))
	;
      else
	return string("Could not scan value  (hexadecimal fomat) ")+string(args[2]);
      sprintf(answer,"%x",myDet->writeRegister(addr,val));
    } else if (cmd=="adcreg") {
      if (sscanf(args[2],"%x",&val))
	;
      else
	return string("Could not scan value  (hexadecimal fomat) ")+string(args[2]);
      sprintf(answer,"%x",myDet->writeAdcRegister(addr,val));
    } else {
      
      if (sscanf(args[2],"%d",&n))
	;
      else
	return string("Could not scan bit number ")+string(args[2]);

      if (n<0 || n>31)
	return string("Bit number out of range")+string(args[2]);
      
      if (cmd=="setbit")
	sprintf(answer,"%x",myDet->writeRegister(addr,myDet->readRegister(addr)| 1<<n));
      if (cmd=="clearbit")
	sprintf(answer,"%x",myDet->writeRegister(addr,myDet->readRegister(addr) & ~(1<<n)));
    }


  } else {
    if (cmd=="setbit")	
      return string("Cannot get");
    if (cmd=="clearbit")	
      return string("Cannot get");
    if (cmd=="adcreg")	
      return string("Cannot get");

    if (cmd=="reg") {
      if (narg<2) 
	return string("wrong usage: should specify address  (hexadecimal fomat) ");
      if (sscanf(args[1],"%x",&addr))
	;
      else
	return string("Could not scan address  (hexadecimal fomat) ")+string(args[1]);
      
	
      sprintf(answer,"%x",myDet->readRegister(addr));

    }
    
    if (cmd=="getbit") {

      if(narg<3) 
	return string("wrong usage: should specify both address (hexadecimal fomat) and bit number");


      if (sscanf(args[1],"%x",&addr))
	;
      else
	return string("Could not scan address  (hexadecimal fomat) ")+string(args[1]);
      
      if (sscanf(args[2],"%d",&n))
	;
      else
	return string("Could not scan bit number ")+string(args[2]);
      
      if (n<0 || n>31)
	return string("Bit number out of range")+string(args[2]);
      

      sprintf(answer,"%d",(myDet->readRegister(addr) >> n) & 1);
    }
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
  int mode=0;

  int idac=-1;
  if (sscanf(args[0],"dac:%d",&idac)==1) {
    printf("chiptestboard!\n");
    dac=(dacIndex)idac;
  }
  else if (cmd=="adcvpp")
    dac=ADC_VPP;
  else if (cmd=="vthreshold")
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

  else if (cmd=="dac0")
	  dac=V_DAC0;
  else if (cmd=="dac1")
	  dac=V_DAC1;
  else if (cmd=="dac2")
	  dac=V_DAC2;
  else if (cmd=="dac3")
	  dac=V_DAC3;
  else if (cmd=="dac4")
	  dac=V_DAC4;
  else if (cmd=="dac5")
	  dac=V_DAC5;
  else if (cmd=="dac6")
	  dac=V_DAC6;
  else if (cmd=="dac7")
	  dac=V_DAC7;




  else if (cmd== "vsvp")
	  dac=E_SvP;
  else if (cmd=="vsvn")
	  dac=E_SvN;
  else if (cmd=="vtr")
	  dac=E_Vtr;
  else if (cmd=="vrf")
	  dac=E_Vrf;
  else if (cmd=="vrs")
	  dac=E_Vrs;
  else if (cmd== "vtgstv")
	  dac=E_Vtgstv;
  else if (cmd== "vcmp_ll")
	  dac=E_Vcmp_ll;
  else if (cmd=="vcmp_lr")
	  dac=E_Vcmp_lr;
  else if (cmd== "vcall")
	  dac=E_cal;
  else if (cmd== "vcmp_rl")
	  dac=E_Vcmp_rl;
  else if (cmd== "vcmp_rr")
	  dac=E_Vcmp_rr;
  else if (cmd== "rxb_rb")
	  dac=E_rxb_rb;
  else if (cmd== "rxb_lb")
	  dac=E_rxb_lb;
  else if (cmd== "vcp")
	  dac=E_Vcp;
  else if (cmd=="vcn")
	  dac=E_Vcn;
  else if (cmd== "vis")
	  dac=E_Vis;
  else if (cmd== "iodelay")
	  dac=IO_DELAY;
  else
    return string("cannot decode dac ")+cmd;
  
  myDet->setOnline(ONLINE_FLAG);

  if (action==PUT_ACTION) {

	  if(narg >= 3)
		  if(!strcasecmp(args[2],"mv"))
			  mode = 1;
#ifdef DACS_INT

	if (sscanf(args[1],"%d", &val))
#else
    if (sscanf(args[1],"%f", &val))
#endif
	;
      else
	return string("cannot scan DAC value ")+string(args[1]);

    myDet->setDAC(val,dac,mode);
  }
  
  else if(narg >= 2)
	  if(!strcasecmp(args[1],"mv"))
		  mode = 1;

#ifdef DACS_INT
  sprintf(answer,"%d",myDet->setDAC(-1,dac,mode));
#else
  sprintf(answer,"%f",myDet->setDAC(-1,dac,mode));
#endif
  if(mode)
	  strcat(answer,"mV");
  return string(answer);

}


string slsDetectorCommand::helpDAC(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "vthreshold dacu\t sets the detector threshold in dac units (0-1024) or mV. The energy is approx 800-15*keV" << std::endl;
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


    os << "dac0 " << "dacu\t sets dac 0" << std::endl;
    os << "dac1 " << "dacu\t sets dac 1" << std::endl;
    os << "dac2 " << "dacu\t sets dac 2" << std::endl;
    os << "dac3 " << "dacu\t sets dac 3" << std::endl;
    os << "dac4 " << "dacu\t sets dac 4" << std::endl;
    os << "dac5 " << "dacu\t sets dac 5" << std::endl;
    os << "dac6 " << "dacu\t sets dac 6" << std::endl;
    os << "dac7 " << "dacu\t sets dac 7" << std::endl;

    os << "vsvp"	<< "dacu\t sets vsvp" << std::endl;
    os << "vsvn" 	<< "dacu\t sets vsvn" << std::endl;
    os << "vtr" 	<< "dacu\t sets vtr" << std::endl;
    os << "vrf" 	<< "dacu\t sets vrf" << std::endl;
    os << "vrs" 	<< "dacu\t sets vrs" << std::endl;
    os << "vtgstv" 	<< "dacu\t sets vtgstv" << std::endl;
    os << "vcmp_ll" << "dacu\t sets vcmp_ll" << std::endl;
    os << "vcmp_lr" << "dacu\t sets vcmp_lr" << std::endl;
    os << "vcall" 	<< "dacu\t sets vcall" << std::endl;
    os << "vcmp_rl" << "dacu\t sets vcmp_rl" << std::endl;
    os << "vcmp_rr" << "dacu\t sets vcmp_rr" << std::endl;
    os << "rxb_rb" 	<< "dacu\t sets rxb_rb" << std::endl;
    os << "rxb_lb" 	<< "dacu\t sets rxb_lb" << std::endl;
    os << "vcp" 	<< "dacu\t sets vcp " << std::endl;
    os << "vcn" 	<< "dacu\t sets vcn " << std::endl;
    os << "vis" 	<< "dacu\t sets vis " << std::endl;


    os << "<dac name> mv <value> if you want in mV else <dac name> <value> in dac units " << std::endl;
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


    os << "dac0 " << "\t gets dac 0" << std::endl;
    os << "dac1 " << "\t gets dac 0" << std::endl;
    os << "dac2 " << "\t gets dac 0" << std::endl;
    os << "dac3 " << "\t gets dac 0" << std::endl;
    os << "dac4 " << "\t gets dac 0" << std::endl;
    os << "dac5 " << "\t gets dac 0" << std::endl;
    os << "dac6 " << "\t gets dac 0" << std::endl;
    os << "dac7 " << "\t gets dac 0" << std::endl;

    os << "vsvp"	<< "dacu\t gets vsvp" << std::endl;
    os << "vsvn" 	<< "dacu\t gets vsvn" << std::endl;
    os << "vtr" 	<< "dacu\t gets vtr" << std::endl;
    os << "vrf" 	<< "dacu\t gets vrf" << std::endl;
    os << "vrs" 	<< "dacu\t gets vrs" << std::endl;
    os << "vtgstv" 	<< "dacu\t gets vtgstv" << std::endl;
    os << "vcmp_ll" << "dacu\t gets vcmp_ll" << std::endl;
    os << "vcmp_lr" << "dacu\t gets vcmp_lr" << std::endl;
    os << "vcall" 	<< "dacu\t gets vcall" << std::endl;
    os << "vcmp_rl" << "dacu\t gets vcmp_rl" << std::endl;
    os << "vcmp_rr" << "dacu\t gets vcmp_rr" << std::endl;
    os << "rxb_rb" 	<< "dacu\t gets rxb_rb" << std::endl;
    os << "rxb_lb" 	<< "dacu\t gets rxb_lb" << std::endl;
    os << "vcp" 	<< "dacu\t gets vcp " << std::endl;
    os << "vcn" 	<< "dacu\t gets vcn " << std::endl;
    os << "vis" 	<< "dacu\t gets vis " << std::endl;


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
  else if (cmd=="temp_fpgaext")
	  adc=TEMPERATURE_FPGAEXT;
  else if (cmd=="temp_10ge")
	  adc=TEMPERATURE_10GE;
  else if (cmd=="temp_dcdc")
	  adc=TEMPERATURE_DCDC;
  else if (cmd=="temp_sodl")
	  adc=TEMPERATURE_SODL;
  else if (cmd=="temp_sodr")
	  adc=TEMPERATURE_SODR;
  else
    return string("cannot decode adc ")+cmd;

  myDet->setOnline(ONLINE_FLAG);
#ifdef DACS_INT
  if (myDet->getDetectorsType() == EIGER)
	  sprintf(answer,"%.2f",(double)myDet->getADC(adc)/1000.00);
  else sprintf(answer,"%d",myDet->getADC(adc));
#else
  sprintf(answer,"%f",myDet->getADC(adc));
#endif
  //if ((adc == TEMPERATURE_ADC) || (adc == TEMPERATURE_FPGA))
  strcat(answer,"°C");
  return string(answer);

}

string slsDetectorCommand::helpADC(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "temp_adc " << "Cannot be set" << std::endl;
    os << "temp_fpga " << "Cannot be set" << std::endl;
    os << "temp_fpgaext " << "Cannot be set" << std::endl;
    os << "temp_10ge " << "Cannot be set" << std::endl;
    os << "temp_dcdc " << "Cannot be set" << std::endl;
    os << "temp_sodl " << "Cannot be set" << std::endl;
    os << "temp_sodr " << "Cannot be set" << std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    os << "temp_adc " << "\t gets the temperature of the adc" << std::endl;
    os << "temp_fpga " << "\t gets the temperature of the fpga" << std::endl;
    os << "temp_fpgaext " << "\t gets the temperature close to the fpga" << std::endl;
    os << "temp_10ge " << "\t gets the temperature close to the 10GE" << std::endl;
    os << "temp_dcdc " << "\t gets the temperature close to the dc dc converter" << std::endl;
    os << "temp_sodl " << "\t gets the temperature close to the left so-dimm memory" << std::endl;
    os << "temp_sodr " << "\t gets the temperature close to the right so-dimm memory" << std::endl;
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
    os << string("timing \t gets the timing mode of the detector (auto, trigger, ro_trigger, gating, triggered_gating)\n");
  if (action==PUT_ACTION || action==HELP_ACTION)
    os << string("timing mode \t sets synchronization mode of the detector. Can be auto, trigger, ro_trigger, gating, triggered_gating \n");
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
  else if (cmd=="subexptime")
    index=SUBFRAME_ACQUISITION_TIME;
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
  else if (cmd=="measurements")
    index=MEASUREMENTS_NUMBER;
  else
    return string("could not decode timer ")+cmd;

  
  if (action==PUT_ACTION) {
    if (sscanf(args[1],"%lf", &val))
      ;
    else
      return string("cannot scan timer value ")+string(args[1]);
    if (index==ACQUISITION_TIME || index==SUBFRAME_ACQUISITION_TIME || index==FRAME_PERIOD || index==DELAY_AFTER_TRIGGER)
      t=(int64_t)(val*1E+9);
    else t=(int64_t)val;
  }
  
  
  myDet->setOnline(ONLINE_FLAG);

  ret=myDet->setTimer(index,t);
  if (index==ACQUISITION_TIME || index==SUBFRAME_ACQUISITION_TIME || index==FRAME_PERIOD || index==DELAY_AFTER_TRIGGER)
    rval=(double)ret*1E-9;
  else rval=ret;
  

  //set frame index
  if (index==FRAME_NUMBER || index==CYCLES_NUMBER ){
	  if ((myDet->setTimer(FRAME_NUMBER,-1)*myDet->setTimer(CYCLES_NUMBER,-1))>1)
		  myDet->setFrameIndex(0);
	  else
		  myDet->setFrameIndex(-1);
  }


  sprintf(answer,"%0.9f",rval);
  return string(answer);
  



}


string slsDetectorCommand::helpTimer(int narg, char *args[], int action) {
  
  ostringstream os;  
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "exptime t \t sets the exposure time in s" << std::endl;
    os << "subexptime t \t sets the exposure time of subframe in s" << std::endl;
    os << "period t \t sets the frame period in s" << std::endl;
    os << "delay t \t sets the delay after trigger in s" << std::endl;
    os << "frames t \t sets the number of frames per cycle (e.g. after each trigger)" << std::endl;
    os << "cycles t \t sets the number of cycles (e.g. number of triggers)" << std::endl;
    os << "probes t \t sets the number of probes to accumulate (max 3! cycles should be set to 1, frames to the number of pump-probe events)" << std::endl;
    os << std::endl;


  }
  if (action==GET_ACTION || action==HELP_ACTION) {
    
    os << "exptime  \t gets the exposure time in s" << std::endl;
    os << "subexptime  \t gets the exposure time of subframe in s" << std::endl;
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
  else if (cmd=="nframes")
    index=FRAMES_FROM_START;
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
  else if (cmd=="phasestep") {
    index=PHASE_SHIFT;
    t=100000;
  }  else if (cmd=="oversampling")
    index=OVERSAMPLING;
  else if (cmd=="adcclk")
    index=ADC_CLOCK;
  else if (cmd=="adcphase") {
    index=ADC_PHASE;
    t=100000;
  }  else if (cmd=="adcpipeline")
    index=ADC_PIPELINE;
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

int retval;
char answer[1000]="";

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
      else if (sval=="parallel")
	flag=PARALLEL;
      else if (sval=="nonparallel")
	flag=NONPARALLEL;
      else if (sval=="safe")
	flag=SAFE;
      else
	return string("could not scan flag ")+string(args[1]);
    }
  

    myDet->setOnline(ONLINE_FLAG);

    retval = myDet->setReadOutFlags(flag);

    if(retval == NORMAL_READOUT)
    	return string("none");

    if(retval & STORE_IN_RAM)
    	strcat(answer,"storeinram ");
    if(retval & TOT_MODE)
    	strcat(answer,"tot ");
    if(retval & CONTINOUS_RO)
    	strcat(answer,"continous ");
    if(retval & PARALLEL)
    	strcat(answer,"parallel ");
    if(retval & NONPARALLEL)
    	strcat(answer,"nonparallel ");
    if(retval & SAFE)
    	strcat(answer,"safe ");
    if(strlen(answer))
    	return string(answer);

    return string("unknown");

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
    os << "flags mode \t sets the readout flags to mode. can be none, storeinram, tot, continous, parallel, nonparallel, safe, unknown" << std::endl;
    
  }
  if (action==GET_ACTION || action==HELP_ACTION) {

    os << "extsig:i \t gets the mode of the external signal i. can be  \n \t \t \t off, \n \t \t \t gate_in_active_high, \n \t \t \t gate_in_active_low, \n \t \t \t trigger_in_rising_edge, \n \t \t \t trigger_in_falling_edge, \n \t \t \t ro_trigger_in_rising_edge, \n \t \t \t ro_trigger_in_falling_edge, \n \t \t \t gate_out_active_high, \n \t \t \t gate_out_active_low, \n \t \t \t trigger_out_rising_edge, \n \t \t \t trigger_out_falling_edge, \n \t \t \t ro_trigger_out_rising_edge, \n \t \t \t ro_trigger_out_falling_edge" << std::endl;
    os << "flags \t gets the readout flags. can be none, storeinram, tot, continous, parallel, nonparallel, safe, unknown" << std::endl;

  } 
  return os.str();








}




string slsDetectorCommand::cmdConfiguration(int narg, char *args[], int action) {


  
  if (action==HELP_ACTION)
    return helpConfiguration(narg, args, action);
  
  string sval;
  
  if (narg<2 && cmd != "rx_printconfig")
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
  } else if (cmd=="rx_printconfig"){
	  if (action==PUT_ACTION)
		  return string("cannot put");
	  return string(""+myDet->printReceiverConfiguration());
  }else if (cmd=="parameters") {
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


string slsDetectorCommand::cmdReceiver(int narg, char *args[], int action) {
  char answer[100];
  int ival = -1;

  if (action==HELP_ACTION)
    return helpReceiver(narg, args, action);


  myDet->setOnline(ONLINE_FLAG);

  if(cmd=="receiver"){
    if (action==PUT_ACTION) {
      if(!strcasecmp(args[1],"start"))
    	  myDet->startReceiver();
      else if(!strcasecmp(args[1],"stop")){
    	  myDet->startReceiverReadout();
    	  /*runStatus s = myDet->getReceiverStatus();
    	  while(s != RUN_FINISHED){
    		  usleep(50000);
    		  s = myDet->getReceiverStatus();
    	  }*/
    	  myDet->stopReceiver();
      }
      else
	return helpReceiver(narg, args, action);
    }
    return myDet->runStatusType(myDet->getReceiverStatus());
  }


  else if(cmd=="framescaught"){
    if (action==PUT_ACTION)
      return string("cannot put");
    else{
      sprintf(answer,"%d",myDet->getFramesCaughtByReceiver());
      return string(answer);
    }
  }

  else if(cmd=="resetframescaught"){
    if (action==GET_ACTION)
      return string("cannot get");
    else{
    	if(myDet->resetFramesCaught() == FAIL)
    		strcpy(answer,"failed");
    	else
    		strcpy(answer,"successful");
      return string(answer);
    }
  }

  else if(cmd=="frameindex"){
    if (action==PUT_ACTION)
      return string("cannot put");
    else{
      sprintf(answer,"%d",myDet->getReceiverCurrentFrameIndex());
      return string(answer);
    }
  }
  else if(cmd=="r_readfreq"){
	  if (action==PUT_ACTION){
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan read frequency mode ")+string(args[1]);
		  if(ival>=0)
			  myDet->setReadReceiverFrequency(1,ival);
	  }
	  sprintf(answer,"%d",myDet->setReadReceiverFrequency(1));
	  return string(answer);

  }
  else if(cmd=="r_compression"){
	  if (action==PUT_ACTION){
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan receiver compression input ")+string(args[1]);
		  if(ival>=0)
			  sprintf(answer,"%d",myDet->enableReceiverCompression(ival));
	  }else
		  sprintf(answer,"%d",myDet->enableReceiverCompression());
	  return string(answer);

  }

  else if(cmd=="tengiga"){
	  if (action==PUT_ACTION){
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan tengiga input ")+string(args[1]);
		  if(ival>=0)
			  sprintf(answer,"%d",myDet->enableTenGigabitEthernet(ival));
	  }else
		  sprintf(answer,"%d",myDet->enableTenGigabitEthernet());
	  return string(answer);

  }


  else if(cmd=="rx_fifodepth"){
	  if (action==PUT_ACTION){
		  if (!sscanf(args[1],"%d",&ival))
			  return string("Could not scan rx_fifodepth input ")+string(args[1]);
		  if(ival>=0)
			  sprintf(answer,"%d",myDet->setReceiverFifoDepth(ival));
	  }else
		  sprintf(answer,"%d",myDet->setReceiverFifoDepth());
	  return string(answer);

  }

    return string("could not decode command");

}



string slsDetectorCommand::helpReceiver(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "receiver [status] \t starts/stops the receiver to listen to detector packets. - can be start or stop" << std::endl;
    os << "resetframescaught [any value] \t resets frames caught by receiver" << std::endl;
  	os << "r_readfreq \t sets the gui read frequency of the receiver, 0 if gui requests frame, >0 if receiver sends every nth frame to gui" << std::endl;
	os << "tengiga \t sets system to be configure for 10Gbe if set to 1, else 1Gbe if set to 0" << std::endl;
	os << "rx_fifodepth [val]\t sets receiver fifo depth to val" << std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION){
    os << "receiver \t returns the status of receiver - can be running or idle" << std::endl;
    os << "framescaught \t returns the number of frames caught by receiver(average for multi)" << std::endl;
    os << "frameindex \t returns the current frame index of receiver(average for multi)" << std::endl;
    os << "r_readfreq \t returns the gui read frequency of the receiver" << std::endl;
    os << "tengiga \t returns 1 if the system is configured for 10Gbe else 0 for 1Gbe" << std::endl;
    os << "rx_fifodepth \t returns receiver fifo depth" << std::endl;
  }
  return os.str();








}

string slsDetectorCommand::helpPattern(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "pattern fname \t loads pattern file" << std::endl;
  	os << "patword addr word \t writes pattern word - only very advanced users!" << std::endl;
	os << "patioctrl reg\t configures inputs/outputs of the chiptest board - only advanced users!" << std::endl;
	os << "patclkctrl reg\t configures output clk enable of the chiptest board- only advanced users! " << std::endl;
	os << "patlimits addr1 addr2\t defines pattern limits between addr1 and addr2" << std::endl;
	os << "patloop0 addr1 adrr2 \t configures the limits of the 0 loop " << std::endl;
	os << "patloop1 addr1 adrr2 \t configures the limits of the 1 loop " << std::endl;
	os << "patloop2 addr1 adrr2 \t configures the limits of the 2 loop " << std::endl;
	os << "patnloop0 n \t sets number of cycles of the 0 loop " << std::endl;
	os << "patnloop1 n \t sets number of cycles of the 1 loop " << std::endl;
	os << "patnloop2 n \t sets number of cycles of the 2 loop " << std::endl;
	os << "patwait0 addr \t configures pattern wait 0 address " << std::endl;
	os << "patwait1 addr \t configures pattern wait 1 address " << std::endl;
	os << "patwait2 addr \t configures pattern wait 2 address " << std::endl;
	os << "patwaittime0 nclk \t sets wait 0 waiting time in clock number " << std::endl;
	os << "patwaittime1 nclk \t sets wait 1 waiting time in clock number " << std::endl;
	os << "patwaittime2 nclk \t sets wait 2 waiting time in clock number " << std::endl;
	os << "adcinvert mask\t  sets the adcinversion mask (hex)" << std::endl;
	os << "adcdisable mask\t  sets the adcdisable mask (hex)" << std::endl;

  }	
  if (action==GET_ACTION || action==HELP_ACTION){
    os << "pattern \t cannot get" << std::endl;
  	os << "patword \t cannot get" << std::endl;
	os << "patioctrl \t returns inputs/outputs of the chiptest board - only advanced users!" << std::endl;
	os << "patclkctrl\t returns output clk enable of the chiptest board- only advanced users! " << std::endl;
	os << "patlimits \t returns pattern limits between addr1 and addr2" << std::endl;
	os << "patloop0  \t returns the limits of the 0 loop " << std::endl;
	os << "patloop1  \t returns the limits of the 1 loop " << std::endl;
	os << "patloop2  \t returns the limits of the 2 loop " << std::endl;
	os << "patnloop0 \t returns the number of cycles of the 0 loop " << std::endl;
	os << "patnloop1 \t returns the number of cycles of the 1 loop " << std::endl;
	os << "patnloop2 \t  returns the number of cycles of the 2 loop " << std::endl;
	os << "patwait0 \t  returns the pattern wait 0 address " << std::endl;
	os << "patwait1 \t  returns the pattern wait 1 address " << std::endl;
	os << "patwait2 \t  returns the pattern wait 2 address " << std::endl;
	os << "patwaittime0 \t  returns the wait 0 waiting time in clock number " << std::endl;
	os << "patwaittime1 \t  returns the wait 1 waiting time in clock number " << std::endl;
	os << "patwaittime2 \t  returns the wait 2 waiting time in clock number " << std::endl;
	os << "adcinvert \t  returns the adcinversion mask " << std::endl;

	os << "adcdisable \t  returns the adcdisable mask " << std::endl;

  }
  return os.str();

}


string slsDetectorCommand::cmdPattern(int narg, char *args[], int action) {

  
  if (action==HELP_ACTION)
    return helpPattern(narg, args, action);
  /********
	   
  Must implement set ctb functions in slsDetector and multiSlsDetector
  
  **********/
  string fname;
  int addr, start, stop, n;
  uint64_t word, t;

  myDet->setOnline(ONLINE_FLAG);

  ostringstream os;
  if (cmd=="pattern") {
    //get fname fron stdin
    
    if (action==PUT_ACTION) {
      fname=string(args[1]);
      os << myDet->setCTBPattern(fname);
    }   else if (action==GET_ACTION) 
      os << "Cannot get";
  }  	else if (cmd=="patword") {

    if (action==PUT_ACTION) {
      //get addr, word from stdin
      
      if(narg<3) 
	return string("wrong usage: should specify both address and value (hexadecimal fomat) ");
      
	if (sscanf(args[1],"%x",&addr))
	  ;
	else
	  return string("Could not scan address (hexadecimal fomat) ")+string(args[1]);
      
	if (sscanf(args[2],"%llx",&word))
	  ;
	else
	  return string("Could not scan value  (hexadecimal fomat) ")+string(args[2]);
      

	os << hex << myDet->setCTBWord(addr,word) << dec;    
    }   else if (action==GET_ACTION) 
      os << "Cannot get";
    
      
  }	else if (cmd=="patioctrl") {
    //get word from stdin
    
    if (action==PUT_ACTION) {
      
      if (sscanf(args[1],"%llx",&word))
	;
      else
	return string("Could not scan value  (hexadecimal fomat) ")+string(args[1]);
      

      myDet->setCTBWord(-1,word);
    }

    os << hex << myDet->setCTBWord(-1,-1) << dec;
  }	else if (cmd=="patclkctrl") {
    //get word from stdin



    if (action==PUT_ACTION) {
      
      if (sscanf(args[1],"%llx",&word))
	;
      else
	return string("Could not scan value  (hexadecimal fomat) ")+string(args[1]);
      

      myDet->setCTBWord(-2,word);
    }

    os << hex << myDet->setCTBWord(-2,-1) << dec;


  } else if (cmd=="patlimits") {
    //get start, stop from stdin  
    if (action==PUT_ACTION) {
      if(narg<3)	return string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
      n=-1;
      if (sscanf(args[1],"%x",&start))
      ;
    else
      return string("Could not scan start address  (hexadecimal fomat) ")+string(args[1]);

    
      if (sscanf(args[2],"%x",&stop))
	;
      else
	return string("Could not scan stop address  (hexadecimal fomat) ")+string(args[2]);
      
      myDet->setCTBPatLoops(-1,start, stop,n);
    }
    
    start=-1;
    stop=-1;
    n=-1;
    myDet->setCTBPatLoops(-1,start, stop,n);
    os << hex << start << " " << stop;// << " "<< dec << n ; 
  } else if (cmd=="patloop0") {
    //get start, stop from stdin


    //get start, stop from stdin  
    if (action==PUT_ACTION) {
      if(narg<3)	return string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
      n=-1;
      if (sscanf(args[1],"%x",&start))
      ;
    else
      return string("Could not scan start address  (hexadecimal fomat) ")+string(args[1]);

    
      if (sscanf(args[2],"%x",&stop))
	;
      else
	return string("Could not scan stop address  (hexadecimal fomat) ")+string(args[2]);
      
      myDet->setCTBPatLoops(0,start, stop,n);
    }
    
    start=-1;
    stop=-1;
    n=-1;  
    myDet->setCTBPatLoops(0,start, stop,n);
    os << hex << start << " " << stop;// << " "<< dec << n ; 


  } else if (cmd=="patloop1") {

    //get start, stop from stdin  
    if (action==PUT_ACTION) {
      if(narg<3)	return string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
      n=-1;
      if (sscanf(args[1],"%x",&start))
      ;
    else
      return string("Could not scan start address  (hexadecimal fomat) ")+string(args[1]);

    
      if (sscanf(args[2],"%x",&stop))
	;
      else
	return string("Could not scan stop address  (hexadecimal fomat) ")+string(args[2]);
      
      myDet->setCTBPatLoops(1,start, stop,n);
    }
    
    start=-1;
    stop=-1;
    n=-1; 
    myDet->setCTBPatLoops(1,start, stop,n);
    os << hex << start << " " << stop;// << " "<< dec << n ; 



    
  } else if (cmd=="patloop2") {

    //get start, stop from stdin  
    if (action==PUT_ACTION) {
      if(narg<3)	return string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
      n=-1;
      if (sscanf(args[1],"%x",&start))
      ;
    else
      return string("Could not scan start address  (hexadecimal fomat) ")+string(args[1]);

    
      if (sscanf(args[2],"%x",&stop))
	;
      else
	return string("Could not scan stop address  (hexadecimal fomat) ")+string(args[2]);
      
      myDet->setCTBPatLoops(2,start, stop,n) ;
    }
    
    start=-1;
    stop=-1;
    n=-1; 
    myDet->setCTBPatLoops(2,start, stop,n);
    os << hex << start << " " << stop << dec;// << " "<< dec << n ; 


  } else if (cmd=="patnloop0") {
    start=-1;
    stop=-1;

    
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%d",&n))
      ;
    else
      return string("Could not scan number of loops ")+string(args[1]);

    
      myDet->setCTBPatLoops(0,start, stop,n) ;
    }


    
    start=-1;
    stop=-1;
    n=-1;  
    myDet->setCTBPatLoops(0,start, stop,n);
    os << n ; 
  } else if (cmd=="patnloop1") {


    start=-1;
    stop=-1;

    
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%d",&n))
      ;
    else
      return string("Could not scan number of loops ")+string(args[1]);

    
      myDet->setCTBPatLoops(1,start, stop,n) ;
    }


    
    start=-1;
    stop=-1;
    n=-1; 
    myDet->setCTBPatLoops(1,start, stop,n);
    os << n ; 




  }	else if (cmd=="patnloop2") {


    start=-1;
    stop=-1;

    
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%d",&n))
      ;
    else
      return string("Could not scan number of loops ")+string(args[1]);

    
      myDet->setCTBPatLoops(2,start, stop,n) ;
    }


    
    start=-1;
    stop=-1;
    n=-1; 
    myDet->setCTBPatLoops(2,start, stop,n);
    os << n ; 



  } else if (cmd=="patwait0") {




   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan wait address (hex format)")+string(args[1]);

    
     myDet->setCTBPatWaitAddr(0,addr);
    }


    
   os <<   hex << myDet->setCTBPatWaitAddr(0,-1) << dec; 






  } else if (cmd=="patwait1") {


   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan wait address (hex format)")+string(args[1]);

    
     myDet->setCTBPatWaitAddr(1,addr);
    }


    
   os <<  hex <<  myDet->setCTBPatWaitAddr(1,-1) << dec; 



  } else if (cmd=="patwait2") {

   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan wait address (hex format)")+string(args[1]);

    
     myDet->setCTBPatWaitAddr(2,addr);
    }


    
   os << hex <<   myDet->setCTBPatWaitAddr(2,-1) << dec ; 

  }	else if (cmd=="patwaittime0") {





   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%lld",&t))
      ;
    else
      return string("Could not scan wait time")+string(args[1]);

    
     myDet->setCTBPatWaitTime(0,t);
    }


    
    os <<  myDet->setCTBPatWaitTime(0,-1); 







  } else if (cmd=="patwaittime1") {

   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%lld",&t))
      ;
    else
      return string("Could not scan wait time ")+string(args[1]);

    
     myDet->setCTBPatWaitTime(1,t);
    }


    
    os <<  myDet->setCTBPatWaitTime(1,-1); 



  }	else if (cmd=="patwaittime2") {
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%lld",&t))
      ;
    else
      return string("Could not scan wait time ")+string(args[1]);

    
     myDet->setCTBPatWaitTime(2,t);
    }


    
    os <<  myDet->setCTBPatWaitTime(2,-1); 

  }   else if  (cmd=="adcinvert") {
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan adcinvert reg ")+string(args[1]);

    
      myDet->writeRegister(67,addr);
    }


    
   os << hex << myDet->readRegister(67) << dec; 

  } else if  (cmd=="adcdisable") {
   if (action==PUT_ACTION) {
     
      if (sscanf(args[1],"%x",&addr))
      ;
    else
      return string("Could not scan adcdisable reg ")+string(args[1]);

    
      myDet->writeRegister(94,addr);
    }


    
   os << hex << myDet->readRegister(94) << dec; 

  } 



else  return helpPattern(narg, args, action);
  




  return os.str();

}



string slsDetectorCommand::helpPulse(int narg, char *args[], int action) {

  ostringstream os;
  if (action==PUT_ACTION || action==HELP_ACTION) {
    os << "pulse [n] [x] [y] \t pulses pixel at coordinates (x,y) n number of times" << std::endl;
	os << "pulsenmove [n] [x] [y]\t pulses pixel n number of times and moves relatively by x value (x axis) and y value(y axis)" << std::endl;
	os << "pulsechip [n] \t pulses chip n number of times, while n=-1 will reset it to normal mode" << std::endl;
  }
  if (action==GET_ACTION || action==HELP_ACTION){
    os << "pulse \t cannot get" << std::endl;
  	os << "pulsenmove \t cannot get" << std::endl;
  	os << "pulsechip \t cannot get" << std::endl;
  }
  return os.str();

}


string slsDetectorCommand::cmdPulse(int narg, char *args[], int action) {
	int retval = FAIL;

	if (action==HELP_ACTION)
		return helpPulse(narg, args, action);
	else if (action==GET_ACTION)
		return string("cannot get ")+cmd;

	myDet->setOnline(ONLINE_FLAG);


	int ival1=-1;
    if (!sscanf(args[1],"%d",&ival1))
    	return string("Could not scan 1st argument ")+string(args[1]);

	if (string(args[0])==string("pulsechip"))
		retval = myDet->pulseChip(ival1);


	else{
		//next commands requires 3 addnl. arguments
		int ival2=-1,ival3=-1;
		if(narg<4)
			return string("insufficient arguments:\n" + helpPulse(narg, args, action));
		if (!sscanf(args[2],"%d",&ival2))
			return string("Could not scan 2nd argument ")+string(args[2]);
		if (!sscanf(args[3],"%d",&ival3))
			return string("Could not scan 3rd argument ")+string(args[3]);


		if (string(args[0])==string("pulse"))
			retval = myDet->pulsePixel(ival1,ival2,ival3);

		else if (string(args[0])==string("pulsenmove"))
			retval = myDet->pulsePixelNMove(ival1,ival2,ival3);

		else return string("could not decode command")+cmd;

	}

	return string("");
/*
	if(retval == OK)
		return string(" successful");
	else
		return string(" failed");
		*/
}

























