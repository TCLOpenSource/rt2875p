/*
 * Realtek
 */

#include <linux/sysctl.h>
#include <linux/kernel.h>
#include <linux/init.h>

int	sysctl_keypad_forbidden;
static struct ctl_table_header	*keypad_table_header;

static struct ctl_table	keypad_table[] = {
	{	.procname	= "keypad_forbidden",
		.data		= &sysctl_keypad_forbidden,
		.maxlen		= sizeof(int),
		.mode		= 0666,
		.proc_handler	= proc_dointvec
	}, 
	{ }
};

static struct ctl_table	keypad_dir_table[] = {
	{ .procname	= "keypad", 
	  .mode		= 0555, 
	  .child	= keypad_table }, 
	{ }
};
static struct ctl_table	keypad_root_table[] = {
	{ .procname	= "dev", 
	  .mode		= 0555, 
	  .child	= keypad_dir_table }, 
	{ }
};

int keypad_init(void)
{
	keypad_table_header=register_sysctl_table(keypad_root_table);
	if(!keypad_table_header)
		return	-ENOMEM;
	return	0;
}

void keypad_exit(void)
{
	unregister_sysctl_table(keypad_table_header);
}



