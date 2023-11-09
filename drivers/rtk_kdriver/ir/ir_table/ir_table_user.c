#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include "ir_input.h"
#include "irrc_filter.h"

#define MAX_IR_USER_KEY_NUM   1024

static unsigned char g_ir_table[8192];

static unsigned long g_ir_boot_memory_address = 0;
extern IR_SCANCODE_FILTER_LIST g_scancode_filter_list;
extern uint32_t g_scancode_filter_function_enable ;
IR_USER_KEY_TABLE g_ir_user_key_table = {
    .keys = NULL,
    .size = 0,
    .is_init = 0,
};

int venus_ir_user_key_table_init(void)
{
    unsigned long flags = 0;

    if(g_ir_user_key_table.is_init)
        return 0;

    g_ir_boot_memory_address = 0;
    rwlock_init(&g_ir_user_key_table.lock);
    write_lock_irqsave(&g_ir_user_key_table.lock, flags);
    g_ir_user_key_table.keys = (IR_USER_KEY *)g_ir_table;
    g_ir_user_key_table.size = 0;
    g_ir_user_key_table.is_init = 1;
    write_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return 0;
}

void venus_ir_user_key_table_uninit(void)
{
    unsigned long flags = 0;
    write_lock_irqsave(&g_ir_user_key_table.lock, flags);
    g_ir_user_key_table.size = 0;
    write_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
}

unsigned int venus_ir_user_key_table_find_scancode(unsigned short protocol, unsigned short keycode)
{
    unsigned int i = 0;
    unsigned int scancode = 0;
    unsigned long flags = 0;

    read_lock_irqsave(&g_ir_user_key_table.lock, flags);
    for(i = 0; i < g_ir_user_key_table.size; i++) {
        if(g_ir_user_key_table.keys[i].protocol == protocol
        && g_ir_user_key_table.keys[i].keycode == keycode) {
            scancode = g_ir_user_key_table.keys[i].scancode;
            break;
        }
    }
    read_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return scancode;
}

unsigned int venus_ir_user_key_table_find_scancodes(unsigned short protocol, unsigned short keycode,
                                        unsigned int *scancode_arrays, unsigned int array_len)
{
    unsigned int i = 0;
    unsigned long flags = 0;
    unsigned int num = 0;
    if(!scancode_arrays || !array_len)
        return 0;

    read_lock_irqsave(&g_ir_user_key_table.lock, flags);
    for(i = 0; i < g_ir_user_key_table.size; i++) {
        if(g_ir_user_key_table.keys[i].protocol == protocol
        && g_ir_user_key_table.keys[i].keycode == keycode) {
            if(g_scancode_filter_function_enable){
                if(ir_query_scancode_filter(&g_scancode_filter_list, g_ir_user_key_table.keys[i].scancode, 0)){
            scancode_arrays[num] = g_ir_user_key_table.keys[i].scancode;
            num++;
                }
            } else {
                scancode_arrays[num] = g_ir_user_key_table.keys[i].scancode;
                num++;
            }
            if(num >= array_len)
                break;
        }
    }
    read_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return num;
}

unsigned short venus_ir_user_key_table_getkeycode_by_scancode(unsigned int scancode, unsigned int *p_index, unsigned int *p_protocol)
{
    unsigned short keycode = KEY_RESERVED;
    unsigned long flags = 0;
    int low = 0, high = 0, mid = 0;

    read_lock_irqsave(&g_ir_user_key_table.lock, flags);

    if(g_ir_user_key_table.size) {
        low = 0;
        high = g_ir_user_key_table.size - 1;
        while ( low <= high){
            mid = (low + high) / 2;
            if (scancode < g_ir_user_key_table.keys[mid].scancode) {
                high = mid - 1;
            } else if(scancode > g_ir_user_key_table.keys[mid].scancode){
                low = mid + 1;
            } else {
                keycode = g_ir_user_key_table.keys[mid].keycode;
                if(p_index)
                    *p_index = mid;
                if(p_protocol)
                    *p_protocol = g_ir_user_key_table.keys[mid].protocol;
                break;
            }
        }
    }
    read_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return keycode;
}

unsigned short venus_ir_user_key_table_getkeycode_by_index(unsigned int index, unsigned int *p_scancode, unsigned int *p_protocol)
{
    unsigned short keycode = KEY_RESERVED;
    unsigned long flags = 0;

    read_lock_irqsave(&g_ir_user_key_table.lock, flags);

    if(index < g_ir_user_key_table.size) {
        keycode = g_ir_user_key_table.keys[index].keycode;
        if(p_scancode)
            *p_scancode = g_ir_user_key_table.keys[index].scancode;
        if(p_protocol)
            *p_protocol = g_ir_user_key_table.keys[index].protocol;
    }
    read_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return keycode;
}

