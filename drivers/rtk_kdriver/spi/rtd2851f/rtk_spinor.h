#ifndef __LINUX_RTK_SPINOR_H
#define __LINUX_RTK_SPINOR_H

#include <mach/iomap.h>
#include <rtk_kdriver/io.h>

#define EMPTY_VALUE                0
/*********************************************************
*
* spinor_info_t define
*
**********************************************************/
// device id
#define MX_25L12835D_128Mbit            0xc22018//      0x1820c2
#define MX_25L6406E_64Mbit          0xc22017
#define MX_25L3205D_32Mbit          0xc22016
#define MX_25L8006E_8Mbit          0xc22014
#define MX_25L1606E_16Mbit			0xc22015

#define WINBOND_W32Q64_64Mbit           0xef4017
#define WINBOND_W25Q128FV_128Mbit       0xef4018
#define WINBOND_W25Q16JV_16Mbit                0xef4015
#define WINBOND_W25X40CL_4Mbit         0xef3013

#define WINBOND_W25Q256FV_256Mbit	0xef4019
#define WINBOND_W25M512JV_512Mbit	0xef7119


#define GD25Q64C_64Mbit             0xc84017
#define GD25Q128C_128Mbit                   0xc84018
#define GD25Q40C_4Mbit                   0xc84013
#define F25L64QA_64Mbit             0x8c4117

typedef enum {
        eMODE_READ_NORMAL = 0,
        eMODE_READ_NORMAL_FAST,
        eMODE_READ_DUAL_FAST,
        eMODE_READ_QUAD_FAST
} read_mode;

typedef enum {
        eMODE_WRITE_NORMAL = 0,
        eMODE_WRITE_QUAD_FAST
} write_mode;

// QUAD MODE   RDSR(read status reg)   WRSR(write status reg)
#define CMD_QUAD_RDSR1            0x05
#define CMD_QUAD_RDSR2            0x35
#define CMD_QUAD_RDSR3            0x15
#define CMD_QUAD_WRSR1            0X01
#define CMD_QUAD_WRSR2            0X31
#define CMD_QUAD_WRSR3            0X11
#define BIT7                        0x80
#define BIT6                        0x40
#define BIT5                        0x20
#define BIT4                        0x10
#define BIT3                        0x08
#define BIT2                        0x04
#define BIT1                        0x02
#define BIT0                        0x01

// spinor_info_t->erase_capacity
#define ERASE_4K           0x01
#define ERASE_32K          0x02
#define ERASE_64K          0x04
#define ERASE_256K         0x08

#define ERASE_SZIE_64K  0x10000
#define ERASE_SZIE_32K  0x8000
#define ERASE_SZIE_4K   0x1000

typedef struct {
        unsigned int    id ;
        unsigned char   erase_capacity;
        unsigned int    min_erase_size;
        unsigned char   page_program ;
        unsigned int    size;
        unsigned char   *string;
        read_mode       read_mode;
        write_mode      write_mode;

        // Quad Enabel
        unsigned char   cmd_quad_rdsr;
        unsigned char   cmd_quad_wrsr;
        unsigned char   cmd_quad_bit;

        /*4bytes address enable*/
        unsigned char	address_4bytes_enable;

        /*multi die flash*/
        unsigned char	multi_die_support;
        unsigned int	per_die_size;
} spinor_info_t;

