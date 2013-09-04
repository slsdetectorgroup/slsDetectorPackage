#ifndef __XFS_TYPES_H__
#define __XFS_TYPES_H__

/******************************************************************************/
/* types                                                                      */
/******************************************************************************/

typedef unsigned int xfs_u32;
typedef unsigned short xfs_u16;
typedef unsigned char xfs_u8;

typedef signed int xfs_i32;
typedef signed short xfs_i16;
typedef signed char xfs_i8;


// UDP Header
typedef struct 
{
  // ethternet frame  (14 byte)
  unsigned char dst_mac[6];
  unsigned char src_mac[6];   
  unsigned char len_type[2];   
  
  // ip header    (20 byte)
  unsigned char ver_headerlen[1];   
  unsigned char service_type[1];   
  unsigned char total_length[2];   
  unsigned char identification[2];   
  unsigned char flags[1];   
  unsigned char frag_offset[1];   
  unsigned char time_to_live[1];   
  unsigned char protocol[1];   
  unsigned char ip_header_checksum[2];   
  unsigned char src_ip[4];   
  unsigned char dst_ip[4];   

  // udp header   (8 byte)
  unsigned char src_port[2];   
  unsigned char dst_port[2];   
  unsigned char udp_message_len[2];   
  unsigned char udp_checksum[2];   

} udp_header_type;



#endif // __XFS_TYPES_H__


