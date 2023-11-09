#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/completion.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_otp_region_api.h>
#include "ehci.h"
#include "dwc2.h"
#include "ehci-plat.h"

#define USB_PHY_DELAY                         1
#define INSNREG_VBUSY    (1 << 17)
#define IS_INSNREG_VBUSY(p)           ((p) & INSNREG_VBUSY)

const U2_PHY_REG u2_phy_reg_table[] =
{
        {0, 0xE0},
        {0, 0xE1},
        {0, 0xE2},
        {0, 0xE3},
        {0, 0xE4},
        {0, 0xE5},
        {0, 0xE6},
        {0, 0xE7},
        {1, 0xE0},
        {1, 0xE1},
        {1, 0xE2},
        {1, 0xE3},
        {1, 0xE4},
        {1, 0xE5},
        {1, 0xE6},
        {1, 0xE7},
        {0, 0xF0},
        {0, 0xF1},
        {0, 0xF2},
        {0, 0xF3},
        {0, 0xF4},
        {0, 0xF5},
        {0, 0xF6},
        {0, 0xF7},
        {},
};

/* TODO: Correct wrapper vstatus regs if SoC change */
struct vstatus_reg vstatus_regs[MAX_EHCI_CONTROLLER_NUM];


/* TODO: Add/remove completion if SoC change */
struct ehci_completion ehci_completions[MAX_EHCI_CONTROLLER_NUM];

unsigned long *id_get_ehci_vstatus_regs(struct ehci_platform_priv *priv)
{
        unsigned long *regs = NULL;

        if(priv->id < 0 || priv->id >= MAX_EHCI_CONTROLLER_NUM)
                return regs;

        vstatus_regs[priv->id].id = priv->id;

        regs = vstatus_regs[priv->id].regs;

        /*just support max three ports*/
        regs[0] = priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_REG;
        regs[1] = priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_2PORT_REG;
        regs[2] = priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_3PORT_REG;

        return regs;
}


struct completion *id_get_ehci_completion(struct ehci_platform_priv *priv)
{
        if(priv->id < 0 || priv->id >= MAX_EHCI_CONTROLLER_NUM)
                return NULL;

        ehci_completions[priv->id].id = priv->id;

        init_completion(&ehci_completions[priv->id].phy_mac_completion);
        return &ehci_completions[priv->id].phy_mac_completion;
}


/*********************************EHCI PHY INIT Releated*********************************************/
static int is_invalid_port_num(int n_ports, int port1)
{
        return (port1 < 1 || (port1 > n_ports)) ? -EINVAL : 0;
}

/* We only pass E0~F7 phy_reg, although E0~F7 is just for writing.
 * For reading purpose, the _get_ehci_usb2_phy will do the mapping task.
 * E0 ~ F7 --> C0 ~ D7 */
static int is_invalid_phy_reg(int phy_reg)
{
        if (phy_reg < 0xE0 || phy_reg > 0xF7)
                return -EINVAL;
        return 0;
}
/*-----------------------------------------------------------------
 * U2 Phy Setting API
 * The USB2 Phy registers can be accessed by UTMI interface.
 *-----------------------------------------------------------------*/
/*-----------------------------------------------
 * Func : _get_ehci_usb2_phy
 *
 * Desc : read usb2 phy register with UTMI
 *
 * Parm : ehci      : ehci_hcd
 *        port1     : port number (start from 1)
 *        phy_reg   : phy register address
 *
 * Retn : others : phy register value
 *        -1 : failed
 *-----------------------------------------------*/
