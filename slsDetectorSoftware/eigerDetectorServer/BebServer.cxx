
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>    // std::remove_if

#include <iostream>
#include <string>
#include <map>
#include <time.h>
#include <string.h>

#include "Beb.h"
#include "slsDetectorServer_defs.h" //include port number

using namespace std;

enum cmd_string {evNotFound,
		 evRequestImages,evTestRequest,
		 evSetBitMode,
		 evSetupTableEntry,evSetDstParameters,
		 evTest,evTestSend,
		 evExitServer};

map<string, cmd_string> enum_map;

void init(){

  enum_map["requestimages"]          = evRequestImages;     //<dst>
  enum_map["testrequest"]            = evTestRequest;       //<dst>
  
  enum_map["setbitmode"]             = evSetBitMode;        // (resets on_dst and dst_requested)

  enum_map["setuptableentry"]        = evSetupTableEntry;
  enum_map["setdstparameters"]       = evSetDstParameters; //<one_ten> <ndsts> <nimages_per_request> (resets on_dst and dst_requested)

  enum_map["test"]                   = evTest; 
  enum_map["testsend"]               = evTestSend;
  enum_map["exitserver"]             = evExitServer;

}

int server_list_s;
int server_conn_s;
int AccpetConnectionAndWaitForData(char* buffer, int maxlength);
bool WriteNClose(const char* buffer, int length);
bool SetupListenSocket(unsigned short int port);


string LowerCase(string str);
string GetNextString(string str,bool start_from_beginning=0);
void AddNumber(string& str, int n, int location=-1);//-1 means append

