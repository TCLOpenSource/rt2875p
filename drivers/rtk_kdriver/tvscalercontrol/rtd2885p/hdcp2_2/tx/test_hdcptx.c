//#include <stdio.h>
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
extern int enable_hdcptx_22(void);
#endif
int tx_main(void)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
	enable_hdcptx_22();
#endif
	return 0;
}
