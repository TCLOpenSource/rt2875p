/**************************************************************
// Spec Version                  : 0.9
// Parser Version                : DVR_Parser_6.11(120105)
// CModelGen Version             : 5.1 2012.01.05
// Naming Rule                   :  Register_Name
// Naming Rule                   : Module_Register_Name_reg
// Parse Option                  : Only Parse _op1
// Parse Address Region          : All Address Region
// Decode bit number             : 12 bits
// Firmware Header Generate Date : 2013/12/5 16:43:43
***************************************************************/
#include <rbus/emmc_wrap_reg.h>

#ifndef _CR_REG_H_INCLUDED_
#define _CR_REG_H_INCLUDED_


/* Register Definition (head only) */
#define SD_CONFIGURE1               (0x000UL)     //0xb8010800
#define SD_CONFIGURE2               (0x004UL)     //0xb8010804
#define SD_CONFIGURE3               (0x008UL)     //0xb8010808
#define SD_STATUS                   (0x00CUL)     //0xb801080C
#define SD_STATUS2                  (0x010UL)     //0xb8010810
#define SD_BUS_STATUS               (0x014UL)     //0xb8010814
#define SD_CMD_CODE                 (0x018UL)     //0xb8010818
#define SD_SAMPLE_POINT_CTL         (0x01CUL)     //0xb801081C
#define SD_PUSH_POINT_CTL           (0x020UL)     //0xb8010820
#define SD_CMD0                     (0x024UL)     //0xb8010824
#define SD_CMD1                     (0x028UL)     //0xb8010828
#define SD_CMD2                     (0x02CUL)     //0xb801082C
#define SD_CMD3                     (0x030UL)     //0xb8010830
#define SD_CMD4                     (0x034UL)     //0xb8010834
#define SD_CMD5                     (0x038UL)     //0xb8010838
#define SD_BYTE_CNT_L               (0x03CUL)     //0xb801083C
#define SD_BYTE_CNT_H               (0x040UL)     //0xb8010840
#define SD_BLOCK_CNT_L              (0x044UL)     //0xb8010844
#define SD_BLOCK_CNT_H              (0x048UL)     //0xb8010848
#define SD_TRANSFER                 (0x04CUL)     //0xb801084C
#define SD_DDR_CHK_START            (0x050UL)     //0xb8010850
#define SD_CMD_STATE                (0x054UL)     //0xb8010854
#define SD_DATA_STATE               (0x058UL)     //0xb8010858
#define SD_BUS_TA_TIME              (0x05CUL)     //0xb801085C
#define SD_STOP_SDCLK_CFG           (0x060UL)     //0xb8010860
#define SD_AUTO_RST_FIFO            (0x064UL)     //0xb8010864
#define SD_DAT_PAD                  (0x068UL)     //0xb8010868
#define SD_DUMMY_4                  (0x06CUL)     //0xb801086C
#define SD_DUMMY_5                  (0x070UL)     //0xb8010870
#define SD_DUTY_CTL                 (0x074UL)     //0xb8010874
#define SD_RW_STOP_CTL              (0x078UL)     //0xb8010878
#define SD_DUMMY_3                  (0x07CUL)     //0xb801087C
#define SD_ADDR_L                   (0x080UL)     //0xb8010880
#define SD_ADDR_H                   (0x084UL)     //0xb8010884
#define SD_START_ADDR_0             (0x088UL)     //0xb8010888
#define SD_START_ADDR_1             (0x08CUL)     //0xb801088C
#define SD_START_ADDR_2             (0x090UL)     //0xb8010890
#define SD_START_ADDR_3             (0x094UL)     //0xb8010894
#define SD_RSP_MASK_1               (0x098UL)     //0xb8010898
#define SD_RSP_MASK_2               (0x0A0UL)     //0xb80108A0
#define SD_RSP_MASK_3               (0x0A4UL)     //0xb80108A4
#define SD_RSP_MASK_4               (0x0A8UL)     //0xb80108A8
#define SD_RSP_DATA_1               (0x0ACUL)     //0xb80108AC
#define SD_RSP_DATA_2               (0x0B0UL)     //0xb80108B0
#define SD_RSP_DATA_3               (0x0B4UL)     //0xb80108B4
#define SD_RSP_DATA_4               (0x0B8UL)     //0xb80108B8
#define SD_WR_DELAY                 (0x0BCUL)     //0xb80108BC
#define SD_RD_DELAY                 (0x0C0UL)     //0xb80108C0
#define CR_DMA_PTR_STATUS           (0x0D0UL)     //0xb80108D0
#define CR_DMA_IN_ADDR              (0x0D4UL)     //0xb80108D4
#define CR_DMA_OUT_ADDR             (0x0D8UL)     //0xb80108D8
#define CR_INT_STATUS               (0x0DCUL)     //0xb80108DC
#define CR_INT_MASK                 (0x0E0UL)     //0xb80108E0
#define CR_REG_MODE_CTRL            (0x0E4UL)     //0xb80108E4
#define CR_DES_MODE_CTRL            (0x0E8UL)     //0xb80108E8
#define DES_CMD_WR_PORT             (0x0ECUL)     //0xb80108EC
#define CR_BIST_0                   (0x0F0UL)     //0xb80108F0
#define CR_BIST_1                   (0x0F4UL)     //0xb80108F4
#define CR_BIST_MODE                (0x0F8UL)     //0xb80108F8
#define CR_BIST_DONE                (0x0FCUL)     //0xb80108FC
#define CR_BIST_FAIL                (0x100UL)     //0xb8010900
#define CR_BIST_DRF_MODE            (0x104UL)     //0xb8010904
#define CR_BIST_DRF_RESUME          (0x108UL)     //0xb8010908
#define CR_BIST_DRF_DONE            (0x10CUL)     //0xb801090C
#define CR_BIST_DRF_PAUSE           (0x110UL)     //0xb8010910
#define CR_BIST_DRF_FAIL            (0x114UL)     //0xb8010914
#define CR_CTRL                     (0x118UL)     //0xb8010918
#define ARB_STS                     (0x11CUL)     //0xb801091C
#define SRAM_DATA                   (0x120UL)     //0xb8010920
#define CLK_PHASE_SEL               (0x124UL)     //0xb8010924
#define RSP_17B_0                   (0x128UL)     //0xb8010928
#define RSP_17B_1                   (0x12CUL)     //0xb801092C
#define RSP_17B_2                   (0x130UL)     //0xb8010930
#define RSP_17B_3                   (0x134UL)     //0xb8010934
#define RSP_17B_4                   (0x138UL)     //0xb8010938
#define RSP_17B_5                   (0x13CUL)     //0xb801093C
#define RSP_17B_6                   (0x140UL)     //0xb8010940
#define RSP_17B_7                   (0x144UL)     //0xb8010944
#define KCPU_SEL                    (0x148UL)     //0xb8010948
#define PADDR_START                 (0x14CUL)     //0xb801094C
#define PADDR_END                   (0x150UL)     //0xb8010950
#define CR_INT_STATUS_K             (0x154UL)     //0xb8010954
#define CR_INT_MASK_K               (0x158UL)     //0xb8010958
#define KCPU_ST                     (0x15CUL)     //0xb801095C
#define SCPU_SEL                    (0x170UL)     //0xb8010970
#define CPU_ST                      (0x174UL)     //0xb8010974
#define CR_INT_STATUS_FREE          (0x178UL)     //0xb8010978
#define PIN_CTRL                    (0x17CUL)     //0xb801097C

