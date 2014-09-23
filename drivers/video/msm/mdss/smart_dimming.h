#ifndef _SMART_DIMMING_H_
#define _SMART_DIMMING_H_

struct smartdim_conf{
	void (*generate_gamma)(int cd, char *str);
	void (*get_min_lux_table)(char *str, int size);
	void (*init)(void);
	void (*print_aid_log)(void);
	char *mtp_buffer;
	int *lux_tab;
	int lux_tabsize;
	unsigned int man_id;
};

/* Define the smart dimming LDIs*/
struct smartdim_conf *smart_S6E3_get_conf(void);
struct smartdim_conf *smart_S6E8FA0_get_conf(void);
struct smartdim_conf *smart_S6E3FA0_get_conf(void);
#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_VIDEO_WVGA_S6E88A0_PT_PANEL)
struct smartdim_conf *smart_S6E88A0_get_conf(void);
#endif

#if defined(CONFIG_LCD_HMT)
struct smartdim_conf_hmt {
	void (*generate_gamma)(int cd, char *str, int dual);
	void (*get_min_lux_table)(char *str, int size);
	void (*init)(int dual);
	void (*print_aid_log)(void);
	char *mtp_buffer;
	int *lux_tab;
	int lux_tabsize;
	unsigned int man_id;
};

struct smartdim_conf_hmt *smart_S6E3_get_conf_hmt(int dual);
#endif

enum mipi_samsung_cmd_list {

        PANEL_READY_TO_ON,
        PANEL_DISP_OFF,
        PANEL_DISPLAY_ON,
        PANEL_DISPLAY_OFF,
        PANEL_DISPLAY_UNBLANK,
        PANEL_DISPLAY_BLANK,
        PANEL_ALL_PIXEL_OFF,
        PANEL_BRIGHT_CTRL,
        PANEL_MTP_ENABLE,
        PANEL_MTP_DISABLE,
        PANEL_NEED_FLIP,
        PANEL_ACL_OFF,
        PANEL_ACL_ON,
        PANEL_LATE_ON,
        PANEL_EARLY_OFF,
        PANEL_TOUCHSENSING_ON,
        PANEL_TOUCHSENSING_OFF,
        PANEL_TEAR_ON,
        PANEL_TEAR_OFF,
        PANEL_LDI_FPS_CHANGE,
        PANEL_LDI_SET_VDDM_OFFSET, /*LDI_ADJ_VDDM_OFFSET*/
        PANEL_PARTIAL_ON,
        PANEL_PARTIAL_OFF,
        PANEl_FORCE_500CD
};

#endif /* _SMART_DIMMING_H_ */
