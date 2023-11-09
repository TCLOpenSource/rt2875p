#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "japan4k_dmx.h"
#include <linux/kthread.h>
#include <linux/pageremap.h>

#include <rbus/tp_reg.h>
#include <tp/tp_dbg.h>
#include <tp/tp_drv_global.h>
#include <tp/tp_reg_ctrl.h>
#include "tlv_lib.h"
#include <linux/freezer.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <rbus/timer_reg.h>
#include <rbus/pll_reg_reg.h>

#include <linux/uaccess.h>

#define japan4k_dmx_malloc(size)        kmalloc(size, GFP_KERNEL)
#define japan4k_dmx_free(addr)          kfree(addr)

#define CID_IP_MAPPING_MAX_NUM  16
#define MAX_JAPAN4K_DMX     2
struct japan4k_dmx g_japan4k_dmx[MAX_JAPAN4K_DMX];

JP4K_DELIVERY_MAPPING_INFO g_jp4k_cid_ip_mappingInfo[MAX_TP_COUNT][CID_IP_MAPPING_MAX_NUM];

extern struct  file *rec_tlv_fd[MAX_TP_COUNT];
extern struct  file *rec_filter_tlv_fd[MAX_TP_COUNT];
void japa4k_dmx_print_tlvfilterInfo(const char* str, unsigned int ch, unsigned int cnt, JP4K_DELIVERY_DMX_FILTER_T *pfiltet_info_list);
void japa4k_dmx_print_tlvfilterFromUserInfo(const char* str, unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list);

void dump_channel_tlv(UINT8 tp_id, UINT8 *p_data, UINT32 len)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
    _tp_dump_lock();
    if(rec_tlv_fd[tp_id] != NULL) {
        kernel_write(rec_tlv_fd[tp_id], p_data, len, &rec_tlv_fd[tp_id]->f_pos);
    }
    _tp_dump_unlock();
#endif
}

void dump_channel_filtered_tlv(UINT8 tp_id, UINT8 *p_data, UINT32 len)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
    _tp_dump_lock();
    if(rec_filter_tlv_fd[tp_id] != NULL) {
        kernel_write(rec_filter_tlv_fd[tp_id], p_data, len, &rec_filter_tlv_fd[tp_id]->f_pos);
    }
    _tp_dump_unlock();
#endif
}

void japan4k_update_cid_ip_mappingInfo(UINT8 tp_id, UINT16 cid, UINT8 *p_src_ipInfo, UINT8 *p_dst_ipInfo, UINT32 ipLen, UINT8 ipflag)
{
    UINT8 i;
    for(i = 0; i < CID_IP_MAPPING_MAX_NUM; i++){
        /* update mapping info */
        if(cid == g_jp4k_cid_ip_mappingInfo[tp_id][i].cid){
            g_jp4k_cid_ip_mappingInfo[tp_id][i].ipflag = ipflag;
            memcpy(g_jp4k_cid_ip_mappingInfo[tp_id][i].src_ip, p_src_ipInfo, ipLen);
            memcpy(g_jp4k_cid_ip_mappingInfo[tp_id][i].dst_ip, p_dst_ipInfo, ipLen);
            break;
        }
    }
    if(i == CID_IP_MAPPING_MAX_NUM){
        for(i = 0; i <  CID_IP_MAPPING_MAX_NUM; i++){
            /* no cid equal, find a new to store */
            if(TLV_INVALID_CID_VALUE == g_jp4k_cid_ip_mappingInfo[tp_id][i].cid){
                g_jp4k_cid_ip_mappingInfo[tp_id][i].ipflag = ipflag;
                g_jp4k_cid_ip_mappingInfo[tp_id][i].cid = cid;
                memcpy(g_jp4k_cid_ip_mappingInfo[tp_id][i].src_ip, p_src_ipInfo, ipLen);
                memcpy(g_jp4k_cid_ip_mappingInfo[tp_id][i].dst_ip, p_dst_ipInfo, ipLen);
                break;
            }
        }
    }

    if(i == CID_IP_MAPPING_MAX_NUM)
        JAPAN4K_DMX_ERROR("cid:%d ip mapping failed since array > 16!!", cid);
    return ;
}

void japan4k_get_ip_mappingInfo_bycid(UINT8 tp_id, UINT16 cid, UINT8 *p_src_ipInfo, UINT8 *p_dst_ipInfo)
{
    UINT8 i, ipLen, ipflag;
    for(i = 0; i < CID_IP_MAPPING_MAX_NUM; i++){
        /* get mapping info */
        if(cid == g_jp4k_cid_ip_mappingInfo[tp_id][i].cid){
            ipflag = g_jp4k_cid_ip_mappingInfo[tp_id][i].ipflag;
            if(ipflag == 2)
                ipLen = 16;
            else
                ipLen = 4;
            memcpy(p_src_ipInfo, g_jp4k_cid_ip_mappingInfo[tp_id][i].src_ip, ipLen);
            memcpy(p_dst_ipInfo, g_jp4k_cid_ip_mappingInfo[tp_id][i].dst_ip, ipLen);
            break;
        }
    }
    if(i == CID_IP_MAPPING_MAX_NUM)
        JAPAN4K_DMX_DBG("cid:%d ip mapping find failed !!", cid);
    return ;
}

