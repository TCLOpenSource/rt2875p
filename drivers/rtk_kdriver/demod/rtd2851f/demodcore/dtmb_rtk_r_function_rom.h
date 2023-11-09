#define DTMB_ROMCODE_REDUCE 1 //1: reduce dtmb function for ROM code 

#define U8  unsigned char
#define U16 unsigned int
#define U32 unsigned long
#define S8  char
#define S16 int
#define S32 long

#define MASK_GEN_1B(msb, lsb)  (0xff<<(lsb))&       (0xff>>(7-(msb)))
#define MASK_GEN_2B(msb, lsb)  (0xffff<<(lsb))&     (0xffff>>(7-(msb)))
#define MASK_GEN_3B(msb, lsb)  (0xffffff<<(lsb))&   (0xffffff>>(7-(msb)))
#define MASK_GEN_4B(msb, lsb)  (0xffffffff<<(lsb))& (0xffffffff>>(7-(msb)))

//DTMB Register table
//20091012 updata register.h (reversion:1245)
#define DTMB_BASE_ADR 0xC000

//fixed PN ind  u(1, 0f)
#define DTMB_R_FIXED_PN_ADR 0x0A33 + DTMB_BASE_ADR
#define DTMB_R_FIXED_PN_MSB 0
#define DTMB_R_FIXED_PN_LSB 0
#define DTMB_R_FIXED_PN_MASK_1B MASK_GEN_1B(DTMB_R_FIXED_PN_MSB, DTMB_R_FIXED_PN_LSB)

//cfo_est_r  s(23, 26f)
#define DTMB_R_CFO_EST_ADR 0x0994 + DTMB_BASE_ADR
#define DTMB_R_CFO_EST_MSB 6
#define DTMB_R_CFO_EST_LSB 0
#define DTMB_R_CFO_EST_MASK_3B MASK_GEN_3B(DTMB_R_CFO_EST_MSB, DTMB_R_CFO_EST_LSB)

//sfoaq_out   s(14, 20f)
#define DTMB_R_SFOAQ_OUT_ADR 0x0A21 + DTMB_BASE_ADR
#define DTMB_R_SFOAQ_OUT_MSB 5
#define DTMB_R_SFOAQ_OUT_LSB 0
#define DTMB_R_SFOAQ_OUT_MASK_2B MASK_GEN_2B(DTMB_R_SFOAQ_OUT_MSB, DTMB_R_SFOAQ_OUT_LSB)

//tr_out_r  s(17, 23f)
#define DTMB_R_TR_OUT_ADR 0x0A7C + DTMB_BASE_ADR
#define DTMB_R_TR_OUT_MSB 0
#define DTMB_R_TR_OUT_LSB 0
#define DTMB_R_TR_OUT_MASK_3B MASK_GEN_3B(DTMB_R_TR_OUT_MSB, DTMB_R_TR_OUT_LSB)


#define DTMB_R_IF_AGC_ADR 0x0914 + DTMB_BASE_ADR
#define DTMB_R_IF_AGC_MSB 5
#define DTMB_R_IF_AGC_LSB 0
#define DTMB_R_IF_AGC_MASK_2B MASK_GEN_2B(DTMB_R_IF_AGC_MSB, DTMB_R_IF_AGC_LSB)

#define DTMB_R_RF_AGC_ADR 0x0916 + DTMB_BASE_ADR
#define DTMB_R_RF_AGC_MSB 5
#define DTMB_R_RF_AGC_LSB 0
#define DTMB_R_RF_AGC_MASK_2B MASK_GEN_2B(DTMB_R_RF_AGC_MSB, DTMB_R_RF_AGC_LSB)

#define DTMB_R_FSM_STATE_ADR 0x09C0 + DTMB_BASE_ADR
#define DTMB_R_FSM_STATE_MSB 4
#define DTMB_R_FSM_STATE_LSB 0
#define DTMB_R_FSM_STATE_MASK_1B MASK_GEN_1B(DTMB_R_FSM_STATE_MSB, DTMB_R_FSM_STATE_LSB)

