#include <tvscalercontrol/scalerdrv/fraction.h>
#include <rtd_log/rtd_module_log.h>

/*
* fraction remove, mutilple setting table
*/

struct fraction_info fractionInfo[] = {
    {235, 245,      {
                        {60,    48,     1,  2},
                        {60,    60,     2,  5},
                        {120,   60,     1,  5},
                        {144,   144,    1,  6},
                        {165,   165,    1,  6}
                    }
    },

    {245, 255,      {
                        {60,    50,     1,  2},
                        {60,    60,     5,  12},
                        {120,   50,     1,  4},
                        {120,   60,     5,  24},
                        {144,   125,    1,  5},
                        {165,   150,    1,  6}
                    }
    },

    {295, 305,      {
                        {60,    60,     1,  2},
                        {120,   60,     1,  4},
                        {144,   120,    1,  4},
                        {165,   150,    1,  5}
                    }
    },

    {475, 485,      {
                        {60,    48,     1,  1},
                        {60,    60,     4,  5},
                        {120,   48,     1,  2},
                        {144,   144,    1,  3},
                        {165,   144,    1,  3}
                    }
    },

    {495, 505,      {
                        {60,    50,     1,  1},
                        {60,    60,     5,  6},
                        {120,   50,     1,  2},
                        {120,   60,     5,  12},
                        {144,   100,    1,  2},
                        {165,   150,    1,  3}
                    }
    },

    {595, 605,      {
                        {60,    60,     1,  1},
                        {120,   60,     1,  2},
                        {144,   120,    1,  2},
                        {165,   120,    1,  2}
                    }
    },

    {994, 1005,     {
                        {120,  50,     1,  1},
                        {144,  100,    1,  1},
                        {165,  100,    1,  1}
                    }
    },

    {1191, 1206,    {
                        {120, 60,     1,  1},
                        {144, 120,    1,  1},
                        {165, 120,    1,  1}
                    }
    },

    {1435, 1445,    {
                        {144, 144,    1,  1},
                        {165, 144,    1,  1}
                    }
    },

    {1645, 1655,    {
                        {165, 165,    1,  1}
                    }
    },

    {2395, 2405,    {
                        {120, 60,     1,  1},
                        {240, 240,    1,  1}
                    }
    },

    {2875, 2885,    {
                        {144, 60,     1,  1},
                        {288, 288,    1,  1}
                    }
    }
};

void get_scaler_remove_mutiple(unsigned intputFrame, unsigned int dtg_master_v_freq, unsigned int panel_refresh, unsigned int *remove, unsigned int *multiple)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int count = sizeof(fractionInfo) / sizeof(struct fraction_info);

    if(remove == NULL || multiple == NULL)
    {
        rtd_pr_vbe_err("[%s %d][ERROR]param error\n", __FUNCTION__, __LINE__);
        return;
    }

    for(i = 0;  i < count; i++)
    {
        if(intputFrame > fractionInfo[i].inputFrameRateStart
            && intputFrame < fractionInfo[i].inputFrameRateEnd)
        {
            for(j = 0; j < REMOVE_MUTIPLE_MAX; j++)
            {
                //rtd_pr_vbe_notice("%d %d %d %d\n", fractionInfo[i].rm_multiple[j].panel_refresh, fractionInfo[i].rm_multiple[j].dtg_master_v_freq, fractionInfo[i].rm_multiple[j].remove, fractionInfo[i].rm_multiple[j].multiple);

                if(panel_refresh == fractionInfo[i].rm_multiple[j].panel_refresh)
                {
                    if(dtg_master_v_freq == fractionInfo[i].rm_multiple[j].dtg_master_v_freq)
                    {
                        *remove = fractionInfo[i].rm_multiple[j].remove;
                        *multiple = fractionInfo[i].rm_multiple[j].multiple;

                        rtd_pr_vbe_notice("remove : multiple = %d : %d\n", *remove, *multiple);

                        return;
                    }
                }
            }
        }
    }

    if(panel_refresh > 60)
    {
        if(intputFrame > 900)
        {
            *remove = 1;
            *multiple = 1;
        }
        else
        {
            *remove = 1;
            *multiple = 2;
        }
    }
    else
    {
        *remove = 1;
        *multiple = 1;
    }

    rtd_pr_vbe_notice("[ERR] can not find valied rm/mutiple info, please check remove mutiple info config!!!, defalut(%d:%d)\n", *remove, *multiple);
}