/* extra define */
#define CR_CARD_RESP6_0             SD_CMD0       //0xb8010824
#define CR_CARD_RESP6_1             SD_CMD1       //0xb8010828
#define CR_CARD_RESP6_2             SD_CMD2       //0xb801082C
#define CR_CARD_RESP6_3             SD_CMD3       //0xb8010830
#define CR_CARD_RESP6_4             SD_CMD4       //0xb8010834
#define CR_CARD_RESP6_5             SD_CMD5       //0xb8010838

/* register item define */
// SD_CONFIGURE1 0xb8010800
#define CLOCK_DIV_SHT               (6)
#define MASK_CLOCK_DIV              (0x3UL<<CLOCK_DIV_SHT)
#define CLOCK_DIV_NON               (0x0UL<<CLOCK_DIV_SHT)
#define CLOCK_DIV_256               (0x3UL<<CLOCK_DIV_SHT)
#define CLOCK_DIV_128               (0x2UL<<CLOCK_DIV_SHT)
#define SD30_ASYNC_FIFO_RST_N       (4)
#define Mode_SEL_SHT                (2)
#define MASK_Mode_SEL               (0x03UL<<Mode_SEL_SHT)
#define Mode_SEL_SD20               (0x00UL<<Mode_SEL_SHT)
#define Mode_SEL_DDR                (0x01UL<<Mode_SEL_SHT)
#define Mode_SEL_SD30               (0x02UL<<Mode_SEL_SHT)
#define MASK_BUS_WIDTH              (0x00000003UL)
#define BUS_WIDTH_1                 (0x00000000UL)
#define BUS_WIDTH_4                 (0x00000001UL)
#define BUS_WIDTH_8                 (0x00000002UL)

