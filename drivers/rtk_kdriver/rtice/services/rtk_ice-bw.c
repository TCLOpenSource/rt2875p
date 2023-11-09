#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <asm/page.h>
#include <rtk_kdriver/io.h>
#include <linux/version.h>
#include "rtk_ice-bw.h"
#include "../core/rtk_ice-aux.h"
#include "../core/rtk_ice-debug.h"
#include <rbus/sys_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/dc1_mc_reg.h>
#include <rbus/dc_sys_reg.h>
#include <rbus/scpu_core_reg.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <asm-generic/bug.h>
#include "rtk_ice-latency.h"

#define SKIP_FOREFRONT_CNT 1 /*< Throw result on the forefront checking time.*/
#define TRIPLET_IP(x) ((x) && (x)->p_next && (x)->p_next->p_next)

#define NA_VALUE 0xFFFFFFFF
#define LAT_NA_VALUE 0xFFFF
//#define SUPPORT_DCU2
//#define CONFIG_RTICE_BW_EFFI

#define BW_DATA_CLR(x) {\
	if((x) != NULL){\
		(x)->m_curBw = NA_VALUE;\
		(x)->m_maxBw = NA_VALUE;\
		(x)->m_minBw = NA_VALUE;\
		(x)->m_sumBw = 0;\
		(x)->m_hisLen = 0;\
		(x)->m_curLat = LAT_NA_VALUE;\
		(x)->m_maxLat = LAT_NA_VALUE;\
		(x)->m_minLat = LAT_NA_VALUE;\
		(x)->m_sumLat = 0;\
		(x)->m_hisLenLat = 0;\
	}\
}

#define BW_DATA_W(x,y) {\
	if(x != NULL){\
		(x)->m_curBw = (y);\
		if((x)->m_minBw == NA_VALUE || (x)->m_curBw < (x)->m_minBw){\
			(x)->m_minBw = (x)->m_curBw;\
		}\
		if((x)->m_maxBw == NA_VALUE || (x)->m_curBw > (x)->m_maxBw){\
			(x)->m_maxBw = (x)->m_curBw;\
		}\
		{\
			u64 BW_DATA_u64 = (x)->m_sumBw + (x)->m_curBw;\
			if(BW_DATA_u64 < (x)->m_sumBw){\
				(x)->m_sumBw = (x)->m_curBw;\
				(x)->m_hisLen = 0;\
			}else{\
				(x)->m_hisLen = (x)->m_hisLen + 1;\
				(x)->m_sumBw = BW_DATA_u64;\
			}\
		}\
	}\
}


#define BW_LAT_DATA_W(x,y,z) {\
	if(x != NULL){\
		(x)->m_curBw = (y);\
		if((x)->m_minBw == NA_VALUE || (x)->m_curBw < (x)->m_minBw){\
			(x)->m_minBw = (x)->m_curBw;\
		}\
		if((x)->m_maxBw == NA_VALUE || (x)->m_curBw > (x)->m_maxBw){\
			(x)->m_maxBw = (x)->m_curBw;\
		}\
		{\
			u64 BW_DATA_u64 = (x)->m_sumBw + (x)->m_curBw;\
			if(BW_DATA_u64 < (x)->m_sumBw){\
				(x)->m_sumBw = (x)->m_curBw;\
				(x)->m_hisLen = 0;\
			}else{\
				(x)->m_hisLen = (x)->m_hisLen + 1;\
				(x)->m_sumBw = BW_DATA_u64;\
			}\
		}\
		(x)->m_curLat = (z);\
		if((x)->m_minLat == LAT_NA_VALUE || (x)->m_curLat < (x)->m_minLat){\
			(x)->m_minLat = (x)->m_curLat;\
		}\
		if((x)->m_maxLat == LAT_NA_VALUE || (x)->m_curLat > (x)->m_maxLat){\
			(x)->m_maxLat = (x)->m_curLat;\
		}\
		{\
			u64 Lat_DATA_u64 = (x)->m_sumLat + (x)->m_curLat;\
			if(Lat_DATA_u64 < (x)->m_sumLat){\
				(x)->m_sumLat = (x)->m_curLat;\
				(x)->m_hisLenLat = 0;\
			}else{\
				(x)->m_hisLenLat = (x)->m_hisLenLat + 1;\
				(x)->m_sumLat = Lat_DATA_u64;\
			}\
		}\
	}\
}

#define BW_DATA_R(x,y) ((y) == BW_IP_QUERY_CURRENT ? (x)->m_curBw : \
	((y) == BW_IP_QUERY_MIN ? (x)->m_minBw : \
	((y) == BW_IP_QUERY_MAX ? (x)->m_maxBw : \
	((y) == BW_IP_QUERY_MEAN ? ((x)->m_hisLen == 0 ? \
	(x)->m_curBw : (unsigned int)div64_u64((x)->m_sumBw,(x)->m_hisLen)) : 0))))

#define LAT_DATA_R(x,y) ((y) == BW_IP_QUERY_CURRENT ? (x)->m_curLat : \
	((y) == BW_IP_QUERY_MIN ? (x)->m_minLat : \
	((y) == BW_IP_QUERY_MAX ? (x)->m_maxLat : \
	((y) == BW_IP_QUERY_MEAN ? ((x)->m_hisLenLat == 0 ? \
	(x)->m_curLat : (unsigned int)div64_u64((x)->m_sumLat,(x)->m_hisLenLat)) : 0))))