static int _get_ehci_usb2_phy(struct ehci_hcd *ehci, int port1, int phy_reg)
{
        struct ehci_platform_priv *priv = (struct ehci_platform_priv *)ehci->priv;
        int n_ports = HCS_N_PORTS(ehci_readl(ehci, &ehci->caps->hcs_params));
        int phy_value;
        int timeout_cnt;

        if (is_invalid_port_num(n_ports, port1))
        {
                ehci_plat_warn("get_usb2_phy failed, invalid port - %d\n", port1);
                return -ENODEV;
        }
        if (is_invalid_phy_reg(phy_reg))
        {
                ehci_plat_warn("get_usb2_phy failed, invalid phy_reg 0x%x\n", phy_reg);
                return -ENXIO;
        }

        ehci_writel(ehci, 0x00020000 | (phy_reg & 0x0f) << 8 | (port1 << 13), &priv->synop_spec_regs->insnreg05);
        ndelay(10);
        timeout_cnt = 10000;
        while (IS_INSNREG_VBUSY(ehci_readl(ehci, &priv->synop_spec_regs->insnreg05)))
        {
                ndelay(10);
                if(--timeout_cnt < 0)
                        break;
        }

        ehci_writel(ehci, 0x00020000 | (phy_reg & 0xd0) << 4 | (port1 << 13), &priv->synop_spec_regs->insnreg05);
        ndelay(10);
        timeout_cnt = 10000;
        while (IS_INSNREG_VBUSY(ehci_readl(ehci, &priv->synop_spec_regs->insnreg05)))
        {
                ndelay(10);
                if(--timeout_cnt < 0)
                        break;
        }

        phy_value = ehci_readl(ehci, &priv->synop_spec_regs->insnreg05) & 0xff;

        return (phy_value & 0xff);
}


/*-----------------------------------------------
 * Func : _set_ehci_usb2_phy
 *
 * Desc : set usb2 phy register with UTMI
 *
 * Parm : ehci      : ehci_hcd
 *        port1     : port number (start from 1)
 *        phy_reg   : phy register address
 *        phy_value : phy register value
 *
 * Retn : 0 : successed,
 *       -1 : failed
 *-----------------------------------------------*/
static int _set_ehci_usb2_phy(struct ehci_hcd *ehci, int port1, int phy_reg, int phy_value)
{
        struct ehci_platform_priv *priv = (struct ehci_platform_priv *)ehci->priv;
        int n_ports = HCS_N_PORTS(ehci_readl(ehci, &ehci->caps->hcs_params));
        int value = 0;
        int timeout_cnt;

        if (is_invalid_port_num(n_ports, port1))
        {
                ehci_plat_warn("set_usb2_phy failed, invalid port - %d\n", port1);
                return -ENODEV;
        }

        if (is_invalid_phy_reg(phy_reg))
        {
                ehci_plat_warn("get_usb2_phy failed, invalid phy_reg 0x%x\n", phy_reg);
                return -ENXIO;
        }

        do
        {
                // write value
                rtd_outl(priv->vstatus_regs[port1 - 1], 
        		(rtd_inl(priv->vstatus_regs[port1 - 1]) & (~0xFF)) | phy_value);
                udelay(USB_PHY_DELAY);

                ehci_writel(ehci, 0x00020000 | (phy_reg & 0x0f) << 8 | (port1 << 13), &priv->synop_spec_regs->insnreg05);
                ndelay(10);
                timeout_cnt = 10000;
                while (IS_INSNREG_VBUSY(ehci_readl(ehci, &priv->synop_spec_regs->insnreg05)))
                {
                        ndelay(10);
                        if(--timeout_cnt < 0)
                                break;
                }

                ehci_writel(ehci, 0x00020000 | (phy_reg & 0xf0) << 4 | (port1 << 13), &priv->synop_spec_regs->insnreg05);
                ndelay(10);
                timeout_cnt = 10000;
                while (IS_INSNREG_VBUSY(ehci_readl(ehci, &priv->synop_spec_regs->insnreg05)))
                {
                        ndelay(10);
                        if(--timeout_cnt < 0)
                                break;
                }

                value = _get_ehci_usb2_phy(ehci, port1, phy_reg);

                if ((phy_value & 0xff) != value)
                        ehci_plat_warn("%s, port %d phy_reg 0x%.2x set 0x%x get 0x%x <--- not matched \n",
                                       __func__, port1, phy_reg, phy_value & 0xff, value);
                else
                        ehci_plat_dbg("%s, port %d phy_reg 0x%.2x set 0x%x get 0x%x\n",
                                      __func__, port1, phy_reg, phy_value & 0xff, value);

        }
        while(0);   /* just write once */

        return 0;
}