int japan4k_dmx_tlv_filter_param_compare(struct japan4k_dmx* p_this, JP4K_DELIVERY_PAKET_FILTER_INFO*  p_tlv_packet_param)
{
    int i;
    /*  if mmtp_type is 0, drop this packet, if is 2, always return to user level
        0: ignore ip address, compare mmtp packet_id
        1: ipv4: if ip address is not null, compare ip address and store cid, then compare packet id
                 if ip address is null, compare cid with the stored cid by ip, then compare packet id
        2: ipv6: if ip address is not null, compare ip address and store cid, then compare packet id
                 if ip address is null, compare cid with the stored cid by ip, then compare packet id
        other: no compare action
        return value: 0: means drop this packet
                      1: means return packet to user level
    */
    if(p_tlv_packet_param->mmtp_type == TLV_PAKET_MMTP_PAYLAD_SI)
        return 1;

    for(i = 0; i < p_this->tlv_filter_cnt; i++){
        if(p_this->tlv_filter_param_list[i].flag == 0){
            if( p_this->tlv_filter_param_list[i].packet_id == p_tlv_packet_param->filter_info.packet_id ){
                return 1;
            }
        }
        else if(p_this->tlv_filter_param_list[i].flag == 1){
            if( (0 == memcmp(p_this->tlv_filter_param_list[i].dst_ip, p_tlv_packet_param->filter_info.dst_ip, 4))
                && (0 == memcmp(p_this->tlv_filter_param_list[i].src_ip, p_tlv_packet_param->filter_info.src_ip, 4)) ){
                p_this->cid_mapping_list[i] = p_tlv_packet_param->cid;
            }
            if( (p_this->cid_mapping_list[i] == p_tlv_packet_param->cid) && (p_this->tlv_filter_param_list[i].packet_id == p_tlv_packet_param->filter_info.packet_id) ){
                return 1;
            }
        }
        else if(p_this->tlv_filter_param_list[i].flag == 2){
            if( (0 == memcmp(p_this->tlv_filter_param_list[i].dst_ip, p_tlv_packet_param->filter_info.dst_ip, 16))
                && (0 == memcmp(p_this->tlv_filter_param_list[i].src_ip, p_tlv_packet_param->filter_info.src_ip, 16)) ){
                p_this->cid_mapping_list[i] = p_tlv_packet_param->cid;
            }
            if( (p_this->cid_mapping_list[i] == p_tlv_packet_param->cid) && (p_this->tlv_filter_param_list[i].packet_id == p_tlv_packet_param->filter_info.packet_id)){
                return 1;
            }
        }
    }
    return 0;
}

/*======================================================
 * Func  : japan4k_dmx_tlv_packet_filter
 * Desc  :
 * Param : tlv packet
 * Retn  : 1 : this TLV packet matched by packet id or (parket id and ip addr)
 *         0 : not matched
 *======================================================*/