/*#define SUPPORT_SB_BW_CHECK*/
#define MAX_BW_CHECK_IP 300
#define BW_SCALEUP 100
/*event*/
static DECLARE_WAIT_QUEUE_HEAD (rtice_bw_wait_thread);
static DECLARE_WAIT_QUEUE_HEAD (rtice_bw_wait_main);
/*param and info struct for bw checking*/
static CBwCheck g_bwCheck = {
	NULL		/*checking timer*/,\
	0		/*Current check count*/,\
	NULL		/*Next checked ip*/,\
	false		/*Run flag of thread*/,\
	false		/*Do not send stop signal to checking thread.*/,\
	false		/*Current checking thread is stopped.*/,\
	BW_CHECK_MODE_SIZE      /*Invalid check mode*/,\
	256		/*256ms check period*/,\
	256		/*256ms measure period*/,\
	0		/*group number*/,
	0		/*ip info list is empty*/,\
	NULL		/*ip list header point to null*/,\
	NULL		/*ip list tail point to null*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*pc total*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*pc ack*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*pc idle*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*scpu*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi total*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi rd*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi wr*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*bank conflict*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*w2r count*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*r2w count*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*dcu2 pc total*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*dcu2 pc ack*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*dcu2 pc idle*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*dcu2 scpu*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi total2*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi rd2*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*effi wr2*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*bank2 conflict*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*w2r2 count*/,\
	{0,0,0xFFFFffff,0,0,0,0,0}  /*r2w2 count*/\
};

/**
 * \brief Realocate address of dcu accroding to dcu type.
 * \param a Source addr.
 * \param fdcu2 Wheather query address about dcu2.
 * \return Returns stand.
**/
static unsigned int dcu_addr(unsigned int a,int fdcu2)
{
	unsigned int naddr = a;
	if(fdcu2){
		if(naddr >= 0xb8007000 && naddr <= 0xb8007500)
			naddr = (naddr & 0xffff0fff) | 0x00003000;
		else if(naddr >= 0xb8008800 && naddr <= 0xb8008fff)
			naddr = (naddr & 0xffff0fff) | 0x00004000;
	}
	return naddr;
}
/**
 * \brief  Insert an ip info into global ip info list.
 * \param pIp : Pointer of inserted ip info.
 * \param index : Order index for inserting.
 * \return : Returns order index of the operation while successed insert,
 * otherwise returns -1 .
**/
static int bw_ip_list_insert(CBwIp*pIp,unsigned int index)
{
	if(!pIp)
		return -1;
	if(index == 0){/*< insert into header*/
		if(g_bwCheck.p_ipHeader)
			g_bwCheck.p_ipHeader->p_prev = pIp;
		pIp->p_next = g_bwCheck.p_ipHeader;
		pIp->p_prev = NULL;
		g_bwCheck.p_ipHeader = pIp;
		if(index == g_bwCheck.m_ipListCnt)
			g_bwCheck.p_ipTail = pIp;
	}else if(index == g_bwCheck.m_ipListCnt){
		if(g_bwCheck.p_ipTail)
			g_bwCheck.p_ipTail->p_next = pIp;
		pIp->p_next = NULL;
		pIp->p_prev = g_bwCheck.p_ipTail;
		g_bwCheck.p_ipTail = pIp;
	}else if(index < g_bwCheck.m_ipListCnt){
		unsigned int i = index;
		CBwIp *pNext = g_bwCheck.p_ipHeader;
		if(index < g_bwCheck.m_ipListCnt - index){
			while(i-- && pNext){
				pNext = pNext->p_next;
			}
		}else{
			unsigned int i = g_bwCheck.m_ipListCnt - index;
			pNext = g_bwCheck.p_ipTail;
			while(i-- && pNext){
				pNext = pNext->p_prev;
			}
		}
		if(pNext && pNext->p_prev && pNext->p_next){
			pIp->p_next = pNext;
			pIp->p_prev = pNext->p_prev;
			pNext->p_prev = pIp;
			pNext->p_prev->p_next = pIp;
		}else{
			return -1;
		}

	}else{
		return -1;
	}
	g_bwCheck.m_ipListCnt++;
	return index;
}

static void bw_ip_list_clear(void)
{
	CBwIp* pIp = g_bwCheck.p_ipHeader;
	g_bwCheck.m_ipListCnt = 0;
	g_bwCheck.p_ipHeader = NULL;
	g_bwCheck.p_ipTail = NULL;
	while(pIp){
		CBwIp* pNext = pIp->p_next;
		kfree(pIp);
		pIp = pNext;
	}
}
/**
 * \brief  Init or Reorder bw check ip list..
 * \param ipCnt : Count of ips needed checking.
 * \param ips : Reorder ip checking list according to the id of ip specified by
 * 'ips'.No work while ips is null.
 * \return : Returns count of current checking ips in bw checking list.
**/
static int bw_ip_list_init(unsigned int ipCnt,unsigned char*ips)
{
	unsigned int i = 0;
	unsigned cnt = 0;

	for(i = 0;i < ipCnt;i++){
		CBwIp *pIp = NULL;
		unsigned char id = *ips++;
			
		pIp = (CBwIp*)kmalloc(sizeof(CBwIp),GFP_ATOMIC);
		if(pIp){
			pIp->id = id;
			pIp->p_next = NULL;
			pIp->p_prev = NULL;
		}
		/*clear data of ip bw result*/
		BW_DATA_CLR(pIp);
		/*insert ip info to global list.*/
		if(bw_ip_list_insert(pIp,i) == i)
			cnt++;
	}
	return cnt;
}

