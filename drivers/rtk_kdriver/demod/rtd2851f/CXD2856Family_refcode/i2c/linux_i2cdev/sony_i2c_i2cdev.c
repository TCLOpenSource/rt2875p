/*------------------------------------------------------------------------------
  Copyright 2014-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "sony_i2c_i2cdev.h"
#include "sony_stdlib.h"

#define BURST_WRITE_MAX 128 /* Max length of burst write */

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_i2c_i2cdev_Read (sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData, uint32_t size, uint8_t mode)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_Read");

    SONY_ARG_UNUSED (mode);

    if ((!pI2c) || (!pData) || (size == 0)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    messages.addr  = deviceAddress >> 1;
    messages.flags = I2C_M_RD;
    messages.len   = size;
    messages.buf   = (__u8*)pData;
    packets.msgs  = &messages;
    packets.nmsgs = 1;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

static sony_result_t sony_i2c_i2cdev_Write (sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t* pData, uint32_t size, uint8_t mode)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_Write");

    SONY_ARG_UNUSED (mode);

    if ((!pI2c) || (!pData) || (size == 0)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    messages.addr  = deviceAddress >> 1;
    messages.flags = 0;
    messages.len   = size;
    messages.buf   = (__u8*)pData;
    packets.msgs  = &messages;
    packets.nmsgs = 1;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

static sony_result_t sony_i2c_i2cdev_ReadGw (sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData, uint32_t size, uint8_t mode)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    uint8_t buffer[2];
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_ReadGw");

    SONY_ARG_UNUSED (mode);

    if ((!pI2c) || (!pData) || (size == 0)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    buffer[0] = pI2c->gwSub;
    /* deviceAddress is in 8bit form */
    buffer[1] = (deviceAddress & 0xFE) | 0x01;

    messages[0].addr  = pI2c->gwAddress >> 1;
    messages[0].flags = 0;
    messages[0].len   = 2;
    messages[0].buf   = (__u8*)buffer;
    messages[1].addr  = pI2c->gwAddress >> 1;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = size;
    messages[1].buf   = (__u8*)pData;
    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
}

static sony_result_t sony_i2c_i2cdev_WriteGw (sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t* pData, uint32_t size, uint8_t mode)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;
    uint8_t buffer[BURST_WRITE_MAX + 2];
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_WriteGw");

    SONY_ARG_UNUSED (mode);

    if ((!pI2c) || (!pData) || (size == 0)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (size > BURST_WRITE_MAX) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    buffer[0] = pI2c->gwSub;
    /* deviceAddress is in 8bit form */
    buffer[1] = (deviceAddress & 0xFE) | 0x00;
    sony_memcpy(&buffer[2], pData, size);

    messages.addr  = pI2c->gwAddress >> 1;
    messages.flags = 0;
    messages.len   = size + 2;
    messages.buf   = (__u8*)buffer;
    packets.msgs  = &messages;
    packets.nmsgs = 1;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

static sony_result_t sony_i2c_i2cdev_ReadRegister (sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_ReadRegister");

    if ((!pI2c) || (!pData) || (size == 0)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    messages[0].addr  = deviceAddress >> 1;
    messages[0].flags = 0;
    messages[0].len   = 1;
    messages[0].buf   = (__u8*)&subAddress;
    messages[1].addr  = deviceAddress >> 1;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = size;
    messages[1].buf   = (__u8*)pData;
    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

static sony_result_t sony_i2c_i2cdev_WriteRegisterGw(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;
    uint8_t buffer[BURST_WRITE_MAX + 3];
    sony_i2c_i2cdev_t * pI2cI2cdev = NULL;

    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_WriteRegisterGw");

    if((!pI2c) || (!pData) || (size == 0)){
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_OVERFLOW);
    }

    pI2cI2cdev = (sony_i2c_i2cdev_t*)(pI2c->user);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    buffer[0] = pI2c->gwSub;
    /* deviceAddress is in 8bit form */
    buffer[1] = (deviceAddress & 0xFE) | 0x00;
    buffer[2] = subAddress;
    sony_memcpy(&buffer[3], pData, size);

    messages.addr  = pI2c->gwAddress >> 1;
    messages.flags = 0;
    messages.len   = size + 3;
    messages.buf   = (__u8*)buffer;
    packets.msgs  = &messages;
    packets.nmsgs = 1;

    if (ioctl(pI2cI2cdev->fd, I2C_RDWR, &packets) < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_i2c_i2cdev_Initialize (sony_i2c_i2cdev_t* pI2cI2cdev, uint8_t busNum)
{
    char devName[64];
    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_Initialize");

    if (!pI2cI2cdev) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    snprintf (devName, sizeof (devName), "/dev/i2c-%d", busNum);
    /* Open i2c-dev driver */
    pI2cI2cdev->fd = open (devName, O_RDWR);
    if (pI2cI2cdev->fd < 0) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_i2c_i2cdev_Finalize (sony_i2c_i2cdev_t* pI2cI2cdev)
{
    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_Finalize");

    if (!pI2cI2cdev) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pI2cI2cdev->fd >= 0) {
        close (pI2cI2cdev->fd);
        pI2cI2cdev->fd = -1;
    }

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_i2c_i2cdev_CreateI2c (sony_i2c_t* pI2c, sony_i2c_i2cdev_t* pI2cI2cdev)
{
    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_CreateI2c");

    if ((!pI2c) || (!pI2cI2cdev)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = sony_i2c_i2cdev_Read;
    pI2c->Write = sony_i2c_i2cdev_Write;
    pI2c->ReadRegister = sony_i2c_i2cdev_ReadRegister;
    pI2c->WriteRegister = sony_i2c_CommonWriteRegister;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = 0;
    pI2c->gwSub = 0;
    pI2c->user = pI2cI2cdev; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_i2c_i2cdev_CreateI2cGw (sony_i2c_t* pI2c, sony_i2c_i2cdev_t* pI2cI2cdev, uint8_t gwAddress, uint8_t gwSub)
{
    SONY_TRACE_I2C_ENTER ("sony_i2c_i2cdev_CreateI2cGw");

    if ((!pI2c) || (!pI2cI2cdev)) {
        SONY_TRACE_I2C_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = sony_i2c_i2cdev_ReadGw;
    pI2c->Write = sony_i2c_i2cdev_WriteGw;
    pI2c->ReadRegister = sony_i2c_CommonReadRegister;
    pI2c->WriteRegister = sony_i2c_i2cdev_WriteRegisterGw;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = gwAddress;
    pI2c->gwSub = gwSub;
    pI2c->user = pI2cI2cdev; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN (SONY_RESULT_OK);
}