/*-----------------------------------------------
 * Func : _usb2_load_phy_setting
 *
 * Desc : load usb2 phy setting
 *
 * Parm : p_reg : register list
 *        n_reg : register value
 *
 * Retn : N/A
 *-----------------------------------------------*/
static int _usb2_load_phy_setting(struct ehci_hcd *ehci, U2_PHY_REGISTER* p_reg, unsigned int n_reg)
{
        int err = 0;
        while(n_reg--)
        {
                //ehci_plat_info(ehci, "ehci port %x %x %x", p_reg->port1, p_reg->addr, p_reg->val);
                err = _set_ehci_usb2_phy(ehci, p_reg->port1, p_reg->addr, p_reg->val);
                if (err)
                        break;
                p_reg++;
        }
        return err;
}


/* ============================================================================
The table of Z0 VS resistor

Z0 gear | resistor
0x6     | 0R~1.5R
0x5     | 1.6R~3R
0x4     | 3.1R~5.1R
============================================================================ */
static unsigned char ehci_z0_resister_2_gear(unsigned char resistor)
{

        unsigned char nible1;
        unsigned char nible2;

        nible1 = (resistor & 0xf0)>>4;
        nible2 = (resistor & 0xf);
        if( (nible1>9) || (nible2>9) )
                return 0;

        nible1 = (nible1*10)+ nible2;
        if(nible1 <= 15)
                return Z0_GEAR(6);
        else if(nible1 <= 30)
                return Z0_GEAR(5);
        else if(nible1 <= 51)
                return Z0_GEAR(4);
        else
                return 0;
}

static int ehci_usb2_adjust_phy_params(struct ehci_hcd *ehci, U2_PHY_REGISTER*p_reg, unsigned int ofst, int count)
{
        int i;
        unsigned long long z0_value_form_uboot = 0;

        ehci_plat_info("Check PCB enum of USB.\n");
        if(pcb_mgr_get_enum_info_byname(Z0_EHCI_PHY_NAME, &z0_value_form_uboot) == 0)
        {
                ehci_plat_info("%s is 0x%0llx", Z0_EHCI_PHY_NAME, z0_value_form_uboot);
        }
        else
        {
                ehci_plat_info("There is no Z0 setting form PCB enum.\n");
                return 0;
        }

        p_reg += ofst;
        for(i=0; i<count; i++)
        {
                unsigned char new_sis = 0;
                unsigned char new_z0 = 0;

                new_sis = (unsigned char)((z0_value_form_uboot >> (8*i))&0xff);
                //ehci_plat_info(ehci, "port%d resistor is %d.%dR.\n",i+1,(new_sis&0xf0)>>4,new_sis&0xf);
                new_z0 = ehci_z0_resister_2_gear(new_sis);
                if(!new_z0)
                {
                        ehci_plat_info("There is no Z0 of port%d.\n",i+1);
                        continue;
                }
                //ehci_plat_info(ehci, "port%d old Z0 is 0x%x,new Z0 is 0x%x\n", i+1, p_reg->val,new_z0);
                p_reg->val = new_z0;
                ehci_plat_info("port%d Z0 is 0x%x", i+1, p_reg->val);

                p_reg++;
        }

        return 0;
}

int set_ehci_usb2_phy_reg(
        struct ehci_hcd     *ehci,
        unsigned char       port1,
        unsigned char       page,
        unsigned char       addr,
        unsigned char       val)
{
        int ret = 0;

        if (page > 3)
        {
                ehci_plat_warn("%s failed, invalid page - %d\n", __func__, page);
                return -ENXIO;
        }

        ret = _set_ehci_usb2_phy(ehci, port1, 0xF4, (page<<5) | 0x9b);  // bit [6:5] : page select = page
        if (ret)
                return ret;

        return _set_ehci_usb2_phy(ehci, port1, addr, val);
}

