/*------------------------------------------------------------------------------
  Copyright 2012-2013 Sony Semiconductor Solutions Corporation

  Last Updated  : 2013/05/15
  File Revision : 1.0.3.0
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 Based on HORUS3A(CXD2832A) application note 1.1.2
------------------------------------------------------------------------------*/

#include "sony_horus3a.h"

/*------------------------------------------------------------------------------
 Definitions of static functions
------------------------------------------------------------------------------*/
/**
 @brief Configure the HORUS3A tuner from Power On to Power Save state.
*/
static sony_result_t Initialize(sony_horus3a_t *pTuner);
/**
 @brief Configure the HORUS3A tuner to Active state.
*/
static sony_result_t Tune(sony_horus3a_t *pTuner, uint32_t frequencykHz,
    sony_horus3a_tv_system_t tvSystem, uint32_t symbolRateksps);
/**
 @brief Configure the HORUS3A tuner to Power Save state.
*/
static sony_result_t EnterPowerSave(sony_horus3a_t *pTuner);
/**
 @brief Configure the HORUS3A tuner to go out from Power Save state.
*/
static sony_result_t LeavePowerSave(sony_horus3a_t *pTuner);

/*------------------------------------------------------------------------------
 Implementation
------------------------------------------------------------------------------*/

sony_result_t sony_horus3a_Create(sony_horus3a_t *pTuner, uint32_t xtalFreqMHz,
    uint8_t i2cAddress, sony_i2c_t *pI2c, uint32_t flags)
{
    SONY_TRACE_ENTER("sony_horus3a_Create");

    if((!pTuner) || (!pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if((xtalFreqMHz != 16) && (xtalFreqMHz != 24) && (xtalFreqMHz != 27)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pTuner->state = SONY_HORUS3A_STATE_UNKNOWN; /* Chip is not accessed for now */
    pTuner->xtalFreqMHz = xtalFreqMHz;
    pTuner->pI2c = pI2c;
    pTuner->i2cAddress = i2cAddress;
    pTuner->flags = flags;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_HORUS3A_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;
    pTuner->user = NULL;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_horus3a_Initialize(sony_horus3a_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_horus3a_Initialize");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Wait 4ms after power on */
    SONY_SLEEP(4);

    result = Initialize(pTuner);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    pTuner->state = SONY_HORUS3A_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_HORUS3A_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_horus3a_Tune(sony_horus3a_t *pTuner, uint32_t frequencykHz,
    sony_horus3a_tv_system_t tvSystem, uint32_t symbolRateksps)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_horus3a_Tune");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if((pTuner->state != SONY_HORUS3A_STATE_SLEEP) && (pTuner->state != SONY_HORUS3A_STATE_ACTIVE)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Rough frequency range check */
    if((frequencykHz < 500000) || (frequencykHz > 2500000)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
    }

    if(pTuner->state == SONY_HORUS3A_STATE_SLEEP){
        result = LeavePowerSave(pTuner);
        if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }
    }

    result = Tune(pTuner, frequencykHz, tvSystem, symbolRateksps);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    pTuner->state = SONY_HORUS3A_STATE_ACTIVE;
    /* pTuner->frequencykHz will be updated in Tune function */
    pTuner->tvSystem = tvSystem;
    pTuner->symbolRateksps = symbolRateksps;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_horus3a_Sleep(sony_horus3a_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_horus3a_Sleep");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if((pTuner->state != SONY_HORUS3A_STATE_SLEEP) && (pTuner->state != SONY_HORUS3A_STATE_ACTIVE)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = EnterPowerSave(pTuner);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    pTuner->state = SONY_HORUS3A_STATE_SLEEP;
    pTuner->frequencykHz = 0;
    pTuner->tvSystem = SONY_HORUS3A_TV_SYSTEM_UNKNOWN;
    pTuner->symbolRateksps = 0;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_horus3a_SetGPO(sony_horus3a_t *pTuner, uint8_t output)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_horus3a_SetGPO");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Write GPIO[1:0] */
    result = sony_i2c_SetRegisterBits(pTuner->pI2c, pTuner->i2cAddress, 0x2D, (uint8_t)(output ? 0x40 : 0x00), 0xC0);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Implementation of static functions
------------------------------------------------------------------------------*/

static sony_result_t Initialize(sony_horus3a_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("Initialize");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* IQ Generator disable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x2A, 0x79);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x06 - 0x08 */
    {
        uint8_t data[3];
        /* REF_R = Xtal Frequency */
        data[0] = (uint8_t)(pTuner->xtalFreqMHz);

        /* FIN = Xtal Frequency */
        data[1] = (uint8_t)(pTuner->xtalFreqMHz);

        data[2] = 0x00;

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x06, data, sizeof(data));
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    switch(pTuner->flags & SONY_HORUS3A_CONFIG_IQOUT_MASK){
    case SONY_HORUS3A_CONFIG_IQOUT_DIFFERENTIAL:
        /* IQ Out = Differential */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0A, 0x00);
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    case SONY_HORUS3A_CONFIG_IQOUT_SINGLEEND:
        /* IQ Out = Single Ended */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0A, 0x40);
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    case SONY_HORUS3A_CONFIG_IQOUT_SINGLEEND_LOWGAIN:
        /* IQ Out = Single Ended (Low Gain) */
        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0A, 0xC0);
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_OTHER);
    }

    {
        uint8_t data = 0;

        /* XOSC_SEL setting */
        if(pTuner->flags & SONY_HORUS3A_CONFIG_EXT_REF){
            data = 0x00;
        }else{
            switch(pTuner->xtalFreqMHz){
            case 27:
                data = 0x1F; break;
            case 24:
                data = 0x10; break;
            case 16:
                data = 0x0C; break;
            default:
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_OTHER);
            }
            data <<= 2; /* XOSC_SEL is Bit[6:2] */
        }

        if(!(pTuner->flags & SONY_HORUS3A_CONFIG_REFOUT_OFF)){
            /* REFOUT ON  */
            data |= 0x80;
        }

        result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x0E, data);
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Power save setting */
    result = EnterPowerSave(pTuner);
    if(result != SONY_RESULT_OK){ SONY_TRACE_RETURN(result); }

    SONY_SLEEP(3); /* Wait 3ms */

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t Tune(sony_horus3a_t *pTuner, uint32_t frequencykHz,
    sony_horus3a_tv_system_t tvSystem, uint32_t symbolRateksps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t mixdiv = 0;
    uint8_t mdiv = 0;
    uint32_t ms = 0;
    uint8_t f_ctl = 0;
    uint8_t g_ctl = 0;
    uint8_t fc_lpf = 0;

    SONY_TRACE_ENTER("Tune");

    /* frequency should be X MHz (X : integer) */
    frequencykHz = ((frequencykHz + 500) / 1000) * 1000;

    switch(tvSystem){
    case SONY_HORUS3A_STV_ISDBS:
        if(frequencykHz <= 1100000){
            mixdiv = 4;
            mdiv = 1;
        }else{
            mixdiv = 2;
            mdiv = 0;
        }
        break;
    case SONY_HORUS3A_STV_DVBS:
    case SONY_HORUS3A_STV_DVBS2:
        if(frequencykHz <= 1155000){
            mixdiv = 4;
            mdiv = 1;
        }else{
            mixdiv = 2;
            mdiv = 0;
        }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
    }

    /* Assumed that fREF == 1MHz (1000kHz) */
    ms = ((frequencykHz * mixdiv)/2 + 1000/2) / 1000; /* Round */
    if(ms > 0x7FFF){ /* 15 bit */
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid frequency */
    }

    /* Setup RF tracking filter */
    if(pTuner->flags & SONY_HORUS3A_CONFIG_LNA_ENABLE){
        if(frequencykHz < 975000){
            /* F_CTL=11100 G_CTL=001 */
            f_ctl = 0x1C; g_ctl = 0x01;
        }else if(frequencykHz < 1050000){
            /* F_CTL=11000 G_CTL=010 */
            f_ctl = 0x18; g_ctl = 0x02;
        }else if(frequencykHz < 1150000){
            /* F_CTL=10100 G_CTL=010 */
            f_ctl = 0x14; g_ctl = 0x02;
        }else if(frequencykHz < 1250000){
            /* F_CTL=10000 G_CTL=011 */
            f_ctl = 0x10; g_ctl = 0x03;
        }else if(frequencykHz < 1350000){
            /* F_CTL=01100 G_CTL=100 */
            f_ctl = 0x0C; g_ctl = 0x04;
        }else if(frequencykHz < 1450000){
            /* F_CTL=01010 G_CTL=100 */
            f_ctl = 0x0A; g_ctl = 0x04;
        }else if(frequencykHz < 1600000){
            /* F_CTL=00111 G_CTL=101 */
            f_ctl = 0x07; g_ctl = 0x05;
        }else if(frequencykHz < 1800000){
            /* F_CTL=00100 G_CTL=110 */
            f_ctl = 0x04; g_ctl = 0x06;
        }else if(frequencykHz < 2000000){
            /* F_CTL=00010 G_CTL=110 */
            f_ctl = 0x02; g_ctl = 0x06;
        }else{
            /* F_CTL=00000 G_CTL=111 */
            f_ctl = 0x00; g_ctl = 0x07;
        }
    }else{
        if(frequencykHz < 975000){
            /* F_CTL=11100 G_CTL=001 */
            f_ctl = 0x1C; g_ctl = 0x01;
        }else if(frequencykHz < 1050000){
            /* F_CTL=11000 G_CTL=010 */
            f_ctl = 0x18; g_ctl = 0x02;
        }else if(frequencykHz < 1150000){
            /* F_CTL=10100 G_CTL=010 */
            f_ctl = 0x14; g_ctl = 0x02;
        }else if(frequencykHz < 1250000){
            /* F_CTL=10000 G_CTL=011 */
            f_ctl = 0x10; g_ctl = 0x03;
        }else if(frequencykHz < 1350000){
            /* F_CTL=01100 G_CTL=100 */
            f_ctl = 0x0C; g_ctl = 0x04;
        }else if(frequencykHz < 1450000){
            /* F_CTL=01010 G_CTL=100 */
            f_ctl = 0x0A; g_ctl = 0x04;
        }else if(frequencykHz < 1600000){
            /* F_CTL=00111 G_CTL=101 */
            f_ctl = 0x07; g_ctl = 0x05;
        }else if(frequencykHz < 1800000){
            /* F_CTL=00100 G_CTL=010 */
            f_ctl = 0x04; g_ctl = 0x02;
        }else if(frequencykHz < 2000000){
            /* F_CTL=00010 G_CTL=001 */
            f_ctl = 0x02; g_ctl = 0x01;
        }else{
            /* F_CTL=00000 G_CTL=000 */
            f_ctl = 0x00; g_ctl = 0x00;
        }
    }

    /* LPF cutoff frequency setting */
    switch(tvSystem){
    case SONY_HORUS3A_STV_ISDBS:
        fc_lpf = 22; /* 22MHz */
        break;
    case SONY_HORUS3A_STV_DVBS:
        /*
            rolloff = 0.35

            SR <= 4.3
              fc_lpf = 5
            4.3 < SR <= 10
              fc_lpf = SR * (1 + rolloff) / 2 + SR / 2 = SR * 1.175 = SR * (47/40)
            10 < SR
              fc_lpf = SR * (1 + rolloff) / 2 + 5 = SR * 0.675 + 5 = SR * (27/40) + 5
            NOTE: The result should be round up.
        */
        if(symbolRateksps <= 4300){
            fc_lpf = 5;
        }else if(symbolRateksps <= 10000){
            fc_lpf = (uint8_t)((symbolRateksps * 47 + (40000-1)) / 40000);
        }else{
            fc_lpf = (uint8_t)((symbolRateksps * 27 + (40000-1)) / 40000 + 5);
        }

        if(fc_lpf > 36){
            fc_lpf = 36; /* 5 <= fc_lpf <= 36 is valid */
        }
        break;
    case SONY_HORUS3A_STV_DVBS2:
        /*
            rolloff = 0.2

            SR <= 4.5
              fc_lpf = 5
            4.5 < SR <= 10
              fc_lpf = SR * (1 + rolloff) / 2 + SR / 2 = SR * 1.1 = SR * (11/10)
            10 < SR
              fc_lpf = SR * (1 + rolloff) / 2 + 5 = SR * 0.6 + 5 = SR * (3/5) + 5
            NOTE: The result should be round up.
        */
        if(symbolRateksps <= 4500){
            fc_lpf = 5;
        }else if(symbolRateksps <= 10000){
            fc_lpf = (uint8_t)((symbolRateksps * 11 + (10000-1)) / 10000);
        }else{
            fc_lpf = (uint8_t)((symbolRateksps * 3 + (5000-1)) / 5000 + 5);
        }

        if(fc_lpf > 36){
            fc_lpf = 36; /* 5 <= fc_lpf <= 36 is valid */
        }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG); /* Invalid system */
    }

    /* 0x00 - 0x04 */
    {
        uint8_t data[5];

        data[0] = (uint8_t)((ms >> 7) & 0xFF);
        data[1] = (uint8_t)((ms << 1) & 0xFF);
        data[2] = 0x00;
        data[3] = 0x00;
        data[4] = (uint8_t)(mdiv << 7);

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x00, data, sizeof(data));
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    /* Write G_CTL, F_CTL */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x09, (uint8_t)((g_ctl << 5) | f_ctl));
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Write LPF cutoff frequency */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x37, (uint8_t)(0x80 | (fc_lpf << 1)));
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* Start Calibration */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x05, 0x80);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* IQ Generator enable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x2A, 0x7B);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    SONY_SLEEP(10);

    /* Store tuned frequency to the struct */
    pTuner->frequencykHz = ms * 2 * 1000 / mixdiv;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t EnterPowerSave(sony_horus3a_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("EnterPowerSave");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* IQ Generator disable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x2A, 0x79);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* MDIV_EN = 0 */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x29, 0x70);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* VCO disable preparation */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x28, 0x3E);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* VCO buffer disable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x2A, 0x19);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* VCO calibration disable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1C, 0x00);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* 0x11 - 0x12 */
    {
        uint8_t data[2];
        if(pTuner->flags & SONY_HORUS3A_CONFIG_POWERSAVE_ENABLEXTAL){
            /* Power save setting (xtal is not stopped) */
            data[0] = 0xC0;
        }else{
            /* Power save setting (xtal is stopped) */
            data[0] = 0x80;
        }

        if(pTuner->flags & SONY_HORUS3A_CONFIG_LNA_ENABLE){
            if(pTuner->flags & SONY_HORUS3A_CONFIG_POWERSAVE_STOPLNA){
                /* LNA is stopped */
                data[1] = 0x23;
            }else{
                /* LNA is not stopped */
                data[1] = 0x27;
            }
        }else{
            /* LNA is Disabled */
            data[1] = 0xA7;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x11, data, sizeof(data));
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t LeavePowerSave(sony_horus3a_t *pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("LeavePowerSave");

    if((!pTuner) || (!pTuner->pI2c)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* 0x11 - 0x12 */
    {
        uint8_t data[2];

        /* Disable power save */
        data[0] = 0x00;

        if(pTuner->flags & SONY_HORUS3A_CONFIG_LNA_ENABLE){
            /* LNA is Enabled */
            data[1] = 0x27;
        }else{
            /* LNA is Disabled */
            data[1] = 0xA7;
        }

        result = pTuner->pI2c->WriteRegister(pTuner->pI2c, pTuner->i2cAddress, 0x11, data, sizeof(data));
        if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }
    }


    /* VCO buffer enable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x2A, 0x79);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* VCO calibration enable */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x1C, 0xC0);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    /* MDIV_EN = 1 */
    result = pTuner->pI2c->WriteOneRegister(pTuner->pI2c, pTuner->i2cAddress, 0x29, 0x71);
    if(result != SONY_RESULT_OK) { SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C); }

    if(!(pTuner->flags & SONY_HORUS3A_CONFIG_POWERSAVE_ENABLEXTAL)){
        /* Wait Xtal stable */
        SONY_SLEEP(5);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
