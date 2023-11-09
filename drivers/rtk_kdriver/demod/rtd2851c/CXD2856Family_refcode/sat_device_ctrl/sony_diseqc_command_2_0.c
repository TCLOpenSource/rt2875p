/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_diseqc_command_2_0.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_diseqc_command_ClrReset (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_ClrReset");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x01;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_SetContend (sony_diseqc_message_t * pMessage,
                                              sony_diseqc_framing_t framing,
                                              sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_SetContend");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x04;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Contend (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Contend");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x05;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_ClrContend (sony_diseqc_message_t * pMessage,
                                              sony_diseqc_framing_t framing,
                                              sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_ClrContend");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x06;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Address (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Address");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x07;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_MoveC (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address,
                                         uint8_t data)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_MoveC");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x08;
    pMessage->data[3] = data;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Move (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address,
                                        uint8_t data)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Move");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x09;
    pMessage->data[3] = data;
    pMessage->length = 4;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Status (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Status");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x10;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Config (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Config");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x11;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Switch0 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Switch0");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x14;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_Switch1 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_Switch1");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x15;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_ReadA0 (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_ReadA0");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x40;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_ReadA1 (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_ReadA1");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x41;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LOstring (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LOstring");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x50;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LOnow (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LOnow");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x51;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LOLo (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LOLo");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x52;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_diseqc_command_LOHi (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_LOHi");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x53;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