int venus_ir_user_key_table_setkeycode(unsigned int scancode, unsigned int keycode, unsigned int protocol, unsigned int *old_keycode)
{
    int ret = -1;
    unsigned int i = 0;
    unsigned long flags = 0;

    write_lock_irqsave(&g_ir_user_key_table.lock, flags);
    for(i = 0; i < g_ir_user_key_table.size; i++) {
        if(g_ir_user_key_table.keys[i].scancode >= scancode)
            break;
    }
    if(i < g_ir_user_key_table.size) {
        if(g_ir_user_key_table.keys[i].scancode == scancode) {
                if(old_keycode)
                    *old_keycode =  g_ir_user_key_table.keys[i].keycode;
                g_ir_user_key_table.keys[i].keycode = keycode;
                g_ir_user_key_table.keys[i].protocol = protocol;
                ret = 0;
        } else {
            if(g_ir_user_key_table.size < MAX_IR_USER_KEY_NUM) {
                unsigned int j = 0;
                for(j = g_ir_user_key_table.size - 1; j >= i; j--)
                    g_ir_user_key_table.keys[j+1] = g_ir_user_key_table.keys[j];
                g_ir_user_key_table.keys[i].scancode = scancode;
                g_ir_user_key_table.keys[i].keycode = keycode;
                g_ir_user_key_table.keys[i].protocol= protocol;
                g_ir_user_key_table.size++;
                if(old_keycode)
                    *old_keycode =  KEY_CNT;
                ret = 0;
            }
        }
    } else {
        if(g_ir_user_key_table.size < MAX_IR_USER_KEY_NUM) {
            g_ir_user_key_table.keys[g_ir_user_key_table.size].scancode = scancode;
            g_ir_user_key_table.keys[g_ir_user_key_table.size].keycode = keycode;
            g_ir_user_key_table.keys[g_ir_user_key_table.size].protocol= protocol;
            g_ir_user_key_table.size++;
            if(old_keycode)
                *old_keycode =  KEY_CNT;
            ret = 0;
        }
    }
    write_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return ret;
}


int venus_ir_user_key_table_check_keycode_exist(unsigned int keycode)
{
    int ret = -1;
    unsigned int i = 0;
    unsigned long flags = 0;

    read_lock_irqsave(&g_ir_user_key_table.lock, flags);
    for(i = 0; i < g_ir_user_key_table.size; i++) {
        if(g_ir_user_key_table.keys[i].keycode == keycode) {
            ret = 0;
            break;
        }
    }
    read_unlock_irqrestore(&g_ir_user_key_table.lock, flags);
    return ret;
}

#ifdef CONFIG_RTK_KDRV_IR_MODULE
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
int venus_ir_input_table_addr_parse(void)
{
    unsigned long ir_table_phy_address = 0;
    char str[20] = {0};
    if (0 == rtk_parse_commandline_equal("ir_table", str, sizeof(str)))
    {
        IR_INFO("no define ir_table\n");
        return -1;
    }

    if(sscanf(str, "%lx", &ir_table_phy_address) < 1){
        return 1;
    }

    g_ir_boot_memory_address = ir_table_phy_address;
    IR_INFO("g_ir_boot_memory_address == %lx\n", g_ir_boot_memory_address);
    return 0;
}
#endif

int venus_ir_input_early_init(void)
{
    unsigned int *ir_boot_table = NULL;

#ifdef CONFIG_RTK_KDRV_IR_MODULE
    venus_ir_input_table_addr_parse();
#endif

    //carvedout_buf_query(CARVEDOUT_IR_TABLE, &ir_boot_table);
    if(g_ir_boot_memory_address) {
        ir_boot_table = (unsigned int *)phys_to_virt (g_ir_boot_memory_address);
        if(!ir_boot_table) {
            g_ir_boot_memory_address = 0;
            return -1;
        }
        IR_INFO("venus_ir_input__table_parse: %p, %x,%x\n", ir_boot_table, ir_boot_table[0], ir_boot_table[1]);
        rwlock_init(&g_ir_user_key_table.lock);
        g_ir_user_key_table.keys = (IR_USER_KEY *)g_ir_table;
        if(ir_boot_table[0] == 0x49525442 &&  ir_boot_table[1] > 0
                && ir_boot_table[1] <= MAX_IR_USER_KEY_NUM) {
            g_ir_user_key_table.size = ir_boot_table[1];
            memcpy(g_ir_table, ir_boot_table + 2, g_ir_user_key_table.size * sizeof(IR_USER_KEY));
        } else {
            g_ir_user_key_table.size = 0;
        }
        g_ir_user_key_table.is_init = 1;
        IR_INFO("venus_ir_input__table_size: %d\n",g_ir_user_key_table.size);
    }
    return 0;
}

#ifdef CONFIG_RTK_KDRV_IR
core_initcall(venus_ir_input_early_init);

int venus_ir_input_table_addr_parse(char *options)
{
    unsigned long ir_table_phy_address = 0;

    if(options == NULL)
        return 0;

    if (sscanf(options, "%lx", &ir_table_phy_address) != 1)
        return 0;

    g_ir_boot_memory_address = ir_table_phy_address;
    IR_INFO("g_ir_boot_memory_address == %lx\n", g_ir_boot_memory_address);
    return 0;
}

early_param("ir_table", venus_ir_input_table_addr_parse);
#endif


