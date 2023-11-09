#ifndef AUDIO_HW_EARC_IOCTL_H
#define AUDIO_HW_EARC_IOCTL_H

#define AUDIO_EARC_DEVICE_NAME	"earc0"
#define AUDIOHW_EARC_CAPABILITY_BYTES 256

typedef enum {
    AUDIOHW_EARC_PORT_NONE = 0,
    AUDIOHW_EARC_PORT_1,
    AUDIOHW_EARC_PORT_2,
    AUDIOHW_EARC_PORT_3,
    AUDIOHW_EARC_PORT_4,
    AUDIOHW_EARC_PORT_ALL,
} AUDIOHW_EARC_PORT_T;

typedef enum {
    AUDIOHW_EARC_DISABLE = 0,
    AUDIOHW_EARC_ENABLE  = 1,
} AUDIOHW_EARC_EN_T;

typedef enum {
    AUDIOHW_EARC_IDLE1 = 0,
    AUDIOHW_EARC_IDLE2 = 1,
    AUDIOHW_EARC_DISC1 = 2,
    AUDIOHW_EARC_DISC2 = 3,
    AUDIOHW_EARC_EARC  = 4,
} AUDIOHW_EARC_STATUS_T;

typedef struct {
    AUDIOHW_EARC_PORT_T port;
    AUDIOHW_EARC_EN_T earc_enable_state;
} AUDIOHW_EARC_T;

typedef struct {
    AUDIOHW_EARC_PORT_T port;
    AUDIOHW_EARC_STATUS_T status;
    unsigned char capability[AUDIOHW_EARC_CAPABILITY_BYTES]; // Short Audio
                                                              // Descriptior Max
                                                              // 256 byte
    unsigned char erx_latency_req; // 0~255
    unsigned char erx_latency;     // 0~255
    unsigned char erx_readcap_ready;    
} AUDIOHW_EARC_INFO_T;

/*
 * audio earc commands
 */
#define AUDIO_IOC_EARC_MAGIC					'e'
#define AUDIO_IOC_SET_EARC_EN						_IOW(AUDIO_IOC_EARC_MAGIC, 0, AUDIOHW_EARC_T)
#define AUDIO_IOC_GET_EARC_EN						_IOR(AUDIO_IOC_EARC_MAGIC, 1, AUDIOHW_EARC_T)
#define AUDIO_IOC_EARC_INFO						_IOR(AUDIO_IOC_EARC_MAGIC, 2, AUDIOHW_EARC_INFO_T)
#define AUDIO_IOC_EARC_SET_ERX_LATENCY_REQ		_IOW(AUDIO_IOC_EARC_MAGIC, 3, AUDIOHW_EARC_INFO_T)
#define AUDIO_IOC_EARC_RESET_HDMI_HPD_BIT		_IO(AUDIO_IOC_EARC_MAGIC, 4)

#define AUDIO_IOC_EARC_MAXNR					5

/* 
 * NOTICE: kernel space routines. Should not be exported to the user space api
 */
#ifdef __KERNEL__

struct file;

long audio_hw_earc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif /* __KERNEL__ */

#endif