int japan4k_dmx_tlv_packet_filter(struct japan4k_dmx* p_this, unsigned char* tlv_packet)
{
    TLV_HEADER*  tlv_header  = NULL;
    IPV4_HEADER* ipv4_header = NULL;
    IPV6_HEADER* ipv6_header = NULL;
    HCFB_HEADER* hcfb_header = NULL;
    PARTIAL_IPV4_HEADER* partial_ipv4_header = NULL;
    PARTIAL_IPV6_HEADER* partial_ipv6_header = NULL;
    PARTIAL_UDP_HEADER*  partial_udp_header = NULL;
    TCP_HEADER*  tcp_header = NULL;
    UDP_HEADER*  udp_header = NULL;
    MMTP_HEADER* mmtp_header = NULL;
    JP4K_DELIVERY_PAKET_FILTER_INFO  tlv_packet_param;

    /*filter disable*/
    if(0 == p_this->filter_en)
        return 1;

    tlv_header = (TLV_HEADER*)tlv_packet;
    memset(&tlv_packet_param, 0, sizeof(JP4K_DELIVERY_PAKET_FILTER_INFO));

    switch(tlv_header->paket_type) {
        case TLV_PAKET_TYPE_IPv4:
            tlv_packet_param.cid = TLV_INVALID_CID_VALUE;
            ipv4_header = (IPV4_HEADER*)(tlv_header + 1);
            if(TLV_IP_PACkET_TCP == ipv4_header->proto){
                tcp_header  = (TCP_HEADER*)(ipv4_header + 1);
                mmtp_header = (MMTP_HEADER*)(tcp_header + 1);
            }
            else if(TLV_IP_PACkET_UDP == ipv4_header->proto){
                udp_header  = (UDP_HEADER*)(ipv4_header + 1);
                mmtp_header = (MMTP_HEADER*)(udp_header + 1);
            }
            memcpy(tlv_packet_param.filter_info.src_ip, &ipv4_header->saddr, sizeof(IP_ADDRESS));
            memcpy(tlv_packet_param.filter_info.dst_ip, &ipv4_header->daddr, sizeof(IP_ADDRESS));
            JAPAN4K_DMX_DBG("ipv4 protocal is %d", ipv4_header->proto);
            break;
        case TLV_PAKET_TYPE_IPv6:
            tlv_packet_param.cid = TLV_INVALID_CID_VALUE;
            ipv6_header = (IPV6_HEADER*)(tlv_header + 1);
            if(TLV_IP_PACkET_TCP == ipv6_header->next_header){
                tcp_header  = (TCP_HEADER*)(ipv6_header + 1);
                mmtp_header = (MMTP_HEADER*)(tcp_header + 1);
            }
            else if(TLV_IP_PACkET_UDP == ipv6_header->next_header){
                udp_header  = (UDP_HEADER*)(ipv6_header + 1);
                mmtp_header = (MMTP_HEADER*)(udp_header + 1);
            }
            memcpy(tlv_packet_param.filter_info.src_ip, &ipv6_header->saddr, sizeof(IPV6_ADDRESS));
            memcpy(tlv_packet_param.filter_info.dst_ip, &ipv6_header->daddr, sizeof(IPV6_ADDRESS));
            JAPAN4K_DMX_DBG("ipv6 protocal is %d, 0x%x, 0x%x", ipv6_header->next_header, tlv_packet[4], tlv_packet[5]);
            /* only live TV need NTP */
            //if(p_this->tp_id < TP_TP2)
                //TP_japan4k_parse_NTP(tlv_packet, p_this);

            break;
        case TLV_PAKET_TYPE_HCIP:
            hcfb_header = (HCFB_HEADER*)(tlv_header + 1);
            tlv_packet_param.cid = htons(hcfb_header->cid_sn) >> 4;

            if(TLV_HCFB_PARTIAL_IPV4 == hcfb_header->type){
                partial_ipv4_header = (PARTIAL_IPV4_HEADER*)(hcfb_header + 1);
                partial_udp_header  = (PARTIAL_UDP_HEADER*)(partial_ipv4_header + 1);
                mmtp_header = (MMTP_HEADER*)(partial_udp_header + 1);
                memcpy(tlv_packet_param.filter_info.src_ip, &partial_ipv4_header->saddr, sizeof(IP_ADDRESS));
                memcpy(tlv_packet_param.filter_info.dst_ip, &partial_ipv4_header->daddr, sizeof(IP_ADDRESS));
            }
            else if(TLV_HCFB_PARTIAL_IPV6 == hcfb_header->type){
                partial_ipv6_header = (PARTIAL_IPV6_HEADER*)(hcfb_header + 1);
                partial_udp_header  = (PARTIAL_UDP_HEADER*)(partial_ipv6_header + 1);
                mmtp_header = (MMTP_HEADER*)(partial_udp_header + 1);
                memcpy(tlv_packet_param.filter_info.src_ip, &partial_ipv6_header->saddr, sizeof(IPV6_ADDRESS));
                memcpy(tlv_packet_param.filter_info.dst_ip, &partial_ipv6_header->daddr, sizeof(IPV6_ADDRESS));
                japan4k_update_cid_ip_mappingInfo(p_this->tp_id, tlv_packet_param.cid, (unsigned char*)(&partial_ipv6_header->saddr), (unsigned char*)(&partial_ipv6_header->daddr), sizeof(IPV6_ADDRESS), 2);
            }
            else if(TLV_HCFB_IPV4 == hcfb_header->type){
                ipv4_header = (IPV4_HEADER*)(hcfb_header + 1);
                if(TLV_IP_PACkET_TCP == ipv4_header->proto){
                    tcp_header  = (TCP_HEADER*)(ipv4_header + 1);
                    mmtp_header = (MMTP_HEADER*)(tcp_header + 1);
                }
                else if(TLV_IP_PACkET_UDP == ipv4_header->proto){
                    udp_header  = (UDP_HEADER*)(ipv4_header + 1);
                    mmtp_header = (MMTP_HEADER*)(udp_header + 1);
                }
                memcpy(tlv_packet_param.filter_info.src_ip, &ipv4_header->saddr, sizeof(IP_ADDRESS));
                memcpy(tlv_packet_param.filter_info.dst_ip, &ipv4_header->daddr, sizeof(IP_ADDRESS));
                JAPAN4K_DMX_DBG("ipv4 protocal is %d", ipv4_header->proto);
            }
            else if(TLV_HCFB_NO_HEADER == hcfb_header->type){
                mmtp_header = (MMTP_HEADER*)(hcfb_header + 1);
            }
            JAPAN4K_DMX_DBG("hcfb type is %d packet_id=0x%x ", hcfb_header->type, htons(mmtp_header->packet_id));
            break;
        /*for Trasmission contral signal packet, not need filter */
        case TLV_PAKET_TYPE_TCS:
            return 1;
        default:
            JAPAN4K_DMX_DBG("tlv packet is null packet or undefined packet");
            return 0;
    }

    if(NULL == mmtp_header){
        JAPAN4K_DMX_DBG("mmtp header get failed!! ");
        return 0;
    }
    tlv_packet_param.filter_info.packet_id = htons(mmtp_header->packet_id);
    tlv_packet_param.mmtp_type = mmtp_header->type & 0x3F;
    japan4k_get_ip_mappingInfo_bycid(p_this->tp_id, tlv_packet_param.cid, tlv_packet_param.filter_info.src_ip, tlv_packet_param.filter_info.dst_ip);

    return japan4k_dmx_tlv_filter_param_compare(p_this, &tlv_packet_param);
}