static int bw_select_ip(unsigned char ip0,unsigned char ip1,unsigned char ip2)
{
	dc_sys_dc_pc_sysh_prog_ctrl_RBUS     reg_dc_pc_sysh_prog_ctrl;
	reg_dc_pc_sysh_prog_ctrl.regValue = 0;
	/*reg_dc_pc_sysh_prog_ctrl.regValue =  \
	 * rtd_inl(DC_PC_SYSH_PROG_CTRL_reg);*/

	reg_dc_pc_sysh_prog_ctrl.write_en_1 = 1;
	reg_dc_pc_sysh_prog_ctrl.pc_sysh_prog0_sel = ip0 & 0x7f;
	reg_dc_pc_sysh_prog_ctrl.write_en_2 = 1;
	reg_dc_pc_sysh_prog_ctrl.pc_sysh_prog1_sel = ip1 & 0x7f;
	reg_dc_pc_sysh_prog_ctrl.write_en_3 = 1;
	reg_dc_pc_sysh_prog_ctrl.pc_sysh_prog2_sel = ip2 & 0x7f;

	rtd_outl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_ctrl_reg,ip0 & 0x80),\
					reg_dc_pc_sysh_prog_ctrl.regValue);
	/*RTICE_DEBUG("[%s:%d]ip0=%d,ip1=%d,ip2=%d\r\n", \
	 * __func__,__LINE__,ip0,ip1,ip2);*/
	return 0;
}

/**
  * \brief Enable_PC_go Set pc_go bit to start counting.
  * Reset this to stop counting
  * \param pc_go Counter start counting or not
  * \param fdcu2 Whether checking ips belongs to DCU2.
  * \return : 0 - success, others fail
**/
static int set_pc_go_status(char pc_go,int fdcu2)
{
	dc_sys_dc_pc_ctrl_RBUS         reg_dc_pc_ctrl;
	/*dcu*/
	reg_dc_pc_ctrl.regValue = rtd_inl(dcu_addr(DC_SYS_DC_PC_CTRL_reg,fdcu2));
	reg_dc_pc_ctrl.pc_go = pc_go ? 1 : 0;
	rtd_outl(dcu_addr(DC_SYS_DC_PC_CTRL_reg,fdcu2), reg_dc_pc_ctrl.regValue);
#ifdef SUPPORT_DCU2    
	if(pc_go == 0){
		reg_dc_pc_ctrl.regValue = rtd_inl(dcu_addr(DC_SYS_DC_PC_CTRL_reg,1));
		reg_dc_pc_ctrl.pc_go = pc_go;
		rtd_outl(dcu_addr(DC_SYS_DC_PC_CTRL_reg,1), reg_dc_pc_ctrl.regValue);
	}
#endif
	return 0;
}

static int effi_measure_en(char fen,int fdcu2)
{
#ifdef CONFIG_RTICE_BW_EFFI
	dc1_mc_eff_meas_ctrl_RBUS  reg_eff_meas_ctrl;
	/*efficiency*/
	reg_eff_meas_ctrl.regValue = rtd_inl(dcu_addr(DC1_MC_EFF_MEAS_CTRL_reg,fdcu2));
    	reg_eff_meas_ctrl.meas_int_src = 0;
	reg_eff_meas_ctrl.eff_meas_en = fen;
	if(g_bwCheck.m_measPeriod)
		reg_eff_meas_ctrl.meas_period = g_bwCheck.m_measPeriod;
	else
		reg_eff_meas_ctrl.meas_int_src = 1;/*vsync mode*/
	rtd_outl(dcu_addr(DC1_MC_EFF_MEAS_CTRL_reg,fdcu2),reg_eff_meas_ctrl.regValue);
	return 0;
#else
	return 0;
#endif
}

static int is_effi_finish(int fdcu2)
{
#ifdef CONFIG_RTICE_BW_EFFI
	dc1_mc_eff_meas_ctrl_RBUS reg_eff_meas_ctrl;
	reg_eff_meas_ctrl.regValue = rtd_inl(dcu_addr(DC1_MC_EFF_MEAS_CTRL_reg,fdcu2));
	return reg_eff_meas_ctrl.eff_meas_en == 0 ? 1 : 0;
#else
	return false;
#endif
}

/*------------------------------------------------------------------
 * Func : check_result
 *
 * Desc : Caculate and Save the BW value in buffer by mode
 *
 * Parm : ipGroup,group index,it decides what ips' BW value is saved
 * (pc total para& scpu bw is updated every time)
 *
 * Retn : 0 - success, others fail
 *------------------------------------------------------------------*/
