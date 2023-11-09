#ifndef __RTK_MIO_GPIO_H__
#define __RTK_MIO_GPIO_H__

void rtk_mio_gpio_disable_irq_all(void);
void rtk_mio_gpio_clear_isr_all(void);
int rtk_mio_gpio_set_dir(RTK_GPIO_ID gid, unsigned char Out);
int rtk_mio_gpio_get_dir(RTK_GPIO_ID gid);
int rtk_mio_gpio_set_irq_polarity(RTK_GPIO_ID gid, unsigned char positive);
int rtk_mio_gpio_set_irq_enable(RTK_GPIO_ID gid, unsigned char On);
int rtk_mio_gpio_chk_irq_enable(RTK_GPIO_ID gid);
int rtk_mio_gpio_set_dis_irq_enable(RTK_GPIO_ID gid, unsigned char On);
int rtk_mio_gpio_chk_dis_irq_enable(RTK_GPIO_ID gid);
int rtk_mio_gpio_clear_isr(RTK_GPIO_ID gid);
int rtk_mio_gpio_set_debounce(RTK_GPIO_ID gid, unsigned char val);
int rtk_mio_gpio_input(RTK_GPIO_ID gid);
int rtk_mio_gpio_output(RTK_GPIO_ID gid, unsigned char val);
int rtk_mio_gpio_output_get(RTK_GPIO_ID gid);

#endif

