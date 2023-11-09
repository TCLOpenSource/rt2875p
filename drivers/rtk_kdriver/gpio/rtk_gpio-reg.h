#ifndef __RTK_GPIO_REG_H__
#define __RTK_GPIO_REG_H__

#include <rtk_kdriver/io.h>

//////////////////////////////////////////////////////////////////////////////////
// register access apis
//////////////////////////////////////////////////////////////////////////////////

#define gpio_ioread32(reg)          rtd_inl(reg)
#define gpio_iowrite32(reg, val)    rtd_outl(reg, val)
#define gpio_iowrite32_protect(reg, val)    {rtd_outl(reg, (val|0x1));rtd_outl(reg, ~(val|0x1));}
#define ioread_reg_bit(reg, bits)   ((gpio_ioread32(reg) >> (bits)) & 0x1)

void iowrite_reg_bit(unsigned long reg, unsigned char bit,unsigned char val);

//////////////////////////////////////////////////////////////////////////////////
// register wrapper
//////////////////////////////////////////////////////////////////////////////////


#include <rbus/mis_gpio_reg.h>
#include <rbus/iso_misc_reg.h>
#ifdef CONFIG_ARCH_RTK2885P
#include <rbus/iso_gpio_reg.h>
#include <rbus/mio_gpio_reg.h>
#else 
#include <rbus/gpio_reg.h>
#endif



#ifdef CONFIG_RTK_KDRV_MIO

#if  defined(CONFIG_ARCH_RTK6702)
#include <rbus/MIO/mio_gpio_reg.h>
#elif defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
#include <rbus/mio_gpio_reg.h>
#elif  defined(CONFIG_ARCH_RTK2885P)
#include <rbus/iso_gpio_mio_reg.h>
#include <rbus/mio_gpio_reg.h>

#endif


#endif

#define IRQ_ISO                         (IRQ_CEC)

#define MIS_ISR_reg                     MISC_isr_reg
#define MIS_ISR_assert_int_shift        (MISC_isr_gpio_int_shift)
#define MIS_ISR_gpio_int_shift          (MISC_isr_gpio_int_shift)
#define MIS_UMSK_ISR_GP0A_reg           MIS_GPIO_ISR_GP0A_reg
#define MIS_UMSK_ISR_GP0DA_reg          MIS_GPIO_ISR_GP0DA_reg
#define MIS_GP0IE_reg                   MIS_GPIO_GP0IE_reg
#define MIS_GP0DIR_reg                  MIS_GPIO_GP0DIR_reg
#define MIS_GP0DP_reg                   MIS_GPIO_GP0DP_reg
#define MIS_GP0DATI_reg                 MIS_GPIO_GP0DATI_reg
#define MIS_GP0DATO_reg                 MIS_GPIO_GP0DATO_reg
#define MIS_GPDEB_reg                   MIS_GPIO_GPDEB_reg
#define MIS_GPDEB_1_reg                 MIS_GPIO_GPDEB_1_reg
#define MIS_GP_INT_SEL_reg              MIS_GPIO_GP_INT_SEL_reg
#define MIS_GPI_DATI_reg                MIS_GPIO_GPI0_DATI_reg
#define MIS_GPI_GPIE_reg                MIS_GPIO_GPI0_GPIE_reg
#define MIS_GPI_DP_reg                  MIS_GPIO_GPI0_DP_reg
#define MIS_ISR_GPI_ASSERT_reg          MIS_GPIO_ISR_GPI0_ASSERT_reg
#define MIS_ISR_GPI_DEASSERT_reg        MIS_GPIO_ISR_GPI0_DEASSERT_reg

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
#define MIS_GPO_DATO_reg                MISC_GPIO_GPO_DATO_VADDR_reg
#endif

#ifdef CONFIG_ARCH_RTK2885P
#define ISO_ISR_reg                     ISO_MISC_ISR_reg
#define ISO_ISR_assert_int_shift        (ISO_MISC_ISR_gpio_int_scpu_shift)
#define ISO_ISR_gpio_int_shift          (ISO_MISC_ISR_gpio_int_scpu_shift)
#define ISO_ISR_GP0A_reg                ISO_GPIO_ISR_GP_ASSERT_reg
#define ISO_ISR_GP0DA_reg               ISO_GPIO_ISR_GP_DEASSERT_reg
#define ISO_GPIE_reg                    ISO_GPIO_GPIE_reg
#define ISO_GPDIR_reg                   ISO_GPIO_GPDIR_reg
#define ISO_GPDP_reg                    ISO_GPIO_GPDP_reg
#define ISO_GPDATI_reg                  ISO_GPIO_GPDATI_reg
#define ISO_GPDATO_reg                  ISO_GPIO_GPDATO_reg
#define ISO_GPDEB_reg                   ISO_GPIO_GPDEB_reg
#define ISO_GP_INT_SEL_reg              ISO_GPIO_GP_INT_SEL_reg
#define ISO_GPINT_EMCU_reg              ISO_GPIO_GPINT_EMCU_reg
#define ISO_GPINT_SCPU_reg              ISO_GPIO_GPINT_SCPU_reg
#define ISO_GPINT_KCPU_reg              ISO_GPIO_GPINT_KCPU_reg