/*======================================================
 * Func  : japan4k_dmx_get_CIDMappingInfo
 * Desc  : get cid ip mapping info 
 * Param : 
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_get_CIDMappingInfo(unsigned char ch, unsigned int *pcnt, JP4K_DELIVERY_MAPPING_INFO *pMapInfo)
{
    int i, cnt = 0;
    for(i = 0; i < CID_IP_MAPPING_MAX_NUM; i++){
        if(TLV_INVALID_CID_VALUE != g_jp4k_cid_ip_mappingInfo[ch][i].cid){
            if(copy_to_user((void *)(pMapInfo), (const void __user *)&(g_jp4k_cid_ip_mappingInfo[ch][i]), sizeof(JP4K_DELIVERY_MAPPING_INFO))) {
                JAPAN4K_DMX_ERROR("ERROR!! copy to user error L:%d", __LINE__);
                *pcnt = cnt;
                return -1;
            }
            pMapInfo++;
            cnt++;
        }
    }
    *pcnt = cnt;
    return 0;
}

/*======================================================
 * Func  : japan4k_raw_mode_run 
 * Desc  :
 * Param : get tlv data from tp by raw mode
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_get_data_by_rawmode(struct japan4k_dmx* p_this)
{
    unsigned char *p_rp, *p_rp_ori;
    UINT32        tlv_buffer_max_size, tlv_buffer_now_size;
    UINT32        len, len_ori;

    JAPAN4K_DMX_TRACE();

    while (japan4k_file_ring_read_data(p_this->tp_id, &p_rp, &len) == 0 && len ) {
            p_rp_ori = p_rp;
            len_ori  = len;
            while(len > 0) {

                    TP_Get_TLVBufferInformation(p_this->tp_id, &tlv_buffer_max_size, &tlv_buffer_now_size);
                    if( (tlv_buffer_max_size - tlv_buffer_now_size) <= 64 * 1024 )
                            break;

                    JAPAN4K_DMX_DBG("read data from ring (%p) = { %02x.%02x.%02x.%02x....%02x.%02x.%02x.%02x }\n",
                                    p_rp,
                                    p_rp[0], p_rp[1], p_rp[2], p_rp[3],
                                    p_rp[len - 4], p_rp[len - 3], p_rp[len - 2], p_rp[len - 1]);

                    if (p_this->tlv_excepted_zize == 0) {
                            // fid header header....
                            unsigned char* p_tlv;
                            unsigned int   data_len;

                            p_tlv = tlv_find_starter(p_rp, len, 0);
                            if (p_tlv == NULL) {
                                    TLV_RELEASE_DATA(len);
                                    JAPAN4K_DMX_ERROR("tlv re-sync 1 can not find sync byte p_rp=%p len=%d\n", p_rp, len);
                                    continue;
                            }

                            if (p_tlv > p_rp) {
                                    // release spare data...
                                    data_len = (unsigned int)(p_tlv - p_rp);
                                    TLV_RELEASE_DATA(data_len);
                                    JAPAN4K_DMX_ERROR("tlv re-sync 2 p_rp=%p len=%d data_len=%d\n", p_rp, len, data_len);
                            }

                            if (len < TLV_HDR_SIZE) { // data less than 4 bytes
                                    p_this->tlv_excepted_zize = TLV_HDR_SIZE;
                                    p_this->tlv_buff_zize     = len;            // minimum size...
                                    memcpy(p_this->tlv_buffer, p_rp, len);      // store temp buffer
                                    TLV_RELEASE_DATA(len);
                                    continue;
                            } else {
                                    // a complete packet header
                                    p_this->tlv_excepted_zize = TLV_HDR_SIZE + (p_rp[2] << 8) + (p_rp[3]);
    
                                    if(p_this->tlv_excepted_zize > TLV_MAX_SIZE) {
                                            JAPAN4K_DMX_ERROR("TLV data length is too big %d p_rp=%p len=%d\n", p_this->tlv_excepted_zize, p_rp, len);
                                            p_this->tlv_excepted_zize = 0;
                                            p_this->tlv_buff_zize = 0;
                                            TLV_RELEASE_DATA(1);
                                            continue;
                                    }

                                    if (len >= p_this->tlv_excepted_zize) {
                                            JAPAN4K_DMX_DBG("complete data block, just deliver it\n");
                                            // a complete data block : dispatch data directly
                                            if(p_rp[1] != 0xff && japan4k_dmx_tlv_packet_filter(p_this, p_rp) ){
                                                    p_this->cb.callback(p_this, p_rp, p_this->tlv_excepted_zize);
                                                    dump_channel_filtered_tlv(p_this->tp_id, p_rp, p_this->tlv_excepted_zize);
                                            }

                                            TLV_RELEASE_DATA(p_this->tlv_excepted_zize);
                                            p_this->tlv_excepted_zize = 0;
                                            p_this->tlv_buff_zize = 0;
                                            continue;
                                    } else {
                                            // part of block... : store it in the internal buffer
                                            memcpy(p_this->tlv_buffer, p_rp, len);
                                            p_this->tlv_buff_zize = len;
                                            TLV_RELEASE_DATA(len);
    
                                            JAPAN4K_DMX_DBG("part of data block, collect it, buff_size=%lu, excepted size=%u\n",
                                                            p_this->tlv_buff_zize, p_this->tlv_excepted_zize);
                                            continue;
                                    }
                            }
                    } else if (p_this->tlv_excepted_zize == TLV_HDR_SIZE) {
                            // collecting header....
                            int data_len = TLV_HDR_SIZE - p_this->tlv_buff_zize;

                            JAPAN4K_DMX_DBG("collect data header, len = %u, buff_size=%lu, excepted size=%u\n",
                                            len, p_this->tlv_buff_zize, p_this->tlv_excepted_zize);

                            if (data_len > len)
                                    data_len = len;

                            memcpy(p_this->tlv_buffer + p_this->tlv_buff_zize, p_rp, data_len);
                            p_this->tlv_buff_zize += data_len;
                            TLV_RELEASE_DATA(data_len);

                            if (p_this->tlv_buff_zize == p_this->tlv_excepted_zize) {
                                    // parse header
                                    p_this->tlv_excepted_zize += (p_this->tlv_buffer[2] << 8) + (p_this->tlv_buffer[3]);

                                    if(p_this->tlv_excepted_zize > TLV_MAX_SIZE) {
                                            JAPAN4K_DMX_ERROR("TLV data length is too big %d p_rp=%p data_len=%d\n", p_this->tlv_excepted_zize, p_rp, data_len);
                                            p_this->tlv_excepted_zize = 0;
                                            p_this->tlv_buff_zize = 0;
                                    }
                                    if(p_this->tlv_excepted_zize == TLV_HDR_SIZE) {
                                            p_this->tlv_excepted_zize = 0;
                                            p_this->tlv_buff_zize = 0;
                                    }
                                    continue;
                            }
                    } else {
                            // collecting data
                            int data_len = p_this->tlv_excepted_zize - p_this->tlv_buff_zize;       // remain size
                            JAPAN4K_DMX_DBG("collect data block, len = %u, buff_size=%lu, excepted size=%u\n",
                                            len, p_this->tlv_buff_zize, p_this->tlv_excepted_zize);

                            if (data_len > len)
                                    data_len = len;

                            memcpy(p_this->tlv_buffer + p_this->tlv_buff_zize, p_rp, data_len);
                            TLV_RELEASE_DATA(data_len);

                            p_this->tlv_buff_zize += data_len;

                            if (p_this->tlv_buff_zize >= p_this->tlv_excepted_zize) {
                                    // complete block
                                    if(p_this->tlv_buffer[1] != 0xff && japan4k_dmx_tlv_packet_filter(p_this, p_this->tlv_buffer) ){
                                            p_this->cb.callback(p_this, p_this->tlv_buffer, p_this->tlv_excepted_zize);
                                            dump_channel_filtered_tlv(p_this->tp_id, p_this->tlv_buffer, p_this->tlv_excepted_zize);
                                    }
                                    p_this->tlv_excepted_zize = 0;
                                    p_this->tlv_buff_zize = 0;
                            }
                    }
            }
            japan4k_file_ring_release_data(p_this->tp_id, p_rp_ori, len_ori - len);
            TP_Get_TLVBufferInformation(p_this->tp_id, &tlv_buffer_max_size, &tlv_buffer_now_size);
            JAPAN4K_DMX_DBG("[%d] %d %d\n", p_this->tp_id, tlv_buffer_max_size, tlv_buffer_now_size);
            if( (tlv_buffer_max_size - tlv_buffer_now_size) <= 64 * 1024 )
                    break;
    }
    return 0;
}

/*======================================================
 * Func  : japan4k_dmx_enable_TLV_filter
 * Desc  :
 * Param :
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_enable_TLV_filter(unsigned int ch, unsigned char enable)
{
    struct japan4k_dmx* p_this;

    if(NULL == pTp_drv->tp_japan[ch].dmx ){
        JAPAN4K_DMX_ERROR("ERROR!! japan4k demux not opened!!");
        return -1;
    }

    p_this = pTp_drv->tp_japan[ch].dmx;
    p_this->filter_en = enable;
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
    if((TPK_DELI_SRC_TYPE_TLV_HW0 == pTp_drv->tp_japan[ch].src_type) || (TPK_DELI_SRC_TYPE_TLV_HW1 == pTp_drv->tp_japan[ch].src_type)){
        if(enable)
            RHAL_Delivery_HWTLV_EnableFilter(p_this->ptlv_regs);
        else
            RHAL_Delivery_HWTLV_DisableFilter(p_this->ptlv_regs);
    }
#endif
    return 0;
}

/*======================================================
 * Func  : japan4k_dmx_add_TLV_filters
 * Desc  :
 * Param :
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_add_TLV_filters(unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list)
{
    JP4K_DELIVERY_FILTER_T  tlv_filter_param[TLV_PACKET_FILTER_MAX_NUM];
    struct japan4k_dmx* p_this = NULL;
    UINT32 i = 0, j = 0;

    memset(tlv_filter_param, 0, sizeof(JP4K_DELIVERY_FILTER_T)*TLV_PACKET_FILTER_MAX_NUM);

    if(NULL == pTp_drv->tp_japan[ch].dmx || TLV_PACKET_FILTER_MAX_NUM < cnt){
        JAPAN4K_DMX_ERROR("ERROR!! japan4k demux not opened!! cnt=%d", cnt);
        return -1;
    }
    if(copy_from_user( (void *)tlv_filter_param,
                       (const void __user *)to_user_ptr(pfiltet_info_list),
                       sizeof(JP4K_DELIVERY_FILTER_T) * cnt)  ){
        JAPAN4K_DMX_ERROR("ERROR!! copy from user failed!! cnt=%d", cnt);
        return -1;
    }

    japa4k_dmx_print_tlvfilterFromUserInfo("[Add] From user", ch, cnt, tlv_filter_param);

    p_this = pTp_drv->tp_japan[ch].dmx;
    for(i = 0; i < cnt; i++){
        for(j = 0 ; j < p_this->tlv_filter_cnt; j++){
            if( (tlv_filter_param[i].flag == p_this->tlv_filter_param_list[i].flag)
                && (tlv_filter_param[i].packet_id == p_this->tlv_filter_param_list[i].packet_id)
                && (0 == memcmp(tlv_filter_param[i].src_ip, p_this->tlv_filter_param_list[i].src_ip, 16))
                && (0 == memcmp(tlv_filter_param[i].dst_ip, p_this->tlv_filter_param_list[i].dst_ip, 16)) ){
                JAPAN4K_DMX_INFO("The same filter found. Skit this!! index:%d flag=%d packetId:0x%x", i, tlv_filter_param[i].flag, tlv_filter_param[i].packet_id);
                break;
            }
        }
        if(j ==  p_this->tlv_filter_cnt){

            p_this->tlv_filter_param_list[j].flag = tlv_filter_param[i].flag;
            p_this->tlv_filter_param_list[j].packet_id = tlv_filter_param[i].packet_id;
            memcpy(p_this->tlv_filter_param_list[j].src_ip, tlv_filter_param[i].src_ip, 16);
            memcpy(p_this->tlv_filter_param_list[j].dst_ip, tlv_filter_param[i].dst_ip, 16);
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
            if((TPK_DELI_SRC_TYPE_TLV_HW0 == pTp_drv->tp_japan[ch].src_type) || (TPK_DELI_SRC_TYPE_TLV_HW1 == pTp_drv->tp_japan[ch].src_type)){
                p_this->tlv_filter_param_list[j].filter_table_idx = RHAL_Delivery_GetValidFilterTableIdx(ch);
                RHAL_Delivery_HWTLV_AddFilter(p_this->ptlv_regs, p_this->tlv_filter_param_list[j].filter_table_idx, &(tlv_filter_param[i]));
            }
#endif
            p_this->tlv_filter_cnt++;
        }
    }

    japa4k_dmx_print_tlvfilterInfo("[Add] Datebase Updated", ch, p_this->tlv_filter_cnt, p_this->tlv_filter_param_list);
    return 0;
}

/*======================================================
 * Func  : japan4k_dmx_remove_TLV_filters
 * Desc  :
 * Param :
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_remove_TLV_filters(unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list)
{
    JP4K_DELIVERY_FILTER_T  tlv_filter_param[TLV_PACKET_FILTER_MAX_NUM];
    struct japan4k_dmx* p_this = NULL;
    UINT32 i = 0, j = 0;
    UINT16 end;

    if(NULL == pTp_drv->tp_japan[ch].dmx || TLV_PACKET_FILTER_MAX_NUM < cnt){
        JAPAN4K_DMX_ERROR("ERROR!! japan4k demux not opened!! cnt=%d", cnt);
        return -1;
    }

    if(copy_from_user( (void *)tlv_filter_param,
                       (const void __user *)to_user_ptr(pfiltet_info_list),
                       sizeof(JP4K_DELIVERY_FILTER_T) * cnt)  ){
        JAPAN4K_DMX_ERROR("ERROR!! copy from user failed!! cnt=%d", cnt);
        return -1;
    }

    japa4k_dmx_print_tlvfilterFromUserInfo("[Rm] From user", ch, cnt, tlv_filter_param);
    p_this = pTp_drv->tp_japan[ch].dmx;

    for(i = 0; i < cnt; i++){
        for(j = 0 ; j < p_this->tlv_filter_cnt; j++){
            if( (tlv_filter_param[i].flag == p_this->tlv_filter_param_list[j].flag)
                && (tlv_filter_param[i].packet_id == p_this->tlv_filter_param_list[j].packet_id)
                && (0 == memcmp(tlv_filter_param[i].src_ip, p_this->tlv_filter_param_list[j].src_ip, 16))
                && (0 == memcmp(tlv_filter_param[i].dst_ip, p_this->tlv_filter_param_list[j].dst_ip, 16)) ){
                JAPAN4K_DMX_INFO("same filter found!! index:%d flag=%d packetId:0x%x", j, p_this->tlv_filter_param_list[j].flag, p_this->tlv_filter_param_list[j].packet_id);
                break;
            }
        }
        if(j !=  p_this->tlv_filter_cnt){
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
            if((TPK_DELI_SRC_TYPE_TLV_HW0 == pTp_drv->tp_japan[ch].src_type) || (TPK_DELI_SRC_TYPE_TLV_HW1 == pTp_drv->tp_japan[ch].src_type)){
                RHAL_Delivery_HWTLV_RemoveFilter(p_this->ptlv_regs, p_this->tlv_filter_param_list[j].filter_table_idx);
                RHAL_Delivery_ReturnFilterTableIdx(ch, p_this->tlv_filter_param_list[j].filter_table_idx);
            }
#endif
            /*remove current filter info, and copy the last tlv filter info to curret position*/
            end = p_this->tlv_filter_cnt - 1;
            if(end != j){
                p_this->tlv_filter_param_list[j].filter_table_idx = p_this->tlv_filter_param_list[end].filter_table_idx;
                p_this->tlv_filter_param_list[j].flag = p_this->tlv_filter_param_list[end].flag;
                p_this->tlv_filter_param_list[j].packet_id = p_this->tlv_filter_param_list[end].packet_id;
                memcpy(p_this->tlv_filter_param_list[j].src_ip, p_this->tlv_filter_param_list[end].src_ip, 16);
                memcpy(p_this->tlv_filter_param_list[j].dst_ip, p_this->tlv_filter_param_list[end].dst_ip, 16);
            }
            memset(&p_this->tlv_filter_param_list[end], 0, sizeof(JP4K_DELIVERY_DMX_FILTER_T));
            p_this->tlv_filter_cnt--;
        }
    }
    japa4k_dmx_print_tlvfilterInfo("[Rm] Datebase Updated", ch, p_this->tlv_filter_cnt, p_this->tlv_filter_param_list);
    return 0;
}

