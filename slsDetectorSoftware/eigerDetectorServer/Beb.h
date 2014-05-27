
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef BEB_H
#define BEB_H

#include <string>
#include <vector>

#include "LocalLinkInterface.h"


class BebInfo{
  
 private:
  unsigned int beb_number;
  unsigned int serial_address;
  std::string  src_mac_1GbE;
  std::string  src_mac_10GbE;
  std::string  src_ip_1GbE;
  std::string  src_ip_10GbE;
  unsigned int src_port_1GbE;
  unsigned int src_port_10GbE;

 public:
  BebInfo(unsigned int beb_num);
  ~BebInfo(){};

  bool         SetSerialAddress(unsigned int add);
  bool         SetHeaderInfo(bool ten_gig, std::string src_mac, std::string src_ip, unsigned int src_port);//src_port fixed 42000+beb_number or 52000 + beb_number);
  unsigned int GetBebNumber()           {return beb_number;}
  unsigned int GetSerialAddress()       {return serial_address;}
  std::string  GetSrcMAC(bool ten_gig)  {return ten_gig ? src_mac_10GbE  : src_mac_1GbE;}
  std::string  GetSrcIP(bool ten_gig)   {return ten_gig ? src_ip_10GbE   : src_ip_1GbE;}
  unsigned int GetSrcPort(bool ten_gig) {return ten_gig ? src_port_10GbE : src_port_1GbE;}

  void Print();

};



class Beb{ //

 private:
  std::vector<BebInfo*> beb_infos;
  void ClearBebInfos();
  bool InitBebInfos();
  bool ReadSetUpFromFile(std::string file_name);
  bool CheckSourceStuffBebInfo();
  unsigned int GetBebInfoIndex(unsigned int beb_numb);

  LocalLinkInterface* ll;
  
  int           send_ndata;
  unsigned int  send_buffer_size;
  unsigned int* send_data_raw;
  unsigned int* send_data;

  int           recv_ndata;
  unsigned int  recv_buffer_size;
  unsigned int* recv_data_raw;
  unsigned int* recv_data;

  bool WriteTo(unsigned int index);

  bool SetMAC(std::string mac, unsigned char* dst_ptr);
  bool SetIP(std::string ip, unsigned char* dst_ptr);
  bool SetPortNumber(unsigned int port_number, unsigned char* dst_ptr);
  void AdjustIPChecksum(udp_header_type *ip);

  bool SetHeaderData(unsigned int beb_number, bool ten_gig, std::string dst_mac, std::string dst_ip, unsigned int dst_port);
  bool SetHeaderData(std::string src_mac, std::string src_ip, unsigned int src_port, std::string dst_mac, std::string dst_ip, unsigned int dst_port);

  void SwapDataFun(bool little_endian, unsigned int n, unsigned int *d);
  bool SetByteOrder();

  short bit_mode;

 public:
  Beb();
  virtual ~Beb();

  bool SetBebSrcHeaderInfos(unsigned int beb_number, bool ten_gig, std::string src_mac, std::string src_ip, unsigned int src_port);
  bool SetUpUDPHeader(unsigned int beb_number, bool ten_gig, unsigned int header_number, std::string dst_mac, std::string dst_ip, unsigned int dst_port);

  bool SendMultiReadRequest(unsigned int beb_number, unsigned int  left_right, bool ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, bool stop_read_when_fifo_empty=1);


  bool SetUpTransferParameters(short the_bit_mode);
  bool RequestNImages(unsigned int beb_number, unsigned int left_right, bool ten_gig, unsigned int dst_number, unsigned int nimages, bool test_just_send_out_packets_no_wait=0); //all images go to the same destination!


  bool Test(unsigned int beb_number);

};


#endif 

