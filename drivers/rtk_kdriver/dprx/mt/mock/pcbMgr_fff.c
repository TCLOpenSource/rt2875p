#include <stdio.h>
#include <string.h>
#include <mach/pcbMgr.h>

static PCB_ENUM_T*   g_p_enum = NULL;
static unsigned char g_n_enum = 0;


int pcb_mgr_set_pcb_enum_list(PCB_ENUM_T* p_enum, unsigned char n_enum)
{
    g_p_enum = p_enum;
    g_n_enum = n_enum;
    return 0;
}

int pcb_mgr_get_enum_info_byname(char *p_enum_name, unsigned long long *p_value)
{
    int i;

    if (p_enum_name==NULL || p_value==NULL || g_p_enum==NULL || g_n_enum==0)
        return -1;

    for (i=0; i<g_n_enum ; i++)
    {
        if (g_p_enum[i].name && strcmp(g_p_enum[i].name, p_enum_name)==0)
        {
            *p_value = g_p_enum[i].info;
            printf("PCBMGR : name=%s, val=%llx\n", p_enum_name, *p_value);
            return 0;
        }
    }

    printf("PCBMGR : name=%s, not found, return -1\n", p_enum_name);
    return -1;
}
