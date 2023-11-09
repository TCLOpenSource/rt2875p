#ifndef _FRACTION_H_
#define _FRACTION_H_

//if need support more, please change this config
#define REMOVE_MUTIPLE_MAX 6

struct rmove_mutiple_info{
    unsigned int panel_refresh;

    //dependence modestate_decide_dtg_m_mode() or modestate_decide_dtg_m_modeByDisp()
    unsigned int dtg_master_v_freq;  
    unsigned int remove;
    unsigned int multiple;
};

struct fraction_info{
    unsigned int inputFrameRateStart;
    unsigned int inputFrameRateEnd;
    struct rmove_mutiple_info rm_multiple[REMOVE_MUTIPLE_MAX];
};

void get_scaler_remove_mutiple(unsigned intputFrame, unsigned int dtg_master_v_freq, unsigned int panel_refresh, unsigned int *remove, unsigned int *multiple);

#endif