int main(int argc, char* argv[]){
  cout<<endl<<endl;

  /*
  if(argc<2){
    cout<<"Usage: eiger_beb_server port_number"<<endl<<endl;
    return 1;
  }
  */
  init();

  int arg1;
  Beb *bebs;

  if(argc>1)
	  bebs = new Beb(atoi(argv[1]));
  else
	  bebs = new Beb(-1);

  //  unsigned short int port_number = atoi(argv[1]);

  unsigned short int port_number = BEB_PORT;
  if(!SetupListenSocket(port_number)) return 1;
  

  int  length=1000;
  char data[1000];

  int stop = 0;
  time_t rawtime;
  struct tm *timeinfo;

  bool send_to_ten_gig = 0;
  int  ndsts_in_use=32;
  unsigned int nimages_per_request=1;

  int  on_dst=0;
  bool dst_requested[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  while(!stop){

    cout<<endl<<"\n\n\n\nWaiting for command -> "<<flush;
      int nread = AccpetConnectionAndWaitForData(data,length);
      if(nread<=0) return 0;

      time(&rawtime); timeinfo=localtime(&rawtime);
      cout<<asctime(timeinfo);
      cout<<"  Command received: "<<data<<endl<<endl;



    string tmp_str[5];
    //float v1,v2,v3,v4,v5;
    int n[5];

    string cmd  = GetNextString(data,1);
    //static char retval_st[100]; 
    int    ret_val = 1;
    string return_message = "    Command recieved: ";
      return_message.append(data);
      return_message.append("\n");
      
    int return_start_pos;
    while(cmd.length()>0){
      return_start_pos = return_message.length();

      switch(enum_map.find(LowerCase(cmd))->second){

        case evRequestImages :
	  tmp_str[0] = GetNextString(data);
	  n[0] = atoi(tmp_str[0].data()); //dst number
	  if(tmp_str[0].length()<1||n[0]<0||n[0]>=ndsts_in_use){
	    return_message.append("\tError executing: RequestImages <dst_number> (note dst_number must be less than ndsts_in_use that is set with SetDstParameters\n");
	    ret_val = 1;
	  }else{
	    dst_requested[n[0]] = 1;
	    ret_val=0;
	    tmp_str[1] = "  ( ";
	    while(dst_requested[on_dst]){
	      //waits on data
	      if((ret_val = (!bebs->RequestNImages(0,1,send_to_ten_gig,on_dst,nimages_per_request)||!bebs->RequestNImages(0,2,send_to_ten_gig,0x20|on_dst,nimages_per_request)))) break;
	      AddNumber(tmp_str[1],on_dst);tmp_str[1].append(" ");
	      dst_requested[on_dst++]=0;
	      on_dst%=ndsts_in_use;
	    }
	    if(ret_val) return_message.append("\tError executing: RequestImages <dst_number>");
	    else{       return_message.append("\tExecuted:  RequestImages "); AddNumber(return_message,n[0]);}
	    return_message.append(tmp_str[1]);	      
	    return_message.append(" )\n");
	  }
	  break;

        case evTestRequest :
	  tmp_str[0] = GetNextString(data);
	  n[0] = atoi(tmp_str[0].data()); //dst number
	  if(tmp_str[0].length()<1||n[0]<0||n[0]>=ndsts_in_use){
	    return_message.append("\tError executing: TestRequest <dst_number> (note dst_number must be less than 2xndsts_in_use that is set with SetDstParameters\n");
	    ret_val = 1;
	  }else{
	    ret_val = (!bebs->RequestNImages(0,1,send_to_ten_gig,n[0],nimages_per_request,1)||!bebs->RequestNImages(0,2,send_to_ten_gig,0x20|n[0],nimages_per_request,1));
	    if(ret_val) return_message.append("\tError executing: TestRequest <dst_number>\n");
	    else{ return_message.append("\tExecuted:  TestRequest "); AddNumber(return_message,n[0]); return_message.append("\n");}
	  }
	  break;

        case evSetBitMode :
	  on_dst = 0;
	  for(n[0]=0;n[0]<32;n[0]++) dst_requested[n[0]] = 0; //clear dst requested
	  n[0] = atoi(GetNextString(data).data());
	  if((ret_val = !bebs->SetUpTransferParameters(n[0]))) return_message.append("\tError executing: SetBitMode <bit_mode 4,8,16,32>\n");
	  else{ return_message.append("\tExecuted: SetBitMode ");AddNumber(return_message,n[0]);return_message.append("\n");}
	  break;

        case evSetDstParameters : //move below  //<one_ten_gigabit> <ndsts> <nimages_per_request>
	  on_dst = 0;
	  for(n[0]=0;n[0]<32;n[0]++) dst_requested[n[0]] = 0; //clear dst requested
	  n[0] = atoi(GetNextString(data).data()); //<1GbE(0) or 10GbE(1)>
	  n[1] = atoi(GetNextString(data).data()); //<ndsts (1 to 32)>
	  n[2] = atoi(GetNextString(data).data()); // <nimages_per_request (>0)>

	  if((n[0]!=0&&n[0]!=1)||(n[1]<1||n[1]>32)||n[2]<1){
	    return_message.append("\tError executing: SetDstParameters  <1GbE(0) or 10GbE(1)> <ndsts> <nimages_per_request>\n");
	    ret_val=1;
	  }
	  else{
	    send_to_ten_gig = n[0];
	    ndsts_in_use=n[1];
	    nimages_per_request=n[2];
	    return_message.append("\tExecuted: SetDstParameters ");
	    AddNumber(return_message,n[0]);return_message.append(" ");
	    AddNumber(return_message,n[1]);return_message.append(" ");
	    AddNumber(return_message,n[2]);return_message.append(" ");
	    ret_val=0;
	  }
	  break;

        case evSetupTableEntry : 
	  n[0] = atoi(GetNextString(data).data()); //beb_number;
	  n[1] = atoi(GetNextString(data).data()); //<1GbE(0) or 10GbE(1)>
	  n[2] = atoi(GetNextString(data).data()); //header_number
	  tmp_str[0] = GetNextString(data); //src_mac
	  tmp_str[1] = GetNextString(data); //src_ip
	  n[3] = atoi(GetNextString(data).data()); //src_port
	  tmp_str[2] = GetNextString(data); //dst_mac
	  tmp_str[3] = GetNextString(data); //dst_ip
	  n[4] = atoi((tmp_str[4]=GetNextString(data)).data()); //dst_port

	  if(n[0]<1||(n[1]!=0&&n[1]!=1)||(n[2]<0||n[2]>63)||tmp_str[0].length()<1||tmp_str[1].length()<1||n[3]<0||tmp_str[2].length()<1||tmp_str[3].length()<1||n[4]<0||tmp_str[4].length()<1){
	    return_message.append("\tError executing: SetupTableEntry <beb_number> <1GbE(0) or 10GbE(1)> <dst_number> <src_mac> <src_ip> <src_port> <dst_mac> <dst_ip> <dst_port>\n");
	    ret_val = 1;
	  }else{
		  for(int i=0;i<32;i++)/** modified for Aldo*/
	    ret_val = !bebs->SetBebSrcHeaderInfos(n[0],n[1],tmp_str[0],tmp_str[1],n[3])||!bebs->SetUpUDPHeader(n[0],n[1],n[2]+i,tmp_str[2],tmp_str[3],n[4]);
	    
	    if(ret_val) return_message.append("\tError Executing: SetupTableEntry ");
	    else        return_message.append("\tExecuted: SetupTableEntry ");
	    AddNumber(return_message,n[0]);return_message.append(" ");
	    AddNumber(return_message,n[1]);return_message.append(" ");
	    AddNumber(return_message,n[2]);return_message.append(" ");
	    return_message.append(tmp_str[0]);return_message.append(" ");
	    return_message.append(tmp_str[1]);return_message.append(" ");
	    AddNumber(return_message,n[3]);return_message.append(" ");
	    return_message.append(tmp_str[2]);return_message.append(" ");
	    return_message.append(tmp_str[3]);return_message.append(" ");
	    AddNumber(return_message,n[4]);
	  }
	  break;

        case evTest : 
	  n[0] = atoi(GetNextString(data).data());
	  if(n[0]<1){
	    return_message.append("\tError executing: Test <beb_number>\n");
	    ret_val = 1;
	  }else{
	    ret_val = !bebs->Test(n[0]);
	    if(ret_val) return_message.append("\tError Executing: Test ");
	    else        return_message.append("\tExecuted: Test ");
	    AddNumber(return_message,n[0]);
	  }
	  break;


        case evTestSend : 
	  n[0] = atoi(GetNextString(data).data()); //beb_number;
	  n[1] = atoi(GetNextString(data).data()); //giga bit, ten giga bit
	  n[2] = atoi((tmp_str[0]=GetNextString(data)).data()); //header_number

	  if(n[0]<1||(n[1]!=0&&n[1]!=1)||(n[2]<0||n[2]>63)||tmp_str[0].length()<1){
	    return_message.append("\tError executing: TestSend <beb_number>  <1GbE(0) or 10GbE(1)> <dst_number>\n");
	    ret_val = 1;
	  }else{
	    ret_val = !bebs->SendMultiReadRequest(n[0],1,n[1],n[2],1,0);
	    
	    if(ret_val) return_message.append("\tError Executing: TestSend ");
	    else        return_message.append("\tExecuted: TestSend ");
	    AddNumber(return_message,n[0]);return_message.append(" ");
	    AddNumber(return_message,n[1]);return_message.append(" ");
	    AddNumber(return_message,n[2]);return_message.append(" ");
	  }
	  break;

        case evExitServer :
	  return_message.append("\tExiting Server ....\n");
	  stop = 1;
	  ret_val = -200;
	  break;

        default :
          return_message.append("\tWarning command \"");
	  return_message.append(cmd);
	  return_message.append("\" not found.\n");
	  return_message.append("\t\tValid commands: ");
	  map<string, cmd_string>::iterator it = enum_map.begin();
	  while(it!=enum_map.end()){
	    return_message.append((it++)->first);
	    return_message.append(" ");
	  }

          ret_val=-100;
          break;
      }

      return_message.append("\n");
      AddNumber(return_message,ret_val,return_start_pos);
      if(ret_val!=0) break;

      cmd = GetNextString(data);
    }
    return_message.append("\n\n\n");

    AddNumber(return_message,ret_val,0);
    cout<<return_message.c_str()<<endl;
    cout<<"\treturn: "<<ret_val<<endl;

    if(!WriteNClose(return_message.c_str(),return_message.length())) return 0;
  }

  
  delete bebs;

  return 0;
}

string LowerCase(string str){
  string s = str;
  string::iterator i = s.begin();
  while(i!=s.end()) *i=tolower(*(i++));
  return s;
}

string GetNextString(string str,bool start_from_beginning){
  static string::size_type start_pos = 0;
    if(start_from_beginning) start_pos = 0;
    
  while(start_pos != string::npos){
    string::size_type found = str.find_first_of(" ",start_pos);
    string sub = str.substr(start_pos,found-start_pos);

    start_pos = found;
    if(start_pos != string::npos) start_pos+=1;

    sub.erase(remove_if(sub.begin(),sub.end(), ::isspace ),sub.end());

    if(sub.length()>0) return sub;
  }
  
  return "";    
}



void AddNumber(string& str, int n, int location){
  static char retval_st[100]; 
  sprintf(retval_st,"%d",n);

  if(location<0) str.append(retval_st);
  else           str.insert(location,retval_st);
}


bool SetupListenSocket(unsigned short int port){
  server_list_s=0;
  server_conn_s=0;

  if((server_list_s = socket(AF_INET, SOCK_STREAM, 0))<0) return 0;
  
  struct sockaddr_in servaddr;  /*  socket address structure  */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

  if(bind(server_list_s,(struct sockaddr *) &servaddr,sizeof(servaddr))<0) return 0;

  if(listen(server_list_s,32) < 0){      // 1024   /*  Backlog for listen()   */
    return 0;
  }

  return 1;
}


int AccpetConnectionAndWaitForData(char* buffer, int maxlength){
  if(server_list_s==0||maxlength<=0) return 0;

  if((server_conn_s = accept(server_list_s,NULL,NULL))< 0) return 0;

  int nread = read(server_conn_s,buffer,maxlength-1);

  if(nread<0) return 0;

  buffer[nread]='\0';
  return nread;
}

bool WriteNClose(const char* buffer, int length){
  if(server_conn_s==0||length<=0) return 0;
  
  int nsent = write(server_conn_s,buffer,length);
  if(close(server_conn_s)<0) return 0;
  
  server_conn_s=0;
  return (nsent==length);
}




