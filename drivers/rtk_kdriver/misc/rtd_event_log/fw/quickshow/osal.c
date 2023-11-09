#include <fw/quickshow/osal.h>
#include <core/event_log_core.h>
unsigned int rtk_timer_misc_90k_ms( void )
{
        unsigned int time = (unsigned int) rtd_inl(TIMER_SCPU_CLK90K_LO_reg)/90;
        return time;
}

unsigned int rtk_timer_misc_90k_us( void )
{
        unsigned int time = (unsigned int) rtd_inl(TIMER_SCPU_CLK90K_LO_reg)*11;
        return time;
}

