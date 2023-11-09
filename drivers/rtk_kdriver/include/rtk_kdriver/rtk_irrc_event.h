#ifndef __RTK_IRRC_EVENT_H__
#define __RTK_IRRC_EVENT_H__


typedef struct irrc_event_handler {
    void            (*callback_func)(void);
    unsigned long    data;
}IRRC_EVENT_HANDLER;

int irrc_event_register_callback(IRRC_EVENT_HANDLER * p_handler);
int irrc_event_unregister_callback(IRRC_EVENT_HANDLER * p_handler);
void irrc_event_notify(void);

#endif