// SD_CONFIGURE2 0xb8010804
#define CRC7_CAL_DIS                (0x01UL<<7)
#define CRC16_CAL_DIS               (0x01UL<<6)
#define WAIT_WRT_CRC_TIMEOUT_EN     (0x01UL<<5)
#define IGNORE_WRT_CRC_ERR_EN       (0x01UL<<4)
#define WAIT_BUSY_EN                (0x01UL<<3)
#define CRC7_CHK_DIS                (0x01UL<<2)
#define RESP_TYPE_SHT               (0x00UL)
#define MASK_RESP_TYPE              (0x03UL<<RESP_TYPE_SHT)
#define RESP_TYPE_NON               (0x00UL<<RESP_TYPE_SHT)
#define RESP_TYPE_6B                (0x01UL<<RESP_TYPE_SHT)
#define RESP_TYPE_17B               (0x02UL<<RESP_TYPE_SHT)

// SD_CONFIGURE3 0xb8010808
#define DUMMY_REG0_SHT              (1)
#define MASK_DUMMY_REG0             (0xFEUL<<DUMMY_REG0_SHT)
#define CMD_RESP_TIMEOUT_EN         (0x01UL)

// SD_STATUS 0xb801080C
#define CRC7_STATUS                 (0x01UL<<7)
#define CRC16_STATUS                (0x01UL<<6)
#define WRT_ERR_BIT                 (0x01UL<<5)
#define WRT_ERR_STA_SHT             (2)
#define MASK_WRT_ERR_STA            (0x00000007UL<<WRT_ERR_STA_SHT)
#define WRT_STA_ACCEPT              (0x02UL<<WRT_ERR_STA_SHT)
#define WRT_STA_REJECT_CRC_ERR      (0x05UL<<WRT_ERR_STA_SHT)
#define WRT_STA_REJECT_WRT_ERR      (0x07UL<<WRT_ERR_STA_SHT)
#define WRT_CRC_TIMEOUT_ERR         (0x01UL<<1)
#define PATTERN_COMPARE_ERR         (0x01UL<<0)
#define MASK_SD_STAT_ERR            (CRC7_STATUS|CRC16_STATUS|WRT_ERR_BIT|WRT_CRC_TIMEOUT_ERR|PATTERN_COMPARE_ERR)

// SD_STATUS2 0xb8010810
#define RESP_TIMEOUT_ERR            (0x01UL<<0)

// SD_BUS_STATUS 0xb8010814
#define CLK_TOGGLE_EN               (0x01UL<<7)
#define CLK_TOGGLE_DIS_WHEN_ACT     (0x01UL<<6)
#define DAT3_LEVEL                  (0x01UL<<4)
#define DAT2_LEVEL                  (0x01UL<<3)
#define DAT1_LEVEL                  (0x01UL<<2)
#define DAT0_LEVEL                  (0x01UL<<1)
#define CMD_LEVEL                   (0x01UL<<0)

//SD_CMD_CODE          0xb8010818
#define DUMMY_REG1_SHT              (0)
#define MASK_DUMMY_REG1             (0xFEUL<<DUMMY_REG1_SHT)


//SD_SAMPLE_POINT_CTL  0xb801081C
#define SAMPLE_PT_TYP_DDR_DAT       (0x01UL<<7)
#define SAMPLE_PT_SEL_DDR_DAT       (0x01UL<<6)
#define SAMPLE_PT_TYP_DDR_CMD       (0x01UL<<5)
#define SAMPLE_PT_SEL_DDR_CMD       (0x01UL<<4)
#define SAMPLE_PT_SEL_SD20          (0x01UL<<3)