#define DTMB_R_EST_SNR_ADR 0x0B3E + DTMB_BASE_ADR
#define DTMB_R_EST_SNR_MSB 0
#define DTMB_R_EST_SNR_LSB 0
#define DTMB_R_EST_SNR_MASK_2B MASK_GEN_2B(DTMB_R_EST_SNR_MSB, DTMB_R_EST_SNR_LSB)

#define DTMB_R_ITER_NUM_ADR 0x0C32 + DTMB_BASE_ADR
#define DTMB_R_ITER_NUM_MSB 7
#define DTMB_R_ITER_NUM_LSB 1
#define DTMB_R_ITER_NUM_MASK_1B MASK_GEN_1B(DTMB_R_ITER_NUM_MSB, DTMB_R_ITER_NUM_LSB)

#define DTMB_R_PER_ADR 0x0C2E + DTMB_BASE_ADR
#define DTMB_R_PER_MSB 7
#define DTMB_R_PER_LSB 0
#define DTMB_R_PER_MASK_2B MASK_GEN_2B(DTMB_R_PER_MSB, DTMB_R_PER_LSB)

#define DTMB_R_BER_ADR 0x0C34 + DTMB_BASE_ADR
#define DTMB_R_BER_MSB 1
#define DTMB_R_BER_LSB 0
#define DTMB_R_BER_MASK_3B MASK_GEN_3B(DTMB_R_BER_MSB, DTMB_R_BER_LSB)

#define DTMB_R_TPS_ADR 0x0B2A + DTMB_BASE_ADR
#define DTMB_R_TPS_MSB 5
#define DTMB_R_TPS_LSB 1
#define DTMB_R_TPS_MASK_1B MASK_GEN_1B(DTMB_R_TPS_MSB, DTMB_R_TPS_LSB)

#define DTMB_R_CE_STATE_ADR 0x0B28 + DTMB_BASE_ADR
#define DTMB_R_CE_STATE_MSB 3
#define DTMB_R_CE_STATE_LSB 0
#define DTMB_R_CE_STATE_MASK_1B MASK_GEN_1B(DTMB_R_CE_STATE_MSB, DTMB_R_CE_STATE_LSB)

#define DTMB_R_DAGC_GAIN_ADR 0x09B4 + DTMB_BASE_ADR
#define DTMB_R_DAGC_GAIN_MSB 4
#define DTMB_R_DAGC_GAIN_LSB 1
#define DTMB_R_DAGC_GAIN_MASK_2B MASK_GEN_2B(DTMB_R_DAGC_GAIN_MSB, DTMB_R_DAGC_GAIN_LSB)

#define DTMB_R_RX_MODE_ADR 0x0A17 + DTMB_BASE_ADR
#define DTMB_R_RX_MODE_MSB 1
#define DTMB_R_RX_MODE_LSB 0
#define DTMB_R_RX_MODE_MASK_1B MASK_GEN_1B(DTMB_R_RX_MODE_MSB, DTMB_R_RX_MODE_LSB)

#define DTMB_R_EST_CARRIER_ADR 0x0B2A + DTMB_BASE_ADR
#define DTMB_R_EST_CARRIER_MSB 0
#define DTMB_R_EST_CARRIER_LSB 0
#define DTMB_R_EST_CARRIER_MASK_1B MASK_GEN_1B(DTMB_R_EST_CARRIER_MSB, DTMB_R_EST_CARRIER_LSB)

#define DTMB_R_SP_INV_ADR 0x0131 + DTMB_BASE_ADR
#define DTMB_R_SP_INV_MSB 1
#define DTMB_R_SP_INV_LSB 1
#define DTMB_R_SP_INV_MASK_1B MASK_GEN_1B(DTMB_R_SP_INV_MSB, DTMB_R_SP_INV_LSB)

#define DTMB_R_EN_DCR_ADR 0x0131 + DTMB_BASE_ADR
#define DTMB_R_EN_DCR_MSB 0
#define DTMB_R_EN_DCR_LSB 0
#define DTMB_R_EN_DCR_MASK_1B MASK_GEN_1B(DTMB_R_EN_DCR_MSB, DTMB_R_EN_DCR_LSB)

