 
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>


#include "EigerRegisterDefs.h"
#include "Eiger.h"

using namespace std;

//GetDAQStatusRegister(512,current_mode_bits_from_fpga)){

const unsigned int Module::ndacs         = 16;
const string       Module::dac_names[16] = {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};


Module::Module(unsigned int number, unsigned int address_top){
  module_number        = number;
  top_address_valid    = 1;
  top_left_address     = 0x100 | (0xff & address_top);
  top_right_address    = 0x200 | (0xff & address_top);
  bottom_address_valid = 0;
  bottom_left_address  = 0;
  bottom_right_address = 0;

  high_voltage          = -1;
  top_dac              = new float [ndacs];
  bottom_dac           = new float [ndacs];
  for(unsigned int i=0;i<ndacs;i++) top_dac[i]    = top_address_valid    ? -1:0;
  for(unsigned int i=0;i<ndacs;i++) bottom_dac[i] = bottom_address_valid ? -1:0;
}

Module::Module(unsigned int number, unsigned int address_top, unsigned int address_bottom){
  module_number        = number;
  top_address_valid    = 1;
  top_left_address     = 0x100 | (0xff & address_top);
  top_right_address    = 0x200 | (0xff & address_top);
  bottom_address_valid = 1;
  bottom_left_address  = 0x100 | (0xff & address_bottom);
  bottom_right_address = 0x200 | (0xff & address_bottom);

  high_voltage         = -1;

  for(unsigned int i=0;i<4;i++) idelay_top[i]=idelay_bottom[i]=0;

  top_dac              = new float [ndacs];
  bottom_dac           = new float [ndacs];
  for(unsigned int i=0;i<ndacs;i++) top_dac[i]    = top_address_valid    ? -1:0;
  for(unsigned int i=0;i<ndacs;i++) bottom_dac[i] = bottom_address_valid ? -1:0;
}


Module::~Module(){
  delete [] top_dac;
  delete [] bottom_dac;
}


Eiger::Eiger(){
  
  staticBits=acquireNReadoutMode=triggerMode=externalEnableMode=subFrameMode=0;

  trimbit_size=263680;
  last_downloaded_trimbits = new unsigned char [trimbit_size];

  Init();
}

Eiger::~Eiger(){
  delete[] last_downloaded_trimbits;
  ClearModules();
}

void Eiger::ClearModules(){
  for(unsigned int i=0;i<modules.size();i++) delete modules[i];
  modules.clear();
}

bool Eiger::Init(){
  ClearModules();

  AddModule(0,0xfff); //global send

  cout<<endl<<"Default Settings:"<<endl;
  nimages                = 1;
  exposure_time_in_sec   = 1;
  exposure_period_in_sec = 0;
  SetTestModeVariable(0);
  //SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
  //SetRateCorrection(0); //deactivate rate correction
  SetDynamicRange(16);
  SetPhotonEnergy(8000);
  SetReadoutMode();
  SetReadoutSpeed();
  SetTriggerMode();
  SetExternalEnableMode();
  cout<<endl<<endl;


  ReadSetUpFileToAddModules("setup.txt");
  cout<<"Setting detector defaults:"<<endl;
    ReadSetUpFile(0,"setup.txt"); //send defaults to all
    for(unsigned int i=1;i<modules.size();i++){
      char st[2000];
      sprintf(st,"setup_mod%04d.txt",modules[i]->GetModuleNumber());
      ReadSetUpFile(modules[i]->GetModuleNumber(),st);
    }

  return CheckSetup();
}


bool Eiger::ReadSetUpFileToAddModules(string file_name){
  static ifstream infile;
  static string   line;
  static char     cmd_st[2000];
  static int      value_i[3];
  
  infile.open(file_name.c_str(),ios::in);
  if(!infile.is_open()) return 0;

  cout<<endl;
  cout<<"Setting up detector:"<<endl;
  while(std::getline(infile,line)){
    if(line.length()<1) continue;
    istringstream iss(line);
      iss>>cmd_st;
    if(!strcmp("add_module",cmd_st)){
      if(!(iss>>value_i[0]>>value_i[1]>>value_i[2])){
	cout<<"Error adding module from "<<file_name<<"."<<endl;
	exit(0);
      }
      if(!AddModule(value_i[0],value_i[1],value_i[2])){
	cout<<"Error adding module, parameter was assigned twice in setup file: "<<file_name<<endl;
	exit(0);
      }
    }else if(!strcmp("add_half_module",cmd_st)){
      if(!(iss>>value_i[0]>>value_i[1])){
	cout<<"Error adding half module from "<<file_name<<"."<<endl;
	exit(0);
      }
      if(!AddModule(value_i[0],value_i[1])){
	cout<<"Error adding module, parameter was assigned twice in setup file: "<<file_name<<endl;
	exit(0);
      }
    }
  }

  infile.close();
  PrintModuleList();
  unsigned int nfebs = 0;
  unsigned int* feb_list = new unsigned int [modules.size()*4];
  for(unsigned int i=1;i<modules.size();i++){
    if(modules[i]->TopAddressIsValid()){
      feb_list[nfebs++] = modules[i]->GetTopRightAddress();
      feb_list[nfebs++] = modules[i]->GetTopLeftAddress();
    }
    if(modules[i]->BottomAddressIsValid()){
      feb_list[nfebs++] = modules[i]->GetBottomRightAddress();
      feb_list[nfebs++] = modules[i]->GetBottomLeftAddress();
    }
  }
  
  SendCompleteFebList(nfebs,feb_list);
  delete [] feb_list;

  cout<<endl;

  return CheckCommunication();
}

