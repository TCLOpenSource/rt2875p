#ifndef __JAPAN4K_DMX_H__
#define __JAPAN4K_DMX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtd_log/rtd_module_log.h>

#define JAPAN4K_DMX_DBG_EN

#define TLV_RELEASE_DATA(tlv_len) \
        p_rp = p_rp + tlv_len; \
        len  = len - tlv_len;
#define TLV_INVALID_CID_VALUE     0xFFFF
#define TLV_MAX_SIZE  65536
#define TLV_PACKET_FILTER_MAX_NUM 32

#define TLV_PAKET_TYPE_IPv4   0x01       //ipv4 packet
#define TLV_PAKET_TYPE_IPv6   0x02       //ipv6 packet
#define TLV_PAKET_TYPE_HCIP   0x03       //header compressed ip packet
#define TLV_PAKET_TYPE_TCS    0xFE       //Trasmission contral signal packet
#define TLV_PAKET_TYPE_NULL   0xFF       //NULL packet

#define	TLV_HCFB_PARTIAL_IPV4 0x20	     //partial ipv4,the header excluding packet length, header
#define	TLV_HCFB_IPV4         0x21	     //full ipv4 header
#define	TLV_HCFB_PARTIAL_IPV6 0x60       //partial ipv6,the header excluding payload length from IPv6 header
#define	TLV_HCFB_NO_HEADER    0x61		 //0x61:no header 

#define TLV_IP_PACkET_TCP     6
#define TLV_IP_PACkET_UDP     17

//#pragma pack(1)

// ehter protocol define
typedef struct ether_header {
	unsigned char  ether_dhost[6];		// des address
	unsigned char  ether_shost[6];		// src address
	unsigned short ether_type;		    // type
}ETHER_HEADER;

/* 4 bytes IP address */
typedef struct ip_address{
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
}IP_ADDRESS;

/* 4 bytes IP address */
typedef struct ipv6_address{
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
	unsigned char byte8;
	unsigned char byte9;
	unsigned char byte10;
	unsigned char byte11;
	unsigned char byte12;
	unsigned char byte13;
	unsigned char byte14;
	unsigned char byte15;
	unsigned char byte16;
}IPV6_ADDRESS;

/* IPv4 header */
typedef struct ipv4_header{
	unsigned char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
	unsigned char  tos;            // Type of service 
	unsigned short tlen;           // Total length 
	unsigned short identification; // Identification
	unsigned short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
	unsigned char  ttl;            // Time to live
	unsigned char  proto;          // Protocol
	unsigned short crc;            // Header checksum
	IP_ADDRESS     saddr;      // Source address
	IP_ADDRESS     daddr;      // Destination address
	//uint32_t   op_pad;    // Option + Padding
}IPV4_HEADER;

/* partial IPv4 header */
//The partial IPv4 header shall be the header excluding packet length, header checksum and
//extension information from the IPv4 header.
typedef struct partial_ipv4_header{
	unsigned char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
	unsigned char  tos;            // Type of service 
	unsigned short identification; // Identification
	unsigned short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
	unsigned char  ttl;            // Time to live
	unsigned char  proto;          // Protocol
	IP_ADDRESS  saddr;      // Source address
	IP_ADDRESS  daddr;      // Destination address
	//uint32_t   op_pad;    // Option + Padding
}PARTIAL_IPV4_HEADER;

/* IPv6 header */
typedef struct ipv6_header{
	unsigned int   ver_tra_flow_label;          // version, traffic class, flow label
	unsigned short playload_length;
    unsigned char  next_header;
    unsigned char  hop_limit;
	IPV6_ADDRESS     saddr;      // 128bit Source address
	IPV6_ADDRESS     daddr;      // 128bit Destination address
}IPV6_HEADER;

/*partial IPv6 header, compared with ipv6, loss 2 bytes */
typedef struct partial_ipv6_header{
	unsigned int   ver_tra_flow_label;          // version, traffic class, flow label
	//unsigned short playload_length;
    unsigned char  next_header;
    unsigned char  hop_limit;
	IPV6_ADDRESS     saddr;      // 128bit Source address
	IPV6_ADDRESS     daddr;      // 128bit Destination address
}PARTIAL_IPV6_HEADER;

/* UDP header*/
typedef struct partial_udp_header{
	unsigned short sport;          // Source port
	unsigned short dport;          // Destination port
}PARTIAL_UDP_HEADER;

/* UDP header*/
typedef struct udp_header{
	unsigned short sport;          // Source port
	unsigned short dport;          // Destination port
	unsigned short len;            // Datagram length
	unsigned short crc;            // Checksum
}UDP_HEADER;

