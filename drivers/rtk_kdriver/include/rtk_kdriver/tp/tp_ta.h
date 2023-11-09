struct optee_ta {
        struct tee_context *ctx;
        __u32 session;
};

int call_tp_ta(void);
int optee_tp_init(void);
void optee_tp_deinit(void);
int optee_tp_write_register(UINT32 addr, UINT32 val);
int optee_tp_read_register(UINT32 addr, UINT32 *val);
int optee_tp_get_section_data(UINT32 addr, UINT8 size);
int optee_tp_get_channel_protect_data(UINT32 channel, UINT8 *protect_status);
int optee_tp_test_01(UINT32 para1, UINT32 para2);
int optee_tp_test_02(UINT32 para1, UINT32 para2);

INT32 RHAL_TP_TA_set_mem_prison(UINT32 startAddr, UINT32 endAddr, TP_TA_MEMORY_PRISON_TYPE_T memoryType, UINT8 isEnable);