static int check_result(unsigned int*pRet,unsigned int len,int fdcu2)
{
	unsigned int N = 0.0,  F = 0.0,PDIV = 0.0;
	int i = 0,DIV = 0;
	unsigned int ipbw[3];
	unsigned int FreqH;
	u64 dividend,divisor;
	unsigned int *ptotal_num = &pRet[0],*ptotal_ack = &pRet[1],*ptotal_idle = &pRet[2];
	unsigned int *pscup_bw = &pRet[3];
	unsigned int *eff = &pRet[4];
	unsigned int *ptotal_num2 = &pRet[10],*ptotal_ack2 = &pRet[11];
	unsigned int *ptotal_idle2 = &pRet[12],*pscpu_bw2 = &pRet[13];
	unsigned int *eff2 = &pRet[14];
	unsigned int *ip = &pRet[20];
#ifdef CONFIG_RTICE_BW_EFFI
	unsigned int memNum = 2;
#endif

	pll_reg_sys_pll_bush1_RBUS      reg_sys_pll_bush1;
	pll_reg_sys_pll_bush2_RBUS      reg_sys_pll_bush2;
#ifdef CONFIG_RTICE_BW_EFFI
	dc1_mc_read_cmd_RBUS  reg_read_cmd;
	dc1_mc_write_cmd_RBUS reg_write_cmd;
	dc1_mc_tmctrl3_RBUS    reg_tmctrl3;
	dc1_mc_eff_meas_ctrl_RBUS    reg_eff_meas_ctrl;
	dc1_mc_read_chop_cmd_RBUS reg_read_chop_cmd;
	dc1_mc_write_chop_cmd_RBUS reg_write_chop_cmd;

	dc1_mc_parst_acc_rd_cnt_RBUS mc_acc_rd_cnt_reg;
    	dc1_mc_parst_acc_wr_cnt_RBUS mc_acc_wr_cnt_reg;
#endif
	/*total cycle,ack,idle number of dcu1~2*/
	*ptotal_num = rtd_inl(DC_SYS_DC_PC_TOTA_MON_NUM_reg);
	*ptotal_ack = rtd_inl(DC_SYS_DC_PC_TOTA_ACK_NUM_reg);
	*ptotal_idle = rtd_inl(DC_SYS_DC_PC_TOTA_IDL_NUM_reg);
#ifdef SUPPORT_DCU2
	*ptotal_num2 = rtd_inl(dcu_addr(DC_SYS_DC_PC_TOTA_MON_NUM_reg,1));
	*ptotal_ack2 = rtd_inl(dcu_addr(DC_SYS_DC_PC_TOTA_ACK_NUM_reg,1));
	*ptotal_idle2 = rtd_inl(dcu_addr(DC_SYS_DC_PC_TOTA_IDL_NUM_reg,1));
#else
	*ptotal_num2 = 0;
	*ptotal_ack2 = 0;
	*ptotal_idle2 = 0;
#endif
	RTICE_DEBUG("dcu1:total=%u;ack=%u;idle=%u\r\n",*ptotal_num,*ptotal_ack,*ptotal_idle);
	/*RTICE_DEBUG("dcu2:total=%u;ack=%u;idle=%u\r\n",*ptotal_num2,*ptotal_ack2,*ptotal_idle2);*/
	/*efficiency measure*/
#ifdef CONFIG_RTICE_BW_EFFI
	reg_eff_meas_ctrl.regValue = rtd_inl(DC1_MC_EFF_MEAS_CTRL_reg);
	reg_tmctrl3.regValue = rtd_inl(DC1_MC_TMCTRL3_reg);
	reg_read_cmd.regValue = rtd_inl(DC1_MC_READ_CMD_reg);
	reg_write_cmd.regValue = rtd_inl(DC1_MC_WRITE_CMD_reg);
	reg_read_chop_cmd.regValue = rtd_inl(DC1_MC_READ_CHOP_CMD_reg);
	reg_write_chop_cmd.regValue = rtd_inl(DC1_MC_WRITE_CHOP_CMD_reg);
	dividend = (u64)40000 * reg_read_cmd.rd_cnt - (u64)20000 * reg_read_chop_cmd.rd_chop_cnt;
	divisor = (u64)16 * (reg_tmctrl3.refcycle + 1) * reg_eff_meas_ctrl.meas_period * 4;
	eff[1] = reg_eff_meas_ctrl.meas_period == 0 ? (u64)0 : div64_u64(dividend,divisor);
	dividend = (u64)40000 * reg_write_cmd.wr_cnt - (u64)20000 * reg_write_chop_cmd.wr_chop_cnt;
	eff[2] = reg_eff_meas_ctrl.meas_period == 0 ? (u64)0 : div64_u64(dividend,divisor);
	eff[0] = eff[1] + eff[2];
	/*efficient extension info(conflict ,w2r,r2w count)*/
	//bg_unbalance_cnt
	eff[3] = DC1_MC_BG_UNBALANCE_CNT_get_bg_unbalance_cnt(rtd_inl(DC1_MC_BG_UNBALANCE_CNT_reg));
    	/*small range ratio*/
	mc_acc_rd_cnt_reg.regValue = rtd_inl(DC1_MC_PARST_ACC_RD_CNT_reg);
	mc_acc_wr_cnt_reg.regValue = rtd_inl(DC1_MC_PARST_ACC_WR_CNT_reg);
	if(DC_SYS_DC_SYS_MISC_get_mem_num(rtd_inl(DC_SYS_DC_SYS_MISC_reg)) == 2)
		memNum = 3;
	else
		memNum = 2;
	dividend = (u64)memNum * ((u64)reg_read_cmd.rd_cnt + (u64)reg_write_cmd.wr_cnt);
	divisor = (u64)reg_read_chop_cmd.rd_chop_cnt + (u64)reg_write_chop_cmd.wr_chop_cnt;
	divisor += (u64)mc_acc_rd_cnt_reg.parst_acc_rd_cnt + (u64)mc_acc_wr_cnt_reg.parst_acc_wr_cnt;
	if(dividend > divisor)
		dividend -= divisor;
	else
		dividend = 0;
	divisor = (u64)reg_read_cmd.rd_cnt + (u64)reg_write_cmd.wr_cnt;
	eff[4] = divisor == 0 ? (u64)0 : div64_u64((u64)10000 * dividend,divisor);
	eff[5] = 0;//placeholder
#else
	eff[0] = eff[1] = eff[2] = eff[3] = eff[4] = eff[5] = 0;  
#endif
	/*efficiency measure of dcu2*/
#if defined(SUPPORT_DCU2) && defined(CONFIG_RTICE_BW_EFFI)     
	reg_eff_meas_ctrl.regValue = rtd_inl(dcu_addr(DC1_MC_EFF_MEAS_CTRL_reg,1));
	reg_tmctrl3.regValue = rtd_inl(dcu_addr(DC1_MC_TMCTRL3_reg,1));
	reg_read_cmd.regValue = rtd_inl(dcu_addr(DC1_MC_READ_CMD_reg,1));
	reg_write_cmd.regValue = rtd_inl(dcu_addr(DC1_MC_WRITE_CMD_reg,1));
	reg_read_chop_cmd.regValue = rtd_inl(dcu_addr(DC1_MC_READ_CHOP_CMD_reg,1));
	reg_write_chop_cmd.regValue = rtd_inl(dcu_addr(DC1_MC_WRITE_CHOP_CMD_reg,1));
	dividend = (u64)40000 * reg_read_cmd.rd_cnt - (u64)20000 * reg_read_chop_cmd.rd_chop_cnt;
	divisor = (u64)16 * (reg_tmctrl3.refcycle + 1) * reg_eff_meas_ctrl.meas_period * 4;
	eff2[1] = reg_eff_meas_ctrl.meas_period == 0 ? (u64)0 : div64_u64(dividend,divisor);
	dividend = (u64)40000 * reg_write_cmd.wr_cnt - (u64)20000 * reg_write_chop_cmd.wr_chop_cnt;
	eff2[2] = reg_eff_meas_ctrl.meas_period == 0 ? (u64)0 : div64_u64(dividend,divisor);
	eff2[0] = eff2[1] + eff2[2];
	/*efficient extension info(conflict ,w2r,r2w count)*/
	eff2[3] = DC1_MC_BG_UNBALANCE_CNT_get_bg_unbalance_cnt(rtd_inl(dcu_addr(DC1_MC_BG_UNBALANCE_CNT_reg,1)));//bg_unbalance_cnt
	/*small range ratio*/
	mc_acc_rd_cnt_reg.regValue = rtd_inl(dcu_addr(DC1_MC_PARST_ACC_RD_CNT_reg);
	mc_acc_wr_cnt_reg.regValue = rtd_inl(dcu_addr(DC1_MC_PARST_ACC_WR_CNT_reg);
	dividend = (u64)2*reg_read_cmd.rd_cnt - (u64)reg_read_chop_cmd.rd_chop_cnt - (u64)mc_acc_rd_cnt_reg.parst_acc_rd_cnt;
	dividend += (u64)reg_write_cmd.wr_cnt - (u64)reg_write_chop_cmd.wr_chop_cnt - (u64)mc_acc_wr_cnt_reg.parst_acc_wr_cnt;
	divisor = (u64)reg_read_cmd.rd_cnt + (u64)reg_write_cmd.wr_cnt;
	eff2[4] = divisor==0?(u64)0 : div64_u64(dividend,divisor);//Parst_acc_rd_cnt
	eff2[5] = 0;//Parst_acc_wr_cnt
#else
	eff2[0] = eff2[1] = eff2[2] = eff2[3] = eff2[4] = eff2[5] = 0;    
#endif				
	if(len > 14 && (len - 14) % 3 == 0){
		/*PLL Frequency = 27 / (PDIV + 1) * (NCODE + 3 + FCODE/2048) / 2^DIV / 2*/
		reg_sys_pll_bush1.regValue = rtd_inl(PLL_REG_SYS_PLL_BUSH1_reg);
		reg_sys_pll_bush2.regValue = rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg);
		N = reg_sys_pll_bush2.pllbush_ncode;
		F = reg_sys_pll_bush2.pllbush_fcode;
		PDIV = reg_sys_pll_bush1.pllbush_prediv;
		DIV = reg_sys_pll_bush1.pllbush_o;
		i = 1;
		while(DIV-- > 0){
			i *= 2;
		}
		FreqH = 2700 * (N * 2048 + 3 + F) / 2048 / (PDIV + 1) / i / 2 / 100;
		/*ack num of ips*/
	#ifdef SUPPORT_DCU2
		ipbw[0] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_0_ACK_NUM_reg,fdcu2));
		ipbw[1] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_1_ACK_NUM_reg,fdcu2));
		ipbw[2] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_2_ACK_NUM_reg,fdcu2));
		divisor = (fdcu2 ? (u64)*ptotal_num2 : (u64)*ptotal_num);
		dividend = (u64)1600 * (u64)ipbw[0] * (u64)FreqH;
		dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
		ip[0] = dividend;
		dividend = (u64)1600 * (u64)ipbw[1] * (u64)FreqH;
		dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
		ip[1] = dividend;
		dividend = (u64)1600 * (u64)ipbw[2] * (u64)FreqH;
		dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
		ip[2] = dividend;
		/*Max Latency*/
		ip[3] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_0_MAX_LAT_reg,fdcu2)) & 0xFFFF;
		ip[4] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_1_MAX_LAT_reg,fdcu2)) & 0xFFFF;
		ip[5] = rtd_inl(dcu_addr(DC_SYS_DC_PC_SYSH_prog_2_MAX_LAT_reg,fdcu2)) & 0xFFFF;
	#else
		if(!fdcu2){
			ipbw[0] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_0_ACK_NUM_reg));
			ipbw[1] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_1_ACK_NUM_reg));
			ipbw[2] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_2_ACK_NUM_reg));
			divisor = (u64)*ptotal_num;
			dividend = (u64)1600 * (u64)ipbw[0] * (u64)FreqH;
			dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
			ip[0] = dividend;
			dividend = (u64)1600 * (u64)ipbw[1] * (u64)FreqH;
			dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
			ip[1] = dividend;
			dividend = (u64)1600 * (u64)ipbw[2] * (u64)FreqH;
			dividend =  divisor == 0 ? 0 :div64_u64(dividend,divisor);
			ip[2] = dividend;
			/*Max Latency*/
			ip[3] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_0_MAX_LAT_reg)) & 0xFFFF;
			ip[4] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_1_MAX_LAT_reg)) & 0xFFFF;
			ip[5] = rtd_inl((DC_SYS_DC_PC_SYSH_prog_2_MAX_LAT_reg)) & 0xFFFF;
		}else{
			ipbw[0] = ipbw[1] = ipbw[2] = 0;
			ip[0] = ip[1] = ip[2] = ip[3] = ip[4] = ip[5] = 0;
		}
	#endif
	}
	/*scpu bw*/
	*pscup_bw = 0;
	*pscpu_bw2 = 0;
	return 0;

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) 
static void check_entry(struct timer_list *t);
#else
static void check_entry(unsigned long data);
#endif