void Eiger::PrintModuleList(){
  cout<<"\tModule list:"<<endl;
  for(unsigned int i=0;i<modules.size();i++){
    if(i==0)      cout<<"\t\t"<<i<<") All    modules: ";
    else if(i==1) cout<<"\t\t"<<i<<") Master module : ";
    else          cout<<"\t\t"<<i<<")        module : ";
    cout<<modules[i]->GetModuleNumber()<<"    ";
    if(modules[i]->TopAddressIsValid())    cout<<hex<<modules[i]->GetTopBaseAddress()<<" (top)    "<<dec;
    if(modules[i]->BottomAddressIsValid()) cout<<hex<<modules[i]->GetBottomBaseAddress()<<" (bottom)"<<dec;
    cout<<endl;
  }
}

bool Eiger::GetModuleIndex(unsigned int module_number, unsigned int& module_index){
  for(unsigned int i=0;i<modules.size();i++){
    if(modules[i]->GetModuleNumber()==module_number){
      module_index=i;
      return 1;
    }
  }

  return 0;
}

bool Eiger::CheckModuleAddresses(unsigned int top_address, unsigned int bottom_address){
  bool found_t = 0;
  bool found_b = 0;
  for(unsigned int i=0;i<modules.size();i++){
    if(top_address!=0    && (top_address==modules[i]->GetTopBaseAddress()    || top_address==modules[i]->GetBottomBaseAddress())) found_t=1;
    if(bottom_address!=0 && (bottom_address==modules[i]->GetTopBaseAddress() || bottom_address==modules[i]->GetBottomBaseAddress())) found_b=1;
  }

  if(top_address==bottom_address) cout<<"\tWarning: top and bottom address are the same "<<top_address<<"."<<endl; 
  if(found_t) cout<<"\tWarning: top address "<< top_address<<" already used."<<endl; 
  if(found_b) cout<<"\tWarning: bottom address "<< bottom_address<<" already used."<<endl; 

  return !(top_address==bottom_address||found_t||found_b);
}

bool Eiger::AddModule(unsigned int module_number, unsigned int top_address){
  return AddModule(module_number,top_address,0,1);
}
bool Eiger::AddModule(unsigned int module_number, unsigned int top_address, unsigned int bottom_address, bool half_module){ //bot_address 0 for half module
  bool parameters_ok  = 1;
  unsigned int pre_module_index = 0;
  if(GetModuleIndex(module_number,pre_module_index)){
    cout<<"\tRemoving previous assignment of module number "<<module_number<<"."<<endl; 
    delete modules[pre_module_index]; 
    modules.erase(modules.begin()+pre_module_index);
    parameters_ok = 0;
  }
  parameters_ok&&CheckModuleAddresses(top_address,bottom_address);

  if(half_module){
    cout<<"\tAdding half module number "<<module_number<<" with base address: "<<top_address<<endl;
    modules.push_back(new Module(module_number,top_address));
  }else{
    cout<<"\tAdding full module number "<<module_number<<" with top and bottom base addresses: "<<top_address<<"  "<<bottom_address<<endl;
    modules.push_back(new Module(module_number,top_address,bottom_address));
  }


  return parameters_ok;
}


bool Eiger::ReadSetUpFile(unsigned int module_num, string file_name){
  static ifstream infile;
  static string   line;
  static char     cmd_st[2000];
  static int      value_i[3];
  static float    value_f;
  
  infile.open(file_name.c_str(),ios::in);
  if(!infile.is_open()){
    cout<<"Warning setup file not found: "<<file_name<<"."<<endl;
    return 0;
  }

  while(std::getline(infile,line)){
    if(line.length()<1) continue;
    istringstream iss(line);
      iss>>cmd_st;

    if(cmd_st[0]=='#'||!strcmp("add_module",cmd_st)||!strcmp("add_half_module",cmd_st)){ ;//  cout<<"do nothing "<<cmd_st<<"   "<<value_st<<endl;

    }else if(!strcmp("iodelay",cmd_st)){
      if(!(iss>>value_i[0])) cout<<"Error reading iodelay from "<<file_name<<"."<<endl;
      SetIDelays(module_num,value_i[0]);
    }else if(!strcmp("high_voltage",cmd_st)){
      iss>>value_f;
      SetHighVoltage(module_num,value_f);
    }else if(!strcmp("photon_energy",cmd_st)){
      iss>>value_f;
      SetPhotonEnergy(value_f);
    }else if(!strcmp("dynamic_range",cmd_st)){
      iss>>value_i[0];
      SetDynamicRange(value_i[0]);
    }else if(!strcmp("readout_speed",cmd_st)){
      iss>>value_i[0];
      SetReadoutSpeed(value_i[0]);
    }else if(!strcmp("readout_mode",cmd_st)){
      iss>>value_i[0];
      SetReadoutMode(value_i[0]);
    }else{
      iss>>value_f;
      if(module_num>0)            sprintf(cmd_st,"mod%d::%s",module_num,cmd_st);
      if(!SetDAC(cmd_st,value_f)) cout<<"error in string: "<<cmd_st<<endl;
    }
    // cout<<cmd_st<<"     "<<value_st<<endl;
  }
  infile.close();
  return 1;
}


