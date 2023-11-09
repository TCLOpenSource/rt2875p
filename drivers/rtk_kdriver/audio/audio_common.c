/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/
#include <linux/slab.h>
#include <linux/fs.h>
#include "audio_inc.h"
#include "audio_rpc.h"

#include <rtk_kdriver/io.h>

#define MEM_MAGIC_NUMBER (0xCCCCCCCC)
typedef struct{
    UINT32 address;
    UINT32 size;
#if defined(ANDROID) //Ronald debug
    struct list_head list;
}rtkmalloc_info;
#define malloc_info rtkmalloc_info
#else
    struct list_head list;
}malloc_info;
#endif

typedef struct{
    UINT32 pAddr;
    UINT32 vAddr;
    UINT32 size;
    struct list_head list;
}rtkaudio_malloc_info;

typedef struct{
    UINT32 agentID;
    UINT32 type;
    struct list_head list;
}agent_info;

typedef struct{
    UINT32 agentID;
    Base* module_handle;
    struct list_head list;
}map_info;

static struct list_head agent_list           = LIST_HEAD_INIT(agent_list);
static struct list_head module_list          = LIST_HEAD_INIT(module_list);
static struct list_head id2module_map        = LIST_HEAD_INIT(id2module_map);

static DEFINE_SEMAPHORE(agent_sem);
static DEFINE_SEMAPHORE(module_sem);
static DEFINE_SEMAPHORE(id2module_sem);

extern int rtkaudio_get_audio_quick_show_init_table(quickshow_cmd_info *quickshow_cmd);

int64_t getpts(void)
{
	int64_t ret;
	unsigned int ptrlo, ptrhi;

	ptrlo = (unsigned int) rtd_inl(0xB801B6B8);
	ptrhi = (unsigned int) rtd_inl(0xB801B6BC);

	ret = ptrlo;
	ret = ret|(((int64_t)ptrhi) << 32);

	rtd_pr_adsp_debug("ALSA PTS: %08x\n", ret);

	return ret;
}

RTKAUDIO_QS_TYPE rtkaudio_quickshow_init_table(AUDIO_QUICK_SHOW_PARAM *param)
{
    quickshow_cmd_info quickshow_cmd;
    unsigned long rpc_res;
    int i;

    memset(&quickshow_cmd, 0, sizeof(quickshow_cmd_info));
    quickshow_cmd.type = ENUM_AUDIO_QUICK_SHOW_GET_INIT_TABLE;
    quickshow_cmd.size = sizeof(unsigned int) * QS_DATA_SIZE;

    rpc_res = rtkaudio_get_audio_quick_show_init_table(&quickshow_cmd);
    if (rpc_res != S_OK)
    {
        ERROR("[QuickShow] %s fail\n", __FUNCTION__);
    }
    else
    {
        for(i = 0; i < 10; i++)
        {
            param->value[i] = quickshow_cmd.data[i];
        }
    }

    if (quickshow_cmd.data[10] == QS_DP) {
        return RTKAUDIO_QS_DP;
    }

    return RTKAUDIO_QS_HDMI;
}

/**** common functions ****/
UINT32 CreateAgent(UINT32 type)
{
    RPCRES_LONG res;
    AUDIO_RPC_INSTANCE instance;
    UINT32 instanceID = UNDEFINED_AGENT_ID;

    instance.type = type;
    instance.instanceID = instanceID;

    RTKAUDIO_RPC_CREATE_AGENT_SVC(&instance, &res);
    if(res.result == S_OK)
    {
        agent_info* a_info = (agent_info*)kmalloc(sizeof(agent_info), GFP_KERNEL);
        if(a_info != NULL)
        {
            a_info->agentID = res.data;
            a_info->type = type;
            down(&agent_sem);
            list_add(&a_info->list, &agent_list);
            up(&agent_sem);
        }
        //show_agent_summary();
        instanceID = res.data;
    }
    return instanceID;
}