int get_ehci_usb2_phy_reg (
        struct ehci_hcd    *ehci,
        unsigned char       port1,
        unsigned char       page,
        unsigned char       addr
)
{
        int ret = 0;

        if (page > 3)
        {
                ehci_plat_warn("set_u2_phy_reg failed, invalid page - %d\n", page);
                return -ENXIO;
        }

        ret = _set_ehci_usb2_phy(ehci, port1, 0xF4, (page << 5) | 0x9b);  // bit [6:5] : page select = page
        if (ret)
                return ret;

        return _get_ehci_usb2_phy(ehci, port1, addr);
}

static inline int ehci_usb2_set_phy_from_otp(struct platform_device *pdev)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)

        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct device_node *p_node;
        int i;
        int ret = -1;
        int otp_val_array_size;

        p_node =  pdev->dev.of_node;

        otp_val_array_size = of_property_count_elems_of_size(p_node, "usb2_otp_values", 20);
        if(otp_val_array_size <= 0)
        {
                ehci_plat_info("usb2 no otp info from dts\n");
                return 0;
        }

        for(i  = 0; i < otp_val_array_size; i++)
        {
                unsigned int otp_field_val;
                unsigned int otp_field_done;
                unsigned int otp_offset;
                unsigned int otp_bits;
                unsigned int page;
                unsigned int reg;
                unsigned int bits_offset;
                const char *otp_field_name = NULL;
                const char *otp_filed_done_name = NULL;
                if(of_property_read_string_index(p_node, "usb2_otp_names", 2 * i,  &otp_field_name) != 0)
                {
                        ehci_plat_info("usb2 get otp field name failed\n");
                        goto EXIT;
                }

                if(of_property_read_string_index(p_node, "usb2_otp_names", 2 * i + 1,  &otp_filed_done_name) != 0)
                {
                        ehci_plat_info("usb2 get otp field done name failed\n");
                        goto EXIT;
                }

                if(rtk_otp_field_read_by_name(otp_filed_done_name, (unsigned char *)&otp_field_done, sizeof(otp_field_done)) != true)
                {
                        ehci_plat_err("get %s from otp failed\n", otp_filed_done_name);
                        ret = -EIO;
                        goto EXIT;
                }

                if(!otp_field_done)
                        continue;

                if(rtk_otp_field_read_by_name(otp_field_name, (unsigned char *)&otp_field_val, sizeof(otp_field_val)) != true)
                {
                        ehci_plat_err("get %s from otp failed\n", otp_field_name);
                        ret = -EIO;
                        goto EXIT;
                }

                if(of_property_read_u32_index(p_node, "usb2_otp_values", 5 * i, &otp_offset)
                   || of_property_read_u32_index(p_node, "usb2_otp_values", 5 * i + 1, &otp_bits)
                   || of_property_read_u32_index(p_node, "usb2_otp_values", 5 * i + 2, &page)
                   || of_property_read_u32_index(p_node, "usb2_otp_values", 5 * i + 3, &reg)
                   || of_property_read_u32_index(p_node, "usb2_otp_values", 5 * i + 4, &bits_offset))
                {
                        ret = -EIO;
                        goto EXIT;
                }
                ehci_plat_info("usb2 otp info : name=%s, done_name=%s, done_value=%x,  value=%x, offset=%x, otp_bits=%x, page=%x, reg=%x, bits_offset=%x\n",
                               otp_field_name, otp_filed_done_name, otp_field_done, otp_field_val, otp_offset, otp_bits, page, reg, bits_offset);

                {
                        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
                        int portnum = 0;
                        for (portnum = 0; portnum < priv->usb2_port_cnt; portnum++)
                        {
                                unsigned int val;
                                unsigned int tmp_otp;
                                unsigned int mask = ((1 << otp_bits) - 1);
                                val = get_ehci_usb2_phy_reg(ehci, portnum + 1, page, reg);
                                tmp_otp = otp_field_val >> (portnum * otp_bits + otp_offset);
                                tmp_otp = tmp_otp & mask;
                                /*
                                    If you need to modify the setting value of a port,
                                    change it here
                                */
                                val = (val & ~(mask << bits_offset)) | (tmp_otp << bits_offset);
                                ret = set_ehci_usb2_phy_reg(ehci, portnum + 1, page, reg, val);
                                if (ret)
                                        goto EXIT;
                                ehci_plat_info("set port(%d) swing level to 0x%x \n",
                                               (portnum + 1),
                                               (get_ehci_usb2_phy_reg(ehci, portnum + 1, page, reg) & (mask << bits_offset)) >> bits_offset);
                        }
                }


        }
        ret = 0;