bool Eiger::CheckSetup(){
  bool ok = 1;

  for(unsigned int i=0;i<modules.size();i++){
    for(unsigned int j=0;j<4;j++){
      if(modules[i]->GetTopIDelay(j)<0){
	cout<<"Warning: module "<<modules[i]->GetModuleNumber()<<"'s idelay top number "<<j<<"not set."<<endl;
	ok=0;
      }
      if(modules[i]->GetBottomIDelay(j)<0){
	cout<<"Warning: module "<<modules[i]->GetModuleNumber()<<"'s idelay bottom number "<<j<<"not set."<<endl;
	ok=0;
      }
    }
    if(modules[i]->GetHighVoltage()<0){
      cout<<"Warning: module "<<modules[i]->GetModuleNumber()<<"'s high voltage not set."<<endl;
      ok=0;
    }
    for(unsigned int j=0;j<Module::ndacs;j++){
      if(modules[i]->GetTopDACVoltage(j)<0){
	cout<<"Warning: module "<<modules[i]->GetModuleNumber()<<"'s top \""<<Module::dac_names[i]<<"\" dac is not set."<<endl;
	ok=0;
      }
      if(modules[i]->GetBottomDACVoltage(j)<0){
	cout<<"Warning: module "<<modules[i]->GetModuleNumber()<<"'s bottom \""<<Module::dac_names[i]<<"\" dac is not set."<<endl;
	ok=0;
      }
    }
  }

  return ok;			     
}

unsigned int Eiger::GetNModules(){
  if(modules.size()<=0) return 0;
  return modules.size() - 1;
}

unsigned int Eiger::GetNHalfModules(){
  unsigned int n_half_modules = 0;
  for(unsigned int i=0;i<modules.size();i++){
    if(modules[i]->TopAddressIsValid()) n_half_modules++;
    if(modules[i]->BottomAddressIsValid()) n_half_modules++;
  }
  
  return n_half_modules;
}

bool Eiger::SetPhotonEnergy(unsigned int full_energy_eV){
  photon_energy_eV = full_energy_eV;
  cout<<"Setting photon energy to: "<<photon_energy_eV<<" eV"<<endl;

  return 1;
}

bool Eiger::SetIDelays(unsigned int module_num, unsigned int ndelay_units){
  return SetIDelays(module_num,0,ndelay_units)&&SetIDelays(module_num,1,ndelay_units)&&SetIDelays(module_num,2,ndelay_units)&&SetIDelays(module_num,3,ndelay_units);
}

bool Eiger::SetIDelays(unsigned int module_num, unsigned int chip_pos, unsigned int ndelay_units){  //chip_pos 0=ll,1=lr,0=rl,1=rr
  //currently set same for top and bottom
  if(chip_pos>3){
    cout<<"Error SetIDelay chip_pos "<<chip_pos<<" doesn't exist."<<endl;
    return 0;
  }

  unsigned int module_index=0;
  if(!GetModuleIndex(module_num,module_index)){
    cout<<"Error could not set i delay module number "<<module_num<<" invalid."<<endl;
    return 0;
  }

  bool ok = 1;
  if(chip_pos/2==0){ //left fpga
    if(modules[module_index]->TopAddressIsValid()){
      if(SendIDelays(modules[module_index]->GetTopLeftAddress(),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) modules[module_index]->SetTopIDelay(chip_pos,ndelay_units);
	else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetTopIDelay(chip_pos,ndelay_units);
      }else{
	cout<<"Error could not set idelay module number "<<module_num<<" (top_left)."<<endl;
	ok=0;
      }
    }
    if(modules[module_index]->BottomAddressIsValid()){
      if(SendIDelays(modules[module_index]->GetBottomLeftAddress(),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) modules[module_index]->SetBottomIDelay(chip_pos,ndelay_units);
	else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetBottomIDelay(chip_pos,ndelay_units);
      }else{
	cout<<"Error could not set idelay module number "<<module_num<<" (bottom_left)."<<endl;
	ok=0;
      }
    }
  }else{
    if(modules[module_index]->TopAddressIsValid()){
      if(SendIDelays(modules[module_index]->GetTopRightAddress(),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) modules[module_index]->SetTopIDelay(chip_pos,ndelay_units);
	else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetTopIDelay(chip_pos,ndelay_units);
      }else{
	cout<<"Error could not set idelay module number "<<module_num<<" (top_right)."<<endl;
	ok=0;
      }
    }
    if(modules[module_index]->BottomAddressIsValid()){
      if(SendIDelays(modules[module_index]->GetBottomRightAddress(),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) modules[module_index]->SetBottomIDelay(chip_pos,ndelay_units);
	else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetBottomIDelay(chip_pos,ndelay_units);
      }else{
	cout<<"Error could not set idelay module number "<<module_num<<" (bottom_right)."<<endl;
	ok=0;
      }
    }
  }

  return ok;
}


