#ifndef __RTK_LED_H__
#define __RTK_LED_H__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>


//Register
#define	LED_1_PWM_RED_3238 	 0x05
#define	LED_1_PWM_GREEN_3238 	 0x09
#define	LED_1_PWM_BLUE_3238	 0x01
#define	LED_2_PWM_RED_3238 	 0x11
#define	LED_2_PWM_GREEN_3238 	 0x15
#define	LED_2_PWM_BLUE_3238 	 0x0D
#define	LED_3_PWM_RED_3238 	 0x1D
#define	LED_3_PWM_GREEN_3238 	 0x21
#define	LED_3_PWM_BLUE_3238 	 0x19
#define	LED_4_PWM_RED_3238 	 0x29
#define	LED_4_PWM_GREEN_3238 	 0x2D
#define	LED_4_PWM_BLUE_3238 	 0x25

#define	UPDATE_PWM_REGISTER_3238 0x49

#define	LED_1_PWM_RED_3206	0x04
#define	LED_1_PWM_GREEN_3206 	0x05
#define	LED_1_PWM_BLUE_3206 	0x06
#define	LED_2_PWM_RED_3206 	0x07
#define	LED_2_PWM_GREEN_3206 	0x08
#define	LED_2_PWM_BLUE_3206     0x09
#define	LED_3_PWM_RED_3206	0x0A
#define	LED_3_PWM_GREEN_3206    0x0B
#define	LED_3_PWM_BLUE_3206     0x0C
#define	LED_4_PWM_RED_3206      0x0D
#define	LED_4_PWM_GREEN_3206    0x0E
#define	LED_4_PWM_BLUE_3206 	0x0F

#define	UPDATE_PWM_REGISTER_3206 0x13
#define GLOBAL_LED_REGISTER_3206         0x26
#define OUTPUT_FREQUENCY_REGISTER_3206   0x27
#define SHUTDOWN_REGISTER_3206   0x00
#define RESET_REGISTER_3206      0x2F

#define LED_NUM  (4)

typedef struct
{
    int index;
    int color[3]; //The corresponding color (0x00RRGGBB)
}LED_RGB;

typedef struct  {
//    LED_RGB led[LED_NUM];
    unsigned char  led_data[ LED_4_PWM_GREEN_3238 +1];
    int count;
//    int sn;
    int led_i2c_id ;
    unsigned short slave_addr;
    struct work_struct led_work;
} led_job;


void led_init_3238(int led_i2c_id, unsigned short slave_addr);
void led_init_3206(int led_i2c_id, unsigned short slave_addr);
void led_func_3206(int led_i2c_id, unsigned short slave_addr,int color[],int count);
void led_func_3238(int led_i2c_id, unsigned short slave_addr,int color[],int count);
int led_dump_all(const char *buf, int led_i2c_id,unsigned short slave_addr);
int led_detect_3206(int led_i2c_id, unsigned short slave_addr);
int led_detect_3238(int led_i2c_id, unsigned short slave_addr);
int led_init_io_3206(int led_i2c_id, unsigned short slave_addr);
int led_init_io_3238(int led_i2c_id, unsigned short slave_addr);
void _led_work(struct work_struct *work);

#endif
