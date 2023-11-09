#ifndef __QUICK_SHOW_H__
#define __QUICK_SHOW_H__
#include <base_types.h>
#ifndef BUILD_QUICK_SHOW
#include <linux/errno.h>
#else
#include <errno.h>
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_QUICKSHOW)

#define QS_STATUS_reg                 0xb8060150
#define QS_STATUS_active_shift        (0)
#define QS_STATUS_active_mask         (0x00000001)
#define QS_STATUS_hdmi_shift          (1)
#define QS_STATUS_hdmi_mask           (0x00000002)
#define QS_STATUS_dp_shift            (2)
#define QS_STATUS_dp_mask             (0x00000004)
#define QS_STATUS_panel_shift         (3)
#define QS_STATUS_panel_mask          (0x00000008)
#define QS_STATUS_scaler_shift        (4)
#define QS_STATUS_scaler_mask         (0x00000010)
#define QS_STATUS_i2c_shift           (5)
#define QS_STATUS_i2c_mask            (0x00000020)
#define QS_STATUS_pq_shift            (6)
#define QS_STATUS_pq_mask             (0x00000040)
#define QS_STATUS_acpu_shift          (7)
#define QS_STATUS_acpu_mask           (0x00000080)
#define QS_STATUS_vcpu_shift          (8)
#define QS_STATUS_vcpu_mask           (0x00000100)
#define QS_STATUS_amp_shift           (9)
#define QS_STATUS_amp_mask            (0x00000200)
#define QS_STATUS_hp_shift            (10)
#define QS_STATUS_hp_mask             (0x00000400)
#define QS_STATUS_portnum_shift       (11)
#define QS_STATUS_portnum_mask        (0x00001800)
#define QS_STATUS_typec_shift         (13)
#define QS_STATUS_typec_mask          (0x00002000)
#define QS_STATUS_volume_shift        (14)
#define QS_STATUS_volume_mask         (0x001FC000)
#define QS_STATUS_vo_shift            (21)
#define QS_STATUS_vo_mask             (0x00200000)
#define QS_STATUS_mute_shift          (22)
#define QS_STATUS_mute_mask           (0x00400000)
#define QS_STATUS_detect_done_shift   (23)
#define QS_STATUS_detect_done_mask    (0x00800000)
#define QS_STATUS_magic_shift         (24)
#define QS_STATUS_magic_mask          (0xFF000000)
#define QS_STATUS_magic_valid_data    (0xDD)

#define QS_PIC_MODE_reg               0xb8060154
#define QS_PIC_MODE_GAME_shift        (0)
#define QS_PIC_MODE_GAME_mask         (0x00000001)
#define QS_PIC_MODE_PC_shift          (1)
#define QS_PIC_MODE_PC_mask           (0x00000002)
#define QS_ALLM_EN_shift              (2)
#define QS_ALLM_EN_mask               (0x00000004)
#define QS_FREESYNC_EN_shift          (3)
#define QS_FREESYNC_EN_mask           (0x00000008)



bool is_QS_active(void);
bool is_QS_hdmi_enable(void);
bool is_QS_dp_enable(void);
bool is_QS_panel_enable(void);
bool is_QS_scaler_enable(void);
bool is_QS_i2c_enable(void);
bool is_QS_pq_enable(void);
bool is_QS_acpu_enable(void);
bool is_QS_vcpu_enable(void);
bool is_QS_amp_enable(void);
bool is_QS_hp_enable(void);
int get_QS_portnum(void);
bool is_QS_game_mode(void);
bool is_QS_pc_mode(void);
bool is_QS_allm_enable(void);
bool is_QS_freesync_enable(void);
void qs_hwp_prescale_register(void (*func)(int, int));
void qs_hwp_prescale_unregister(void (*func)(int, int));

extern bool in_quick_show;

#else

static inline bool is_QS_active(void) { return false; }
static inline bool is_QS_hdmi_enable(void) { return false; }
static inline bool is_QS_dp_enable(void) { return false; }
static inline bool is_QS_panel_enable(void) { return false; }
static inline bool is_QS_scaler_enable(void) { return false; }
static inline bool is_QS_i2c_enable(void) { return false; }
static inline bool is_QS_pq_enable(void) { return false; }
static inline bool is_QS_acpu_enable(void) { return false; }
static inline bool is_QS_vcpu_enable(void) { return false; }
static inline bool is_QS_amp_enable(void) { return false; }
static inline bool is_QS_hp_enable(void) { return false; }
static inline int get_QS_portnum(void) { return -ENODEV; }
static inline bool is_QS_pc_mode(void) { return false; }
static inline bool is_QS_game_mode(void) { return false; }
static inline bool is_QS_allm_enable(void) { return false; }
static inline bool is_QS_freesync_enable(void) { return false; }
static bool in_quick_show = false;

#endif /* CONFIG_RTK_KDRV_QUICKSHOW */

#endif /*__QUICK_SHOW_H__ */
