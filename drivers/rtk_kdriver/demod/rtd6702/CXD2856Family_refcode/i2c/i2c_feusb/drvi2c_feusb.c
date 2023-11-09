/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/

#include "drvi2c_feusb.h"
#include "i2c_feusb.h"

sony_result_t drvi2c_feusb_Read(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{
    drvi2c_feusb_t *pDrvI2c = NULL;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Read");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_Read(pDrvI2c->handle, deviceAddress, pData, size))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

sony_result_t drvi2c_feusb_Write(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
    drvi2c_feusb_t *pDrvI2c = NULL;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Write");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_Write(pDrvI2c->handle, deviceAddress, pData, size))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* For gateway read access */
sony_result_t drvi2c_feusb_ReadGw(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{
    drvi2c_feusb_t *pDrvI2c = NULL;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_ReadGw");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_ReadGw(pDrvI2c->handle, deviceAddress, pData, size,
                             pI2c->gwAddress, pI2c->gwSub))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* For gateway write access */
sony_result_t drvi2c_feusb_WriteGw(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
    drvi2c_feusb_t *pDrvI2c = NULL;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_WriteGw");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_WriteGw(pDrvI2c->handle, deviceAddress, pData, size,
                              pI2c->gwAddress, pI2c->gwSub))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

sony_result_t drvi2c_feusb_ReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress,
                  uint8_t* pData, uint32_t size)
{
    drvi2c_feusb_t *pDrvI2c = NULL;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_ReadRegister");

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_ReadRegister(pDrvI2c->handle, deviceAddress, subAddress, pData, size))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* Driver initialization */
sony_result_t drvi2c_feusb_Initialize(drvi2c_feusb_t* pDrvI2c)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Initialize");

    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_Initialize(&(pDrvI2c->handle)))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* Driver finalization */
sony_result_t drvi2c_feusb_Finalize(drvi2c_feusb_t* pDrvI2c)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Finalize");

    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_Finalize(pDrvI2c->handle))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* I2c object creation */
sony_result_t drvi2c_feusb_CreateI2c(sony_i2c_t* pI2c, drvi2c_feusb_t* pDrvI2c)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_CreateI2c");

    if((!pI2c) || (!pDrvI2c)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = drvi2c_feusb_Read;
    pI2c->Write = drvi2c_feusb_Write;
    pI2c->ReadRegister = drvi2c_feusb_ReadRegister;
    pI2c->WriteRegister = sony_i2c_CommonWriteRegister;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = 0;
    pI2c->gwSub = 0;
    pI2c->user = pDrvI2c; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
}

/* I2c object creation for gateway access */
sony_result_t drvi2c_feusb_CreateI2cGw(sony_i2c_t* pI2c, drvi2c_feusb_t* pDrvI2c, uint8_t gwAddress, uint8_t gwSub)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_CreateI2cGw");

    if((!pI2c) || (!pDrvI2c)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = drvi2c_feusb_ReadGw;
    pI2c->Write = drvi2c_feusb_WriteGw;
    pI2c->ReadRegister = sony_i2c_CommonReadRegister;
    pI2c->WriteRegister = sony_i2c_CommonWriteRegister;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = gwAddress;
    pI2c->gwSub = gwSub;
    pI2c->user = pDrvI2c; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
}