#else
#define ISO_ISR_reg                     ISO_MISC_ISR_reg
#define ISO_ISR_assert_int_shift        (ISO_MISC_ISR_gpio_int_scpu_shift)
#define ISO_ISR_gpio_int_shift          (ISO_MISC_ISR_gpio_int_scpu_shift)
#define ISO_ISR_GP0A_reg                GPIO_ISR_GP_ASSERT_reg
#define ISO_ISR_GP0DA_reg               GPIO_ISR_GP_DEASSERT_reg
#define ISO_GPIE_reg                    GPIO_GPIE_reg
#define ISO_GPDIR_reg                   GPIO_GPDIR_reg
#define ISO_GPDP_reg                    GPIO_GPDP_reg
#define ISO_GPDATI_reg                  GPIO_GPDATI_reg
#define ISO_GPDATO_reg                  GPIO_GPDATO_reg
#define ISO_GPDEB_reg                   GPIO_GPDEB_reg
#define ISO_GP_INT_SEL_reg              GPIO_GP_INT_SEL_reg
#define ISO_GPINT_EMCU_reg              GPIO_GPINT_EMCU_reg
#define ISO_GPINT_SCPU_reg              GPIO_GPINT_SCPU_reg
#define ISO_GPINT_KCPU_reg              GPIO_GPINT_KCPU_reg
#endif



#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#define GPI_MIS_ISR_reg                 MISC_isr_reg
#define GPI_MIS_ISR_assert_int_shift   (MISC_isr_gpio_int_shift)
#define GPI_MIS_ISR_int_shift          (MISC_isr_gpio_int_shift)
#define GPI_MIS_DATI_reg                MIS_GPIO_GPI0_DATI_reg
#define GPI_MIS_GPIE_reg                MIS_GPIO_GPI0_GPIE_reg
#define GPI_MIS_DP_reg                  MIS_GPIO_GPI0_DP_reg
#define GPI_MIS_ISR_ASSERT_reg          MIS_GPIO_ISR_GPI0_ASSERT_reg
#define GPI_MIS_ISR_DEASSERT_reg        MIS_GPIO_ISR_GPI0_DEASSERT_reg
#endif //RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
#define GPI_ISO_ISR_reg                 ISO_MISC_ISR_reg
#define GPI_ISO_ISR_assert_int_shift   (ISO_MISC_ISR_gpio_int_scpu_shift)
#define GPI_ISO_ISR_int_shift          (ISO_MISC_ISR_gpio_int_scpu_shift)
#define GPI_ISO_DATI_reg                GPIO_GPI_DATI_reg
#define GPI_ISO_GPIE_reg                GPIO_GPI_GPIE_reg
#define GPI_ISO_DP_reg                  GPIO_GPI_DP_reg
#define GPI_ISO_ISR_ASSERT_reg          GPIO_ISR_GPI_ASSERT_reg
#define GPI_ISO_ISR_DEASSERT_reg        GPIO_ISR_GPI_DEASSERT_reg
#endif //RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
#define ISO_GPO_DATO_reg                ISO_MISC_GPIO_GPO_DATO_VADDR_reg
#endif

#if  defined(CONFIG_RTK_KDRV_MIO)
#if defined(CONFIG_ARCH_RTK6702) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
#define MIO_ISR_reg                     ISO_MISC_ISR_reg
#define MIO_ISR_assert_int_shift        (ISO_MISC_ISR_gpio_int_scpu_shift)
#define MIO_ISR_gpio_int_shift          (ISO_MISC_ISR_gpio_int_scpu_shift)
#define MIO_ISR_GP0A_reg                MIO_GPIO_ISR_GP_ASSERT_reg
#define MIO_ISR_GP0DA_reg               MIO_GPIO_ISR_GP_DEASSERT_reg
#define MIO_GPIE_reg                    MIO_GPIO_GPIE_reg
#define MIO_GPIE_DA_reg                 MIO_GPIO_GPIE_DA_reg
#define MIO_GPDIR_reg                   MIO_GPIO_GPDIR_reg
#define MIO_GPDP_reg                    MIO_GPIO_GPDP_reg
#define MIO_GPDATI_reg                  MIO_GPIO_GPDATI_reg
#define MIO_GPDATO_reg                  MIO_GPIO_GPDATO_reg
#define MIO_GPDEB_reg                   MIO_GPIO_GPDEB_reg
#define MIO_GP_INT_SEL_reg              MIO_GPIO_GP_INT_SEL_reg
#define MIO_GPINT_EMCU_reg              MIO_GPIO_GPINT_EMCU_reg
#define MIO_GPINT_SCPU_reg              MIO_GPIO_GPINT_SCPU_reg
#elif  defined(CONFIG_ARCH_RTK2885P)

