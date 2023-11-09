#include "ir_input.h"
#include "irrc_core.h"
#include "ir_table.h"
#include "irrc_common.h"
#include "ir_input_intercept.h"

int ir_mouse_pointer_speed = 3;
int ir_mouse_pointer_threshold = 16;
int ir_mouse_pointer_timer = 1000;
int ir_mouse_pointer_step = 4;

int start_accel_time = 0;
int click_down = 0;

int venus_ir_mouse_init(struct venus_ir_data *data)
{
    struct input_dev *input_dev;
    int result;

    input_dev = input_allocate_device();
    if (!input_dev) {
        result = -ENOMEM;
        IR_ERR("venus IR: can't allocate mouse device.\n");
        goto exit;
    }

    data->mouse_dev = input_dev;
    data->mouse_dev->name = "venus_IR_mouse";
    data->mouse_dev->phys = "venus/input1";

    data->mouse_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL) | BIT_MASK(EV_MSC);
    data->mouse_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT);
    data->mouse_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
    data->mouse_dev->mscbit[0] = BIT_MASK(EV_MSC);

    result = input_register_device(data->mouse_dev);
    if (result) {
        IR_ERR("Venus IR: cannot register mouse device.\n");
        goto fail_register_mouse_dev;
    }
    IR_ERR("Venus IR: success to reigster as mouse device\n");

    return 0;

fail_register_mouse_dev:
    input_free_device(data->mouse_dev);

exit:
    return -1;
}

void venus_ir_mouse_cleanup(struct venus_ir_data *data)
{
    input_unregister_device(data->mouse_dev);
    input_free_device(data->mouse_dev);
}

#ifdef CONFIG_RTK_KDRV_TV005_IR
void venus_ir_mouse_report_event(struct venus_ir_data *data, uint32_t value)
{
    int i, x, y, click, abs_x, abs_y;

    if ((value & 0x0000ffff) == 0x0040) {
        x = 0x7f & (value >> 17);
        y = 0x7f & (value >> 25);
        click = 0x01 & (value >> 16);
        abs_x = x;
        abs_y = y;
        if(x >= 64) {
            abs_x = 0x7f & ~(x - 1);
            x = -abs_x;
        }

        if(y >= 64) {
            abs_y = 0x7f & ~(y - 1);
            y = -abs_y;
        }

        IR_ALERT("(x, y) = (%d, %d), click = %d\n", x, y, click);
        if (ir_mouse_pointer_threshold * ir_mouse_pointer_threshold < abs_x * abs_x + abs_y * abs_y) {
            if (start_accel_time == 0) {
                start_accel_time = jiffies + msecs_to_jiffies(ir_mouse_pointer_timer);
            }

        } else {
            start_accel_time = 0;
        }

        if (start_accel_time != 0 && jiffies >= start_accel_time) {
            x *= ir_mouse_pointer_speed;
            y *= ir_mouse_pointer_speed;
        }


        x /= ir_mouse_pointer_step;
        y /= ir_mouse_pointer_step;
        for (i = 0; i < ir_mouse_pointer_step; i++) {
            input_event(data->mouse_dev, EV_REL, REL_X, x);
            input_event(data->mouse_dev, EV_REL, REL_Y, y);
            input_sync(data->mouse_dev);
            msleep(1);
        }

        if (click == 1 && click_down == 0) {
            input_event(data->mouse_dev, EV_MSC, MSC_SCAN, 0x90001);
            input_event(data->mouse_dev, EV_KEY, BTN_LEFT, 1);
            input_sync(data->mouse_dev);
            click_down = 1;
        }
    }
}

void venus_ir_mouse_report_end(struct venus_ir_data *data, uint32_t value)
{
    start_accel_time = 0;
    if (click_down == 1) {
        input_event(data->mouse_dev, EV_MSC, MSC_SCAN, 0x90001);
        input_event(data->mouse_dev, EV_KEY, BTN_LEFT, 0);
        input_sync(data->mouse_dev);
        click_down = 0;
    }
}

#else
void venus_ir_to_mouse_event(struct venus_ir_data *data, uint32_t x, uint32_t y)
{
    input_event(data->mouse_dev, EV_REL, REL_X, x * data->mouseRepeat);
    input_event(data->mouse_dev, EV_REL, REL_Y, y * data->mouseRepeat);
    input_sync(data->mouse_dev);
}

void venus_ir_to_mouse_click_event(struct venus_ir_data *data, uint32_t down)
{
    input_event(data->mouse_dev, EV_MSC, MSC_SCAN, 0x90001);
    input_event(data->mouse_dev, EV_KEY, BTN_LEFT, down);
    input_sync(data->mouse_dev);
}

void venus_ir_mouse_report_event(struct venus_ir_data *data, uint32_t value)
{
    s32 x = 0;
    s32 y = 0;

    switch(value) {
        case KEY_LEFT:
            IR_DBG("#####[LEFT] repeat: %d\n", data->mouseRepeat);
            x = -ir_mouse_pointer_speed;
            venus_ir_to_mouse_event(data, x, y);
            break;
        case KEY_RIGHT:
            IR_DBG("#####[RIGHT] repeat: %d\n", data->mouseRepeat);
            x = ir_mouse_pointer_speed;
            venus_ir_to_mouse_event(data, x, y);
            break;
        case KEY_UP:
            IR_DBG("#####[UP] repeat: %d\n", data->mouseRepeat);
            y = -ir_mouse_pointer_speed;
            venus_ir_to_mouse_event(data, x, y);
            break;
        case KEY_DOWN:
            IR_DBG("#####[DOWN] repeat: %d\n", data->mouseRepeat);
            y = ir_mouse_pointer_speed;
            venus_ir_to_mouse_event(data, x, y);
            break;
        case KEY_ENTER:
        case KEY_SELECT:
            IR_DBG("#####[Enter]#####\n");
            venus_ir_to_mouse_click_event(data, 1);
            break;
   }
}

void venus_ir_mouse_report_end(struct venus_ir_data *data, uint32_t value)
{
    if(value == KEY_ENTER || value == KEY_SELECT) {
        IR_DBG("#####[Enter] release event #####\n");
        venus_ir_to_mouse_click_event(data, 0);
    }
}
#endif

int venus_ir_is_simulated_key(struct venus_ir_data *data, u32 key)
{
    if (key != KEY_LEFT && key != KEY_RIGHT && key != KEY_UP && key != KEY_DOWN && key != KEY_ENTER && key != KEY_SELECT) {
        IR_DBG("The key is not simulated key\n");
        return 0;
    }

    return 1;
}