//SD_PUSH_POINT_CTL    0xb8010820
#define PUSH_PT_SEL_TYPE_DDR        (0x01UL<<7)
#define CMD_OR_DAT                  (0x01UL<<6)
#define PUSH_PT_SEL_DDR_DAT         (0x01UL<<5)
#define PUSH_PT_SEL_DDR_CMD         (0x01UL<<4)
#define PUSH_PT_SEL_SD20            (0x01UL<<3)
#define CLK_SOLUTION_DDR_SHT        (1)
#define MASK_CLK_SOLUTION_DDR       (0x03UL<<CLK_SOLUTION_DDR_SHT)
#define ANALOG_SOLUTION             (0x00UL<<CLK_SOLUTION_DDR_SHT)
#define DRIVER_SOLUTION             (0x02UL<<CLK_SOLUTION_DDR_SHT)
#define PROHIBITED                  (0x03UL<<CLK_SOLUTION_DDR_SHT)
#define CLK_SWAP                    (0x01UL<<0)

//SD_TRANSFER          0xb801084C
#define START_EN                    (0x01UL<<7)
#define END_STATE                   (0x01UL<<6)
#define STATE_MACH_IDLE             (0x01UL<<5)
#define ERR_BIT                     (0x01UL<<4)

#define CMD_CODE_SHT                (0)
#define MASK_CMD_CODE               (0x0F<<CMD_CODE_SHT)
#define SD_NORMALWRITE              (0x00<<CMD_CODE_SHT)
#define SD_AUTOWRITE3               (0x01<<CMD_CODE_SHT)
#define SD_AUTOWRITE4               (0x02<<CMD_CODE_SHT)
#define SD_AUTOREAD3                (0x05<<CMD_CODE_SHT)
#define SD_AUTOREAD4                (0x06<<CMD_CODE_SHT)
#define SD_SENDCMDGETRSP            (0x08<<CMD_CODE_SHT)
#define SD_AUTOWRITE1               (0x09<<CMD_CODE_SHT)
#define SD_AUTOWRITE2               (0x0A<<CMD_CODE_SHT)
#define SD_NORMALREAD               (0x0C<<CMD_CODE_SHT)
#define SD_AUTOREAD1                (0x0D<<CMD_CODE_SHT)
#define SD_AUTOREAD2                (0x0E<<CMD_CODE_SHT)
#define SD_TUNING                   (0x0F<<CMD_CODE_SHT)
#define SD_CMD_UNKNOW               (0xFF<<CMD_CODE_SHT)

//SD_DDR_CHK_START     0xb8010850
#define DET_CMD_ERR                 (0x01UL<<7)
#define FORCE_CMD1                  (0x01UL<<6)
#define FORCE_CMD0                  (0x01UL<<5)
#define CMD_START_BIT               (0x01UL<<4)
#define DET_DATA_ERR                (0x01UL<<3)
#define FORCE_DATA1                 (0x01UL<<2)
#define FORCE_DATA0                 (0x01UL<<1)
#define DATA_START_BIT              (0x01UL<<0)

//SD_CMD_STATE         0xb8010854
#define CMD_STATE_MACH_IDLE         (0x01UL<<7)
#define CMD_STATE_MACH_SHT          (0)
#define MASK_CMD_STATE_MACH         (0x0fUL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_IDLE           (0x00UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_INIT           (0x08UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_TX             (0x09UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_TXCRC          (0x0aUL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_TXE            (0x0bUL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_WAIT           (0x01UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_RX             (0x02UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_RXCEC          (0x03UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_RXE            (0x04UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_END            (0x05UL<<CMD_STATE_MACH_SHT)
#define CMD_STA_MACH_BUSY           (0x0dUL<<CMD_STATE_MACH_SHT)

//SD_DATA_STATE        0xb8010858
#define DATA_STATE_MACH_IDLE        (0x01UL<<7)
#define DATA_STATE_MACH_SHT         (0)
#define MASK_DATA_STATE_MACH        (0x0fUL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_IDLE          (0x00UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_INIT          (0x08UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_TX            (0x09UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_TXCRC         (0x0aUL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_TXE           (0x0bUL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_WAIT          (0x01UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_RX            (0x02UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_RXCEC         (0x03UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_RXE           (0x04UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_END           (0x05UL<<CMD_STATE_MACH_SHT)
#define DATA_STA_MACH_BUSY          (0x0dUL<<CMD_STATE_MACH_SHT)

