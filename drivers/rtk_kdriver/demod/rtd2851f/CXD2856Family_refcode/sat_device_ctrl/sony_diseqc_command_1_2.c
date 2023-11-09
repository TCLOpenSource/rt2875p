/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_diseqc_command_1_2.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t generateDriveXData(sony_diseqc_motor_mode_t mode,
                                        uint8_t amount,
                                        uint8_t * pData);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_diseqc_command_WriteA0 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_WriteA0");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x48;
    pMessage->data[3] = data;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_WriteA1 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_WriteA1");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x49;
    pMessage->data[3] = data;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Halt (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Halt");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x60;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LimitsOff (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LimitsOff");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x63;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LimitE (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LimitE");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x66;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LimitW (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LimitW");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x67;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_DriveEast (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address,
                                             sony_diseqc_motor_mode_t mode,
                                             uint8_t amount)
{
    int32_t index = 0;
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_diseqc_command_DriveEast");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x68;
    result = generateDriveXData(mode, amount, &pMessage->data[3]);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_DriveWest (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address,
                                             sony_diseqc_motor_mode_t mode,
                                             uint8_t amount)
{
    int32_t index = 0;
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_diseqc_command_DriveWest");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x69;
    result = generateDriveXData(mode, amount, &pMessage->data[3]);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Storenn (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t posNumber)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Storenn");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x6A;
    pMessage->data[3] = posNumber;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Gotonn (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address,
                                          uint8_t posNumber)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Gotonn");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x6B;
    pMessage->data[3] = posNumber;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_SetPosns (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address,
                                            uint8_t * pData,
                                            uint8_t length)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_SetPosns");

    if ((!pMessage) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x6F;
    if (length == 1){
        pMessage->data[3] = *pData;
        pMessage->length = 4;
    } else if (length == 3){
        pMessage->data[3] = *(pData);
        pMessage->data[4] = *(pData + 1);
        pMessage->data[5] = *(pData + 2);
        pMessage->length = 6;
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t generateDriveXData(sony_diseqc_motor_mode_t mode,
                                        uint8_t amount,
                                        uint8_t * pData)
{
    SONY_TRACE_ENTER ("generateDriveXData");

    if (!pData) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(mode)
    {
    case SONY_DISEQC_MOTOR_MODE_STEPPED:
        if((amount > 128) || (amount == 0)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            *pData = (uint8_t)((0xFF - amount) + 1);
        }
        break;

    case SONY_DISEQC_MOTOR_MODE_TIMED:
        if((amount > 127) || (amount == 0)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            *pData = amount;
        }
        break;

    case SONY_DISEQC_MOTOR_MODE_NONSTOP:
        /* amount value is ignored. */
        *pData = 0;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