#define DTMB_R_AAGC_HOLD_ADR 0x0108 + DTMB_BASE_ADR
#define DTMB_R_AAGC_HOLD_MSB 3
#define DTMB_R_AAGC_HOLD_LSB 3
#define DTMB_R_AAGC_HOLD_MASK_1B MASK_GEN_1B(DTMB_R_AAGC_HOLD_MSB, DTMB_R_AAGC_HOLD_LSB)

#define DTMB_R_BBIN_EN_ADR 0x0174 + DTMB_BASE_ADR
#define DTMB_R_BBIN_EN_MSB 0
#define DTMB_R_BBIN_EN_LSB 0
#define DTMB_R_BBIN_EN_MASK_1B MASK_GEN_1B(DTMB_R_BBIN_EN_MSB, DTMB_R_BBIN_EN_LSB)

#define DTMB_R_MOB_CHK_EN_ADR 0x0298 + DTMB_BASE_ADR
#define DTMB_R_MOB_CHK_EN_MSB 0
#define DTMB_R_MOB_CHK_EN_LSB 0
#define DTMB_R_MOB_CHK_EN_MASK_1B MASK_GEN_1B(DTMB_R_MOB_CHK_EN_MSB, DTMB_R_MOB_CHK_EN_LSB)

#define DTMB_R_PSET_MOB_ADR 0x0298 + DTMB_BASE_ADR
#define DTMB_R_PSET_MOB_MSB 2
#define DTMB_R_PSET_MOB_LSB 1
#define DTMB_R_PSET_MOB_MASK_1B MASK_GEN_1B(DTMB_R_PSET_MOB_MSB, DTMB_R_PSET_MOB_LSB)

//mask_cnt  R  u(1,0f)
#define DTMB_R_MASK_CNT_ADR 0x0B30 + DTMB_BASE_ADR
#define DTMB_R_MASK_CNT_MSB 1
#define DTMB_R_MASK_CNT_LSB 0
#define DTMB_R_MASK_CNT_MASK_2B MASK_GEN_2B(DTMB_R_MASK_CNT_MSB, DTMB_R_MASK_CNT_LSB)

//mobchk_ste_r  R  u(2,0f)
#define DTMB_R_MOBCHK_STE_ADR 0x0948 + DTMB_BASE_ADR
#define DTMB_R_MOBCHK_STE_MSB 4
#define DTMB_R_MOBCHK_STE_LSB 0
#define DTMB_R_MOBCHK_STE_MASK_2B MASK_GEN_2B(DTMB_R_MOBCHK_STE_MSB, DTMB_R_MOBCHK_STE_LSB)

//btrk_en  u(1,0f)
#define DTMB_R_BTRK_EN_ADR 0x028D + DTMB_BASE_ADR
#define DTMB_R_BTRK_EN_MSB 0
#define DTMB_R_BTRK_EN_LSB 0
#define DTMB_R_BTRK_EN_MASK_1B MASK_GEN_1B(DTMB_R_BTRK_EN_MSB, DTMB_R_BTRK_EN_LSB)


//pnppeak_idx_r  R  u(13, 0f)
#define DTMB_R_PNPEAK_IDX_ADR 0x0948 + DTMB_BASE_ADR
#define DTMB_R_PNPEAK_IDX_MSB 4
#define DTMB_R_PNPEAK_IDX_LSB 0
#define DTMB_R_PNPEAK_IDX_MASK_2B MASK_GEN_2B(DTMB_R_PNPEAK_IDX_MSB, DTMB_R_PNPEAK_IDX_LSB)


//en_notch R u(2,0f)
#define DTMB_R_EN_NOTCH_ADR 0x0973 + DTMB_BASE_ADR
#define DTMB_R_EN_NOTCH_MSB 1
#define DTMB_R_EN_NOTCH_LSB 0
#define DTMB_R_EN_NOTCH_MASK_1B MASK_GEN_1B(DTMB_R_EN_NOTCH_MSB, DTMB_R_EN_NOTCH_LSB)