//SD_BUS_TA_TIME       0xb801085C
#define TURN_AROUND_TIME_EN         (0x01UL<<3)
#define BUS_TA_TIME_SHT             (0)
#define MASK_TA_TIME                (0x07UL<<BUS_TA_TIME_SHT)

//SD_STOP_SDCLK_CFG    0xb8010860
#define STOP_SD20_CLK_CFG_EN        (0x01UL<<5)
#define STOP_SD20_CLK_CFG1          (0x01UL<<4)
#define STOP_SD20_CLK_CFG0          (0x01UL<<3)
#define STOP_SD30_CLK_CFG_EN        (0x01UL<<2)
#define STOP_SD30_CLK_CFG1          (0x01UL<<1)
#define STOP_SD30_CLK_CFG0          (0x01UL<<0)

//SD_AUTO_RST_FIFO     0xb8010864
#define AUTO_RST_FIFO_EN            (0x01UL<<0)

//SD_DAT_PAD           0xb8010868
#define AUTO_RST_FIFO_EN            (0x01UL<<0)

//SD_RW_STOP_CTL       0xb8010878
#define INFINITE_MODE_EN            (0x01UL<<2)
#define CONTROLLER_BUSY             (0x01UL<<1)
#define CONTROLLER_STOP             (0x01UL<<0)

//SD_WR_DELAY          0xb80108BC
#define WRITE_DELAY_SHT             (0)
#define MASK_WRITE_DELAY            (0x0fUL<<WRITE_DELAY_SHT)

//SD_RD_DELAY          0xb80108C0
#define READ_DELAY_SHT              (0)
#define MASK_READ_DELAY             (0x0fUL<<READ_DELAY_SHT)

//CR_DMA_PTR_STATUS    0xb80108D0
#define DMA_OUT_IDX_SHT             (16)
#define MASK_DMA_OUT_IDX            (0x3F<<DMA_OUT_IDX_SHT)
#define DMA_IN_IDX_SHT              (0)
#define MASK_DMA_IN_IDX             (0x3F<<DMA_IN_IDX_SHT)

//CR_INT_STATUS        0xb80108DC
#define RTKCR_INT_STOP_BY_KCPU      (0x01UL<<6)
#define RTKCR_INT_WRT_PROTEC        (0x01UL<<5)
#define RTKCR_INT_CARD_DETEC        (0x01UL<<4)
#define RTKCR_INT_DECODE_ERROR      (0x01UL<<2)
#define RTKCR_INT_DECODE_FINISH     (0x01UL<<1)
#define RTKCR_INT_STATUS_CHANGE     (0x01UL<<0)

#define RTKCR_INT_DEC_STA      (RTKCR_INT_DECODE_ERROR|RTKCR_INT_DECODE_FINISH)
#define RTKCR_INT_CARD_STA     (RTKCR_INT_CARD_DETEC|RTKCR_INT_STATUS_CHANGE)
#define RTKCR_INT_ALL_STA      (RTKCR_INT_DEC_STA|RTKCR_INT_CARD_STA)


//CR_INT_MASK          0xb80108E0
#define RTKCR_CARD_DET_MASK         (0x01UL<<3)
#define RTKCR_DECODE_ERR_MASK       (0x01UL<<2)
#define RTKCR_DECODE_FINISH_MASK    (0x01UL<<1)
#define RTKCR_CARD_STATUS_MASK      (0x01UL<<0)

#define RTKCR_INT_DEC_MASK      (RTKCR_DECODE_ERR_MASK|RTKCR_DECODE_FINISH_MASK)
#define RTKCR_INT_CARD_MASK     (RTKCR_CARD_DET_MASK|RTKCR_CARD_STATUS_MASK)
#define RTKCR_INT_ALL_MASK      ( RTKCR_INT_DEC_MASK|RTKCR_INT_CARD_MASK)

//CR_REG_MODE_CTRL     0xb80108E4
#define REG_STOP                    (0x01UL<<3)

