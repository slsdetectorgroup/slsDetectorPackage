#ifndef __XFS_TYPES_H__
#define __XFS_TYPES_H__

//#include "types.h"
#include <stdint.h>
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
struct udp_header_type
{
  // ethternet frame  (14 byte)
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint8_t len_type[2];
  
  // ip header    (20 byte)
  uint8_t ver_headerlen[1];
  uint8_t service_type[1];
  uint8_t total_length[2];
  uint8_t identification[2];
  uint8_t flags[1];
  uint8_t frag_offset[1];
  uint8_t time_to_live[1];
  uint8_t protocol[1];
  uint8_t ip_header_checksum[2];
  uint8_t src_ip[4];
  uint8_t dst_ip[4];

  // udp header   (8 byte)
  uint8_t src_port[2];
  uint8_t dst_port[2];
  uint8_t udp_message_len[2];
  uint8_t udp_checksum[2];

};



#endif // __XFS_TYPES_H__


