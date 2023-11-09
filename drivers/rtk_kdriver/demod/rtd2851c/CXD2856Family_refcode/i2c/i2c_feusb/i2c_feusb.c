/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/*
 I2C using "feusb" driver for windows env.
 Check "feusb" document in detail.
*/

#include <windows.h>
#include <winioctl.h>
#include "feusb.h"
#include <stdlib.h>
#include "i2c_feusb.h"
#include <tchar.h>

/* static function prototype */
static int i2c_feusb_CheckResponse(HANDLE handle);

int i2c_feusb_Read(HANDLE handle, uint8_t deviceAddress, uint8_t* pData, uint32_t numberOfBytes)
{
    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST sVendorReq;

    if(handle == NULL) return I2C_FEUSB_ERROR_OTHER;
    if(!pData) return I2C_FEUSB_ERROR_ARG;
    if(numberOfBytes <= 0) return I2C_FEUSB_OK; /* No Data */

    /* Convert I2C address to 7bit form */
    deviceAddress >>= 1;

    sVendorReq.Function      = URB_FUNCTION_VENDOR_ENDPOINT;
    sVendorReq.TransferFlags = USBD_SHORT_TRANSFER_OK;
    sVendorReq.ReservedBits  = 0;
    sVendorReq.Request       = RQCTL_FEUSB_READWOSUBADR;
    sVendorReq.Value         = (USHORT)(numberOfBytes);
    sVendorReq.Index         = (unsigned short)((deviceAddress << 8));

    bResult = DeviceIoControl(handle,
        IOCTL_FEUSB_VENDORREQUEST_IN,
        &sVendorReq,
        sizeof(PARAM_VENDOR_REQUEST),
        pData,
        numberOfBytes,
        &dwByteReturned,
        NULL);

    if(!bResult) return I2C_FEUSB_ERROR_ACCESS;

    return i2c_feusb_CheckResponse(handle);
}

int i2c_feusb_Write(HANDLE handle, uint8_t deviceAddress, const uint8_t * pData, uint32_t numberOfBytes)
{
    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST* pVendorReq = NULL;

    if(handle == NULL) return I2C_FEUSB_ERROR_OTHER;
    if(!pData) return I2C_FEUSB_ERROR_ARG;
    if(numberOfBytes <= 0) return I2C_FEUSB_OK; /* No Data */

    /* Convert I2C address to 7bit form */
    deviceAddress >>= 1;

    pVendorReq = (PARAM_VENDOR_REQUEST*)malloc(sizeof(PARAM_VENDOR_REQUEST)+numberOfBytes-1);
    if(pVendorReq == NULL) return I2C_FEUSB_ERROR_OTHER;
    pVendorReq->Function      = URB_FUNCTION_VENDOR_ENDPOINT;
    pVendorReq->TransferFlags = USBD_SHORT_TRANSFER_OK;
    pVendorReq->ReservedBits  = 0;
    pVendorReq->Request       = RQCTL_FEUSB_WRITE;
    pVendorReq->Value         = (USHORT)(numberOfBytes-1);
    pVendorReq->Index         = (unsigned short)((deviceAddress << 8) | pData[0]);

    if(numberOfBytes >= 2){
        memcpy(((unsigned char*)pVendorReq)+sizeof(PARAM_VENDOR_REQUEST),&(pData[1]),numberOfBytes-1);
    }

    bResult = DeviceIoControl(handle,
        IOCTL_FEUSB_VENDORREQUEST_OUT,
        pVendorReq,
        sizeof(PARAM_VENDOR_REQUEST) + numberOfBytes-1,
        NULL,
        0,
        &dwByteReturned,
        NULL);

    free(pVendorReq);
    if(!bResult) return I2C_FEUSB_ERROR_ACCESS;

    return i2c_feusb_CheckResponse(handle);
}

