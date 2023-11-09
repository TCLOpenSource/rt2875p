#include "rtk_led_interface.h"
#include <linux/i2c.h>
#include <linux/delay.h>
#include <rtk_kdriver/i2c-rtk-api.h>
extern struct workqueue_struct *led_wq ;
extern struct mutex  g_led_mutex;
extern led_job job;


void led_init_3206(int led_i2c_id, unsigned short slave_addr) {

	unsigned char data[2] ={0};
	int i=0;
	data[0] = SHUTDOWN_REGISTER_3206;
	data[1] = 0x00;// 00: shutdown ; 01:normal operation

	LED_INFO("alc1310_func slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}

	for(i=0x04;i<=0x0f;i++)
	{
		 //IS_IIC_WriteByte(Addr_GND,i,0x00);//PWM
		data[0] = i;
		data[1] = 0x00;
		if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
			LED_ERR("I2C Write Reg: failed\n");
		}
	}

	for(i=0x17;i<=0x22;i++)
	{
		//  IS_IIC_WriteByte(Addr_GND,i,0x10);//enable all LED channel
		data[0] = i;
		data[1] = 0x10;
		if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
			LED_ERR("I2C Write Reg: failed\n");
		}
	}

	data[0] = GLOBAL_LED_REGISTER_3206;
	data[1] = 0x00;//0X00 :enable LED  0X01:disable LED

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}

	data[0] = OUTPUT_FREQUENCY_REGISTER_3206;
	data[1] = 0x00;

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}

	data[0] = SHUTDOWN_REGISTER_3206;
	data[1] = 0x01; // 00: shutdown ; 01:normal operation

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}
}


void led_init_3238(int led_i2c_id, unsigned short slave_addr) {

	unsigned char data[2] ={0};
	int i=0;

	LED_INFO("led init slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);

	for(i=0x4A;i<=0x6D;i++)
	{
		//IS_IIC_WriteByte(Addr_GND,i,0x00);//PWM
		data[0] = i;
		data[1] = 0xFF;
		if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
			LED_ERR("I2C Write Reg: failed\n");
		}
	}

	data[0] = 0x6e;
	data[1] = 0xff;

        if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
                LED_ERR("I2C Write Reg: failed\n");
        }

	data[0] = 0x49;
	data[1] = 0x00;

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}

	data[0] = 0x00;
	data[1] = 0x01; 

	if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
		LED_ERR("I2C Write Reg: failed\n");
	}
}

int *getRGBformInt(int color) {
	//index 0: red
	//index 1: green
	//index 2: blue
	//@param colors The corresponding color (0x00RRGGBB) of the LED light to set to.
	static int color_array[3];
	int j;

	color_array[0] = (color & 0x00ff0000) >> 16 ;
	color_array[1] = (color & 0x0000ff00) >> 8;
	color_array[2] = (color & 0x000000ff);

	for (j=0;j<3;j++) {
		LED_INFO("getRGBformInt  led[j].index  %d : R: %x\n",j ,color_array[j]);
	}
	return color_array;
}

void sendcolor(int led_i2c_id, unsigned short slave_addr,int index,int color) {

	unsigned char data1[2] ={0};
	data1[0] = index;
	data1[1] = color;

	if (i2c_master_send_ex_flag(led_i2c_id, slave_addr, &data1[0], 2,I2C_M_FAST_SPEED & I2C_M_NO_GUARD_TIME ) < 0) {
		LED_ERR("%s I2C Write Reg: failed , led_i2c_id =%d , slave_addr = %d \n", __func__ ,led_i2c_id , slave_addr );
	}

}