bool Eiger::SendIDelays(unsigned int dst_num, bool chip_lr, unsigned int channels, unsigned int ndelay_units){
  //  cout<<"sending idelay :"<<dst_num<<" (lr-"<<chip_lr<<") to "<<ndelay_units<<endl;

  if(ndelay_units>0x3ff) ndelay_units=0x3ff;
  // this is global
  unsigned int delay_data_valid_nclks =  15 - ((ndelay_units&0x3c0)>>6); //data valid delay upto 15 clks
  ndelay_units &= 0x3f; //up to 64 delay units 
  //      ndelay_unit |= 0x40; //and one bit for a full clock delay for the enabled pixels is zero for now

  unsigned int set_left_delay_channels  = chip_lr ? channels:0;
  unsigned int set_right_delay_channels = chip_lr ?        0:channels;

  cout<<"\tSetting delays of ";
  if(set_left_delay_channels!=0)       cout<<"left chips of dst_num "<<dst_num;
  else if(set_right_delay_channels!=0) cout<<"right chips of dst_num "<<dst_num;
  //  else if(dst_num<0&&set_right_delay_channels!=0) cout<<"right chips";
  //  else if(set_left_delay_channels!=0)              cout<<"chip "<<dst_num;
  //  else if(set_right_delay_channels!=0)             cout<<"chip "<<dst_num;
  cout<<", tracks 0x"<<hex<<channels<<dec<<" to: "<<(((15-delay_data_valid_nclks)<<6)|ndelay_units)<<", "<<delay_data_valid_nclks<<" clks and "<<ndelay_units<<" units."<<endl;

  if(!WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG2, 1<<31 | delay_data_valid_nclks<<16 | ndelay_units) || //the 1<<31 time enables the setting of the data valid delays
     !WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG3,set_left_delay_channels)  ||
     !WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG4,set_right_delay_channels) ||
     !WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG_CTRL,CHIP_DATA_OUT_DELAY_SET,1,1)){
    cout<<"Warning: could not SetChipDataInputDelays(...)."<<endl;
    return 0;
  }

  return 1;
}

/*float Eiger::GetIDelays(){
  return idelay;
}
*/

bool Eiger::VoltageToDAC(float value, unsigned int& digital,unsigned int nsteps,float vmin,float vmax){
  if(value<vmin||value>vmax) return 0;
  digital = int(((value-vmin)/(vmax-vmin))*(nsteps-1) + 0.5);
  return 1;
}

float Eiger::DACToVoltage(unsigned int digital,unsigned int nsteps,float vmin,float vmax){
  return vmin+(vmax-vmin)*digital/(nsteps-1);
}


bool Eiger::SetHighVoltage(unsigned int module_num,float value){
  unsigned int module_index=0;
  if(!GetModuleIndex(module_num,module_index)||!modules[module_index]->TopAddressIsValid()){
    cout<<"Error could not set high voltage module number "<<module_num<<" invalid."<<endl;
    return 0;
  }

  if(!SendHighVoltage(modules[module_index]->GetTopRightAddress(),value)) return 0;

  if(module_index!=0)                             modules[module_index]->SetHighVoltage(value);
  else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetHighVoltage(value);

  cout<<"\tHigh voltage of dst "<<modules[module_index]->GetTopRightAddress()<<" set to "<<modules[module_index]->GetHighVoltage()<<"."<<endl;
  return 1;
}


bool Eiger::SendHighVoltage(unsigned int dst_num,float& value){
  //  cout<<"sending high voltage to dst_num "<<dst_num<<"."<<endl;

  static const unsigned int nsteps = 256;
  static const float vmin=0;
  static const float vmax=300;

  unsigned int b = 0;
  if(!VoltageToDAC(value,b,nsteps,vmin,vmax)){
    cout<<"Waring: SetHighVoltage bad value, "<<value<<". The range is 0 to 300 V."<<endl;
    return 0;
  }

  unsigned int r = 0x20000000 | (b&0xff);
  if(!WriteRegister(dst_num,0,r)){
    cout<<"Warning: trouble setting high voltage for dst_num "<<dst_num<<"."<<endl;
    return 0;
  }

  value = DACToVoltage(b,nsteps,vmin,vmax);

  return 1;
}




bool Eiger::SetDAC(string dac_str, float value){

  string       local_s = dac_str;
  unsigned int module_index  = 0;

  size_t p1 = local_s.find("mod");
  size_t p2 = local_s.find("::");
  if(p1!=string::npos&&p2!=string::npos&&(p1+3)<p2){
    unsigned int number = atoi((local_s.substr(p1+3,p2-3)).c_str()); 
    if(!GetModuleIndex(number,module_index)){
      cout<<"Error in dac_name \""<<dac_str<<"\", module number "<<number<<" not in list."<<endl; 
      return 0;
    }
    local_s    = local_s.substr(p2+2);
  }

  bool top    = 1;
  bool bottom = 1;
  if((p1 = local_s.find("top::"))!=string::npos){
    local_s = local_s.substr(p1+5);
    bottom=0;
  }else if((p1 = local_s.find("bottom::"))!=string::npos){
    local_s = local_s.substr(p1+8);
    top=0;
  }
  
  unsigned int dac_ch = 0;
  if(!GetDACNumber(local_s,dac_ch)) return 0;

  if(top&&modules[module_index]->TopAddressIsValid()){
    float v = value;
    if(!SendDACValue(modules[module_index]->GetTopRightAddress(),dac_ch,v)) return 0;
    if(module_index!=0)                             modules[module_index]->SetTopDACVoltage(dac_ch,v);
    else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetTopDACVoltage(dac_ch,v);

  }
  if(bottom&&modules[module_index]->BottomAddressIsValid()){
    float v = value;
    if(!SendDACValue(modules[module_index]->GetBottomRightAddress(),dac_ch,v)) return 0;
    if(module_index!=0)                             modules[module_index]->SetBottomDACVoltage(dac_ch,v);
    else for(unsigned int i=0;i<modules.size();i++) modules[i]->SetBottomDACVoltage(dac_ch,v);
  }

  return 1;
}

