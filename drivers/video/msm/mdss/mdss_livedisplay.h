/*
 * Copyright (c) 2015 The CyanogenMod Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MDSS_LIVEDISPLAY_H
#define MDSS_LIVEDISPLAY_H

#include <linux/of.h>
#include <linux/sysfs.h>

#include "mdss_dsi.h"
#include "mdss_fb.h"

#define MAX_PRESETS 10

struct mdss_livedisplay_ctx {
	uint32_t r, g, b;
	struct msm_fb_data_type *mfd;

	struct mutex lock;
};

int mdss_livedisplay_update(struct mdss_dsi_ctrl_pdata *ctrl_pdata, int types);
int mdss_livedisplay_parse_dt(struct device_node *np, struct mdss_panel_info *pinfo);
int mdss_livedisplay_create_sysfs(struct msm_fb_data_type *mfd);

static inline struct mdss_livedisplay_ctx* get_ctx(struct msm_fb_data_type *mfd)
{
    return mfd->panel_info->livedisplay;
}

static inline struct mdss_dsi_ctrl_pdata* get_ctrl(struct msm_fb_data_type *mfd)
{
    struct mdss_panel_data *pdata = dev_get_platdata(&mfd->pdev->dev);
    return container_of(pdata, struct mdss_dsi_ctrl_pdata, panel_data);
}

#endif
