/*------------------------------------------------------------------------------
  Copyright 2017-2018 Sony Semiconductor Solutions Corporation

  Last Updated  : 2018/10/31
  File Revision : 1.0.2.0
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 Based on HELENE2 application note 0.10.0
------------------------------------------------------------------------------*/

#include "sony_helene2.h"

/*------------------------------------------------------------------------------
 Definitions of static const table
------------------------------------------------------------------------------*/
#define AUTO         (0xFF) /* For IF_OUT_SEL and AGC_SEL, it means that the value is desided by config flags. */
                            /* For RF_GAIN, it means that RF_GAIN_SEL(SubAddr:0x4E) = 1 */
#define OFFSET(ofs)  ((uint8_t)(ofs) & 0x1F)
#define BW_6         (0x00)
#define BW_7         (0x01)
#define BW_8         (0x02)
#define BW_1_7       (0x03)

/**
  @brief Sony silicon tuner setting for each broadcasting system.

         These values are optimized for Sony demodulators.
         The user have to change these values if other demodulators are used.
         Please check Sony silicon tuner application note for detail.
*/
static const sony_helene2_terr_adjust_param_t g_terr_param_table[SONY_HELENE2_TERR_TV_SYSTEM_NUM] = {
    /*
         IF_BPF_GC                                           BW              BW_OFFSET        IS_LOWERLOCAL
     RF_GAIN |     RFOVLD_DET_LV1    IFOVLD_DET_LV  IF_BPF_F0 |   FIF_OFFSET     |       AGC_SEL |
       |     |    (VL)  (VH)  (U)   (VL)  (VH)  (U)    |      |       |          |          |    |          */
    {AUTO, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, BW_6,  OFFSET(0),  OFFSET(0),  AUTO, 0x00}, /**< SONY_HELENE2_TV_SYSTEM_UNKNOWN */
    /* Analog */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_6,  OFFSET(0),  OFFSET(1),  AUTO, 0x00}, /**< SONY_HELENE2_ATV_MN_EIAJ   (System-M (Japan)) */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_6,  OFFSET(0),  OFFSET(1),  AUTO, 0x00}, /**< SONY_HELENE2_ATV_MN_SAP    (System-M (US)) */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_6,  OFFSET(3),  OFFSET(1),  AUTO, 0x00}, /**< SONY_HELENE2_ATV_MN_A2     (System-M (Korea)) */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_7,  OFFSET(11), OFFSET(5),  AUTO, 0x00}, /**< SONY_HELENE2_ATV_BG        (System-B/G) */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_ATV_I         (System-I) */
    {AUTO, 0x04, 0x03, 0x06, 0x05, 0x02, 0x02, 0x02, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_ATV_DK        (System-D/K) */
    {AUTO, 0x02, 0x04, 0x0C, 0x06, 0x05, 0x05, 0x05, 0x00, BW_8,  OFFSET(2),  OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_ATV_L         (System-L) */
    {AUTO, 0x02, 0x04, 0x0C, 0x06, 0x05, 0x05, 0x05, 0x00, BW_8,  OFFSET(-1), OFFSET(4),  AUTO, 0x00}, /**< SONY_HELENE2_ATV_L_DASH    (System-L DASH) */
    /* Digital */
    {AUTO, 0x07, 0x0D, 0x0D, 0x0D, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-6), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_8VSB      (ATSC 8VSB) */
    {AUTO, 0x07, 0x0D, 0x0D, 0x0D, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-9), OFFSET(-5), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ISDBT_6   (ISDB-T 6MHzBW) */
    {AUTO, 0x07, 0x0D, 0x0D, 0x0D, 0x03, 0x03, 0x03, 0x00, BW_7,  OFFSET(-7), OFFSET(-6), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ISDBT_7   (ISDB-T 7MHzBW) */
    {AUTO, 0x07, 0x0D, 0x0D, 0x0D, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-5), OFFSET(-7), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ISDBT_8   (ISDB-T 8MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT_5    (DVB-T 5MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT_6    (DVB-T 6MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT_7    (DVB-T 7MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT_8    (DVB-T 8MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_1_7,OFFSET(-10),OFFSET(-10),AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT2_1_7 (DVB-T2 1.7MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT2_5   (DVB-T2 5MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT2_6   (DVB-T2 6MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT2_7   (DVB-T2 7MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBT2_8   (DVB-T2 8MHzBW) */
    {AUTO, 0x03, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-6), OFFSET(-4), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBC_6    (DVB-C 6MHzBW/ISDB-C/J.83B) */
    {AUTO, 0x03, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(-2), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBC_8    (DVB-C 8MHzBW,7MHzBW) */
    {AUTO, 0x04, 0x05, 0x05, 0x05, 0x02, 0x02, 0x02, 0x00, BW_6,  OFFSET(-6), OFFSET(-2), AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBC2_6   (DVB-C2 6MHzBW) */
    {AUTO, 0x04, 0x05, 0x05, 0x05, 0x02, 0x02, 0x02, 0x00, BW_8,  OFFSET(-2), OFFSET(0),  AUTO, 0x00}, /**< SONY_HELENE2_DTV_DVBC2_8   (DVB-C2 8MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_6,  OFFSET(-8), OFFSET(-3), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ATSC3_6   (ATSC 3.0 6MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_7,  OFFSET(-6), OFFSET(-5), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ATSC3_7   (ATSC 3.0 7MHzBW) */
    {AUTO, 0x08, 0x0C, 0x0C, 0x0C, 0x04, 0x04, 0x04, 0x00, BW_8,  OFFSET(-4), OFFSET(-6), AUTO, 0x00}, /**< SONY_HELENE2_DTV_ATSC3_8   (ATSC 3.0 8MHzBW) */
    {AUTO, 0x03, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x00, BW_6,  OFFSET(-5), OFFSET(2),  AUTO, 0x00}, /**< SONY_HELENE2_DTV_J83B_5_6  (J.83B 5.6Msps) */
    {AUTO, 0x07, 0x0D, 0x0D, 0x0D, 0x03, 0x03, 0x03, 0x00, BW_8,  OFFSET(2),  OFFSET(1),  AUTO, 0x00}  /**< SONY_HELENE2_DTV_DTMB      (DTMB) */
};

/*------------------------------------------------------------------------------
 Definitions of static functions
------------------------------------------------------------------------------*/
/**
 @brief Configure the HELENE2 tuner from Power On to Sleep state.
*/
static sony_result_t X_pon(sony_helene2_t *pTuner);
/**
 @brief Configure the HELENE2 tuner for specified terrestrial broadcasting system.
*/
static sony_result_t TER_tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem, uint8_t vcoCal);
/**
 @brief Configure the HELENE2 tuner for specified satellite broadcasting system.
*/
static sony_result_t SAT_tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem,  uint32_t symbolRateksps, uint8_t vcoCal);
/**
 @brief The last part of terrestrial tuning sequence.
*/
static sony_result_t TER_tune_end(sony_helene2_t *pTuner, sony_helene2_tv_system_t tvSystem);
/**
 @brief The last part of satellite tuning sequence.
*/
static sony_result_t SAT_tune_end(sony_helene2_t *pTuner, sony_helene2_tv_system_t tvSystem);
/**
 @brief Configure the HELENE2 tuner from terrestrial to Power Save state.
*/
static sony_result_t TER_fin(sony_helene2_t *pTuner);
/**
 @brief Configure the HELENE2 tuner from satellite to Power Save state.
*/
static sony_result_t SAT_fin(sony_helene2_t *pTuner);
/**
 @brief Configure the HELENE2 tuner to Oscillation Stop state.
*/
static sony_result_t X_oscdis(sony_helene2_t *pTuner);
/**
 @brief Configure the HELENE2 tuner back from Oscillation Stop state.
*/
static sony_result_t X_oscen(sony_helene2_t *pTuner);
/**
 @brief Reading gain information to calculate IF and RF gain levels.
*/
static sony_result_t X_read_agc(sony_helene2_t *pTuner, uint8_t *pSatCompensationReg, uint8_t *pTerrCompensationReg,
    uint8_t *pIFAGCReg, uint8_t *pRFAGCReg);

/*------------------------------------------------------------------------------
 Definitions of internal used macros
------------------------------------------------------------------------------*/
/**
 @brief Checking that the internal loop filter can be used for the terrestrial broadcasting system.
*/
#define TERR_INTERNAL_LOOPFILTER_AVAILABLE(tvSystem) (SONY_HELENE2_IS_DTV(tvSystem)\
    && ((tvSystem) != SONY_HELENE2_DTV_DVBC_6) && ((tvSystem) != SONY_HELENE2_DTV_DVBC_8)\
    && ((tvSystem) != SONY_HELENE2_DTV_J83B_5_6))

/**
 @brief Macro to check that the system is DVB-T/T2 or not. (ATSC 3.0 uses DVB-T2 setting too)
*/
#define SONY_HELENE2_IS_DVB_T_T2(tvSystem) ((((tvSystem) >= SONY_HELENE2_DTV_DVBT_5) && ((tvSystem) <= SONY_HELENE2_DTV_DVBT2_8))\
    || (((tvSystem) >= SONY_HELENE2_DTV_ATSC3_6) && ((tvSystem) <= SONY_HELENE2_DTV_ATSC3_8)))

/*------------------------------------------------------------------------------
 Implementation of public functions.
------------------------------------------------------------------------------*/

sony_result_t sony_helene2_Create(sony_helene2_t *pTuner, sony_helene2_xtal_t xtalFreq,
    uint8_t i2cAddress, sony_i2c_t *pI2c, uint32_t flags)
{
    SONY_TRACE_ENTER("sony_helene2_Create");

    if((!pTuner) || (!pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* SONY_HELENE2_CONFIG_EXT_REF and SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL cannot be used at the same time. */
    if((flags & SONY_HELENE2_CONFIG_EXT_REF) && (flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    pTuner->state = SONY_HELENE2_STATE_UNKNOWN; /* Chip is not accessed for now. */
    pTuner->xtalFreq = xtalFreq;
    pTuner->pI2c = pI2c;
    pTuner->i2cAddress = i2cAddress;
    pTuner->flags = flags;
    pTuner->frequencykHz = 0;
    pTuner->symbolRateksps = 0;
    pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;
    pTuner->chipId = SONY_HELENE2_CHIP_ID_UNKNOWN;
    pTuner->isFreesatMode = 0;
    pTuner->pTerrParamTable = g_terr_param_table;

    /* Xtal OSC reference value */
    if(flags & SONY_HELENE2_CONFIG_EXT_REF){
        pTuner->xosc_sel = 0x00;
        pTuner->xosc_cap_set = 0x00;
    }else{
        switch(xtalFreq){
        case SONY_HELENE2_XTAL_16000KHz:
            pTuner->xosc_sel = 0x04;     /* 4 x 25 = 100uA */
            pTuner->xosc_cap_set = 0x30; /* 48 x 0.25 = 12pF(6pF Xtal) */
            break;
        case SONY_HELENE2_XTAL_24000KHz:
            pTuner->xosc_sel = 0x04;     /* 4 x 25 = 100uA */
            pTuner->xosc_cap_set = 0x1E; /* 30 x 0.25 = 7.5pF(6pF Xtal) */
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
    }

    pTuner->user = NULL;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_Initialize(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_Initialize");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    {
        uint8_t data = 0x00;

        /* Confirm connected device is HELENE2 */
        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7F, &data, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if((data & 0xFC) == 0x40){
            /* HELENE2 Plus */
            pTuner->chipId = SONY_HELENE2_CHIP_ID_2868;
        }else if((data & 0xFC) == 0x48){
            /* HELENE2 */
            pTuner->chipId = SONY_HELENE2_CHIP_ID_2866;
        }else{
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
    }

    /* X_pon sequence */
    result = X_pon(pTuner);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    /* Dummy tune to set RF parameter if terrestrial RF active mode is used */
    if((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK) == SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE){
        result = TER_tune(pTuner, 666000, SONY_HELENE2_DTV_DVBT_8, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        SONY_SLEEP(50);

        result = TER_tune_end(pTuner, SONY_HELENE2_DTV_DVBT_8);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
        /* Disable Xtal */
        result = X_oscdis(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    pTuner->state = SONY_HELENE2_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_terr_Tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_terr_Tune");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(!SONY_HELENE2_IS_ATV(tvSystem) && !SONY_HELENE2_IS_DTV(tvSystem)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Rough frequency range check */
    if((frequencykHz < 1000) || (frequencykHz > 1200000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_HELENE2_STATE_SLEEP:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;

        if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }

        break;

    case SONY_HELENE2_STATE_ACTIVE_S:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;

        result = SAT_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_HELENE2_STATE_ACTIVE_T:
        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Broadcasting system dependent setting and tuning. */
    result = TER_tune(pTuner, frequencykHz, tvSystem, 1);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTuner->state = SONY_HELENE2_STATE_ACTIVE_T;
    pTuner->frequencykHz = frequencykHz;
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_terr_TuneEnd(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_terr_TuneEnd");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check (terrestrial only) */
    if(pTuner->state != SONY_HELENE2_STATE_ACTIVE_T){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = TER_tune_end(pTuner, pTuner->tvSystem);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_sat_Tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem, uint32_t symbolRateksps)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_sat_Tune");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(!SONY_HELENE2_IS_STV(tvSystem)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Rough frequency range check */
    if((frequencykHz < 500000) || (frequencykHz > 3500000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    /* Symbol rate is fixed in ISDB-S */
    if(tvSystem == SONY_HELENE2_STV_ISDBS){
        symbolRateksps = 28860;
    }else if(tvSystem == SONY_HELENE2_STV_ISDBS3){
        symbolRateksps = 33756;
    }

    /* State check */
    switch(pTuner->state){
    case SONY_HELENE2_STATE_SLEEP:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;

        if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }

        break;

    case SONY_HELENE2_STATE_ACTIVE_T:
        /* Set system to "Unknown". (for safe) */
        pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;

        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_HELENE2_STATE_ACTIVE_S:
        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Broadcasting system dependent setting and tuning. */
    result = SAT_tune(pTuner, frequencykHz, tvSystem, symbolRateksps, 1);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_SLEEP(10);

    result = SAT_tune_end(pTuner, tvSystem);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTuner->state = SONY_HELENE2_STATE_ACTIVE_S;
    pTuner->frequencykHz = ((frequencykHz + 2) / 4) * 4;
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = symbolRateksps;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_ShiftFRF(sony_helene2_t *pTuner, uint32_t frequencykHz)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_ShiftFRF");

    /* Argument check */
    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_HELENE2_STATE_ACTIVE_T:
        /* Rough frequency range check */
        if((frequencykHz < 1000) || (frequencykHz > 1200000)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        result = TER_tune(pTuner, frequencykHz, pTuner->tvSystem, 0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        pTuner->frequencykHz = frequencykHz;

        SONY_SLEEP(10);

        result = TER_tune_end(pTuner, pTuner->tvSystem);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    case SONY_HELENE2_STATE_ACTIVE_S:
        /* Rough frequency range check */
        if((frequencykHz < 500000) || (frequencykHz > 3500000)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        result = SAT_tune(pTuner, frequencykHz, pTuner->tvSystem, pTuner->symbolRateksps, 0);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        pTuner->frequencykHz = frequencykHz;

        SONY_SLEEP(10);

        result = SAT_tune_end(pTuner, pTuner->tvSystem);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_Sleep(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_Sleep");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* State check */
    switch(pTuner->state){
    case SONY_HELENE2_STATE_SLEEP:
        /* Nothing to do */
        SONY_TRACE_RETURN(SONY_RESULT_OK);

    case SONY_HELENE2_STATE_ACTIVE_T:
        result = TER_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        break;

    case SONY_HELENE2_STATE_ACTIVE_S:
        result = SAT_fin(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        break;

    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
        /* Disable Xtal */
        result = X_oscdis(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    pTuner->state = SONY_HELENE2_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_SetGPO(sony_helene2_t *pTuner, uint8_t id, uint8_t val)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_SetGPO");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(id){
    case 0:
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x85, (uint8_t)(val ? 0x01 : 0x00));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    case 1:
        /* GPIO1_IN_SEL = 0 (0x86) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x86, (uint8_t)(val ? 0x01 : 0x00));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_GetGPI1(sony_helene2_t *pTuner, uint8_t *pVal)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER("sony_helene2_GetGPI1");

    if((!pTuner) || (!pTuner->pI2c) || (!pVal)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* GPIO1_IN_SEL = 1 (0x86) */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x86, 0x10, 0x10);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Read GPIO1_IN (0x4B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x4B, &data, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    *pVal = (uint8_t)(data & 0x01);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_SetRfExtCtrl(sony_helene2_t *pTuner, uint8_t enable)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_SetRfExtCtrl");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* RF_EXT bit setting (0x67) */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, (uint8_t)(enable ? 0x01 : 0x00), 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_SetFreesatMode(sony_helene2_t *pTuner, uint8_t enable)
{
    SONY_TRACE_ENTER("sony_helene2_SetFreesatMode");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->isFreesatMode = enable ? 1 : 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_terr_ReadRssi(sony_helene2_t *pTuner, int32_t *pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t ifagcreg = 0;
    uint8_t rfagcreg = 0;
    uint8_t sat_compensate_reg = 0;
    uint8_t terr_compensate_reg = 0;
    int32_t ifgain = 0;
    int32_t rfgain = 0;

    SONY_TRACE_ENTER("sony_helene2_terr_ReadRssi");

    if((!pTuner) || (!pTuner->pI2c) || (!pRssi)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Terrestrial only */
    if(pTuner->state != SONY_HELENE2_STATE_ACTIVE_T){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = X_read_agc(pTuner, &sat_compensate_reg, &terr_compensate_reg, &ifagcreg, &rfagcreg);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /*
        IFGAIN = if(AGC > 0.40){
                     8.2 + IF_BPF_GC
                 }else if(AGC > 0.30){
                     15.2 + IF_BPF_GC - 70 * (AGC - 0.3)
                 }else{
                     38.6 + IF_BPF_GC - 78 * AGC
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.4 / 255
        So...
        IFGAIN(100xdB) = if(IFAGCReg * 140 > 10200){
                             820 + (IF_BPF_GC * 100)
                         }else if(IFAGCReg * 140 > 7650){
                             1520 + (IF_BPF_GC * 100) - (700 * (IFAGCReg * 140 - 7650)) / 2550
                         }else{
                             3860 + (IF_BPF_GC * 100) - (780 * (IFAGCReg * 140)) / 2550
                         }
    */
    {
        const int32_t if_bpf_gc_table[] = {-6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 20};
        uint8_t data = 0;
        int32_t if_bpf_gc_x100 = 0;
        int32_t agcreg_x140 = ifagcreg * 140;

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x69, &data, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if_bpf_gc_x100 = if_bpf_gc_table[data & 0x0F] * 100;

        if(agcreg_x140 > 10200){
            ifgain = 820 + if_bpf_gc_x100;
        }else if(agcreg_x140 > 7650){
            ifgain = 1520 + if_bpf_gc_x100 - (700 * (agcreg_x140 - 7650) + 1275) / 2550; /* Round */
        }else{
            ifgain = 3860 + if_bpf_gc_x100 - (780 * agcreg_x140 + 1275) / 2550; /* Round */
        }
    }

    /*
        RFGAIN = if(max(AGC,RFAGC) < 0.15){
                     RF_GAIN_MAX
                 }else if(max(AGC,RFAGC) < 0.25){
                     RF_GAIN_MAX - 1.60 * (max(AGC,RFAGC) - 0.15)
                 }else if(max(AGC,RFAGC) < 0.30){
                     RF_GAIN_MAX - 0.16 - 13.4 * (max(AGC,RFAGC) - 0.25)
                 }else if(max(AGC,RFAGC) < 0.35){
                     RF_GAIN_MAX - 0.83 - 34.4 * (max(AGC,RFAGC) - 0.30)
                 }else if(max(AGC,RFAGC) < 0.40){
                     RF_GAIN_MAX - 2.58 - 54.21 * (max(AGC,RFAGC) - 0.35)
                 }else if(max(AGC,RFAGC) < 0.45){
                     RF_GAIN_MAX - 5.29 - 74.51 * (max(AGC,RFAGC) - 0.40)
                 }else if(max(AGC,RFAGC) < 0.60){
                     RF_GAIN_MAX - 9.02 - 82.53 * (max(AGC,RFAGC) - 0.45)
                 }else if(max(AGC,RFAGC) < 0.75){
                     RF_GAIN_MAX - 21.39 - 69.79 * (max(AGC,RFAGC) - 0.60)
                 }else if(max(AGC,RFAGC) < 0.95){
                     RF_GAIN_MAX - 31.86 - 74.68 * (max(AGC,RFAGC) - 0.75)
                 }else if(max(AGC,RFAGC) < 1.05){
                     RF_GAIN_MAX - 46.80 - 56.74 * (max(AGC,RFAGC) - 0.95)
                 }else if(max(AGC,RFAGC) < 1.15){
                     RF_GAIN_MAX - 52.47 - 145.92 * (max(AGC,RFAGC) - 1.05)
                 }else if(max(AGC,RFAGC) < 1.25){
                     RF_GAIN_MAX - 67.17 - 166.76 * (max(AGC,RFAGC) - 1.15)
                 }else{
                     RF_GAIN_MAX - 83.84
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.4 / 255
                  RFAGC(V) = RFAGCReg(by X_read_ss) * 1.4 / 255
        So...
        RFGAIN(100xdB) = if(maxagcreg * 140 < 3825){
                             RFGAIN_MAX * 100
                         }else if(maxagcreg * 140 < 6375){
                             RFGAIN_MAX * 100 - 1.60 * (maxagcreg * 140 - 3825)/255
                         }else if(maxagcreg * 140 < 7650){
                             RFGAIN_MAX * 100 - 16 - 13.40 * (maxagcreg * 140 - 6375)/255
                         }else if(maxagcreg * 140 < 8925){
                             RFGAIN_MAX * 100 - 83 - 34.40 * (maxagcreg * 140 - 7650)/255
                         }else if(maxagcreg * 140 < 10200){
                             RFGAIN_MAX * 100 - 258 - 54.21 * (maxagcreg * 140 - 8925)/255
                         }else if(maxagcreg * 140 < 11475){
                             RFGAIN_MAX * 100 - 529 - 74.51 * (maxagcreg * 140 - 10200)/255
                         }else if(maxagcreg * 140 < 15300){
                             RFGAIN_MAX * 100 - 902 - 82.53 * (maxagcreg * 140 - 11475)/255
                         }else if(maxagcreg * 140 < 19125){
                             RFGAIN_MAX * 100 - 2139 - 69.79 * (maxagcreg * 140 - 15300)/255
                         }else if(maxagcreg * 140 < 24225){
                             RFGAIN_MAX * 100 - 3186 - 74.68 * (maxagcreg * 140 - 19125)/255
                         }else if(maxagcreg * 140 < 26775){
                             RFGAIN_MAX * 100 - 4680 - 56.74 * (maxagcreg * 140 - 24225)/255
                         }else if(maxagcreg * 140 < 29325){
                             RFGAIN_MAX * 100 - 5247 - 145.92 * (maxagcreg * 140 - 26775)/255
                         }else if(maxagcreg * 140 < 31875){
                             RFGAIN_MAX * 100 - 6717 - 166.76 * (maxagcreg * 140 - 29325)/255
                         }else{
                             RFGAIN_MAX * 100 - 8384
                         }
        (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
    */
    {
        int32_t maxagcreg_x140 = 0;
        int32_t rfgainmax_x100 = 0;

        if(ifagcreg > rfagcreg){
            maxagcreg_x140 = ifagcreg * 140;
        }else{
            maxagcreg_x140 = rfagcreg * 140;
        }

        if(pTuner->frequencykHz < 55000){
            rfgainmax_x100 = 4690;
        }else if(pTuner->frequencykHz < 65000){
            rfgainmax_x100 = 4800;
        }else if(pTuner->frequencykHz < 86000){
            rfgainmax_x100 = 4920;
        }else if(pTuner->frequencykHz < 125000){
            rfgainmax_x100 = 4960;
        }else if(pTuner->frequencykHz < 142000){
            rfgainmax_x100 = 4890;
        }else if(pTuner->frequencykHz < 165000){
            rfgainmax_x100 = 4770;
        }else if(pTuner->frequencykHz < 172000){
            rfgainmax_x100 = 4610;
        }else if(pTuner->frequencykHz < 200000){
            rfgainmax_x100 = 4580;
        }else if(pTuner->frequencykHz < 225000){
            rfgainmax_x100 = 4680;
        }else if(pTuner->frequencykHz < 250000){
            rfgainmax_x100 = 4770;
        }else if(pTuner->frequencykHz < 320000){
            rfgainmax_x100 = 4840;
        }else if(pTuner->frequencykHz < 350000){
            rfgainmax_x100 = 4740;
        }else if(pTuner->frequencykHz < 400000){
            rfgainmax_x100 = 4750;
        }else if(pTuner->frequencykHz < 464000){
            rfgainmax_x100 = 4750;
        }else if(pTuner->frequencykHz < 532000){
            rfgainmax_x100 = 4450;
        }else if(pTuner->frequencykHz < 600000){
            rfgainmax_x100 = 4530;
        }else if(pTuner->frequencykHz < 664000){
            rfgainmax_x100 = 4580;
        }else if(pTuner->frequencykHz < 766000){
            rfgainmax_x100 = 4630;
        }else if(pTuner->frequencykHz < 868000){
            rfgainmax_x100 = 4630;
        }else if(pTuner->frequencykHz < 900000){
            rfgainmax_x100 = 4600;
        }else if(pTuner->frequencykHz < 950000){
            rfgainmax_x100 = 4480;
        }else{
            rfgainmax_x100 = 4300;
        }

        if(maxagcreg_x140 < 3825){
            rfgain = rfgainmax_x100;
        }else if(maxagcreg_x140 < 6375){
            rfgain = rfgainmax_x100 - (160 * (maxagcreg_x140 - 3825) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 7650){
            rfgain = rfgainmax_x100 - 16 - (1340 * (maxagcreg_x140 - 6375) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 8925){
            rfgain = rfgainmax_x100 - 83 - (3440 * (maxagcreg_x140 - 7650) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 10200){
            rfgain = rfgainmax_x100 - 258 - (5421 * (maxagcreg_x140 - 8925) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 11475){
            rfgain = rfgainmax_x100 - 529 - (7451 * (maxagcreg_x140 - 10200) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 15300){
            rfgain = rfgainmax_x100 - 902 - (8253 * (maxagcreg_x140 - 11475) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 19125){
            rfgain = rfgainmax_x100 - 2139 - (6979 * (maxagcreg_x140 - 15300) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 24225){
            rfgain = rfgainmax_x100 - 3186 - (7468 * (maxagcreg_x140 - 19125) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 26775){
            rfgain = rfgainmax_x100 - 4680 - (5674 * (maxagcreg_x140 - 24225) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 29325){
            rfgain = rfgainmax_x100 - 5247 - (14592 * (maxagcreg_x140 - 26775) + 12750) / 25500; /* Round */
        }else if(maxagcreg_x140 < 31875){
            rfgain = rfgainmax_x100 - 6717 - (16676 * (maxagcreg_x140 - 29325) + 12750) / 25500; /* Round */
        }else{
            rfgain = rfgainmax_x100 - 8384;
        }
    }

    *pRssi = - ifgain - rfgain - (sony_Convert2SComplement (terr_compensate_reg, 4) * 100);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_sat_ReadRssi(sony_helene2_t *pTuner, int32_t *pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t ifagcreg = 0;
    uint8_t rfagcreg = 0;
    uint8_t sat_compensate_reg = 0;
    uint8_t terr_compensate_reg = 0;
    int32_t ifgain = 0;
    int32_t rfgain = 0;

    SONY_TRACE_ENTER("sony_helene2_sat_ReadRssi");

    if((!pTuner) || (!pTuner->pI2c) || (!pRssi)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Satellite only */
    if(pTuner->state != SONY_HELENE2_STATE_ACTIVE_S){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = X_read_agc(pTuner, &sat_compensate_reg, &terr_compensate_reg, &ifagcreg, &rfagcreg);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /*
        IFGAIN = if(AGC > 0.40){
                     15.6
                 }else if(AGC > 0.30){
                     22.9 - 72.7 * (AGC - 0.3)
                 }else if(AGC > 0.10){
                     37.7 - 74.2 * (AGC - 0.1)
                 }else{
                     45.7 - 80.6 * AGC
                 }
        Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.4 / 255
        So...
        IFGAIN(100xdB) = if(IFAGCReg * 140 > 10200){
                             1560
                         }else if(IFAGCReg * 140 > 7650){
                             2290 - (727 * (IFAGCReg * 140 - 7650)) / 2550
                         }else if(IFAGCReg * 140 > 2550){
                             3770 - (742 * (IFAGCReg * 140 - 2550)) / 2550
                         }else{
                             4570 - (806 * (IFAGCReg * 140)) / 2550
                         }
    */
    {
        int32_t agcreg_x140 = ifagcreg * 140;

        if(agcreg_x140 > 10200){
            ifgain = 1560;
        }else if(agcreg_x140 > 7650){
            ifgain = 2290 - (727 * (agcreg_x140 - 7650) + 1275) / 2550; /* Round */
        }else if(agcreg_x140 > 2550){
            ifgain = 3770 - (742 * (agcreg_x140 - 2550) + 1275) / 2550; /* Round */
        }else{
            ifgain = 4570 - (806 * (agcreg_x140) + 1275) / 2550; /* Round */
        }
    }

    if((pTuner->chipId == SONY_HELENE2_CHIP_ID_2868) && (pTuner->frequencykHz > 2150000) && (pTuner->tvSystem == SONY_HELENE2_STV_ISDBS3)){
        /*
            RFGAIN = if(AGC < 0.20){
                         RF_GAIN_MAX - 0.15 * AGC
                     }else if(AGC < 0.25){
                         RF_GAIN_MAX - 0.03 - 35.0 * (AGC - 0.20)
                     }else if(AGC < 0.30){
                         RF_GAIN_MAX - 1.78 - 70.8 * (AGC - 0.25)
                     }else if(AGC < 0.38){
                         RF_GAIN_MAX - 5.32 - 101.3 * (AGC - 0.30)
                     }else if(AGC < 0.45){
                         RF_GAIN_MAX - 13.42 - 80.0 * (AGC - 0.38)
                     }else if(AGC < 0.50){
                         RF_GAIN_MAX - 19.02 - 116.2 * (AGC - 0.45)
                     }else if(AGC < 0.55){
                         RF_GAIN_MAX - 24.38 - 148.22 * (AGC - 0.50)
                     }else if(AGC < 0.60){
                         RF_GAIN_MAX - 32.24 - 84.82 * (AGC - 0.55)
                     }else if(AGC < 0.65){
                         RF_GAIN_MAX - 36.48 - 109.5 * (AGC - 0.60)
                     }else if(AGC < 0.75){
                         RF_GAIN_MAX - 41.96 - 71.60 * (AGC - 0.65)
                     }else if(AGC < 0.82){
                         RF_GAIN_MAX - 49.12 - 44.72 * (AGC - 0.75)
                     }else if(AGC < 0.87){
                         RF_GAIN_MAX - 52.25 - 71.82 * (AGC - 0.82)
                     }else if(AGC < 1.10){
                         RF_GAIN_MAX - 55.84 - 99.00 * (AGC - 0.87)
                     }else{
                         RF_GAIN_MAX - 78.61
                     }
            Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.4 / 255
                      RFAGC(V) = RFAGCReg(by X_read_ss) * 1.4 / 255
            So...
            RFGAIN(100xdB) = if(maxagcreg * 140 < 5100){
                                 RFGAIN_MAX * 100 - 0.15 * (maxagcreg * 140)/255
                             }else if(maxagcreg * 140 < 6375){
                                 RFGAIN_MAX * 100 - 3 - 35.00 * (maxagcreg * 140 - 5100)/255
                             }else if(maxagcreg * 140 < 7650){
                                 RFGAIN_MAX * 100 - 178 - 70.80 * (maxagcreg * 140 - 6375)/255
                             }else if(maxagcreg * 140 < 9690){
                                 RFGAIN_MAX * 100 - 532 - 101.30 * (maxagcreg * 140 - 7650)/255
                             }else if(maxagcreg * 140 < 11475){
                                 RFGAIN_MAX * 100 - 1342 - 80.00 * (maxagcreg * 140 - 9690)/255
                             }else if(maxagcreg * 140 < 12750){
                                 RFGAIN_MAX * 100 - 1902 - 116.20 * (maxagcreg * 140 - 11475)/255
                             }else if(maxagcreg * 140 < 14025){
                                 RFGAIN_MAX * 100 - 2438 - 148.22 * (maxagcreg * 140 - 12750)/255
                             }else if(maxagcreg * 140 < 15300){
                                 RFGAIN_MAX * 100 - 3224 - 84.82 * (maxagcreg * 140 - 14025)/255
                             }else if(maxagcreg * 140 < 16575){
                                 RFGAIN_MAX * 100 - 3648 - 109.50 * (maxagcreg * 140 - 15300)/255
                             }else if(maxagcreg * 140 < 19125){
                                 RFGAIN_MAX * 100 - 4196 - 71.60 * (maxagcreg * 140 - 16575)/255
                             }else if(maxagcreg * 140 < 20910){
                                 RFGAIN_MAX * 100 - 4912 - 44.72 * (maxagcreg * 140 - 19125)/255
                             }else if(maxagcreg * 140 < 22185){
                                 RFGAIN_MAX * 100 - 5225 - 71.82 * (maxagcreg * 140 - 20910)/255
                             }else if(maxagcreg * 140 < 28050){
                                 RFGAIN_MAX * 100 - 5584 - 99.00 * (maxagcreg * 140 - 22185)/255
                             }else{
                                 RFGAIN_MAX * 100 - 7861
                             }
            (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
        */
        {
            int32_t maxagcreg_x140 = 0;
            int32_t rfgainmax_x100 = 0;

            if(ifagcreg > rfagcreg){
                maxagcreg_x140 = ifagcreg * 140;
            }else{
                maxagcreg_x140 = rfagcreg * 140;
            }

            if(pTuner->frequencykHz < 2200000){
                rfgainmax_x100 = 5230;
            }else if(pTuner->frequencykHz < 2430000){
                rfgainmax_x100 = 5230;
            }else if(pTuner->frequencykHz < 2650000){
                rfgainmax_x100 = 5160;
            }else if(pTuner->frequencykHz < 2750000){
                rfgainmax_x100 = 5130;
            }else if(pTuner->frequencykHz < 2850000){
                rfgainmax_x100 = 5090;
            }else if(pTuner->frequencykHz < 2950000){
                rfgainmax_x100 = 5020;
            }else if(pTuner->frequencykHz < 3050000){
                rfgainmax_x100 = 4990;
            }else if(pTuner->frequencykHz < 3200000){
                rfgainmax_x100 = 4960;
            }else if(pTuner->frequencykHz < 3250000){
                rfgainmax_x100 = 4930;
            }else if(pTuner->frequencykHz < 3300000){
                rfgainmax_x100 = 4900;
            }else{
                rfgainmax_x100 = 4900;
            }

            if(maxagcreg_x140 < 5100){
                rfgain = rfgainmax_x100 - (15 * (maxagcreg_x140) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 6375){
                rfgain = rfgainmax_x100 - 3 - (3500 * (maxagcreg_x140 - 5100) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 7650){
                rfgain = rfgainmax_x100 - 178 - (7080 * (maxagcreg_x140 - 6375) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 9690){
                rfgain = rfgainmax_x100 - 532 - (10130 * (maxagcreg_x140 - 7650) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 11475){
                rfgain = rfgainmax_x100 - 1342 - (8000 * (maxagcreg_x140 - 9690) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 12750){
                rfgain = rfgainmax_x100 - 1902 - (11620 * (maxagcreg_x140 - 11475) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 14025){
                rfgain = rfgainmax_x100 - 2438 - (14822 * (maxagcreg_x140 - 12750) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 15300){
                rfgain = rfgainmax_x100 - 3224 - (8482 * (maxagcreg_x140 - 14025) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 16575){
                rfgain = rfgainmax_x100 - 3648 - (10950 * (maxagcreg_x140 - 15300) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 19125){
                rfgain = rfgainmax_x100 - 4196 - (7160 * (maxagcreg_x140 - 16575) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 20910){
                rfgain = rfgainmax_x100 - 4912 - (4472 * (maxagcreg_x140 - 19125) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 22185){
                rfgain = rfgainmax_x100 - 5225 - (7182 * (maxagcreg_x140 - 20910) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 28050){
                rfgain = rfgainmax_x100 - 5584 - (9900 * (maxagcreg_x140 - 22185) + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 7861;
            }
        }
    }else{
        /*
            RFGAIN = if(AGC < 0.1){
                         RF_GAIN_MAX - 1.9 * AGC
                     }else if(AGC < 0.2){
                         RF_GAIN_MAX - 0.19 - 9.1 * (AGC - 0.1)
                     }else if(AGC < 0.25){
                         RF_GAIN_MAX - 1.1 - 53 * (AGC - 0.2)
                     }else if(AGC < 0.3){
                         RF_GAIN_MAX - 3.75 - 94.8 * (AGC - 0.25)
                     }else if(AGC < 0.35){
                         RF_GAIN_MAX - 8.49 - 120.4 * (AGC - 0.3)
                     }else if(AGC < 0.4){
                         RF_GAIN_MAX - 14.51 - 97 * (AGC - 0.35)
                     }else if(AGC < 0.45){
                         RF_GAIN_MAX - 19.36 - 90 * (AGC - 0.4)
                     }else if(AGC < 0.5){
                         RF_GAIN_MAX - 23.86 - 119.62 * (AGC - 0.45)
                     }else if(AGC < 0.55){
                         RF_GAIN_MAX - 29.86 - 101.88 * (AGC - 0.5)
                     }else if(AGC < 0.8){
                         RF_GAIN_MAX - 34.96 - 99.48 * (AGC - 0.55)
                     }else if(AGC < 0.85){
                         RF_GAIN_MAX - 59.83 - 150.07 * (AGC - 0.8)
                     }else if(AGC < 0.95){
                         RF_GAIN_MAX - 67.64 - 96.6 * (AGC - 0.85)
                     }else{
                         RF_GAIN_MAX - 77.3
                     }
            Note that AGC(V) = IFAGCReg(by X_read_agc) * 1.4 / 255
                      RFAGC(V) = RFAGCReg(by X_read_ss) * 1.4 / 255
            So...
            RFGAIN(100xdB) = if(maxagcreg * 140 < 2550){
                                 RFGAIN_MAX * 100- 190 * (maxagcreg * 140)/255
                             }else if(maxagcreg * 140 < 5100){
                                 RFGAIN_MAX * 100 - 19 - 9.10 * (maxagcreg * 140 - 2550)/255
                             }else if(maxagcreg * 140 < 6375){
                                 RFGAIN_MAX * 100 - 110 - 53.00 * (maxagcreg * 140 - 5100)/255
                             }else if(maxagcreg * 140 < 7650){
                                 RFGAIN_MAX * 100 - 375 - 94.80 * (maxagcreg * 140 - 6375)/255
                             }else if(maxagcreg * 140 < 8925){
                                 RFGAIN_MAX * 100 - 849 - 120.40 * (maxagcreg * 140 - 7650)/255
                             }else if(maxagcreg * 140 < 10200){
                                 RFGAIN_MAX * 100 - 1451 - 97.00 * (maxagcreg * 140 - 8925)/255
                             }else if(maxagcreg * 140 < 11475){
                                 RFGAIN_MAX * 100 - 1936 - 90.00 * (maxagcreg * 140 - 10200)/255
                             }else if(maxagcreg * 140 < 12750){
                                 RFGAIN_MAX * 100 - 2386 - 119.62 * (maxagcreg * 140 - 11475)/255
                             }else if(maxagcreg * 140 < 14025){
                                 RFGAIN_MAX * 100 - 2986 - 101.88 * (maxagcreg * 140 - 12750)/255
                             }else if(maxagcreg * 140 < 20400){
                                 RFGAIN_MAX * 100 - 3496 - 99.48 * (maxagcreg * 140 - 14025)/255
                             }else if(maxagcreg * 140 < 21675){
                                 RFGAIN_MAX * 100 - 5983 - 150.07 * (maxagcreg * 140 - 20400)/255
                             }else if(maxagcreg * 140 < 24225){
                                 RFGAIN_MAX * 100 - 6764 - 96.60 * (maxagcreg * 140 - 21675)/255
                             }else{
                                 RFGAIN_MAX * 100 - 7730
                             }
            (NOTE: maxagcreg = max(IFAGCReg, RFAGCReg))
        */
        {
            int32_t maxagcreg_x140 = 0;
            int32_t rfgainmax_x100 = 0;

            if(ifagcreg > rfagcreg){
                maxagcreg_x140 = ifagcreg * 140;
            }else{
                maxagcreg_x140 = rfagcreg * 140;
            }

            if(pTuner->frequencykHz < 950000){
                rfgainmax_x100 = 4450;
            }else if(pTuner->frequencykHz < 1000000){
                rfgainmax_x100 = 4430;
            }else if(pTuner->frequencykHz < 1100000){
                rfgainmax_x100 = 4460;
            }else if(pTuner->frequencykHz < 1300000){
                rfgainmax_x100 = 4490;
            }else if(pTuner->frequencykHz < 1500000){
                rfgainmax_x100 = 4520;
            }else if(pTuner->frequencykHz < 1600000){
                rfgainmax_x100 = 4500;
            }else if(pTuner->frequencykHz < 1700000){
                rfgainmax_x100 = 4480;
            }else if(pTuner->frequencykHz < 1800000){
                rfgainmax_x100 = 4440;
            }else if(pTuner->frequencykHz < 1900000){
                rfgainmax_x100 = 4390;
            }else if(pTuner->frequencykHz < 1950000){
                rfgainmax_x100 = 4360;
            }else if(pTuner->frequencykHz < 2000000){
                rfgainmax_x100 = 4320;
            }else if(pTuner->frequencykHz < 2050000){
                rfgainmax_x100 = 4270;
            }else if(pTuner->frequencykHz < 2100000){
                rfgainmax_x100 = 4220;
            }else if(pTuner->frequencykHz < 2150000){
                rfgainmax_x100 = 4160;
            }else if(pTuner->frequencykHz < 2200000){
                rfgainmax_x100 = 4110;
            }

            if(maxagcreg_x140 < 2550){
                rfgain = rfgainmax_x100 - (190 * (maxagcreg_x140) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 5100){
                rfgain = rfgainmax_x100 - 19 - (910 * (maxagcreg_x140 - 2550) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 6375){
                rfgain = rfgainmax_x100 - 110 - (5300 * (maxagcreg_x140 - 5100) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 7650){
                rfgain = rfgainmax_x100 - 375 - (9480 * (maxagcreg_x140 - 6375) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 8925){
                rfgain = rfgainmax_x100 - 849 - (12040 * (maxagcreg_x140 - 7650) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 10200){
                rfgain = rfgainmax_x100 - 1451 - (9700 * (maxagcreg_x140 - 8925) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 11475){
                rfgain = rfgainmax_x100 - 1936 - (9000 * (maxagcreg_x140 - 10200) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 12750){
                rfgain = rfgainmax_x100 - 2386 - (11962 * (maxagcreg_x140 - 11475) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 14025){
                rfgain = rfgainmax_x100 - 2986 - (10188 * (maxagcreg_x140 - 12750) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 20400){
                rfgain = rfgainmax_x100 - 3496 - (9948 * (maxagcreg_x140 - 14025) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 21675){
                rfgain = rfgainmax_x100 - 5983 - (15007 * (maxagcreg_x140 - 20400) + 12750) / 25500; /* Round */
            }else if(maxagcreg_x140 < 24225){
                rfgain = rfgainmax_x100 - 6764 - (9660 * (maxagcreg_x140 - 21675) + 12750) / 25500; /* Round */
            }else{
                rfgain = rfgainmax_x100 - 7730;
            }
        }
    }

    *pRssi = - ifgain - rfgain - (sony_Convert2SComplement (sat_compensate_reg, 4) * 100);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_helene2_RFFilterConfig(sony_helene2_t *pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_helene2_RFFilterConfig");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if((pTuner->state != SONY_HELENE2_STATE_SLEEP) && (pTuner->state != SONY_HELENE2_STATE_ACTIVE_T)
        && (pTuner->state != SONY_HELENE2_STATE_ACTIVE_S)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    if(pTuner->state == SONY_HELENE2_STATE_SLEEP){
        if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
            /* Enable Xtal */
            result = X_oscen(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }
    }

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0xC4, 0x40};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    {
        uint8_t data[3];

        /* Write multiplier */
        data[0] = coeff;
        data[1] = 0x49;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);

        /* Write offset */
        data[0] = offset;
        data[1] = 0x4B;
        data[2] = 0x03;
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x16, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        SONY_SLEEP(1);
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(pTuner->state == SONY_HELENE2_STATE_SLEEP){
        if((pTuner->flags & SONY_HELENE2_CONFIG_SLEEP_DISABLEXTAL) && !(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF)){
            /* Disable Xtal */
            result = X_oscdis(pTuner);
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Implementation of static functions
------------------------------------------------------------------------------*/

static sony_result_t X_pon(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_pon");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (terrestrial) (0x67) */
    /* RFIN matching in power save (satellite) (0x43, 0x45) */
    /* Power save setting for analog block (0x5E, 0x5F, 0x60) */
    /* Power save setting for analog block (0x0C, 0x0D) */
    /* Power save setting for analog block (0x0E is the mirror of 0x60) */
    {
        uint8_t dataT[4];  /* 0x5E - 0x60, 0x67 */
        uint8_t dataS[2];  /* 0x0C, 0x43 */
        uint8_t dataSH[2]; /* 0x0D, 0x45 */

        switch(pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK){
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RFIN_MATCHING_DISABLE:
            dataT[0] = 0x15;
            dataT[1] = 0x00;
            dataT[2] = 0x00;
            dataT[3] = 0x00;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RFIN_MATCHING_ENABLE:
            dataT[0] = 0x15;
            dataT[1] = 0x00;
            dataT[2] = 0x00;
            dataT[3] = 0x02;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE:
            dataT[0] = 0x06;
            dataT[1] = 0x00;
            dataT[2] = 0x02;
            dataT[3] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        {
            uint8_t lnaOff = (pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF) ? 1 : 0;
            uint8_t terrRfActive = ((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK)
                == SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE) ? 1 : 0;

            switch(pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_SAT_MASK){
            case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RFIN_MATCHING_DISABLE: /* Mode 1 */
                dataS[0] = terrRfActive ? 0x04 : 0x14;     /* 0x0C */
                dataS[1] = lnaOff       ? 0x05 : 0x07;     /* 0x43 */
                dataSH[0] = 0x0;                           /* 0x0D */
                dataSH[1] = lnaOff      ? 0x01 : 0x03;     /* 0x45 */
                break;
            case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RFIN_MATCHING_ENABLE: /* Mode 0 */
                dataS[0] = terrRfActive ? 0x04 : 0x14;
                dataS[1] = lnaOff       ? 0x04 : 0x06;
                dataSH[0] = 0x0;
                dataSH[1] = lnaOff      ? 0x00 : 0x02;
                break;
            case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RF_ACTIVE:
                if(lnaOff){ /* Mode 3 */
                    dataS[0] = terrRfActive ? 0x06 : 0x16;
                    dataS[1] = 0x04;
                    dataSH[0] = 0x20;
                    dataSH[1] = 0x00;
                }else{ /* Mode 2 */
                    dataS[0] = terrRfActive ? 0x05 : 0x15;
                    dataS[1] = 0x06;
                    dataSH[0] = 0x10;
                    dataSH[1] = 0x02;
                }
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x67, dataT[3]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43, dataS[1]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x45, dataSH[1]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, &dataT[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, dataS[0]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0D, dataSH[0]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x99, 0x9A */
    {
        /* Initial setting for internal logic block */
        const uint8_t cdata[] = {0x7A, 0x01};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x99, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x81 - 0x94 */
    {
        uint8_t data[20];

        /* Frequency setting for crystal oscillator (0x81) */
        switch(pTuner->xtalFreq){
        case SONY_HELENE2_XTAL_16000KHz:
            data[0] = 0x10;
            break;
        case SONY_HELENE2_XTAL_24000KHz:
            data[0] = 0x18;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Driver current setting for crystal oscillator (0x82) */
        /* Load capacitance setting for crystal oscillator (0x83) */
        if(pTuner->flags & SONY_HELENE2_CONFIG_EXT_REF){
            /* XOSC_APC_EN = 0, XOSC_SEL= 0uA */
            data[1] = 0x00;
            /* XOSC_CALC_EN = 0, XOSC_CAP_SET = 0pF */
            data[2] = 0x00;
        }else{
            /* XOSC_APC_EN = 1, XOSC_SEL = xosc_sel (sony_helene2_t member) */
            data[1] = (uint8_t)(0x80 | (pTuner->xosc_sel & 0x1F));
            /* XOSC_CALC_EN = 1, XOSC_CAP_SET = xosc_cap_set (sony_helene2_t member) */
            data[2] = (uint8_t)(0x80 | (pTuner->xosc_cap_set & 0x7F));
        }

        /* Setting for REFOUT signal output (0x84) */
        switch(pTuner->flags & SONY_HELENE2_CONFIG_REFOUT_MASK){
        case 0:
            data[3] = 0x00; /* REFOUT_EN = 0, REFOUT_CNT = 0 */
            break;
        case SONY_HELENE2_CONFIG_REFOUT_500mVpp:
            data[3] = 0x80; /* REFOUT_EN = 1, REFOUT_CNT = 0 */
            break;
        case SONY_HELENE2_CONFIG_REFOUT_400mVpp:
            data[3] = 0x81; /* REFOUT_EN = 1, REFOUT_CNT = 1 */
            break;
        case SONY_HELENE2_CONFIG_REFOUT_600mVpp:
            data[3] = 0x82; /* REFOUT_EN = 1, REFOUT_CNT = 2 */
            break;
        case SONY_HELENE2_CONFIG_REFOUT_800mVpp:
            data[3] = 0x83; /* REFOUT_EN = 1, REFOUT_CNT = 3 */
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* GPIO0, GPIO1 port setting (0x85, 0x86) */
        /* GPIO setting should be done by sony_helene2_SetGPO/sony_helene2_GetGPI1 after initialization */
        data[4] = 0x00;
        data[5] = 0x00;

        /* Clock enable for internal logic block (0x87) */
        data[6] = 0xC4;

        /* Start CPU boot-up (0x88) */
        data[7] = 0x40;

        /* For burst-write (0x89) */
        data[8] = 0x10;

        /* Setting for internal RFAGC (0x8A, 0x8B, 0x8C) */
        data[9] = 0x00;
        data[10] = 0x45;
        data[11] = 0x75;

        /* Setting for analog block (0x8D) */
        data[12] = 0x07;

        /* Initial setting for internal analog block (0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94) */
        data[13] = 0x08;
        data[14] = 0x00;
        data[15] = 0x00;
        data[16] = 0x10;
        data[17] = 0x20;
        data[18] = 0x0A;
        data[19] = 0x00;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x81, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for internal RFAGC (0x9B) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9B, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Wait 10ms */
    SONY_SLEEP(10);

    /* Check CPU_STT (0x1A) */
    {
        uint8_t rdata;

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1A, &rdata, sizeof(rdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(rdata != 0x00){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE); /* CPU_STT != 0x00 */
        }
    }

    /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Load capacitance control setting for crystal oscillator (0x80) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x80, 0x01);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Satellite initial setting (0x42) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x42, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Satellite IQ OUTPUT */
    if(pTuner->flags & SONY_HELENE2_CONFIG_IQ_OUTPUT_BALANCED){
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x46, 0x11);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }else{
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x46, 0x00);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Satellite initial setting (0x7B, 0x7C) */
    {
        const uint8_t cdata[] = {0x02, 0x01};

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7B, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem, uint8_t vcoCal)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Mode select (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(vcoCal){
        /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x85, 0x86 */
    /* GPIO0, GPIO1 is changed by sony_helene2_SetGPO/sony_helene2_GetGPI1 */

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0xC4, 0x40};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Setting for internal analog block (0x79, 0x7D, 0x8D) */
    {
        uint8_t data[3];

        data[0] = 0xA1;
        data[1] = 0x00;

        if(SONY_HELENE2_IS_DVB_T_T2(tvSystem)){
            data[2] = 0x05;
        }else{
            data[2] = 0x00;
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x79, data[0]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x7D, data[1]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x8D, data[2]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x91, 0x92 */
    {
        uint8_t data[2];

        /* Initial setting for internal analog block (0x91, 0x92) */
        if(SONY_HELENE2_IS_DVB_T_T2(tvSystem)){
            data[0] = 0x00;
            data[1] = 0x02;
        }else{
            data[0] = 0x0A;
            data[1] = 0x0F;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x91, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x9C */
    {
        uint8_t data;
        /* Setting for analog block (0x9C) */
        if((pTuner->flags & SONY_HELENE2_CONFIG_LOOPFILTER_INTERNAL) && TERR_INTERNAL_LOOPFILTER_AVAILABLE(tvSystem)){
            data = 0x90;
        }else{
            data = 0x00;
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x9C, data);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* 0x5E - 0x66 */
    {
        uint8_t data[9];

        /* Enable for analog block (0x5E, 0x5F, 0x60) */
        data[0] = 0xEE;
        data[1] = 0x02;
        data[2] = 0x9E;

        /* Tuning setting for CPU (0x61) */
        if(vcoCal){
            data[3] = 0x67;
        }else{
            data[3] = 0x45;
        }

        /* Setting for PLL reference divider (REF_R) (0x62) */
        if(SONY_HELENE2_IS_ATV(tvSystem)){
            /* Analog (Fref = 2MHz (frequency > 464MHz) | Fref = 0.5MHz (frequency <= 464MHz)) */
            switch(pTuner->xtalFreq){
            case SONY_HELENE2_XTAL_16000KHz:
                data[4] = (uint8_t)((frequencykHz > 464000) ? 0x08 : 0x20);
                break;
            case SONY_HELENE2_XTAL_24000KHz:
                data[4] = (uint8_t)((frequencykHz > 464000) ? 0x0C : 0x30);
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }else if((tvSystem == SONY_HELENE2_DTV_DVBC_6) || (tvSystem == SONY_HELENE2_DTV_DVBC_8) || (tvSystem == SONY_HELENE2_DTV_J83B_5_6)){
            /* DVB-C (Fref = 1MHz) */
            switch(pTuner->xtalFreq){
            case SONY_HELENE2_XTAL_16000KHz:
                data[4] = 0x10;
                break;
            case SONY_HELENE2_XTAL_24000KHz:
                data[4] = 0x18;
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }else{
            /* Digital (Fref = 8MHz) */
            switch(pTuner->xtalFreq){
            case SONY_HELENE2_XTAL_16000KHz:
                data[4] = 0x02;
                break;
            case SONY_HELENE2_XTAL_24000KHz:
                data[4] = 0x03;
                break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }

        /* Tuning setting for analog block (0x63, 0x64, 0x65, 0x66) */
        if((pTuner->flags & SONY_HELENE2_CONFIG_LOOPFILTER_INTERNAL) && TERR_INTERNAL_LOOPFILTER_AVAILABLE(tvSystem)){
            data[5] = 0x38;
            data[6] = 0x1E;
            data[7] = 0x02;
            data[8] = 0x24;
        }else if(SONY_HELENE2_IS_ATV(tvSystem)){
            data[5] = (uint8_t)((frequencykHz > 464000) ? 0xB4 : 0x38);
            data[6] = 0x78;
            data[7] = 0x08;
            data[8] = (uint8_t)((frequencykHz > 464000) ? 0x3C : 0x1C);
        }else if((tvSystem == SONY_HELENE2_DTV_DVBC_6) || (tvSystem == SONY_HELENE2_DTV_DVBC_8) || (tvSystem == SONY_HELENE2_DTV_J83B_5_6)){
            data[5] = 0x1C;
            data[6] = 0x78;
            data[7] = 0x08;
            data[8] = 0x1C;
        }else{
            data[5] = 0xB4;
            data[6] = 0x78;
            data[7] = 0x08;
            data[8] = 0x30;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* LT_AMP_EN should be 0 (0x67) */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, 0x00, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x68 - 0x78 */
    {
        uint8_t data[17];

        /* Setting for IFOUT_LIMIT (0x68) */
        if(SONY_HELENE2_IS_ATV(tvSystem)){
            data[0] = (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_OUTLMT_ATV_1_2Vpp) ? 0x01 : 0x00);
        }else if(SONY_HELENE2_IS_DTV(tvSystem)){
            data[0] = (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_OUTLMT_DTV_1_2Vpp) ? 0x01 : 0x00);
        }else{
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
        }

        /* Setting for IF BPF buffer gain (0x69) */
        /* RF_GAIN setting */
        if(pTuner->pTerrParamTable[tvSystem].RF_GAIN == AUTO){
            data[1] = 0x80; /* RF_GAIN_SEL = 1 */
        }else{
            data[1] = (uint8_t)((pTuner->pTerrParamTable[tvSystem].RF_GAIN << 4) & 0x70);
        }

        /* IF_BPF_GC setting */
        data[1] |= (uint8_t)(pTuner->pTerrParamTable[tvSystem].IF_BPF_GC & 0x0F);

        /* Setting for internal RFAGC (0x6A, 0x6B, 0x6C) */
        data[2] = 0x00;
        if(frequencykHz <= 172000){
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_VL & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_VL & 0x07);
        }else if(frequencykHz <= 464000){
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_VH & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_VH & 0x07);
        }else{
            data[3] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].RFOVLD_DET_LV1_U & 0x0F);
            data[4] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].IFOVLD_DET_LV_U & 0x07);
        }
        data[4] |= 0x30;

        /* Setting for IF frequency and bandwidth */

        /* IF filter center frequency offset (IF_BPF_F0) (0x6D) */
        data[5] = (uint8_t)((pTuner->pTerrParamTable[tvSystem].IF_BPF_F0 << 4) & 0x30);

        /* IF filter band width (BW) (0x6D) */
        data[5] |= (uint8_t)(pTuner->pTerrParamTable[tvSystem].BW & 0x03);

        /* IF frequency offset value (FIF_OFFSET) (0x6E) */
        data[6] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].FIF_OFFSET & 0x1F);

        /* IF band width offset value (BW_OFFSET) (0x6F) */
        data[7] = (uint8_t)(pTuner->pTerrParamTable[tvSystem].BW_OFFSET & 0x1F);

        /* RF tuning frequency setting (0x70, 0x71, 0x72) */
        data[8]  = (uint8_t)(frequencykHz & 0xFF);         /* FRF_L */
        data[9]  = (uint8_t)((frequencykHz >> 8) & 0xFF);  /* FRF_M */
        data[10] = (uint8_t)((frequencykHz >> 16) & 0x0F); /* FRF_H (bit[3:0]) */

        if(SONY_HELENE2_IS_DTV(tvSystem) && pTuner->pTerrParamTable[tvSystem].IS_LOWERLOCAL){
            data[10] |= 0x20; /* IS_LOWERLOCAL (bit[5]) */
        }

        if(tvSystem == SONY_HELENE2_ATV_L_DASH){
            data[10] |= 0x40; /* IS_L_DASH (bit[6]) */
        }

        if(SONY_HELENE2_IS_ATV(tvSystem)){
            data[10] |= 0x80; /* IS_FP (bit[7]) */
        }

        /* Tuning command (0x73) */
        if(vcoCal){
            data[11] = 0xFF;
        }else{
            data[11] = 0x8F;
        }

        /* Enable IF output, AGC and IFOUT pin selection (0x74) */
        {
            data[12] = 0x01;

            if(pTuner->pTerrParamTable[tvSystem].AGC_SEL == AUTO){
                /* AGC pin setting by config flags */
                if(SONY_HELENE2_IS_ATV(tvSystem)){
                    /* Analog */
                    if(pTuner->flags & SONY_HELENE2_CONFIG_TAGC2_ATV){
                        data[12] |= 0x10;
                    }
                }else{
                    /* Digital */
                    if(pTuner->flags & SONY_HELENE2_CONFIG_TAGC2_DTV){
                        data[12] |= 0x10;
                    }
                }
            }else{
                /* AGC pin setting from parameter table */
                data[12] |= (uint8_t)((pTuner->pTerrParamTable[tvSystem].AGC_SEL << 4) & 0x30);
            }
        }

        /* Tuning setting for analog block (0x75, 0x76, 0x77, 0x78) */
        data[13] = 0xF9;
        data[14] = 0x0F;
        data[15] = 0x05;
        if(SONY_HELENE2_IS_ATV(tvSystem)){
            data[16] = 0x14;
        }else{
            data[16] = 0x44;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x68, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_tune_end(sony_helene2_t *pTuner, sony_helene2_tv_system_t tvSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_tune_end");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_tune(sony_helene2_t *pTuner, uint32_t frequencykHz,
    sony_helene2_tv_system_t tvSystem, uint32_t symbolRateksps, uint8_t vcoCal)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if(vcoCal){
        /* Disable IF signal output (IF_OUT_SEL setting) (0x15) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Analog block setting (0x6A, 0x6B) */
    {
        const uint8_t cdata[2] = {0x00, 0x00};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x6A, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Analog block setting (0x75) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x75, 0xF9);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Tuning setting for CPU (0x40) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x40, (uint8_t)(vcoCal ? 0x07 : 0x05));
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for analog block (0x41) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x41, 0x07);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for analog block (0x48) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x48, 0x07);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x02, 0x03 */
    /* GPIO0, GPIO1 is changed by sony_helene2_SetGPO/sony_helene2_GetGPI1 */

    /* 0x04 - 0x0B */
    {
        uint8_t data[8];

        /* Clock enable for internal logic block, CPU wake-up (0x04, 0x05) */
        data[0] = 0xC4;
        data[1] = 0x40;

        /* Setting for PLL reference divider (REF_R) (0x06) */
        switch(pTuner->xtalFreq){
        case SONY_HELENE2_XTAL_16000KHz:
            data[2] = 0x02;
            break;
        case SONY_HELENE2_XTAL_24000KHz:
            data[2] = 0x03;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Setting for analog block (0x07) */
        if((pTuner->flags & SONY_HELENE2_CONFIG_LOOPFILTER_INTERNAL)
            && (tvSystem != SONY_HELENE2_STV_ISDBS3)){
            data[3] = 0x80;
        }else{
            data[3] = 0x00;
        }

        /* Tuning setting for analog block (0x08, 0x09, 0x0A, 0x0B) */
        if((pTuner->flags & SONY_HELENE2_CONFIG_LOOPFILTER_INTERNAL)
            && (tvSystem != SONY_HELENE2_STV_ISDBS3)){
            data[4] = 0x70;
            data[5] = 0x1E;
            data[6] = 0x02;
            data[7] = 0x24;
        }else{
            if(pTuner->isFreesatMode){
                data[4] = 0x20;
            }else{
                data[4] = 0xB4;
            }
            data[5] = 0x78;
            data[6] = 0x08;
            data[7] = 0x30;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    if((pTuner->chipId == SONY_HELENE2_CHIP_ID_2868) && (frequencykHz > 2150000) && (tvSystem == SONY_HELENE2_STV_ISDBS3)){
        /* Sequence B */

        /* RFIN matching in power save (Sat) reset (0x45) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x45,
            (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF) ? 0x00 : 0x02));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* Satellite mode select (0x01) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x03);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* 0x0C - 0x15 */
        {
            uint8_t data[10];

            /* Enable for analog block (0x0C, 0x0D, 0x0E) */

            if((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_SAT_MASK) == SONY_HELENE2_CONFIG_POWERSAVE_SAT_RF_ACTIVE){
                if(pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF){
                    data[0] = 0x0E;
                }else{
                    data[0] = 0x0D;
                }
            }else{
                data[0] = 0x0C;
            }

            if((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK)
                == SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE){
                data[0] |= 0xE0; /* Clear RFOVLD_DET_ENX (Bit[4]) */
            }else{
                data[0] |= 0xF0;
            }

            if(pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF){
                data[1] = 0x22;
            }else{
                data[1] = 0x32;
            }
            data[2] = 0x9E;

            /* Setting for LPF cutoff frequency (0x0F) */
            switch(tvSystem){
            case SONY_HELENE2_STV_ISDBS:
            case SONY_HELENE2_STV_ISDBS3:
                data[3] = 22; /* 22MHz */
                break;

            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
            }

            /* RF tuning frequency setting (0x10, 0x11, 0x12) */
            {
                uint32_t frequency4kHz = (frequencykHz + 2) / 4;
                data[4] = (uint8_t)(frequency4kHz & 0xFF);         /* FRF_L */
                data[5] = (uint8_t)((frequency4kHz >> 8) & 0xFF);  /* FRF_M */
                data[6] = (uint8_t)((frequency4kHz >> 16) & 0x0F); /* FRF_H (bit[3:0]) */
            }

            /* Tuning command (0x13) */
            if(vcoCal){
                data[7] = 0xFF;
            }else{
                data[7] = 0x8F;
            }

            /* Setting for IQOUT_LIMIT (0x14) */
            data[8] = (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_OUTLMT_STV_0_6Vpp) ? 0x01 : 0x00);

            /* Enable IQ output (0x15) */
            data[9] = 0x01;

            result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, data, sizeof(data));
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        }
    }else{
        /* Sequence A */

        /* RFIN matching in power save (Sat) reset (0x43) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43,
            (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF) ? 0x04 : 0x06));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* Satellite mode select (0x01) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x01);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* 0x0C - 0x15 */
        {
            uint8_t data[10];

            /* Enable for analog block (0x0C, 0x0D, 0x0E) */
            if(pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF){
                data[0] = 0x0E; /* Clear SAT_RF_LNA_EN (Bit[0]) */
            }else{
                data[0] = 0x0F;
            }

            if((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK)
                == SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE){
                data[0] |= 0xE0; /* Clear RFOVLD_DET_ENX (Bit[4]) */
            }else{
                data[0] |= 0xF0;
            }

            data[1] = 0x02;
            if((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_SAT_MASK) == SONY_HELENE2_CONFIG_POWERSAVE_SAT_RF_ACTIVE){
                if(pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF){
                    data[1] |= 0x20;
                }else{
                    data[1] |= 0x10;
                }
            }

            data[2] = 0x9E;

            /* Setting for LPF cutoff frequency (0x0F) */
            switch(tvSystem){
            case SONY_HELENE2_STV_ISDBS:
            case SONY_HELENE2_STV_ISDBS3:
                data[3] = 22; /* 22MHz */
                break;

            case SONY_HELENE2_STV_DVBS:
                /*
                    rolloff = 0.35

                    SR <= 4
                      lpf_cutoff = 5
                    4 < SR <= 10
                      lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.175 = SR * (47/40)
                    10 < SR
                      lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.675 + 5 = SR * (27/40) + 5
                    NOTE: The result should be round up.
                */
                if(symbolRateksps <= 4000){
                    data[3] = 5;
                }else if(symbolRateksps <= 10000){
                    data[3] = (uint8_t)((symbolRateksps * 47 + (40000-1)) / 40000);
                }else{
                    data[3] = (uint8_t)((symbolRateksps * 27 + (40000-1)) / 40000 + 5);
                }

                if(data[3] > 36){
                    data[3] = 36; /* 5 <= lpf_cutoff <= 36 is valid */
                }
                break;

            case SONY_HELENE2_STV_DVBS2:
                /*
                    rolloff = 0.2

                    SR <= 4
                      lpf_cutoff = 5
                    4 < SR <= 10
                      lpf_cutoff = SR / 2 * (2 + rolloff) = SR * 1.1 = SR * (11/10)
                    10 < SR
                      lpf_cutoff = SR / 2 * (1 + rolloff) + 5 = SR * 0.6 + 5 = SR * (3/5) + 5
                    NOTE: The result should be round up.
                */
                if(symbolRateksps <= 4000){
                    data[3] = 5;
                }else if(symbolRateksps <= 10000){
                    data[3] = (uint8_t)((symbolRateksps * 11 + (10000-1)) / 10000);
                }else{
                    data[3] = (uint8_t)((symbolRateksps * 3 + (5000-1)) / 5000 + 5);
                }

                if(data[3] > 36){
                    data[3] = 36; /* 5 <= lpf_cutoff <= 36 is valid */
                }
                break;

            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
            }

            /* RF tuning frequency setting (0x10, 0x11, 0x12) */
            {
                uint32_t frequency4kHz = (frequencykHz + 2) / 4;
                data[4] = (uint8_t)(frequency4kHz & 0xFF);         /* FRF_L */
                data[5] = (uint8_t)((frequency4kHz >> 8) & 0xFF);  /* FRF_M */
                data[6] = (uint8_t)((frequency4kHz >> 16) & 0x0F); /* FRF_H (bit[3:0]) */
            }

            /* Tuning command (0x13) */
            if(vcoCal){
                data[7] = 0xFF;
            }else{
                data[7] = 0x8F;
            }

            /* Setting for IQOUT_LIMIT (0x14) */
            data[8] = (uint8_t)((pTuner->flags & SONY_HELENE2_CONFIG_OUTLMT_STV_0_6Vpp) ? 0x01 : 0x00);

            /* Enable IQ output (0x15) */
            data[9] = 0x01;

            result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, data, sizeof(data));
            if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_tune_end(sony_helene2_t *pTuner, sony_helene2_tv_system_t tvSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_tune_end");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Standby setting for CPU (0x05) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x05, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x04) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t TER_fin(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("TER_fin");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Disable IF signal output (IF_OUT_SEL setting) (0x74) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x74, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (terrestrial) (0x67) */
    /* Power save setting for analog block (0x5E, 0x5F, 0x60) */
    {
        uint8_t data[4]; /* 0x5E - 0x60, 0x67 */

        switch(pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK){
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RFIN_MATCHING_DISABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x00;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RFIN_MATCHING_ENABLE:
            data[0] = 0x15;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x02;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE:
            data[0] = 0x06;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Keep RF_EXT bit */
        result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x67, data[3], 0xFE);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5E, &data[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t SAT_fin(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("SAT_fin");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Disable IQ signal output (IF_OUT_SEL setting) (0x15) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x15, 0x02);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* RFIN matching in power save (satellite) (0x43, 0x45) */
    /* Power save setting for analog block (0x0C, 0x0D, 0x0E) */
    {
        uint8_t data[5]; /* 0x0C - 0x0E, 0x43, 0x45 */
        uint8_t lnaOff = (pTuner->flags & SONY_HELENE2_CONFIG_SAT_LNA_OFF) ? 1 : 0;
        uint8_t terrRfActive = ((pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_TERR_MASK)
            == SONY_HELENE2_CONFIG_POWERSAVE_TERR_RF_ACTIVE) ? 1 : 0;

        switch(pTuner->flags & SONY_HELENE2_CONFIG_POWERSAVE_SAT_MASK){
        case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RFIN_MATCHING_DISABLE: /* Mode 1 */
            data[0] = terrRfActive ? 0x04 : 0x14;
            data[1] = 0x00;
            data[2] = terrRfActive ? 0x02 : 0x00;
            data[3] = lnaOff       ? 0x05 : 0x07;
            data[4] = lnaOff       ? 0x01 : 0x03;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RFIN_MATCHING_ENABLE: /* Mode 0 */
            data[0] = terrRfActive ? 0x04 : 0x14;
            data[1] = 0x00;
            data[2] = terrRfActive ? 0x02 : 0x00;
            data[3] = lnaOff       ? 0x04 : 0x06;
            data[4] = lnaOff       ? 0x00 : 0x02;
            break;
        case SONY_HELENE2_CONFIG_POWERSAVE_SAT_RF_ACTIVE:
            if(lnaOff){ /* Mode 3 */
                data[0] = terrRfActive ? 0x06 : 0x16;
                data[1] = 0x20;
                data[2] = terrRfActive ? 0x02 : 0x00;
                data[3] = 0x04;
                data[4] = 0x00;
            }else{ /* Mode 2 */
                data[0] = terrRfActive ? 0x05 : 0x15;
                data[1] = 0x10;
                data[2] = terrRfActive ? 0x02 : 0x00;
                data[3] = 0x06;
                data[4] = 0x02;

            }
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x43, data[3]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x45, data[4]);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0C, &data[0], 3);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Return to terrestrial mode (0x01) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x01, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x05) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x05, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x04) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x04, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_oscdis(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_oscdis");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Setting for REFOUT signal output (0x84) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x84, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_oscen(sony_helene2_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER("X_oscen");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82, 0x9F);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Setting for REFOUT signal output (0x84) */
    switch(pTuner->flags & SONY_HELENE2_CONFIG_REFOUT_MASK){
    case 0:
        data = 0x00; /* REFOUT_EN = 0, REFOUT_CNT = 0 */
        break;
    case SONY_HELENE2_CONFIG_REFOUT_500mVpp:
        data = 0x80; /* REFOUT_EN = 1, REFOUT_CNT = 0 */
        break;
    case SONY_HELENE2_CONFIG_REFOUT_400mVpp:
        data = 0x81; /* REFOUT_EN = 1, REFOUT_CNT = 1 */
        break;
    case SONY_HELENE2_CONFIG_REFOUT_600mVpp:
        data = 0x82; /* REFOUT_EN = 1, REFOUT_CNT = 2 */
        break;
    case SONY_HELENE2_CONFIG_REFOUT_800mVpp:
        data = 0x83; /* REFOUT_EN = 1, REFOUT_CNT = 3 */
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x84, data);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_SLEEP(10);

    /* Driver current setting for crystal oscillator (0x82) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x82,
        (uint8_t)(0x80 | (pTuner->xosc_sel & 0x1F)));
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t X_read_agc(sony_helene2_t *pTuner, uint8_t *pSatCompensationReg, uint8_t *pTerrCompensationReg,
    uint8_t *pIFAGCReg, uint8_t *pRFAGCReg)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("X_read_agc");

    if((!pTuner) || (!pTuner->pI2c) || (!pSatCompensationReg) || (!pTerrCompensationReg) || (!pIFAGCReg) || (!pRFAGCReg)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Clock enable for internal logic block, CPU wake-up (0x87, 0x88) */
    {
        const uint8_t cdata[2] = {0xC4, 0x41};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Read Compensation data */
    {
        uint8_t data[2] = {0x7E, 0x06};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x17, data, sizeof(data));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        /* Wait at least 1ms */
        SONY_SLEEP(2);

        /* Confirm CPU_STT is 0x00 */
        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1A, &data[0], 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        if(data[0] != 0x00){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x19, &data[0], 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

        *pSatCompensationReg = (data[0] >> 4) & 0x0F;
        *pTerrCompensationReg = data[0] & 0x0F;
    }

    /* Connect IFAGC, Start ADC (0x59, 0x5A) */
    {
        const uint8_t cdata[2] = {0x05, 0x01};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* ADC read out (0x5B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5B, pIFAGCReg, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Connect RFAGC, Start ADC (0x59, 0x5A) */
    {
        const uint8_t cdata[2] = {0x03, 0x01};
        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, cdata, sizeof(cdata));
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* ADC read out (0x5B) */
    result = pTuner->pI2c->ReadRegister(pTuner->pI2c, pTuner->i2cAddress, 0x5B, pRFAGCReg, 1);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* ADC disable (0x59) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x59, 0x04);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for CPU (0x88) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x88, 0x00);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Standby setting for internal logic block (0x87) */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x87, 0xC0);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