EXIT:
        return ret;
#else
        ehci_plat_warn("CONFIG_RTK_KDRV_OTP not define\n");
        return 0;
#endif
}


int ehci_usb2_phy_init(struct platform_device *pdev)
{
        int ret = 0;
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct device_node *p_node;
        U2_PHY_REGISTER *p_reg_array;
        int array_size ;
        unsigned int Z0_ofst;
        unsigned int Z0_count;
        int i;
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);

        char *propert_name = "usb2phys";

        p_node =  pdev->dev.of_node;
        if (!p_node)
        {
                ehci_plat_err("%s get of device fail\n",__func__);
                return -ENODEV;
        }

        if(of_property_read_u32_index(p_node, "usb2portcnt", 0, &priv->usb2_port_cnt) != 0) 
        {
                ehci_plat_err("%s read usb2portcnt error\n",__func__);
                return -EIO;
        }
        
        if(priv->usb2_port_cnt <= 0)
        {
                ehci_plat_err("%s get error usb2portcnt\n",__func__);
                return -EIO;
        }
        ehci_plat_info("%s get  usb2portcnt:%u\n",__func__, priv->usb2_port_cnt);

        array_size = of_property_count_elems_of_size(p_node, propert_name, 12);
        if(array_size <= 0)
        {
                ehci_plat_warn("%s %s has no value",__func__, propert_name);
                return -ENODEV;
        }
        p_reg_array = (U2_PHY_REGISTER *)kzalloc(sizeof(U2_PHY_REGISTER) * array_size, GFP_KERNEL);
        if(!p_reg_array)
        {
                ehci_plat_warn("%s no memory",__func__);
                return -ENOMEM;
        }
        for(i = 0; i < array_size; i++)
        {
                unsigned int port;
                unsigned int addr;
                unsigned int val;
                if(of_property_read_u32_index(p_node, propert_name, 3 * i, &port)
                   || of_property_read_u32_index(p_node, propert_name, 3 * i + 1, &addr)
                   || of_property_read_u32_index(p_node, propert_name, 3 * i + 2, &val))
                {
                        ret = -EIO;
                        goto EXIT;
                }
                p_reg_array[i].port1= port;
                p_reg_array[i].addr = addr;
                p_reg_array[i].val = val;
        }


        if(of_property_read_u32_index(p_node, "usb2physZ0", 0, &Z0_ofst) == 0
           && of_property_read_u32_index(p_node, "usb2physZ0", 1, &Z0_count) == 0)
        {
                if((Z0_ofst < array_size)
                   && ((Z0_ofst + Z0_count) <= array_size))
                        ehci_usb2_adjust_phy_params(ehci, p_reg_array, Z0_ofst, Z0_count);
        }

        ret= _usb2_load_phy_setting(ehci, p_reg_array, array_size);
        if (ret)
                goto EXIT;


        ret = ehci_usb2_set_phy_from_otp(pdev);
        if (ret)
                goto EXIT;

        ret = 0;

