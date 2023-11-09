#include <linux/init.h>
#include <linux/module.h>
#include <core/event_log_core.h>

/************************************************************************
 * MODULR INIT/EXIT
 ************************************************************************/
static int __init sample_init(void)
{

    rtd_test_event_log(TEST_TYPE_1,TEST1_START,0,0);

	return 0;
}

void __exit sample_exit(void)
{
    rtd_test_event_log(TEST_TYPE_2,TEST2_END,0,0);
}

module_init(sample_init);
module_exit(sample_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Baron <baron_yuan@realtek.com>");