/* For gateway read access */
int i2c_feusb_ReadGw(HANDLE handle, uint8_t deviceAddress, uint8_t* pData, uint32_t numberOfBytes,
                      uint8_t gwAddress, uint8_t gwSub)
{
    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST sVendorReq;

    uint32_t nLengthAccess = 0;
    uint32_t nLengthRemain = numberOfBytes;
    uint8_t *pBufferWrite = pData;

    if(handle == NULL) return I2C_FEUSB_ERROR_OTHER;
    if(!pData) return I2C_FEUSB_ERROR_ARG;
    if(numberOfBytes <= 0) return I2C_FEUSB_OK; /* No Data */

    /* Convert I2C address to 7bit form */
    deviceAddress >>= 1;
    gwAddress >>= 1;

    memset(&sVendorReq, 0, sizeof(sVendorReq));
    sVendorReq.Function      = URB_FUNCTION_VENDOR_ENDPOINT;
    sVendorReq.TransferFlags = USBD_SHORT_TRANSFER_OK;
    sVendorReq.ReservedBits  = 0;
    sVendorReq.Request       = RQCTL_FEUSB_READ2; /* Use READ2 interface */
    sVendorReq.Index         = (USHORT)(((gwSub & 0xff) << 8)
                               | ((deviceAddress & 0x7f) << 1) | 0x01 /* Read bit */);
    while(nLengthRemain > 0){
        /* READ2 cannot handle over 255 bytes */
        nLengthAccess = nLengthRemain > 255 ? 255 : nLengthRemain;
        nLengthRemain -= nLengthAccess;

        sVendorReq.Value = (USHORT)(((nLengthAccess & 0xff) << 8) | (gwAddress & 0xff));

        bResult = DeviceIoControl(handle,
            IOCTL_FEUSB_VENDORREQUEST_IN,
            &sVendorReq,
            sizeof(sVendorReq),
            pBufferWrite,
            nLengthAccess,
            &dwByteReturned,
            NULL );

        if(!bResult) return I2C_FEUSB_ERROR_ACCESS;
        pBufferWrite += nLengthAccess;
    }

    return i2c_feusb_CheckResponse(handle);
}

/* For gateway write access */
int i2c_feusb_WriteGw(HANDLE handle, uint8_t deviceAddress, const uint8_t * pData, uint32_t numberOfBytes,
                       uint8_t gwAddress, uint8_t gwSub)
{
    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST* pVendorReq = NULL;

    if(handle == NULL) return I2C_FEUSB_ERROR_OTHER;
    if(!pData) return I2C_FEUSB_ERROR_ARG;
    if(numberOfBytes <= 0) return I2C_FEUSB_OK; /* No Data */

    /* Convert I2C address to 7bit form */
    deviceAddress >>= 1;
    gwAddress >>= 1;

    pVendorReq = (PARAM_VENDOR_REQUEST*)malloc(sizeof(PARAM_VENDOR_REQUEST)+numberOfBytes+1);
    if(pVendorReq == NULL) return I2C_FEUSB_ERROR_OTHER;
    pVendorReq->Function      = URB_FUNCTION_VENDOR_ENDPOINT;
    pVendorReq->TransferFlags = USBD_SHORT_TRANSFER_OK;
    pVendorReq->ReservedBits  = 0;
    pVendorReq->Request       = RQCTL_FEUSB_WRITE;
    pVendorReq->Value         = (USHORT)(numberOfBytes+1);
    pVendorReq->Index         = (unsigned short)((gwAddress << 8) | gwSub); /* Write to demod 0x09 */

    /* deviceAddress is first byte */
    ((unsigned char*)pVendorReq)[sizeof(PARAM_VENDOR_REQUEST)] = ((deviceAddress & 0x7f) << 1) | 0x00;
    memcpy(((unsigned char*)pVendorReq)+sizeof(PARAM_VENDOR_REQUEST)+1,pData,numberOfBytes);

    bResult = DeviceIoControl(handle,
        IOCTL_FEUSB_VENDORREQUEST_OUT,
        pVendorReq,
        sizeof(PARAM_VENDOR_REQUEST) + numberOfBytes+1,
        NULL,
        0,
        &dwByteReturned,
        NULL);

    free(pVendorReq);
    if(!bResult) return I2C_FEUSB_ERROR_ACCESS;

    return i2c_feusb_CheckResponse(handle);
}

int i2c_feusb_ReadRegister(HANDLE handle, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t numberOfBytes)
{
    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST sVendorReq;

    uint32_t nLengthAccess = 0;
    uint32_t nLengthRemain = numberOfBytes;
    uint8_t *pBufferWrite = pData;

    if(handle == NULL)
        return I2C_FEUSB_ERROR_OTHER;
    if(!pData)
        return I2C_FEUSB_ERROR_ARG;
    if(numberOfBytes <= 0)
        return I2C_FEUSB_OK;    /* No Data */

    /* Convert I2C address to 7bit form */
    deviceAddress >>= 1;

    memset (&sVendorReq, 0, sizeof(sVendorReq));
    sVendorReq.Function = URB_FUNCTION_VENDOR_ENDPOINT;
    sVendorReq.TransferFlags = USBD_SHORT_TRANSFER_OK;
    sVendorReq.ReservedBits = 0;
    sVendorReq.Request = RQCTL_FEUSB_READ; /* Use READ interface */


    while(nLengthRemain > 0){
        /* READ cannot handle over 255 bytes */
        nLengthAccess = nLengthRemain > 255 ? 255 : nLengthRemain;
        nLengthRemain -= nLengthAccess;
        sVendorReq.Index = (USHORT)(((deviceAddress & 0xff) << 8) | (subAddress & 0xff));
        sVendorReq.Value = (USHORT) (nLengthAccess & 0xff);

        bResult = DeviceIoControl(handle,
            IOCTL_FEUSB_VENDORREQUEST_IN,
            &sVendorReq,
            sizeof(sVendorReq),
            pBufferWrite,
            nLengthAccess,
            &dwByteReturned,
            NULL);

        if(!bResult)
            return I2C_FEUSB_ERROR_ACCESS;
        pBufferWrite += nLengthAccess;
        subAddress += (uint8_t)nLengthAccess;
    }

    return i2c_feusb_CheckResponse(handle);
}