EXIT:
        kfree(p_reg_array);
        return ret;
}


/****************************************OTHER***************************************************/
void ehci_init_wrapper_regs(struct usb_hcd *hcd, unsigned int reg_start)
{
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);

        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_REG = reg_start + 0xA0000800 + 0x0;
        priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_REG = reg_start + 0xA0000800 + 0x4;
        priv->wrapper_regs.EHCI_USB2_TOP_USBIPINPUT_REG = reg_start + 0xA0000800 + 0x8;
        priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_REG = reg_start + 0xA0000800 + 0xC;
        priv->wrapper_regs.EHCI_USB2_TOP_SELF_LOOP_BACK_REG = reg_start + 0xA0000800 + 0x10;
        priv->wrapper_regs.EHCI_USB2_TOP_VERSION_REG = reg_start + 0xA0000800 + 0x14;
        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_2PORT_REG = reg_start + 0xA0000800 + 0x20;
        priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_2PORT_REG = reg_start + 0xA0000800 + 0x24;
        priv->wrapper_regs.EHCI_USB2_TOP_USBIPINPUT_2PORT_REG = reg_start + 0xA0000800 + 0x28;
        priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_2PORT_REG = reg_start + 0xA0000800 + 0x2c;
        priv->wrapper_regs.EHCI_USB2_TOP_SELF_LOOP_BACK_2PORT_REG = reg_start + 0xA0000800 + 0x30;
        priv->wrapper_regs.EHCI_USB2_TOP_IPNEWINPUT_2PORT_REG = reg_start + 0xA0000800 + 0x34;
        priv->wrapper_regs.EHCI_USB2_TOP_USBPHY_SLB0_REG = reg_start + 0xA0000800 + 0x38;
        priv->wrapper_regs.EHCI_USB2_TOP_USBPHY_SLB1_REG = reg_start + 0xA0000800 + 0x3c;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_DUMMY_REG = reg_start + 0xA0000800 + 0x40;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR0_REG = reg_start + 0xA0000800 + 0x48;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR1_REG = reg_start + 0xA0000800 + 0x4c;
        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_DBG1_REG = reg_start + 0xA0000800 + 0x50;
        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_DBG2_REG = reg_start + 0xA0000800 + 0x54;
        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_DBG3_REG = reg_start + 0xA0000800 + 0x58;

        priv->wrapper_regs.EHCI_USB2_TOP_WRAPP_3PORT_REG = reg_start + 0xA0000800 + 0x60;
        priv->wrapper_regs.EHCI_USB2_TOP_VSTATUS_3PORT_REG = reg_start + 0xA0000800 + 0x64;
        priv->wrapper_regs.EHCI_USB2_TOP_USBIPINPUT_3PORT_REG = reg_start + 0xA0000800 + 0x68;
        priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_3PORT_REG = reg_start + 0xA0000800 + 0x6c;
        priv->wrapper_regs.EHCI_USB2_TOP_SELF_LOOP_BACK_3PORT_REG = reg_start + 0xA0000800 + 0x70;
        priv->wrapper_regs.EHCI_USB2_TOP_USBPHY_SLB2_REG = reg_start + 0xA0000800 + 0x74;

        priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR2_REG = reg_start + 0xA0000800 + 0x78;
        priv->wrapper_regs.EHCI_USB2_TOP_SF_TEST_CTR_REG = reg_start + 0xA0000800 + 0x7c;
        priv->wrapper_regs.EHCI_USB2_TOP_PORT_STATUS_REG = reg_start + 0xA0000800 + 0x80;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG = reg_start + 0xA0000800 + 0x84;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_INT_REG = reg_start + 0xA0000800 + 0x88;
        priv->wrapper_regs.EHCI_USB2_TOP_USB_DEBUG_REG = reg_start + 0xA0000800 + 0x8c;
        priv->wrapper_regs.EHCI_USB2_TOP_WRAP_CTR_REG = reg_start + 0xA0000800 + 0x100;
}