/**
 * \brief Create or Start bw check thread.
**/
static int enter_bw_check(void)
{
	int ret = 0;
	
	g_bwCheck.p_timer = (struct timer_list*)kmalloc(sizeof(struct timer_list),GFP_ATOMIC);
	if(g_bwCheck.p_timer == NULL)
		return -RTICE_IOMEM_BW_CHECK_THREAD;
	/*check param init*/
	g_bwCheck.m_curCnt = 0;
	g_bwCheck.p_curIp = g_bwCheck.p_ipHeader;
	/*start dc check*/
	if(TRIPLET_IP(g_bwCheck.p_curIp))
		bw_select_ip(g_bwCheck.p_curIp->id,g_bwCheck.p_curIp->p_next->id,g_bwCheck.p_curIp->p_next->p_next->id);
	set_pc_go_status(1,0);
#ifdef SUPPORT_DCU2
	set_pc_go_status(1,1);
#endif
	effi_measure_en(1,0);
#ifdef SUPPORT_DCU2
	effi_measure_en(1,1);
#endif
	/* timer init*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_bwCheck.p_timer->expires = jiffies + (g_bwCheck.m_period ? HZ * g_bwCheck.m_period / 1000 : HZ);
	timer_setup(g_bwCheck.p_timer, check_entry, 0); 
#else
	init_timer(g_bwCheck.p_timer);
	g_bwCheck.p_timer->data = (unsigned long)&g_bwCheck;
	g_bwCheck.p_timer->function = check_entry;
	g_bwCheck.p_timer->expires = jiffies + (g_bwCheck.m_period ? HZ * g_bwCheck.m_period / 1000 : HZ);
	/*add timer*/
	add_timer(g_bwCheck.p_timer);
#endif
	/*check start flag*/
	g_bwCheck.f_run = true;
	return ret;
}
/**
 * \brief Exit and destory bw check thread.
**/
static int exit_bw_check(void)
{
	int ret = 0;

	if(g_bwCheck.p_timer){
		/*RTICE_DEBUG("[%s:%d]Free timer\r\n",__func__,__LINE__);*/
		del_timer(g_bwCheck.p_timer);
		kfree(g_bwCheck.p_timer);
		g_bwCheck.p_timer = NULL;
	}
	/*check start flag*/
	g_bwCheck.f_run = false;
	/*try stop all check or measure flow*/
	set_pc_go_status(0,0);
	effi_measure_en(0,0);
#ifdef SUPPORT_DCU2    
	effi_measure_en(0,1);
#endif
	RTICE_DEBUG("[%s:%d]Exit From 'exit_bw_check'\r\n",__func__,__LINE__);
	return ret;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) 