int japan4k_dmx_get_ntp(unsigned int tp_id, unsigned int *ntp_high, unsigned int *ntp_low)
{
    struct japan4k_dmx* p_this = NULL;

    if(NULL == pTp_drv->tp_japan[tp_id].dmx){
        JAPAN4K_DMX_ERROR("ERROR!! japan4k demux not opened!!L:%d", __LINE__);
        return -1;
    }
    p_this = pTp_drv->tp_japan[tp_id].dmx;
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
    return RHAL_Delivery_HWTLV_GetNTP(p_this->ptlv_regs, ntp_high, ntp_low);
#else
    return 0;
#endif
}

/*======================================================
 * Func  : japan4k_dmx_run
 * Desc  :
 * Param : handle of japan4k dmx
 * Retn  : 0 : successed, oethers failed
 *======================================================*/
int japan4k_dmx_run(struct japan4k_dmx* p_this)
{
    if(pTp_drv->tp_japan[p_this->tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_SW0){
        japan4k_dmx_get_data_by_rawmode(p_this);
    }else {
        RHAL_Delivery_ReadDataFromHWTLV(p_this);
    }
    return 0;
}

/*======================================================
 * Func  : create_japan4k_dmx
 *
 * Desc  : create a japan4k demux
 *
 * Param : p_dmx_in : input buffer
 *
 * Retn  : handle of japan4k dmx
 *======================================================*/
struct japan4k_dmx* create_japan4k_dmx(unsigned char tp_id)
{
        struct japan4k_dmx* p_this;
        unsigned char index;
        JAPAN4K_DMX_TRACE();

        //p_this = (struct japan4k_dmx*) japan4k_dmx_malloc(sizeof(struct japan4k_dmx));
        index = (tp_id == TP_TP0 ? 0 : 1);
        p_this = &g_japan4k_dmx[index];
        if (p_this) {
                memset(p_this, 0, sizeof(struct japan4k_dmx));
        }

        return p_this;
}

/*======================================================
 * Func  : destroy_japan4k_dmx
 *
 * Desc  :
 *
 * Param : handle of japan4k dmx
 *
 * Retn  : N/A
 *======================================================*/
void destroy_japan4k_dmx(struct japan4k_dmx* p_this)
{
    (void)p_this;
    JAPAN4K_DMX_TRACE();
    //if (p_this)
        //japan4k_dmx_free(p_this);
}

/*======================================================
 * Func  : japan4k_file_ring_read_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int japan4k_file_ring_read_data(
        unsigned char tp_id,
        unsigned char**         pp_rp,
        unsigned int*                 p_len
)
{
        int result;
        result = TP_ReadData_NoNeedAligment(tp_id, pp_rp, p_len, 1024, TP_BUFFER_MASS_DATA);
        *pp_rp = Tp_Buf_Phy_to_Vir(tp_id, TP_BUFFER_MASS_DATA, (unsigned char*)*pp_rp);
        if(result < 0) {
                *p_len = 0;
        }
        return 0;
}


/*======================================================
 * Func  : japan4k_file_ring_release_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int japan4k_file_ring_release_data(
        unsigned char tp_id,
        unsigned char*          p_rp,
        unsigned int            len
)
{
        dump_channel_tlv(tp_id, p_rp, len);

        p_rp = Tp_Buf_Vir_to_Phy(tp_id, TP_BUFFER_MASS_DATA, (unsigned char*)p_rp);

        TP_ReleaseData_NoNeedAligment(tp_id, p_rp, len, TP_BUFFER_MASS_DATA);
        return 0;
}
void japa4k_dmx_print_tlvfilterFromUserInfo(const char* str, unsigned int ch, unsigned int cnt, JP4K_DELIVERY_FILTER_T *pfiltet_info_list)
{
    int i;
    JAPAN4K_DMX_INFO("CH:%d %s. Total[%d] filter_info:", ch, str, cnt);
    for(i = 0; i < cnt; i++){
        JAPAN4K_DMX_INFO("TLV filter. total filter:%d, curr filter:%d, ipversion:%d, pid:0x%x !!", cnt, i,
                        pfiltet_info_list[i].flag,
                        pfiltet_info_list[i].packet_id);
        if(pfiltet_info_list[i].flag == 1){
            JAPAN4K_DMX_INFO("IPv4 info: src ip:%d:%d:%d:%d dst_ip:%d:%d:%d:%d!!",
                    pfiltet_info_list[i].src_ip[0],
                    pfiltet_info_list[i].src_ip[1],
                    pfiltet_info_list[i].src_ip[2],
                    pfiltet_info_list[i].src_ip[3],
                    pfiltet_info_list[i].dst_ip[0],
                    pfiltet_info_list[i].dst_ip[1],
                    pfiltet_info_list[i].dst_ip[2],
                    pfiltet_info_list[i].dst_ip[3]);
        }
        if(pfiltet_info_list[i].flag == 2){
            JAPAN4K_DMX_INFO("IPv6 info: src ip: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
                    pfiltet_info_list[i].src_ip[0],
                    pfiltet_info_list[i].src_ip[1],
                    pfiltet_info_list[i].src_ip[2],
                    pfiltet_info_list[i].src_ip[3],
                    pfiltet_info_list[i].src_ip[4],
                    pfiltet_info_list[i].src_ip[5],
                    pfiltet_info_list[i].src_ip[6],
                    pfiltet_info_list[i].src_ip[7],
                    pfiltet_info_list[i].src_ip[8],
                    pfiltet_info_list[i].src_ip[9],
                    pfiltet_info_list[i].src_ip[10],
                    pfiltet_info_list[i].src_ip[11],
                    pfiltet_info_list[i].src_ip[12],
                    pfiltet_info_list[i].src_ip[13],
                    pfiltet_info_list[i].src_ip[14],
                    pfiltet_info_list[i].src_ip[15]);
            JAPAN4K_DMX_INFO("IPv6 info: dst_ip: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
                    pfiltet_info_list[i].dst_ip[0],
                    pfiltet_info_list[i].dst_ip[1],
                    pfiltet_info_list[i].dst_ip[2],
                    pfiltet_info_list[i].dst_ip[3],
                    pfiltet_info_list[i].dst_ip[4],
                    pfiltet_info_list[i].dst_ip[5],
                    pfiltet_info_list[i].dst_ip[6],
                    pfiltet_info_list[i].dst_ip[7],
                    pfiltet_info_list[i].dst_ip[8],
                    pfiltet_info_list[i].dst_ip[9],
                    pfiltet_info_list[i].dst_ip[10],
                    pfiltet_info_list[i].dst_ip[11],
                    pfiltet_info_list[i].dst_ip[12],
                    pfiltet_info_list[i].dst_ip[13],
                    pfiltet_info_list[i].dst_ip[14],
                    pfiltet_info_list[i].dst_ip[15]);
        }
    }
}

void japa4k_dmx_print_tlvfilterInfo(const char* str, unsigned int ch, unsigned int cnt, JP4K_DELIVERY_DMX_FILTER_T *pfiltet_info_list)
{
    int i;
    JAPAN4K_DMX_INFO("CH:%d %s. Total[%d] filter_info:", ch, str, cnt);
    for(i = 0; i < cnt; i++){
        JAPAN4K_DMX_INFO("TLV filter. total filter:%d, curr filter:%d, ipversion:%d, pid:0x%x !!", cnt, i,
                        pfiltet_info_list[i].flag,
                        pfiltet_info_list[i].packet_id);
        if(pfiltet_info_list[i].flag == 1){
            JAPAN4K_DMX_INFO("IPv4 info: src ip:%d:%d:%d:%d dst_ip:%d:%d:%d:%d!!",
                    pfiltet_info_list[i].src_ip[0],
                    pfiltet_info_list[i].src_ip[1],
                    pfiltet_info_list[i].src_ip[2],
                    pfiltet_info_list[i].src_ip[3],
                    pfiltet_info_list[i].dst_ip[0],
                    pfiltet_info_list[i].dst_ip[1],
                    pfiltet_info_list[i].dst_ip[2],
                    pfiltet_info_list[i].dst_ip[3]);
        }
        if(pfiltet_info_list[i].flag == 2){
            JAPAN4K_DMX_INFO("IPv6 info: src ip: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
                    pfiltet_info_list[i].src_ip[0],
                    pfiltet_info_list[i].src_ip[1],
                    pfiltet_info_list[i].src_ip[2],
                    pfiltet_info_list[i].src_ip[3],
                    pfiltet_info_list[i].src_ip[4],
                    pfiltet_info_list[i].src_ip[5],
                    pfiltet_info_list[i].src_ip[6],
                    pfiltet_info_list[i].src_ip[7],
                    pfiltet_info_list[i].src_ip[8],
                    pfiltet_info_list[i].src_ip[9],
                    pfiltet_info_list[i].src_ip[10],
                    pfiltet_info_list[i].src_ip[11],
                    pfiltet_info_list[i].src_ip[12],
                    pfiltet_info_list[i].src_ip[13],
                    pfiltet_info_list[i].src_ip[14],
                    pfiltet_info_list[i].src_ip[15]);
            JAPAN4K_DMX_INFO("IPv6 info: dst_ip: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
                    pfiltet_info_list[i].dst_ip[0],
                    pfiltet_info_list[i].dst_ip[1],
                    pfiltet_info_list[i].dst_ip[2],
                    pfiltet_info_list[i].dst_ip[3],
                    pfiltet_info_list[i].dst_ip[4],
                    pfiltet_info_list[i].dst_ip[5],
                    pfiltet_info_list[i].dst_ip[6],
                    pfiltet_info_list[i].dst_ip[7],
                    pfiltet_info_list[i].dst_ip[8],
                    pfiltet_info_list[i].dst_ip[9],
                    pfiltet_info_list[i].dst_ip[10],
                    pfiltet_info_list[i].dst_ip[11],
                    pfiltet_info_list[i].dst_ip[12],
                    pfiltet_info_list[i].dst_ip[13],
                    pfiltet_info_list[i].dst_ip[14],
                    pfiltet_info_list[i].dst_ip[15]);
        }
    }
}

/*======================================================
 * Func  : japan4k_file_ring_get_free_space
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int japan4k_file_ring_get_free_space(
        unsigned char**         pp_wp,
        unsigned long*          p_len
)
{
        return 0;
}

/*======================================================
 * Func  : japan4k_file_ring_put_data
 *
 * Desc  :
 *
 * Param :
 *
 * Retn  : N/A
 *======================================================*/
int japan4k_file_ring_put_data(
        unsigned char*          p_wp,
        unsigned long           len
)
{
        return 0;
}