bool Eiger::GetDAC(std::string s, float& ret_value){
  cout<<"Function Eiger::GetDAC need to be finished....."<<endl;
  ret_value=1.2;
  return 1;
}

bool Eiger::GetDACName(unsigned int dac_num, std::string &s){
  if(dac_num>=Module::ndacs){
    cout<<"Warning: Eiger::GetDACName index out of range, "<<dac_num<<" invalid."<<endl;
    return 0;
  }
  s = Module::dac_names[dac_num];
  return 1;
}

bool Eiger::GetDACNumber(string s, unsigned int& n){
  for(unsigned int i=0;i<Module::ndacs;i++){
    if(!strcmp(Module::dac_names[i].c_str(),s.c_str())){
      n=i;
      return 1;
    }
  }

  return 0;
}
bool Eiger::SendDACValue(unsigned int dst_num, unsigned int ch, float& value){

  static unsigned int nsteps = 4096; //12 bit dac
  static float vmin = 0;
  static float vmax = 2;

  if(ch<0||ch>15){
    cout<<"Warning invalid ch for SetDAC."<<endl;
    return 0;
  }

  //if(voltage<0) return PowerDownDAC(socket_num,ch);

  unsigned int b = 0;
  if(!VoltageToDAC(value,b,nsteps,vmin,vmax)){
    cout<<"Waring: SetDac bad value, "<<value<<". The range is 0 to 2 V."<<endl;
    return 0;
  }

  unsigned int dac_ic = (ch<8) ? 1:2;
  unsigned int dac_ch =  ch%8;
  unsigned int r      = dac_ic<<30 | 3<<16 | dac_ch<<12 | (b&0xfff); //3 write and power up


  if(!WriteRegister(dst_num,0,r,1,0)){
     cout<<"Warning: trouble setting dac "<<ch<<" voltage."<<endl;
     return 0;
  }

  value=DACToVoltage(b,nsteps,vmin,vmax);

  cout<<"\tDac number"<<ch<<" ("<<Module::dac_names[ch]<<")  of dst "<<dst_num<<" set to "<<value<<"."<<endl;
  return 1;
}

/*
float Eiger::GetDAC(string s){
  static unsigned int n;
  if(!GetDACNumber(s,n)) return 0;

  return dac[n];
}
*/

bool Eiger::SetTrimbits(unsigned char *trimbits){
  
  if(!Reset()) cout<<"Warning could not reset DAQ."<<endl;

  for(int l_r=0;l_r<2;l_r++){
    unsigned int disable_chip_mask = l_r ? DAQ_CS_BAR_LEFT : DAQ_CS_BAR_RIGHT;
    
    if(!(WriteRegister(0xfff,DAQ_REG_STATIC_BITS,disable_chip_mask|DAQ_STATIC_BIT_PROGRAM|DAQ_STATIC_BIT_M8)&&SetCommandRegister(DAQ_SET_STATIC_BIT)&&StartDAQOnlyNWaitForFinish())){
      cout<<"Could not select chips"<<endl;
      return 0;
    }

    for(int row_set=0;row_set<16;row_set++){ //16 rows at a time
      if(row_set==0){
	if(!SetCommandRegister(DAQ_RESET_COMPLETELY|DAQ_SEND_A_TOKEN_IN|DAQ_LOAD_16ROWS_OF_TRIMBITS)){
	  cout<<"Warning: Could not SetCommandRegister for loading trim bits."<<endl;
	  return 0;
	}
      }else{
	if(!SetCommandRegister(DAQ_LOAD_16ROWS_OF_TRIMBITS)){
	  cout<<"Warning: Could not SetCommandRegister for loading trim bits."<<endl;
	  return 0;
	}
      }

      static unsigned int trimbits_to_load_l[1024];
      static unsigned int trimbits_to_load_r[1024];
      for(int row=0;row<16;row++){
	int offset   = 2*32*row; //2 lower upper offset, and 32 per row
	for(int sc=0;sc<32;sc++){ //shift_in element number 0-31, ie super column
	  int super_column_start_position_l = 1030*row +       l_r *258 + sc*8; //256 per row, 8 per super column
	  int super_column_start_position_r = 1030*row + 516 + l_r *258 + sc*8; //256 per row, 8 per super column
	  int chip_sc = 31 - sc; //also flipped
	  trimbits_to_load_l[offset+chip_sc] = 0;
	  trimbits_to_load_r[offset+chip_sc] = 0;
	  for(int i=0;i<8;i++){ //pixel number
	    trimbits_to_load_l[offset+chip_sc]    |= ( 0x7  & trimbits[263679 - (row_set*16480+super_column_start_position_l+i)])<<((7-i)*4);//low
	    trimbits_to_load_l[offset+chip_sc+32] |= ((0x38 & trimbits[263679 - (row_set*16480+super_column_start_position_l+i)])>>3)<<((7-i)*4);//upper
	    trimbits_to_load_r[offset+chip_sc]    |= ( 0x7  & trimbits[263679 - (row_set*16480+super_column_start_position_r+i)])<<((7-i)*4);//low
	    trimbits_to_load_r[offset+chip_sc+32] |= ((0x38 & trimbits[263679 - (row_set*16480+super_column_start_position_r+i)])>>3)<<((7-i)*4);//upper
	  }
	}
      }

      if(!WriteMemory(0,0,0,1024,trimbits_to_load_r)||!WriteMemory(1,0,0,1024,trimbits_to_load_l)||!StartDAQOnlyNWaitForFinish()) return 0;
    }
  }

  memcpy(last_downloaded_trimbits,trimbits,trimbit_size*sizeof(unsigned char));

  return SetStaticBits(); //send the static bits
}