#define MIO_ISR_reg                     ISO_MISC_ISR_reg
#define MIO_ISR_assert_int_shift        (ISO_MISC_ISR_gpio_int_scpu_shift)
#define MIO_ISR_gpio_int_shift          (ISO_MISC_ISR_gpio_int_scpu_shift)
#define MIO_ISR_GP0A_reg                MIO_GPIO_ISR_GP_ASSERT_reg
#define MIO_ISR_GP0DA_reg               MIO_GPIO_ISR_GP_DEASSERT_reg
#define MIO_GPIE_reg                    MIO_GPIO_GPIE_reg
#define MIO_GPIE_DA_reg                 MIO_GPIO_GPIE_DA_reg
#define MIO_GPDIR_reg                   MIO_GPIO_GPDIR_reg
#define MIO_GPDP_reg                    MIO_GPIO_GPDP_reg
#define MIO_GPDATI_reg                  MIO_GPIO_GPDATI_reg
#define MIO_GPDATO_reg                  MIO_GPIO_GPDATO_reg
#define MIO_GPDEB_reg                   MIO_GPIO_GPDEB_reg
#define MIO_GP_INT_SEL_reg              MIO_GPIO_GP_INT_SEL_reg
#define MIO_GPINT_EMCU_reg              MIO_GPIO_GPINT_EMCU_reg
#define MIO_GPINT_SCPU_reg              MIO_GPIO_GPINT_SCPU_reg

#endif
#endif

#ifdef CONFIG_ARCH_RTK2885P

#define ISO_GPIO_GPDEB_HWCNT             ISO_GPIO_GPDEB_reg
#define ISO_GPIO_GPDEB_clk5_SHIFT_HWCNT  ISO_GPIO_GPDEB_clk5_shift
#define ISO_GPIO_GPIE_HWCNT              ISO_GPIO_GPIE_1_reg 
#define ISO_GPIO_GPIE_HWCNT_SGIFT_HWCNT  ISO_GPIO_GPIE_1_gp_hwcnt_shift
#define ISO_GPIO_GPDP_HWCNT                       ISO_GPIO_GPDP_1_reg
#define ISO_GPIO_GPDP_HWCNT_SHIFT_HWCNT ISO_GPIO_GPDP_1_gpha_hwcnt_shift
#define ISO_GPIO_HWCNT_1_HWCNT            ISO_GPIO_HWCNT_1_reg
#define ISO_GPIO_HWCNT_1_HWCNT_MASK_HWCNT  ISO_GPIO_HWCNT_1_hw_cnt_gpio_i_sel_mask
#define ISO_GPIO_HWCNT_1_HWCNT_I_SHIFT_HWCNT  ISO_GPIO_HWCNT_1_hw_cnt_gpio_i_sel_shift
#define ISO_GPIO_HWCNT_1_SWEN_MASK_HWCNT     ISO_GPIO_HWCNT_1_sw_en_mask
#define ISO_GPIO_HWCNT_2_HW_ERROR_CNT_MASK_HWCNT   ISO_GPIO_HWCNT_2_hw_error_cnt_mask
#define ISO_GPIO_HWCNT_1_HWCNT_EDGE_SHIFT_HWCNT       ISO_GPIO_HWCNT_1_hwcnt_edge_sel_shift
#define ISO_GPIO_HWCNT_2_HWCNT ISO_GPIO_HWCNT_2_reg
#else
#define ISO_GPIO_GPDEB_HWCNT             GPIO_GPDEB_reg
#define ISO_GPIO_GPDEB_clk5_SHIFT_HWCNT  GPIO_GPDEB_clk5_shift
#define ISO_GPIO_GPIE_HWCNT              GPIO_GPIE_1_reg 
#define ISO_GPIO_GPIE_HWCNT_SGIFT_HWCNT  GPIO_GPIE_1_gp_hwcnt_shift
#define ISO_GPIO_GPDP_HWCNT                       GPIO_GPDP_1_reg
#define ISO_GPIO_GPDP_HWCNT_SHIFT_HWCNT GPIO_GPDP_1_gpha_hwcnt_shift
#define ISO_GPIO_HWCNT_1_HWCNT            GPIO_HWCNT_1_reg
#define ISO_GPIO_HWCNT_1_HWCNT_MASK_HWCNT  GPIO_HWCNT_1_hw_cnt_gpio_i_sel_mask
#define ISO_GPIO_HWCNT_1_HWCNT_I_SHIFT_HWCNT  GPIO_HWCNT_1_hw_cnt_gpio_i_sel_shift
#define ISO_GPIO_HWCNT_1_SWEN_MASK_HWCNT     GPIO_HWCNT_1_sw_en_mask
#define ISO_GPIO_HWCNT_2_HW_ERROR_CNT_MASK_HWCNT   GPIO_HWCNT_2_hw_error_cnt_mask
#define ISO_GPIO_HWCNT_1_HWCNT_EDGE_SHIFT_HWCNT       GPIO_HWCNT_1_hwcnt_edge_sel_shift
#define ISO_GPIO_HWCNT_2_HWCNT GPIO_HWCNT_2_reg
#endif



#endif /* __RTK_GPIO_REG_H__ */