static const spinor_info_t s_device[] = {
        // id                      256 64 32 4                    erase_size    size       string                       read_mode              write_mode
        {MX_25L12835D_128Mbit,       ERASE_4K | ERASE_64K,           0x1000,    1, 0x1000000, "MX_25L12835D_128Mbit",      eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR1, CMD_QUAD_WRSR1, BIT6, 0,  0,  0},
        {MX_25L6406E_64Mbit,         ERASE_4K | ERASE_64K,          ERASE_SZIE_4K,    1, 0x800000,  "MX_25L6406E_64Mbit",        eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL,  EMPTY_VALUE,    EMPTY_VALUE,    EMPTY_VALUE, 0,  0,  0},
        {MX_25L3205D_32Mbit,         ERASE_4K|ERASE_64K,          ERASE_SZIE_4K,    1, 0x400000,  "MX_25L3205D_32Mbit",        eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL,  EMPTY_VALUE,    EMPTY_VALUE,    EMPTY_VALUE, 0,  0,  0},
        {MX_25L1606E_16Mbit,         ERASE_4K|ERASE_64K ,          ERASE_SZIE_4K,    1, 0x200000,  "MX_25L1606E_16Mbit",        eMODE_READ_NORMAL,       eMODE_WRITE_NORMAL,  EMPTY_VALUE,    EMPTY_VALUE,    EMPTY_VALUE, 0, 0, 0},
        {MX_25L8006E_8Mbit,         ERASE_4K|ERASE_64K,          ERASE_SZIE_4K,    1, 0x100000,  "MX_25L8006E_8Mbit",        eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL,  EMPTY_VALUE,    EMPTY_VALUE,    EMPTY_VALUE, 0,  0,  0},

        {WINBOND_W32Q64_64Mbit,      ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x800000, "WINBOND_W32Q64_64Mbit", eMODE_READ_QUAD_FAST,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR2, CMD_QUAD_WRSR1, BIT1, 0,  0,  0},
        {WINBOND_W25Q128FV_128Mbit,  ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x1000000, "WINBOND_W25Q128FV_128Mbit", eMODE_READ_QUAD_FAST,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR2, CMD_QUAD_WRSR2, BIT1, 0,  0,  0},
        {WINBOND_W25Q16JV_16Mbit,    ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x200000,  "WINBOND_W25Q16JV_16Mbit",   eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, EMPTY_VALUE, EMPTY_VALUE, EMPTY_VALUE, 0, 0, 0},
        {WINBOND_W25X40CL_4Mbit,     ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x80000, "WINBOND_W25X40CL_4Mbit", eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, EMPTY_VALUE, EMPTY_VALUE, EMPTY_VALUE, 0, 0, 0},
        
        {WINBOND_W25Q256FV_256Mbit,  ERASE_4K|ERASE_32K|ERASE_64K, ERASE_SZIE_4K,	 1, 		0x2000000, "WINBOND_W25Q256FV_256Mbit", eMODE_READ_NORMAL,		 eMODE_WRITE_NORMAL,  CMD_QUAD_RDSR2, CMD_QUAD_WRSR2, BIT1, 		1,  0,  0},
        {WINBOND_W25M512JV_512Mbit,  ERASE_4K|ERASE_32K|ERASE_64K, ERASE_SZIE_4K,	 1, 		0x4000000, "WINBOND_W25M512JV_512Mbit", eMODE_READ_NORMAL,		 eMODE_WRITE_NORMAL,  EMPTY_VALUE,	  EMPTY_VALUE,	  EMPTY_VALUE,	1,  1,  0x2000000},

        {GD25Q64C_64Mbit,            ERASE_4K | ERASE_32K | ERASE_64K, ERASE_SZIE_4K,    1, 0x800000, "GD25Q64C_64Mbit",     eMODE_READ_QUAD_FAST,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR2, CMD_QUAD_WRSR2, BIT1, 0,  0,  0},
        {GD25Q128C_128Mbit,          ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x1000000, "GD25Q128C_128Mbit",         eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR2, CMD_QUAD_WRSR2, BIT1, 0,  0,  0},
        {GD25Q40C_4Mbit,          ERASE_4K | ERASE_32K | ERASE_64K, 0x1000,    1, 0x80000, "GD25Q40C_4Mbit",         eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR2, CMD_QUAD_WRSR2, BIT1, 0,  0,  0},

        {F25L64QA_64Mbit,            ERASE_4K | ERASE_32K | ERASE_64K,   0x1000,    1, 0x800000, "F25L64QA_64Mbit",         eMODE_READ_QUAD_FAST,    eMODE_WRITE_NORMAL, CMD_QUAD_RDSR1, CMD_QUAD_WRSR1, BIT6, 0,  0,  0},

} ;

static const spinor_info_t default_device = {0x0,       ERASE_4K | ERASE_32K | ERASE_64K,           0x1000,    1, 0x100000, NULL,      eMODE_READ_NORMAL,    eMODE_WRITE_NORMAL, EMPTY_VALUE, EMPTY_VALUE, EMPTY_VALUE, 0,  0,  0};

#define DEV_SIZE_S      (sizeof(s_device)/sizeof(spinor_info_t))

/*********************************************************
*
* spinor_chip_t define
*
**********************************************************/
typedef enum {
    FL_READY,
    FL_WORKING,
    FL_SUSPEND,
}spi_device_state_t;

typedef enum
{
        RHAL_CS0_SPIFLASH = 0,
        RHAL_CS1_FCIC,
        RHAL_CS_MAX,
}RHAL_CS_TYPE_E;

typedef struct
{
        RHAL_CS_TYPE_E cs_type;
        unsigned int clock;
        unsigned int flash_size;
        unsigned int erase_size;
} RHAL_SPI_INIT_PARAM_T;

typedef struct
{
        unsigned char status_register1;
        unsigned char status_register2;
        unsigned char status_register3;
} STATUS_REGISTER_T;

struct spinor_chip_t{
        spinor_info_t *spinor_info;
        spinlock_t spinor_lock;
        unsigned int * spi_reg_array; 
        unsigned char clock_div;
        unsigned char delay_sel;
        unsigned char min_clock_div;	// record the max value of clock
        wait_queue_head_t wq;	
        spi_device_state_t	state;
        void (*read_id)(unsigned int *id);
        int (*read)(struct spinor_chip_t *spinor_chip, loff_t from, size_t len, char *buf);
        int (*write)(struct spinor_chip_t *spinor_chip, loff_t to, size_t len, char *buf);
        int (*erase)(struct spinor_chip_t *spinor_chip, loff_t from, size_t len);
        int (*unlock)(struct spinor_chip_t *spinor_chip);
} ;

#define FOUR_BYTES_MASK   0x3
#define WRITE_BYTES_MASK  0xf
#define MD_PP_DATA_SIZE_SHIFT   8       /* 256 bytes */
#define MD_PP_DATA_SIZE             (1 << MD_PP_DATA_SIZE_SHIFT)

#define SPI_IOC_MAGIC  's'
#define SPI_IOCR_INIT         _IOWR(SPI_IOC_MAGIC,  0, unsigned int)
#define SPI_IOCR_GetStatusRegister         _IOR(SPI_IOC_MAGIC,  1, unsigned int)
#define SPI_IOCR_LockFlash  _IOR(SPI_IOC_MAGIC,  2, unsigned int)
#define SPI_IOCR_UnLockFlash  _IOR(SPI_IOC_MAGIC,  3, unsigned int)
#define SPI_IOCR_SET_CLK  _IOW(SPI_IOC_MAGIC,  4, unsigned int)
#define SPI_IOCR_GetParameter   _IOR(SPI_IOC_MAGIC,  5, unsigned int)

int spi_read_internal(unsigned int type, unsigned long offset, unsigned int count, void *buf);

#endif
