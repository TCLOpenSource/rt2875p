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
sony_result_t sony_singlecable2_command_ODU_Channel_change (sony_diseqc_message_t * pMessage,
                                                            uint8_t userbandId,
                                                            uint32_t ifFreqMHz,
                                                            uint8_t uncommittedSwitches,
                                                            uint8_t committedSwitches)
{
    int32_t index = 0;
    uint16_t T = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_Channel_change");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32) || (uncommittedSwitches > 0x0F) || (committedSwitches > 0x0F)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    if (ifFreqMHz <= 9) {
        /* Special value */
        T = (uint16_t)ifFreqMHz;
    } else {
        /* Calculation T */
        T = (uint16_t)(ifFreqMHz - 100);
    }

    if (T > 0x7FF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    pMessage->data[0] = 0x70;
    pMessage->data[1] = (uint8_t)(((userbandId - 1) << 3) | ((T >> 8) & 0x07));
    pMessage->data[2] = (uint8_t)(T & 0xFF);
    pMessage->data[3] = (uint8_t)((uncommittedSwitches << 4) | (committedSwitches));
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable2_command_ODU_Channel_change_PIN (sony_diseqc_message_t * pMessage,
                                                                uint8_t userbandId,
                                                                uint32_t ifFreqMHz,
                                                                uint8_t uncommittedSwitches,
                                                                uint8_t committedSwitches,
                                                                uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_Channel_change_PIN");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable2_command_ODU_Channel_change (pMessage, userbandId, ifFreqMHz, uncommittedSwitches, committedSwitches);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Overwrite command ID */
    pMessage->data[0] = 0x71;

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable2_command_ODU_PowerOFF (sony_diseqc_message_t * pMessage,
                                                      uint8_t userbandId)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_PowerOFF");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable2_command_ODU_Channel_change (pMessage, userbandId, 0, 0, 0);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable2_command_ODU_PowerOFF_PIN (sony_diseqc_message_t * pMessage,
                                                          uint8_t userbandId, uint8_t pin)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_PowerOFF_PIN");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    result = sony_singlecable2_command_ODU_Channel_change_PIN (pMessage, userbandId, 0, 0, 0, pin);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    result = addPINCode (pMessage, pin);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_singlecable2_command_ODU_UB_avail (sony_diseqc_message_t * pMessage)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_UB_avail");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0x7A;
    pMessage->length = 1;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable2_command_ODU_UB_freq (sony_diseqc_message_t * pMessage,
                                                     uint8_t userbandId)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_UB_freq");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0x7D;
    pMessage->data[1] = (uint8_t)(((userbandId - 1) & 0x1F) << 3);
    pMessage->length = 2;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable2_command_ODU_UB_inuse (sony_diseqc_message_t * pMessage)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_UB_inuse");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0x7C;
    pMessage->length = 1;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable2_command_ODU_UB_PIN (sony_diseqc_message_t * pMessage)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_UB_PIN");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0x7B;
    pMessage->length = 1;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_singlecable2_command_ODU_UB_switches (sony_diseqc_message_t * pMessage,
                                                         uint8_t userbandId)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_singlecable2_command_ODU_UB_switches");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((userbandId < 1) || (userbandId > 32)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = 0x7E;
    pMessage->data[1] = (uint8_t)(((userbandId - 1) & 0x1F) << 3);
    pMessage->length = 2;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
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