//#define DTMB_GETCR dtmb_Readbits3BS(DTMB_R_CFO_EST_ADR, DTMB_R_CFO_EST_MASK_3B, DTMB_R_CFO_EST_MSB, DTMB_R_CFO_EST_LSB)

//need take Big-Endian and Little Endian problem when use BYTE0~BYTE3
#define DEBUG_PAGE_8051   0x0701  +DTMB_BASE_ADR
#define DEBUG_ADDR_8051   0x0702  +DTMB_BASE_ADR
#define DEBUG_FREEZE_8051 0x0703  +DTMB_BASE_ADR
#define DEBUG_BYTE3_8051  0x0704  +DTMB_BASE_ADR //MSB
#define DEBUG_BYTE2_8051  0x0705  +DTMB_BASE_ADR
#define DEBUG_BYTE1_8051  0x0706  +DTMB_BASE_ADR
#define DEBUG_BYTE0_8051  0x0707  +DTMB_BASE_ADR //LSB





/*******************************************
//DTMB function in ROM
********************************************/

//code size ~=instruction cycyle
//1 instruction cycle=4/fclk=4/60M= 66.7ns

//U32  dtmb_Read4b(U16 start_adr);    //read 4 bytes continoues   (size=41 Bytes, time=2.7us)
//U16  dtmb_Read2b(U16 start_adr);   //read 2 bytes continoues    (size=29 Bytes, time=1.9us)

/*
void    Writebits1B(U16 start_adr, U8 val, U8 msb, U8 lsb);  //bitwise write (<=1 byte)  (size=69 Bytes, time=4.6us)
void    Writebits2B(U16 start_adr, U16 val, U8 msb, U8 lsb);  //bitwise write (1<  =<2 byte)  (size=122 Bytes, time=8.1us)
void    Writebits4B(U16 start_adr, U32 val, U8 msb, U8 lsb);  //bitwise write (2<  =<4 byte)
U8   dtmb_Readbits1B(U16 start_adr, U8 msb, U8 lsb); //bitwise read (<=1byte)  (size=41 Bytes, time=2.7us)
U16  dtmb_Readbits2B(U16 start_adr, U8 msb, U8 lsb); //bitwise read (1< <=2bytes)  (msb=(msb of stop byte), lsb=(lsb of start byte)) (size=78 Bytes, time=5.2us)
U32  dtmb_Readbits3B(U16 start_adr, U8 msb, U8 lsb); //bitwise read (2< <=3byte)  (size=119 Bytes, time=7.9us)
U32  dtmb_Readbits4B(U16 start_adr, U8 msb, U8 lsb); //bitwise read (3< <=4byte)   (size=197 Bytes, time=13.1us)
*/
//void dtmb_funclib_rom(void);
//void dtmb_ft_test(void);

//void    Writebits1B_H(U16 start_adr, U8 shifted_val, U8 mask);  //bitwise write (<=1 byte)   (size=16 bytes, time=1.1 us)
//void    Writebits2B_H(U16 start_adr, U16 shifted_val, U16 mask);  //bitwise write (1<  =<2 byte)  (size=46 bytes, time=3.1 us)
//void    Writebits4B_H(U16 start_adr, U32 shifted_val, U32 mask);  //bitwise write (2<  =<4 byte)  (size=118 bytes, time=7.8 us)

//void    Writebits1B(U16 start_adr, U8 val, U8 mask, U8 lsb);  //bitwise write (<=1 byte)   (size=35 bytes, time=2.3 us)
//void    Writebits2B(U16 start_adr, U16 val, U16 mask, U8 lsb);  //bitwise write (1<  =<2 byte)  (size=55 bytes, time=3.7 us)
//void    Writebits4B(U16 start_adr, U32 val, U32 mask, U8 lsb);  //bitwise write (2<  =<4 byte)  (size=141 bytes, time=9.4 us)

