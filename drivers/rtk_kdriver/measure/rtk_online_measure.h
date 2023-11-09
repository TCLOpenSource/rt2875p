#ifndef __RTK_ONLINE_MEASURE_H__
#define __RTK_ONLINE_MEASURE_H__

// Low Level API
extern void          lib_measure_onms_init(MEASURE_ONMS_CHANNEL ch);
extern void          lib_measure_onms_set_ms_src_sel(MEASURE_ONMS_CHANNEL ch, unsigned char ms_src_sel);
extern unsigned char lib_measure_onms_get_ms_src_sel(MEASURE_ONMS_CHANNEL ch);
extern void          lib_measure_onms_set_measure_start(MEASURE_ONMS_CHANNEL ch);
extern unsigned char lib_measure_onms_get_measure_start(MEASURE_ONMS_CHANNEL ch);
extern void          lib_measure_onms_set_measure_popup(MEASURE_ONMS_CHANNEL ch);
extern unsigned char lib_measure_onms_get_measure_popup(MEASURE_ONMS_CHANNEL ch);
extern void          lib_measure_onms_get_measure_timing(MEASURE_ONMS_CHANNEL ch, MEASURE_TIMING_T* p_timing);
extern void          lib_measure_onms_set_watchdog_int_en(MEASURE_ONMS_CHANNEL ch, unsigned int value);
extern unsigned int  lib_measure_onms_get_int_en(MEASURE_ONMS_CHANNEL ch);

#endif // __RTK_ONLINE_MEASURE_H__