UINT32 DestroyAgent(UINT32 *instanceID)
{
    int ret;
    agent_info* a_info;
    if(*instanceID == UNDEFINED_AGENT_ID) return S_FALSE;

    ret = RTKAUDIO_RPC_TOAGENT_DESTROY_SVC(instanceID);

    if(ret != S_OK) return S_FALSE;
    down(&agent_sem);
    list_for_each_entry(a_info, &agent_list, list)
    {
        if(a_info->agentID == *instanceID)
        {
            list_del_init(&a_info->list);
            kfree(a_info);
            break;
        }
    }
    up(&agent_sem);
    *instanceID = UNDEFINED_AGENT_ID;
    return S_OK;
}

Base* map_to_module(UINT32 instanceID)
{
    map_info* map;
    down(&id2module_sem);
    list_for_each_entry(map, &id2module_map, list)
    {
        if(map->agentID == instanceID){
            up(&id2module_sem);
            return map->module_handle;
        }
    }
    up(&id2module_sem);
    return NULL;
}

void add_to_module_list(Base* module_handle)
{
    down(&module_sem);
    list_add(&module_handle->list, &module_list);
    up(&module_sem);
}

void delete_from_module_list(Base* module_handle)
{
    down(&module_sem);
    list_del_init(&module_handle->list);
    up(&module_sem);
}

void show_module_agent(void)
{
    map_info* map;
    list_for_each_entry(map, &id2module_map, list)
    {
        INFO("agent ID: %x, handle: %p\n", map->agentID, map->module_handle);
    }
}

void add_id_to_map(Base* module_handle, UINT32 instanceID)
{
    map_info* map = (map_info*)kmalloc(sizeof(map_info), GFP_KERNEL);
    if(map!=NULL)
    {
        map->agentID = instanceID;
        map->module_handle = module_handle;
        down(&id2module_sem);
        list_add(&map->list, &id2module_map);
        up(&id2module_sem);
    }
}

void del_id_from_map(UINT32 instanceID)
{
    map_info* map;
    down(&id2module_sem);
    list_for_each_entry(map, &id2module_map, list)
    {
        if(map->agentID == instanceID)
        {
            list_del_init(&map->list);
            kfree(map);
            break;
        }
    }
    up(&id2module_sem);
}

static uint32_t reverseInteger(uint32_t value)
{
    uint32_t b0 = value & 0x000000ff;
    uint32_t b1 = (value & 0x0000ff00) >> 8;
    uint32_t b2 = (value & 0x00ff0000) >> 16;
    uint32_t b3 = (value & 0xff000000) >> 24;

    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

static unsigned long long reverseLongInteger(unsigned long long value)
{
    unsigned long long ret;
    unsigned char *des, *src;
    src = (unsigned char *)&value;
    des = (unsigned char *)&ret;
    des[0] = src[7];
    des[1] = src[6];
    des[2] = src[5];
    des[3] = src[4];
    des[4] = src[3];
    des[5] = src[2];
    des[6] = src[1];
    des[7] = src[0];
    return ret;
}

void IPC_WriteU32(BYTE* des, unsigned long data)
{
    if (((int64_t)des & 0x3) != 0)
        ERROR("error in IPC_WriteU32()...\n");

    *(uint32_t *)des = reverseInteger(data);
    //*(unsigned long *)des = (data);
}

void IPC_WriteU64(BYTE* des, uint64_t data)
{
    if (((int64_t)des & 0x3) != 0)
        ERROR("error in IPC_WriteU64()...\n");
    *(uint64_t*)des = reverseLongInteger(data);
}

uint32_t IPC_ReadU32(BYTE* src)
{
    if (((int64_t)src & 0x3) != 0)
        ERROR("error in IPC_ReadU32()...\n");

    return reverseInteger(*(uint32_t *)src);
    //return (*(unsigned long *)src);
}

void IPC_memcpy(int *des, int *src, int nSample)
{
    int32_t i;
    for(i=0; i<nSample; i++) des[i] = reverseInteger(src[i]);
    //for(i=0; i<nSample; i++) des[i] = (src[i]);
}