//CR_CTRL              0xb8010918
#define ASIC_DBG_SEL_SHT            (24)
#define MASK_ASIC_DBG_SEL           (0x0fUL<<ASIC_DBG_SEL_SHT)
#define DBG_SEL_SHT                 (12)
#define MASK_DBG_SEL                (0xffUL<<DBG_SEL_SHT)
#define WRAP_DBG_SEL_SHT            (6)
#define MASK_WRAP_DBG_SEL           (0x03fUL<<WRAP_DBG_SEL_SHT)
#define MAP_SEL                     (0x01fUL<<5)     // data=0x12345678; 0: 0x8765,0x4321; 1:0x5678,0x1234
#define MASK_XFER_DIR               (0x01UL<<4)
#define DIR_READ                    (0x01UL<<4)
#define DIR_WRITE                   (0x00UL<<4)
#define BOOT_FROM_CR                (0x01UL<<3)
#define BUFF_TIMING_SHT             (1)
#define MASK_BUFF_TIMING            (0x03UL<<BUFF_TIMING_SHT)   //if value bigger the delay longer
#define BUFF_TIMING_0               (0x00UL<<BUFF_TIMING_SHT)
#define BUFF_TIMING_1               (0x01UL<<BUFF_TIMING_SHT)
#define BUFF_TIMING_2               (0x02UL<<BUFF_TIMING_SHT)
#define BUFF_TIMING_3               (0x03UL<<BUFF_TIMING_SHT)
#define CP_EN                       (0x01UL)

//CLK_PHASE_SEL        0xb8010924
#define PUSH_PHASE_SEL_SHT          (16)
#define MASK_PUSH_PHASE_SEL         (0xffUL<<PUSH_PHASE_SEL_SHT)
#define SAMP_PHASE_SEL_SHT          (0)
#define MASK_SAMP_PHASE_SEL         (0xffUL<<SAMP_PHASE_SEL_SHT)

//KCPU_SEL             0xb8010948
#define KCPU_LOCK_SRAM_MODE         (0x01UL<<3)
#define KCPU_FORCE_EN               (0x01UL<<2)
#define PR_CHK_EN                   (0x01UL<<1)
#define K_CPU_SEL                   (0x01UL<<0)

//CR_INT_STATUS_K      0xb8010954
#define INT_PR_HIT_K                (0x01UL<<3)
#define INT_DECODE_ERR_K            (0x01UL<<2)
#define INT_DECODE_FINISH_K         (0x01UL<<1)

//CR_INT_MASK_K        0xb8010958
#define PR_HIT_MASK_K               (0x01UL<<3)
#define DECODE_ERR_MASK_K           (0x01UL<<2)
#define DECODE_FINISH_MASK_K        (0x01UL<<1)

//KCPU_ST              0xb801095C
#define K_CPU_ST_SHT                (0)
#define MASK_K_CPU_ST               (0x03UL<<K_CPU_ST_SHT)

//SCPU_SEL             0xb8010970
#define S_CPU_SEL                   (0x01UL<<0)

//CPU_ST               0xb8010974
#define S_CPU_ST_SHT                (0)
#define MASK_S_CPU_ST               (0x03UL<<S_CPU_ST_SHT)
#define S_CPU_ST                    (0x01UL<<0)

//CR_INT_STATUS_FREE   0xb8010978
#define INT_STOP_BY_KCPU_FREE       (0x01UL<<6)
#define INT_WRT_PROTEC_FREE         (0x01UL<<5)
#define INT_CARD_DETEC_FREE         (0x01UL<<4)
#define INT_DECODE_ERROR_FREE       (0x01UL<<2)
#define INT_DECODE_FINISH_FREE      (0x01UL<<1)
#define INT_STATUS_CHANGE_FREE      (0x01UL<<0)

//PIN_CTRL             0xb801097C
#define CLK_INV                     (0x01UL<<16)
#define SEL_TAB_SHT                 (11)
#define MASK_SEL_TAB                (0x1fUL<<SEL_TAB_SHT)
#define SOURCE_SEL                  (0x01UL<<10)
#define PAD_PHASE_SEL_SHT           (0)
#define MASK_PAD_PHASE_SEL          (0xff<PAD_PHASE_SEL_SHT)
#define PAD_PHASE_DELAY0            (0x01<<7)
#define PAD_PHASE_DELAY1            (0x01<<6)
#define PAD_PHASE_DELAY2            (0x01<<5)
#define PAD_PHASE_DELAY3            (0x01<<4)
#define PAD_PHASE_DELAY4            (0x01<<3)
#define PAD_PHASE_DELAY5            (0x01<<2)
#define PAD_PHASE_DELAY6            (0x01<<1)
#define PAD_PHASE_DELAY7            (0x01<<0)

#endif