//U8   dtmb_Readbits1B(U16 start_adr, U8 mask, U8 lsb); //bitwise read (<=1byte)     (size=20 bytes, time=1.3 us)
//U16  dtmb_Readbits2B(U16 start_adr, U16 mask, U8 lsb); //bitwise read (1< <=2bytes)  (msb=(msb of stop byte), lsb=(lsb of start byte))    (size=34+29 bytes, time=2.3+1.9 us)
//U32  dtmb_Readbits3B(U16 start_adr, U32 mask, U8 lsb); //bitwise read (2< <=3byte)   (size=46+41 bytes, time=3.1+2.7 us)
//U32  dtmb_Readbits4B(U16 start_adr, U32 mask, U8 lsb); //bitwise read (3< <=4byte)   (size=46+41 bytes, time=3.1+2.7 us)

//S8   dtmb_Readbits1BS(U16 start_adr, U8 mask, U8 msb, U8 lsb); //bitwise read (<=1byte)     (size= 43 bytes, time= 2.9us)
//S16  dtmb_Readbits2BS(U16 start_adr, U16 mask, U8 msb, U8 lsb); //bitwise read (1< <=2bytes)  (msb=(msb of stop byte), lsb=(lsb of start byte))    (size=61+29 bytes, time=4.1+1.9 us)
//S32  dtmb_Readbits3BS(U16 start_adr, U32 mask, U8 msb, U8 lsb); //bitwise read (2< <=3byte)  (size=65+41 bytes, time=4.3+2.7 us)
//S32  dtmb_Readbits4BS(U16 start_adr, U32 mask, U8 msb, U8 lsb); //bitwise read (3< <=4byte)   (size=65+41 bytes, time=4.3+2.7 us)


//U16 dtmb_getPER(void);  //u(16, 15f)
//S16 dtmb_getSNR(void);  //s(9,2f)
//U8  dtmb_getTPS(void);  // u(5, 0f)

//#if DTMB_ROMCODE_REDUCE==0
//DTMB get function
//S32 dtmb_getCR(void);  //s(32, 26f)*15.12   unit:Hz
//S16 dtmb_getTR(void);  //s(16, 20f)*15.12/48*1e6 ppm
//S16 dtmb_getIFAGC(void);  //s(14, 13f)
//S16 dtmb_getRFAGC(void);  //s(14, 13f)
//U8  dtmb_getFSM(void);   //u(4,0f)
//U8  dtmb_getIterUse(void); //get LPDC iteration used , u(7, 0f)
//U32 dtmb_getBER(void);  // ber= u(18, 17f)/752
//U8  dtmb_getCEstate(void);  //u(4, 0f)
//U16 dtmb_getDAGCgain(void);   //u(12, 8f)
//U8  dtmb_getPNmode(void); //u(2, 0f)
//U8  dtmb_getEstCarrier(void); //u(1, 0)
//U16 dtmb_getMaskCnt(void); //mask_cnt  R  u(1,0f)
//U16 dtmb_getPNpeak_idx(void);  //pnppeak_idx_r  R  u(13, 0f)
//U8  dtmb_GetMobchk_ste(void);  //mobchk_ste_r  R  u(2,0f)
//U8  dtmb_getEnNotch(void);  //en_notch R u(2,0f)

//DTMB set function
//void dtmb_set_en_sp_inv(U8 sp_inv);  //enable spectrum inverse u(1, 0f)
//void dtmb_set_en_dcr(U8 en_dcr);  //enable dcr u(1, 0f)
//void dtmb_set_aagc_hold(U8 aagc_hold);  //hold aagc u(1, 0f)
//void dtmb_set_bbin_en(U8 bbin_en);  //enable bbin u(1, 0f)
//void dtmb_set_mob_chk_en(U8 mob_chk_en);//enable mobdile check u(1, 0f)
//void dtmb_set_pset_mob(U8 pset_mob); //preset mobile mod u(1, 0f)
//void dtmb_set_btrk_en(U8 btrk_en);  //set btrk u(1, 0f)
//#endif


