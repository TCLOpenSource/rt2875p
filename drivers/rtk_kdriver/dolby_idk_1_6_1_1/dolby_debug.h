#ifndef __DOLBY_IDK_1_6_1_1_DOLBY_DEBUG_H__
#define __DOLBY_IDK_1_6_1_1_DOLBY_DEBUG_H__

bool get_flag_print_parser_time(void);
bool get_flag_stop_apply_dolby(void);

void create_dolby_debugfs(void);
void remove_dolby_debugfs(void);

#endif /* __DOLBY_IDK_1_6_1_1_DOLBY_DEBUG_H__ */