static void check_entry(struct timer_list *t)
#else
static void check_entry(unsigned long data)
#endif
{
	CBwIp *pIp = NULL;
	CBwCheck*pCheck = &g_bwCheck;
	unsigned int result_buf[26] = {0};
	unsigned int *pbuf = NULL;
	int j = 0;

	/*stop dc check*/
	set_pc_go_status(0,0);
	/*acquire check result*/
	pIp = pCheck->p_curIp;
	if(TRIPLET_IP(pIp))
		check_result(result_buf,sizeof(result_buf),pIp->id & 0x80);
	else
		check_result(result_buf,sizeof(result_buf) - 3,0);
	/*saving result to ip list.*/
	pbuf = &result_buf[0];
	if(pCheck->m_curCnt >= SKIP_FOREFRONT_CNT){
		BW_DATA_W(&pCheck->m_total_cycle, pbuf[0]);
		BW_DATA_W(&pCheck->m_total_ack,pbuf[1]);
		BW_DATA_W(&pCheck->m_total_idle,pbuf[2]);
		BW_DATA_W(&pCheck->m_scpu,pbuf[3]);
		if(is_effi_finish(0) != 0){/*< dcu1 measure is finished*/
			RTICE_DEBUG("[%s:%d]DCU1 Upgrade Efficiency Result.\r\n",__func__,__LINE__);
			BW_DATA_W(&pCheck->m_eff_total,pbuf[4]);
			BW_DATA_W(&pCheck->m_eff_rd,pbuf[5]);
			BW_DATA_W(&pCheck->m_eff_wr,pbuf[6]);
			BW_DATA_W(&pCheck->m_bankConflict,pbuf[7]);
			BW_DATA_W(&pCheck->m_w2rCnt,pbuf[8]);
			BW_DATA_W(&pCheck->m_r2wCnt,pbuf[9]);
		}
		BW_DATA_W(&pCheck->m_total_cycle_dcu2, pbuf[10]);
		BW_DATA_W(&pCheck->m_total_ack_dcu2,pbuf[11]);
		BW_DATA_W(&pCheck->m_total_idle_dcu2,pbuf[12]);
		BW_DATA_W(&pCheck->m_scpu2,pbuf[13]);
	#ifdef SUPPORT_DCU2
		if(is_effi_finish(1) != 0)/*< dcu2 measure is finished*/
	#endif          
		{
			RTICE_DEBUG("[%s:%d]DCU2 Upgrade Efficiency Result.\r\n",__func__,__LINE__);
			BW_DATA_W(&pCheck->m_eff_total2,pbuf[14]);
			BW_DATA_W(&pCheck->m_eff_rd2,pbuf[15]);
			BW_DATA_W(&pCheck->m_eff_wr2,pbuf[16]);
			BW_DATA_W(&pCheck->m_bankConflict2,pbuf[17]);
			BW_DATA_W(&pCheck->m_w2rCnt2,pbuf[18]);
			BW_DATA_W(&pCheck->m_r2wCnt2,pbuf[19]);
		}
	}
	pbuf += 20;
	if(TRIPLET_IP(pIp)){
		CBwIp *ptIp = pIp;
		/*update current and history max to ip element*/
		j = 0;
		while(j < 3 && ptIp){
			if(pCheck->m_curCnt >= SKIP_FOREFRONT_CNT){
				BW_LAT_DATA_W(ptIp,pbuf[0],(unsigned short)pbuf[3]);
				RTICE_DEBUG("+[id:%02x]cur=%u;sum=%llu;his=%llu;max=%u;min=%u;\r\n",\
					ptIp->id,ptIp->m_curBw,ptIp->m_sumBw, \
					ptIp->m_hisLen,ptIp->m_maxBw, \
					ptIp->m_minBw);
			}
			pbuf++;
			j++;
			ptIp = ptIp->p_next;
		}
	}
	/*finished once checking?*/
	if(TRIPLET_IP(pIp))
		pCheck->p_curIp = pIp->p_next->p_next->p_next;
	if(pCheck->p_ipHeader == NULL || !TRIPLET_IP(pCheck->p_curIp)){
		pCheck->m_curCnt++;
		pCheck->p_curIp = pCheck->p_ipHeader;
		/*finish once check*/
		RTICE_DEBUG("[%s:%d]Check Progress:%d/%d.\r\n", __func__,__LINE__,pCheck->m_curCnt,pCheck->m_count);
	}
	/*finished all checking?*/
	if(pCheck->m_count == 0 || pCheck->m_curCnt <  pCheck->m_count + SKIP_FOREFRONT_CNT){/*< need continue check*/
		/*start dc check*/
		if(TRIPLET_IP(pCheck->p_curIp))
			bw_select_ip(pCheck->p_curIp->id, pCheck->p_curIp->p_next->id,pCheck->p_curIp->p_next->p_next->id);
		set_pc_go_status(1,0);
	#ifdef SUPPORT_DCU2
		set_pc_go_status(1,1);
	#endif
		effi_measure_en(1,0);
	#ifdef SUPPORT_DCU2
		effi_measure_en(1,1);
	#endif
		/*restart timer*/
		if(pCheck->p_timer){
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			g_bwCheck.p_timer->expires = jiffies + (g_bwCheck.m_period ? HZ * g_bwCheck.m_period / 1000 : HZ);
			timer_setup(g_bwCheck.p_timer, check_entry, 0); 
		#else
			pCheck->p_timer->function = check_entry;
			pCheck->p_timer->expires = jiffies + (pCheck->m_period ? HZ * pCheck->m_period / 1000 : HZ);
			add_timer(pCheck->p_timer);
		#endif
		}
	}else{
		pCheck->f_run = false;
		RTICE_DEBUG("===========\r\nAll Finish.\r\n=============\r\n");
	}
}