unsigned char* Eiger::GetTrimbits(){
  return last_downloaded_trimbits;
}




bool Eiger::SetCommandRegister(unsigned int cmd){
  //  if(fifo_enabled){
  //    return WriteRegister(-1,DAQ_REG_CHIP_CMDS,cmd | DAQ_FIFO_ENABLE);
  //  }
  return WriteRegister(0xfff,DAQ_REG_CHIP_CMDS,cmd);
}


bool Eiger::GetDAQStatusRegister(int socket_num, unsigned int &ret_status){

  if(!ReadRegister(socket_num,DAQ_REG_STATUS,ret_status)){
    cout<<"Error: reading status register."<<endl;
    return 0;
  }
  
  ret_status = (0x00FF0000 & ret_status) >> 16;
  return 1;
}


bool Eiger::StartDAQOnlyNWaitForFinish(int sleep_time_us){
  if(!WriteRegister(0xfff,DAQ_REG_CTRL,0)||!WriteRegister(0xfff,DAQ_REG_CTRL,DAQ_CTRL_START)){
    cout<<"Warning: could not start."<<endl;
    return 0;
  }
  
  return WaitForFinishedFlag(sleep_time_us);
}

bool Eiger::WaitForFinishedFlag(int sleep_time_us){
  bool is_running = AcquisitionInProgress();
  while(is_running){
    usleep(sleep_time_us);
    is_running = AcquisitionInProgress();
  }
  if(is_running!=0){
    cout<<endl<<endl<<"Warning WaitForFinishedFlag comunication problem.."<<endl<<endl;
    return 0; //communication problem
  }

  return 1;
}

