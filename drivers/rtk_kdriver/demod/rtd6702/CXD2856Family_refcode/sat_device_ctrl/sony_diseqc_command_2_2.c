/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_diseqc_command_2_2.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_diseqc_command_PosStat (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address)
{
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_diseqc_command_PosStat");

    if (!pMessage){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
        pMessage->data[index] = 0;
    }

    pMessage->data[0] = (uint8_t)framing;
    pMessage->data[1] = (uint8_t)address;
    pMessage->data[2] = 0x64;
    pMessage->length = 3;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