/*----------------------------------------------------------------------
 * Func : _led_work
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void _led_work(struct work_struct *work)
{
    int  count = 0 , len = 0 ;
    led_job *job = container_of(work, led_job, led_work);

    count = job->count ;

    if( count > 0 )
    {
        len = LED_1_PWM_GREEN_3238 +1;
    }

    if (count > 1)
    {
        len = LED_2_PWM_GREEN_3238 +1;
    }

    if (count > 2)
    {
        len = LED_3_PWM_GREEN_3238 +1;
    }

    if (count > 3)
    {
        len = LED_4_PWM_GREEN_3238 +1;
    }

    LED_INFO("%s len = %d \n" , __func__ , len );


    if (i2c_master_send_ex_flag(job->led_i2c_id, job->slave_addr, &job->led_data[0], len,I2C_M_FAST_SPEED & I2C_M_NO_GUARD_TIME ) < 0)
    {
        LED_ERR("%s I2C Write Reg: failed , led_i2c_id =%d , slave_addr = %d \n", __func__ ,job->led_i2c_id , job->slave_addr );
    }

    sendcolor (job->led_i2c_id,job->slave_addr,UPDATE_PWM_REGISTER_3238,0x00);  //update pwm color

}
void led_func_3238(int led_i2c_id, unsigned short slave_addr,int color[],int count)
{
    int j=0;

    LED_RGB led[LED_NUM];

    LED_INFO("led_func slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);


    for (j=0;j<count;j++)
    {
        int *array;

        array = getRGBformInt(color[j]);

        led[j].index = j;
        led[j].color[0] = array[0];
        led[j].color[1] = array[1];
        led[j].color[2] = array[2];

        LED_INFO("led[j].index  %d : R: %x\n",led[j].index ,led[j].color[0]);
        LED_INFO("led[j].index  %d : G: %x\n",led[j].index ,led[j].color[1]);
        LED_INFO("led[j].index  %d : B: %x\n",led[j].index ,led[j].color[2]);

        job.count = count;
        job.led_i2c_id = led_i2c_id;
        job.slave_addr = slave_addr;

        job.led_data[0] = 0x01 ; // led pwm register address begin from 1 , that is all total lengh also need to add one ;
        if( count > 0 )
        {
            job.led_data[LED_1_PWM_RED_3238  ]  = led[0].color[0];
            job.led_data[LED_1_PWM_GREEN_3238]  = led[0].color[1];
            job.led_data[LED_1_PWM_BLUE_3238 ]  = led[0].color[2];
        }

        if( count > 1 )
        {
            job.led_data[LED_2_PWM_RED_3238  ]  = led[1].color[0];
            job.led_data[LED_2_PWM_GREEN_3238]  = led[1].color[1];
            job.led_data[LED_2_PWM_BLUE_3238 ]  = led[1].color[2];
        }

        if( count > 2 )
        {
            job.led_data[LED_3_PWM_RED_3238  ]  = led[2].color[0];
            job.led_data[LED_3_PWM_GREEN_3238]  = led[2].color[1];
            job.led_data[LED_3_PWM_BLUE_3238 ]  = led[2].color[2];
        }

        if( count > 3 )
        {
            job.led_data[LED_4_PWM_RED_3238  ]  = led[3].color[0];
            job.led_data[LED_4_PWM_GREEN_3238]  = led[3].color[1];
            job.led_data[LED_4_PWM_BLUE_3238 ]  = led[3].color[2];
        }
    }

    queue_work(led_wq, &job.led_work);



}

void led_func_3206(int led_i2c_id, unsigned short slave_addr,int color[],int count) {

	int j=0;

	LED_RGB led[LED_NUM];

	LED_INFO("led_func slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);

	for (j=0;j<count;j++) {

	    int *array;
	
	    array = getRGBformInt(color[j]);

	    led[j].index = j;
	    led[j].color[0] = array[0];
	    led[j].color[1] = array[1];
	    led[j].color[2] = array[2];

	    LED_INFO("led[j].index  %d : R: %x\n",led[j].index ,led[j].color[0]);
	    LED_INFO("led[j].index  %d : G: %x\n",led[j].index ,led[j].color[1]);
	    LED_INFO("led[j].index  %d : B: %x\n",led[j].index ,led[j].color[2]);
	}

	sendcolor (led_i2c_id,slave_addr,LED_1_PWM_RED_3206,led[0].color[0]);  //index : 0 , color :RED
	sendcolor (led_i2c_id,slave_addr,LED_1_PWM_GREEN_3206,led[0].color[1]);  //index : 0 , color :GREEN
	sendcolor (led_i2c_id,slave_addr,LED_1_PWM_BLUE_3206,led[0].color[2]);  //index : 0 , color :BLUE

	sendcolor (led_i2c_id,slave_addr,LED_2_PWM_RED_3206,led[1].color[0]);  //index : 1 , color :RED
	sendcolor (led_i2c_id,slave_addr,LED_2_PWM_GREEN_3206,led[1].color[1]);  //index : 1 , color :GREEN
	sendcolor (led_i2c_id,slave_addr,LED_2_PWM_BLUE_3206,led[1].color[2]);  //index : 1 , color :BLUE

	sendcolor (led_i2c_id,slave_addr,LED_3_PWM_RED_3206,led[2].color[0]);  //index : 2 , color :RED
	sendcolor (led_i2c_id,slave_addr,LED_3_PWM_GREEN_3206,led[2].color[1]);  //index : 2 , color :GREEN
	sendcolor (led_i2c_id,slave_addr,LED_3_PWM_BLUE_3206,led[2].color[2]);  //index : 2 , color :BLUE

	sendcolor (led_i2c_id,slave_addr,LED_4_PWM_RED_3206,led[3].color[0]);  //index : 3 , color :RED
	sendcolor (led_i2c_id,slave_addr,LED_4_PWM_GREEN_3206,led[3].color[1]);  //index : 3 , color :GREEN
	sendcolor (led_i2c_id,slave_addr,LED_4_PWM_BLUE_3206,led[3].color[2]);  //index : 3 , color :BLUE


	sendcolor (led_i2c_id,slave_addr,UPDATE_PWM_REGISTER_3206,0x00);  //update pwm color
}

int led_dump_all(const char *buf, int led_i2c_id, unsigned short slave_addr)
{
    return 0;
}

int led_detect_3206(int led_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2] ={0};
    LED_INFO("detect slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);

    data[0] = OUTPUT_FREQUENCY_REGISTER_3206;
    data[1] = 0x00;// 00: 24kHZ ; 01: 3.6kHZ

    if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
        LED_INFO("%s line:%d I2C Write Reg: failed led not detect \n",__func__, __LINE__);
        return -1;
    }
        return 1;
}


int led_detect_3238(int led_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2] ={0};
    LED_INFO("detect slave_addr = %x,  led_i2c_id = %d \n",slave_addr,led_i2c_id);

    data[0] = 0x77;
    data[1] = 0x00;// 00: 24kHZ ; 01: 3.6kHZ

    if (i2c_master_send_ex(led_i2c_id, slave_addr, &data[0], 2) < 0) {
        LED_INFO("%s line:%d I2C Write Reg: failed led not detect \n",__func__, __LINE__);
        return -1;
    }
        return 1;
}

int led_init_io_3206(int led_i2c_id, unsigned short slave_addr)
{
        led_init_3206(led_i2c_id,  slave_addr);

        return 0;
}


int led_init_io_3238(int led_i2c_id, unsigned short slave_addr)
{
	led_init_3238(led_i2c_id,  slave_addr);
	
	return 0;
}