bool Eiger::AcquisitionInProgress(){
  unsigned int status_reg_r=0,status_reg_l=0;
  if(!(GetDAQStatusRegister(512,status_reg_r)&&GetDAQStatusRegister(256,status_reg_l))){
    cout<<"Waring trouble reading status register."<<endl;
    return -1;
  }

  return (status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUNNING;
}


bool Eiger::Reset(){
  if(!WriteRegister(0xfff,DAQ_REG_CTRL,0) || !WriteRegister(0xfff,DAQ_REG_CTRL,DAQ_CTRL_RESET) || !WriteRegister(0xfff,DAQ_REG_CTRL,0)){
    cout<<"Warning: Could not reset daq, no response from board."<<endl;
    return 0;
  }

  return WaitForFinishedFlag();
}




bool Eiger::SetStaticBits(){
  //program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64
  if(!WriteRegister(0xfff,DAQ_REG_STATIC_BITS,staticBits) || !SetCommandRegister(DAQ_SET_STATIC_BIT) || !StartDAQOnlyNWaitForFinish()){
    cout<<"Warning: Could not set static bits"<<endl;
    return 0;
  }

  return 1;
}
bool Eiger::SetStaticBits(unsigned int the_static_bits){
  staticBits = the_static_bits;
  return SetStaticBits();
}

bool Eiger::SetTestModeVariable(bool on){
  if(on) staticBits |=   DAQ_STATIC_BIT_CHIP_TEST;  //setting test bit to high
  else   staticBits &= (~DAQ_STATIC_BIT_CHIP_TEST); //setting test bit to low
  return 1;
}

bool Eiger::GetTestModeVariable(){
  return staticBits&DAQ_STATIC_BIT_CHIP_TEST;
}

bool Eiger::SetDynamicRange(unsigned int four_eight_sixteen_or_thirtytwo){  
  static unsigned int everything_but_bit_mode = DAQ_STATIC_BIT_PROGRAM|DAQ_STATIC_BIT_CHIP_TEST|DAQ_STATIC_BIT_ROTEST;
  if(four_eight_sixteen_or_thirtytwo==4){
    staticBits    = DAQ_STATIC_BIT_M4 | (staticBits&everything_but_bit_mode); //leave test bits in currernt state
    subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
  }else if(four_eight_sixteen_or_thirtytwo==8){
    staticBits    = DAQ_STATIC_BIT_M8 | (staticBits&everything_but_bit_mode);
    subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
  }else if(four_eight_sixteen_or_thirtytwo==16){ 
    staticBits    = DAQ_STATIC_BIT_M12 | (staticBits&everything_but_bit_mode);
    subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
  }else if(four_eight_sixteen_or_thirtytwo==32){ 
    staticBits    = DAQ_STATIC_BIT_M12 | (staticBits&everything_but_bit_mode);
    subFrameMode |= DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
  }else{
    cout<<"Warning: dynamic range ("<<four_eight_sixteen_or_thirtytwo<<") not valid, not setting bit mode."<<endl;
    cout<<"Set dynamic range int must equal 4,8 16, or 32."<<endl;
    return 0;
  }

  cout<<"Dynamic range set to: "<<four_eight_sixteen_or_thirtytwo<<endl;
  return 1;
}

unsigned int Eiger::GetDynamicRange(){
  if(subFrameMode|DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING) return 32;
  else if(DAQ_STATIC_BIT_M4&staticBits)                     return 4;
  else if(DAQ_STATIC_BIT_M8&staticBits)                     return 8;

  return 16;
}

bool Eiger::SetReadoutSpeed(unsigned int readout_speed){ //0->full,1->half,2->quarter or 3->super_slow
  acquireNReadoutMode &= (~DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED);
  if(readout_speed==1){
    acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_HALF_SPEED;
    cout<<"Everything at half speed, ie. reading with 50 MHz main clk (half speed) ...."<<endl;
  }else if(readout_speed==2){
    acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_QUARTER_SPEED;
    cout<<"Everything at quarter speed, ie. reading with 25 MHz main clk (quarter speed) ...."<<endl;
  }else if(readout_speed==3){
    acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED;
    cout<<"Everything at super slow speed, ie. reading with ~0.200 MHz main clk (super slow speed) ...."<<endl;
  }else{
    if(readout_speed){
      cout<<"Warning readout speed "<<readout_speed<<") unknown, defaulting to full speed."<<endl;
      cout<<"Everything at full speed, ie. reading with 100 MHz main clk (full speed) ...."<<endl;
      return 0;
    }
    cout<<"Everything at full speed, ie. reading with 100 MHz main clk (full speed) ...."<<endl;
  }

  return 1;
}

bool Eiger::SetReadoutMode(unsigned int readout_mode){ //0->parallel,1->non-parallel,2-> safe_mode
  acquireNReadoutMode &= (~DAQ_NEXPOSURERS_PARALLEL_MODE);
  if(readout_mode==1){
    acquireNReadoutMode |= DAQ_NEXPOSURERS_NORMAL_NONPARALLEL_MODE;
    cout<<"Readout mode set to normal non-parallel readout mode ... "<<endl;
  }else if(readout_mode==2){
    acquireNReadoutMode |= DAQ_NEXPOSURERS_SAFEST_MODE_ROW_CLK_BEFORE_MODE;
    cout<<"Readout mode set to safest mode, row clk before main clk readout sequence .... "<<endl;
  }else{
    acquireNReadoutMode |= DAQ_NEXPOSURERS_PARALLEL_MODE;
    if(readout_mode){
      cout<<"Warning readout mode "<<readout_mode<<") unknown, defaulting to full speed."<<endl;
      cout<<"Readout mode set to parrallel acquire/read mode ....     "<<endl;
      return 0;
    }
    cout<<"Readout mode set to parrallel acquire/read mode ....     "<<endl;
  }

  return 1;
}

bool Eiger::SetTriggerMode(unsigned int trigger_mode,bool polarity){
  //"00"-> internal exposure time and period,                                                                                    
  //"01"-> external acquistion start and internal exposure time and period, 
  //"10"-> external start trigger and internal exposure time, 
  //"11"-> external triggered start and stop of exposures
  triggerMode  = (~DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP);

  if(trigger_mode == 1){
    triggerMode = DAQ_NEXPOSURERS_EXTERNAL_ACQUISITION_START;
    cout<<"Trigger mode: external start of acquisition sequence, internal exposure length and period."<<endl;
  }else if(trigger_mode == 2){
    triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START;
    cout<<"Trigger mode: external image start, internal exposure time."<<endl;
  }else if(trigger_mode == 3){
    triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP;
    cout<<"Trigger mode: externally controlled, external image window (start and stop)."<<endl;
  }else{
    triggerMode = DAQ_NEXPOSURERS_INTERNAL_ACQUISITION;
    if(trigger_mode) cout<<"Warning trigger "<<trigger_mode<<") unknown, defaulting to internal triggering."<<endl;

    cout<<"Trigger mode: acquisition internally controlled exposure length and period."<<endl;
    return trigger_mode==0;
  }

  if(polarity){
    triggerMode |=  DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY;
    cout<<"External trigger polarity set to positive."<<endl;
  }else{
    triggerMode &= (~DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY);
    cout<<"External trigger polarity set to negitive."<<endl;
  }

  return 1;
}


bool Eiger::SetExternalEnableMode(bool use_external_enable, bool polarity){
  if(use_external_enable){
    externalEnableMode  |= DAQ_NEXPOSURERS_EXTERNAL_ENABLING;
    cout<<"External enabling enabled, ";
    if(polarity){
      externalEnableMode |= DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY;
      cout<<", polarity set to positive."<<endl;
    }else{
      externalEnableMode &= (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY);
      cout<<", polarity set to negative."<<endl;
    }
  }else{
    externalEnableMode  &= (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING);
    cout<<"External enabling disabled."<<endl;
  }

  return 1;
}

bool Eiger::SetNImages(unsigned int n_images){ 
  if(!nimages){
    cout<<"Warning nimages must be greater than zero."<<nimages<<endl;
    return 0;
  }

  nimages = n_images;
  cout<<"Number of images set to: "<<nimages<<endl;
  return 1;
}
unsigned int Eiger::GetNImages(){return nimages;}

bool Eiger::SetExposureTime(float the_exposure_time_in_sec){
  exposure_time_in_sec = the_exposure_time_in_sec;
  cout<<"Exposure time set to: "<<exposure_time_in_sec<<endl;
  return 1;
}
float Eiger::GetExposureTime(){return exposure_time_in_sec;}

bool Eiger::SetExposurePeriod(float the_exposure_period_in_sec){
  exposure_period_in_sec = the_exposure_period_in_sec;
  cout<<"Exposure period set to: "<<exposure_period_in_sec<<endl;
  return 1;
}
float Eiger::GetExposurePeriod(){return exposure_period_in_sec;}

unsigned int Eiger::ConvertTimeToRegister(float time_in_sec){
  float n_clk_cycles = round(time_in_sec/10e-9); //200 MHz ctb clk or 100 MHz feb clk

  unsigned int decoded_time;
  if(n_clk_cycles>(pow(2,29)-1)*pow(10,7)){
    float max_time = 10e-9*(pow(2,28)-1)*pow(10,7);
    cout<<"Warning: time exceeds ("<<time_in_sec<<") maximum exposure time of "<<max_time<<" sec."<<endl;
    cout<<"\t Setting to maximum "<<max_time<<" us."<<endl;
    decoded_time = 0xffffffff;
  }else{
    int power_of_ten = 0;
    while(n_clk_cycles>pow(2,29)-1){ power_of_ten++; n_clk_cycles = round(n_clk_cycles/10.0);}
    decoded_time = int(n_clk_cycles)<<3 | int(power_of_ten);
  }

  return decoded_time;
}


bool Eiger::ResetDataStream(){
  //for(int i=0;i<10;i++) cout<<"Warning need to think about reseting data stream ...."<<endl;
  return 1;
}


bool Eiger::ResetChipCompletely(){
  if(!SetCommandRegister(DAQ_RESET_COMPLETELY) || !StartDAQOnlyNWaitForFinish()){
    cout<<"Warning: could not ResetChipCompletely()."<<endl;
    return 0;
  }   

  return 1;
}


void Eiger::PrintAcquisitionSetup(){
  cout<<"Should print Acquisition here ..."<<endl;
}

bool Eiger::StartAcquisition(){

  static unsigned int reg_nums[20];
  static unsigned int reg_vals[20];

  PrintAcquisitionSetup();

  if(!Reset()||!ResetDataStream()){
    cout<<"Trouble reseting daq or data stream..."<<endl;
    return 0;
  }

  if(!SetStaticBits(staticBits&(DAQ_STATIC_BIT_M4|DAQ_STATIC_BIT_M8))){
    cout<<"Trouble setting static bits ..."<<endl;
    return 0;
  }
  
  if(!ResetChipCompletely()){
    cout<<"Trouble resetting chips ..."<<endl;
    return 0;
  }

  reg_nums[0]=DAQ_REG_CTRL;
  reg_vals[0]=0;
  reg_nums[1]=DAQ_REG_NEXPOSURES;
  reg_vals[1]=nimages;
  reg_nums[2]=DAQ_REG_EXPOSURE_TIMER;
  reg_vals[2]=ConvertTimeToRegister(exposure_time_in_sec);
  reg_nums[3]=DAQ_REG_EXPOSURE_REPEAT_TIMER;
  reg_vals[3]=ConvertTimeToRegister(exposure_period_in_sec);
  reg_nums[4]=DAQ_REG_CHIP_CMDS;
  reg_vals[4]=(acquireNReadoutMode|triggerMode|externalEnableMode|subFrameMode);
  for(int i=5;i<19;i++){
    reg_nums[i]=DAQ_REG_CTRL;
    reg_vals[i]=0;
  }
  reg_nums[19]=DAQ_REG_CTRL;
  reg_vals[19]=ACQ_CTRL_START;
  
  if(!WriteRegisters(0xfff,20,reg_nums,reg_vals)){
    cout<<"Trouble starting acquisition...."<<endl;
    return 0;
  }

  return 1;  
}

bool Eiger::StopAcquisition(){
  return Reset();
}

// bool PulsePixelNMove(int npulses, bool inc_x_pos=0, bool inc_y_pos=0);
// int  PulsePixel(int x, int y, int npulses);
// int  PulseChip(int npulses=1);




