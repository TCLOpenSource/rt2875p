/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod_sat_device_ctrl.h"
#include "sony_singlecable_command.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t addPINCode (sony_diseqc_message_t * pMessage,
                                 uint8_t pin);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_singlecable_command_ODU_Channel_change (sony_diseqc_message_t * pMessage,
                                                           sony_singlecable_address_t address,
                                                           uint8_t userbandId,
                                                           uint32_t ubSlotFreqKHz,
                                                           sony_singlecable_bank_t bank,
                                                           uint32_t centerFreqKHz)
{
    int32_t index = 0;
    uint16_t T = 0;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_Channel_change");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    /* Calculation T */
    T = (uint16_t)(((centerFreqKHz + ubSlotFreqKHz + 2000) / 4000) - 350);

    if (T > 0x3FFF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    pMessage->data[0] = 0xE0;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x5A;

    data  = (uint8_t)(((uint32_t)(userbandId - 1) & 0x07) << 5);
    data |= (uint8_t)(((uint32_t)bank   & 0x07) << 2);
    data |= (uint8_t)((T >> 8) & 0x03);
    pMessage->data[3] = data;

    pMessage->data[4] = (uint8_t)(T & 0xFF);
    pMessage->length = 5;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable_command_ODU_PowerOFF (sony_diseqc_message_t * pMessage,
                                                     sony_singlecable_address_t address,
                                                     uint8_t userbandId)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_PowerOFF");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0xE0;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x5A;
    pMessage->data[3] = (uint8_t)((((uint32_t)(userbandId - 1)) & 0x07) << 5);
    pMessage->data[4] = 0x00;
    pMessage->length = 5;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable_command_ODU_UbxSignal_ON (sony_diseqc_message_t * pMessage,
                                                         sony_singlecable_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_UbxSignal_ON");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0xE0;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x5B;
    pMessage->data[3] = 0x00;
    pMessage->data[4] = 0x00;
    pMessage->length = 5;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable_command_ODU_Config (sony_diseqc_message_t * pMessage,
                                                   sony_singlecable_address_t address,
                                                   uint8_t userbandId,
                                                   sony_singlecable_num_of_sat_pos_t numberOfSatPos,
                                                   sony_singlecable_rf_band_t rfBand,
                                                   sony_singlecable_num_of_ub_slots_t numberOfUBSlots)
{
    int32_t index = 0;

    static const struct{
        sony_singlecable_num_of_sat_pos_t numberOfSatPos;
        sony_singlecable_rf_band_t rfBand;
        sony_singlecable_num_of_ub_slots_t numberOfUBSlots;
        uint8_t Config_Nb;
    } ODU_Config_table[] = {
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_2, 0x10},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_4, 0x11},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_6, 0x12},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_8, 0x13},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_2, 0x14},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_4, 0x15},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_6, 0x16},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_1, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_8, 0x17},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_2, 0x18},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_4, 0x19},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_6, 0x1A},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_STANDARD,  SONY_SINGLECABLE_NUM_OF_UB_8, 0x1B},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_2, 0x1C},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_4, 0x1D},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_6, 0x1E},
        {SONY_SINGLECABLE_NUM_OF_SAT_POS_2, SONY_SINGLECABLE_RF_BAND_WIDE,      SONY_SINGLECABLE_NUM_OF_UB_8, 0x1F}
    };

    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_Config");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0xE0;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x5B;
    pMessage->data[3] = (uint8_t)(((((uint32_t)(userbandId - 1)) & 0x07) << 5) | (uint32_t)0x01);

    for(index = 0; index < (int32_t)(sizeof(ODU_Config_table)/sizeof(ODU_Config_table[0])); index++)
    {
        if((numberOfSatPos == ODU_Config_table[index].numberOfSatPos)         &&
           (rfBand == ODU_Config_table[index].rfBand) &&
           (numberOfUBSlots == ODU_Config_table[index].numberOfUBSlots))
        {
            pMessage->data[4] = ODU_Config_table[index].Config_Nb;
            pMessage->length = 5;
            /* Success */
            SONY_TRACE_RETURN(SONY_RESULT_OK);
        }
    }
    /* Error */
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
}

sony_result_t sony_singlecable_command_ODU_LoFreq (sony_diseqc_message_t * pMessage,
                                                   sony_singlecable_address_t address,
                                                   uint8_t userbandId,
                                                   sony_singlecable_lofreq_t loFreq)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_LoFreq");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0xE0;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x5B;
    pMessage->data[3] = (uint8_t)(((((uint32_t)(userbandId - 1)) & 0x07) << 5) | 0x02);
    pMessage->data[4] = (uint8_t)loFreq;
    pMessage->length = 5;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable_command_ODU_Channel_change_MDU (sony_diseqc_message_t * pMessage,
                                                               sony_singlecable_address_t address,
                                                               uint8_t userbandId,
                                                               uint32_t ubSlotFreqKHz,
                                                               sony_singlecable_bank_t bank,
                                                               uint32_t centerFreqKHz,
                                                               uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_Channel_change_MDU");

    if (!pMessage) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable_command_ODU_Channel_change (pMessage, address, userbandId, ubSlotFreqKHz, bank, centerFreqKHz);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    pMessage->data[2] = 0x5C;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable_command_ODU_PowerOFF_MDU (sony_diseqc_message_t * pMessage,
                                                         sony_singlecable_address_t address,
                                                         uint8_t userbandId,
                                                         uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_PowerOFF_MDU");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable_command_ODU_PowerOFF (pMessage, address, userbandId);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    pMessage->data[2] = 0x5C;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable_command_ODU_UbxSignal_ON_MDU (sony_diseqc_message_t * pMessage,
                                                             sony_singlecable_address_t address,
                                                             uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_UbxSignal_ON_MDU");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_singlecable_command_ODU_UbxSignal_ON (pMessage, address);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    pMessage->data[2] = 0x5D;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable_command_ODU_Config_MDU (sony_diseqc_message_t * pMessage,
                                                       sony_singlecable_address_t address,
                                                       uint8_t userbandId,
                                                       sony_singlecable_num_of_sat_pos_t numberOfSatPos,
                                                       sony_singlecable_rf_band_t rfBand,
                                                       sony_singlecable_num_of_ub_slots_t numberOfUBSlots,
                                                       uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_Config_MDU");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable_command_ODU_Config (pMessage, address, userbandId, numberOfSatPos, rfBand, numberOfUBSlots);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    pMessage->data[2] = 0x5D;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable_command_ODU_LoFreq_MDU (sony_diseqc_message_t * pMessage,
                                                       sony_singlecable_address_t address,
                                                       uint8_t userbandId,
                                                       sony_singlecable_lofreq_t loFreq,
                                                       uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable_command_ODU_LoFreq_MDU");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId <= 0) || (9 <= userbandId)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable_command_ODU_LoFreq (pMessage, address, userbandId, loFreq);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    pMessage->data[2] = 0x5D;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {SONY_TRACE_RETURN (result);}

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t addPINCode (sony_diseqc_message_t * pMessage,
                                 uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("addPIN");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pMessage->length < (SONY_DISEQC_MESSAGE_LENGTH - 1)){
        pMessage->data[pMessage->length] = pin;
        pMessage->length++;
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}