static int i2c_feusb_CheckResponse(HANDLE handle)
{
    int ret = I2C_FEUSB_OK;

    BOOL bResult = FALSE;
    DWORD dwByteReturned = 0;
    PARAM_VENDOR_REQUEST sVendorReq;
    UCHAR nResponse = 0;

    sVendorReq.Function      = URB_FUNCTION_VENDOR_ENDPOINT;
    sVendorReq.TransferFlags = USBD_SHORT_TRANSFER_OK;
    sVendorReq.ReservedBits  = 0;
    sVendorReq.Request       = RQCTL_FEUSB_GET_SERIAL_TRANSFER_RESPONSE;
    sVendorReq.Value         = 0;
    sVendorReq.Index         = 0;

    bResult = DeviceIoControl(handle,
        IOCTL_FEUSB_VENDORREQUEST_IN,
        &sVendorReq,
        sizeof(PARAM_VENDOR_REQUEST),
        &nResponse,
        sizeof(UCHAR),
        &dwByteReturned,
        NULL);

    if(!bResult) return I2C_FEUSB_ERROR_ACCESS;

    switch(nResponse){
    case (ERR_IF_OK):
    case (ERR_ACK):
        ret = I2C_FEUSB_OK;
        break;
    case (ERR_NACK):
        ret = I2C_FEUSB_ERROR_ACK;
        break;
    case (ERR_IF_NG):
    case (ERR_POWER):
        ret = I2C_FEUSB_ERROR_ACCESS;
        break;
    default:
        ret = I2C_FEUSB_ERROR_OTHER;
        break;
    }

    return ret;
}

/* Driver initialization */
int i2c_feusb_Initialize(HANDLE *pHandle)
{
    HANDLE handle = NULL;

    if(pHandle == NULL){
        return I2C_FEUSB_ERROR_ARG;
    }

    /* Open FEUSB4 first */
    handle = CreateFile(
        _T("\\\\.\\feusb4-0"),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL );

    if(handle == INVALID_HANDLE_VALUE){
        /* And then open FEUSB */
        handle = CreateFile(
            _T("\\\\.\\feusb3-0"),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

        if(handle == INVALID_HANDLE_VALUE){
            return I2C_FEUSB_ERROR_OTHER;
        }
        /* Set FX2I2C interface mode (FEUSB3 only) */
        {
            BOOL bResult = FALSE;
            DWORD dwByteReturned = 0;
            PARAM_VENDOR_REQUEST sVendorReq;

            sVendorReq.Function      = URB_FUNCTION_VENDOR_ENDPOINT;
            sVendorReq.TransferFlags = USBD_SHORT_TRANSFER_OK;
            sVendorReq.ReservedBits  = 0;
            sVendorReq.Request       = RQCTL_FEUSB_SET_IF_MODE;
            sVendorReq.Value         = IF_I2C;
            sVendorReq.Index         = 0;

            bResult = DeviceIoControl(handle,
                IOCTL_FEUSB_VENDORREQUEST_OUT,
                &sVendorReq,
                sizeof(PARAM_VENDOR_REQUEST),
                NULL,
                0,
                &dwByteReturned,
                NULL );

            if(bResult == 0){
                CloseHandle(handle);
                return I2C_FEUSB_ERROR_OTHER;
            }
        }
    }

    *pHandle = handle;
    return I2C_FEUSB_OK;
}

/* Driver finalization */
int i2c_feusb_Finalize(HANDLE handle)
{
    if(handle == NULL){
        /* Not initialized ?? */
        return I2C_FEUSB_ERROR_OTHER;
    }

    if(CloseHandle(handle)){
        return I2C_FEUSB_OK;
    }else{
        return I2C_FEUSB_ERROR_OTHER;
    }
}