/*------------------------------------------------------------------
 * Func : rtice_iomem2_command_handler
 *
 * Desc : proc iomem command
 *
 * Parm : cmd_data : command data
 *        command_data_len  : command data len
 *
 * Retn :  < 0    : error
 *         others : length of response data
 *------------------------------------------------------------------*/
int rtice_iomem2_command_handler(
    unsigned char       opcode,
    unsigned char*      cmd_data,
    unsigned int        command_data_len,
    unsigned char*      response_data,
    unsigned int        response_buff_size
    )
{
	int ret = 0;
	switch(opcode){
	case RTICE_IOMEM_CMD_ENTER_BW_CHECK:
	{
		/*stop bw check thread if it is runing now.*/
		exit_bw_check();
		/*check param init*/
		memset(&g_bwCheck,0,sizeof(CBwCheck));
		BW_DATA_CLR(&g_bwCheck.m_total_cycle);
		BW_DATA_CLR(&g_bwCheck.m_total_ack);
		BW_DATA_CLR(&g_bwCheck.m_total_idle);
		BW_DATA_CLR(&g_bwCheck.m_scpu);
		BW_DATA_CLR(&g_bwCheck.m_eff_total);
		BW_DATA_CLR(&g_bwCheck.m_eff_rd);
		BW_DATA_CLR(&g_bwCheck.m_eff_wr);
		BW_DATA_CLR(&g_bwCheck.m_eff_total);
		BW_DATA_CLR(&g_bwCheck.m_eff_rd);
		BW_DATA_CLR(&g_bwCheck.m_eff_wr);
		BW_DATA_CLR(&g_bwCheck.m_total_cycle_dcu2);
		BW_DATA_CLR(&g_bwCheck.m_total_ack_dcu2);
		BW_DATA_CLR(&g_bwCheck.m_total_idle_dcu2);
		BW_DATA_CLR(&g_bwCheck.m_eff_total2);
		BW_DATA_CLR(&g_bwCheck.m_eff_rd2);
		BW_DATA_CLR(&g_bwCheck.m_eff_wr2);
		BW_DATA_CLR(&g_bwCheck.m_eff_total2);
		BW_DATA_CLR(&g_bwCheck.m_eff_rd2);
		BW_DATA_CLR(&g_bwCheck.m_eff_wr2);
		/*checking params length*/
		if(command_data_len < 6 || command_data_len != 3 * (2+cmd_data[5]))
			return -RTICE_IOMEM_BW_INVALID_PARAM;
		g_bwCheck.m_count = cmd_data[0];
		g_bwCheck.m_measPeriod = B2TS(cmd_data[1], cmd_data[2]);
		/*check interval*/
		g_bwCheck.m_period = B2TS( cmd_data[3], cmd_data[4]);
		g_bwCheck.m_grpNum = cmd_data[5];
		RTICE_DEBUG("[%s:%d]DC Period:%d,Meas Period=%d\r\n",__func__,__LINE__,g_bwCheck.m_period, g_bwCheck.m_measPeriod);
		bw_ip_list_clear();
		/*alocate checking ips*/
		bw_ip_list_init(g_bwCheck.m_grpNum*3, &cmd_data[6]);
		return enter_bw_check();
	}
	case RTICE_IOMEM_CMD_BW_CHECK_RESULT:
	{
		unsigned char* presData = response_data;
		CBwIp *pIp = NULL;
		unsigned char grpNum = g_bwCheck.m_grpNum;
		unsigned char i = 0;
		unsigned int bws[4],t32 = 0;
		unsigned short lats[3] = {0};
		unsigned short global_data_len = 0;

		if(command_data_len != 1 || cmd_data[0] >= BW_IP_QUERY_SIZE)
			return -RTICE_IOMEM_BW_INVALID_PARAM;
		/*query info*/
		*presData++ = 0x80 + 1;/*extend data type*/
		/*global area length*/
		global_data_len = 69 + 2;
		STB2(global_data_len,presData[0],presData[1]);
		presData += 2;
		/*global dc & scpu value*/
		t32  = BW_DATA_R(&g_bwCheck.m_total_cycle,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32 = BW_DATA_R(&g_bwCheck.m_total_ack,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_total_idle,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_scpu,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		/*dcu1 efficiency value*/
		t32  = BW_DATA_R(&g_bwCheck.m_eff_total,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32   = BW_DATA_R(&g_bwCheck.m_eff_rd,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32   = BW_DATA_R(&g_bwCheck.m_eff_wr,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32  = BW_DATA_R(&g_bwCheck.m_bankConflict,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_w2rCnt,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_r2wCnt,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		/*dcu2 total param*/
		t32  = BW_DATA_R(&g_bwCheck.m_total_cycle_dcu2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32 = BW_DATA_R(&g_bwCheck.m_total_ack_dcu2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_total_idle_dcu2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_scpu2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		/*dcu2 efficiency value*/
		t32  = BW_DATA_R(&g_bwCheck.m_eff_total2,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32   = BW_DATA_R(&g_bwCheck.m_eff_rd2,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32   = BW_DATA_R(&g_bwCheck.m_eff_wr2,cmd_data[0]);
		STB2(t32  ,presData[0],presData[1]);
		presData += 2;
		t32  = BW_DATA_R(&g_bwCheck.m_bankConflict2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_w2rCnt2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		t32  = BW_DATA_R(&g_bwCheck.m_r2wCnt2,cmd_data[0]);
		LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
		presData += 4;
		/*ip bw value*/
		pIp = g_bwCheck.p_ipHeader;
		while(pIp && grpNum){
			bws[i] = BW_DATA_R(pIp,cmd_data[0]);
			if(i == 2){
				bws[3] = bws[0] + bws[1] + bws[2];
				for(i = 0; i < 4;i++){
					LTB4(bws[i], presData[0], presData[1],presData[2],presData[3]);
					presData += 4;
				}
				for(i = 0;i < 3;i++){
					STB2(lats[i], presData[0], presData[1]);
					presData += 2;
				}
				grpNum--;
				i = 0;
			}else{
				i++;
			}
			pIp = pIp->p_next;
		}
		if(grpNum != 0)
			return -RTICE_IOMEM_BW_WRONG_GROUPNUM;
		/*ret*/
		ret = g_bwCheck.m_grpNum * (4 * 4 + 6) + global_data_len;
	}
	break;
	case RTICE_IOMEM_CMD_EXIT_BW_CHECK:
	{
		return exit_bw_check();
	}
	case RTICE_IOMEM_CMD_BW_CHECK_STATUS:
	{
		if(g_bwCheck.f_run)
			response_data[0] = BW_CHECK_STATUS_RUN;
		else
			response_data[0] = BW_CHECK_STATUS_STOP;
		ret = 1;
	}
	break;
	case RTICE_IOMEM_CMD_START_LATENCY_CHECK:
	{
		unsigned int period = 0;

		if(command_data_len != 5)
			return -RTICE_IOMEM_BW_INVALID_PARAM;
		period = B4TL(cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
		ret = startCheckLatency(period,cmd_data[4]);
	}
	break;
	case RTICE_IOMEM_CMD_STOP_LATENCY_CHECK:
	{
		ret = stopCheckLatency();
	}
	break;
	case RTICE_IOMEM_CMD_GET_ALL_LATENCY:
	{
		unsigned int addr = 0,size = 0;

		if(command_data_len != 8)
			return -RTICE_IOMEM_BW_INVALID_PARAM;
		addr = B4TL(cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3]);
		size = B4TL(cmd_data[4], cmd_data[5], cmd_data[6], cmd_data[7]);
		ret = getClientsLatency(addr,size,response_data);
	}
	break;
	default:
		ret = -RTICE_ERR_UNSUPPORTED_OP_CODE;
	}/*~switch(opcode)*/
	return ret;
}

/*------------------------------------------------------------------
 * Func : rtice_i2c_command_probe
 *
 * Desc : proc i2c command
 *
 * Parm : opcode
 *
 * Retn :
 *------------------------------------------------------------------*/
int rtice_iomem2_command_probe(unsigned char op_code)
{
	return 1;
}

rtice_command_handler_t iomem2_cmd_handler[] =
{
	{
		RTICE_CMD_GROUP_BW_CHECK,
		rtice_iomem2_command_probe,
		rtice_iomem2_command_handler
	},
};

/*------------------------------------------------------------------
 * Func : rtice_iomem_tool_init
 *
 * Desc : register rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/

int rtice_iomem2_tool_init(void)
{
	rtice_register_command_handler(iomem2_cmd_handler, \
		sizeof(iomem2_cmd_handler)/sizeof(rtice_command_handler_t));
	return 0;
}

/*------------------------------------------------------------------
 * Func : rtice_iomem_tool_exit
 *
 * Desc : exit rtice io/mem tool
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void rtice_iomem2_tool_exit(void)
{
	rtice_unregister_command_handler(iomem2_cmd_handler, \
		sizeof(iomem2_cmd_handler)/sizeof(rtice_command_handler_t));
	rtd_pr_rtice_info("rtice_unregister_command_handler() has been executed.\n");
	/*stop and exit from bw checking thread and destory it.*/
	exit_bw_check();
	/*clear bw checking ip list.*/
	bw_ip_list_clear();
}