// TCP header
typedef struct tcp_header{
	unsigned short  sport;              // (16 bits)
	unsigned short  dport;              // (16 bits)
	unsigned int    seq_number;         // Sequence Number (32 bits)
	unsigned int    ack_number;         // Acknowledgment Number (32 bits)
	unsigned short  info_ctrl;          // Data Offset (4 bits), Reserved (6 bits), Control bits (6 bits)
	unsigned short  window;             // (16 bits)
	unsigned short  checksum;           // (16 bits)
	unsigned short  urgent_pointer;     // (16 bits)
}TCP_HEADER;

typedef struct header_compression_for_broadcasting_info{
	unsigned short	cid_sn;			    //context id: 12bit
	                                    //sequence_number: 4bit
	unsigned char	type;				//HCFB_TYPE_E    
}HCFB_HEADER;

typedef struct tlv_header{
    unsigned char  sync_byte;
    unsigned char  paket_type;
    unsigned short data_length;
}TLV_HEADER;

typedef struct mmtp_header{
    unsigned short type;
    unsigned short packet_id;
    unsigned int   timestamp;
    unsigned int   packet_sequence_num;
    unsigned int   packet_counter;
    unsigned int   header_extension;
    unsigned int   payload_data;
}MMTP_HEADER;

typedef struct
{
	unsigned char		flag;		//0: ignore ip address 1:ipv4 2:ipv6 3:mmtp si
	unsigned char		src_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned char		dst_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned short		packet_id;	//mmtp packet_id
}JP4K_DELIVERY_FILTER_T;

typedef struct
{
    unsigned char		filter_table_idx;
	unsigned char		flag;		//0: ignore ip address 1:ipv4 2:ipv6
	unsigned char		src_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned char		dst_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned short		packet_id;	//mmtp packet_id
}JP4K_DELIVERY_DMX_FILTER_T;

typedef struct
{
    JP4K_DELIVERY_FILTER_T filter_info;
    unsigned short		cid;
    unsigned char       mmtp_type;
}JP4K_DELIVERY_PAKET_FILTER_INFO;

typedef struct
{
    unsigned char		ipflag;		//1:ipv4 2:ipv6
    unsigned char		src_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
    unsigned char		dst_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
    unsigned short		cid;
}JP4K_DELIVERY_MAPPING_INFO;

struct japan4k_dmx_callback {
        void (*callback)(void* p_ctx, unsigned char* p_data, unsigned int len);
};

struct japan4k_dmx {
        unsigned char   tlv_buffer[TLV_MAX_SIZE];
        unsigned long   tlv_buff_zize;
        unsigned int    tlv_excepted_zize;
        unsigned char   tp_id;
        unsigned char   *rhal_tlv_buffer;
        unsigned long   rhal_tlv_buffer_size;
        unsigned long   rhal_tlv_buffer_max_size;
        void   *ptlv_regs;

        unsigned short  tlv_filter_cnt;
        JP4K_DELIVERY_DMX_FILTER_T  tlv_filter_param_list[TLV_PACKET_FILTER_MAX_NUM];
        unsigned short		cid_mapping_list[TLV_PACKET_FILTER_MAX_NUM];  /* mapping by ip address */
        unsigned char   filter_en;
        struct japan4k_dmx_callback cb;            // dmx callback
};

///////////////////////////////////////////////////////
struct japan4k_dmx* create_japan4k_dmx(unsigned char tp_id);
void destroy_japan4k_dmx(struct japan4k_dmx* dmx);
int japan4k_file_ring_read_data(unsigned char tp_id, unsigned char** pp_rp, unsigned int* p_len);
int japan4k_file_ring_release_data(unsigned char tp_id, unsigned char* p_rp, unsigned int len);
int japan4k_file_ring_get_free_space(unsigned char** pp_wp, unsigned long* p_len);
int japan4k_file_ring_put_data(unsigned char* p_wp, unsigned long len);
int japan4k_dmx_run(struct japan4k_dmx* p_this);
int japan4k_dmx_enable_TLV_filter(unsigned int ch, unsigned char enable);
int japan4k_dmx_add_TLV_filters(unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list);
int japan4k_dmx_remove_TLV_filters(unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list);
int japan4k_dmx_get_ntp(unsigned int tp_id, unsigned int *ntp_high, unsigned int *ntp_low);
///////////////////////////////////////////////////////

#ifdef JAPAN4K_DMX_DBG_EN
#define JAPAN4K_DMX_TRACE()
#define JAPAN4K_DMX_DBG(fmt, args...)
#define JAPAN4K_DMX_INFO(fmt, args...)     rtd_pr_japan4k_info( fmt, ## args)
#define JAPAN4K_DMX_WARN(fmt, args...)     rtd_pr_japan4k_warn(fmt, ## args)
#define JAPAN4K_DMX_ERROR(fmt, args...)    rtd_pr_japan4k_err( fmt, ## args)

#else
#define JAPAN4K_DMX_TRACE()
#define JAPAN4K_DMX_DBG(fmt, ...)
#define JAPAN4K_DMX_INFO(fmt, ...)
#define JAPAN4K_DMX_WARN(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif
