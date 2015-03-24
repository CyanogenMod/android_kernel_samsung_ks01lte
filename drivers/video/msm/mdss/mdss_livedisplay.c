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

#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#include "mdss_dsi.h"
#include "mdss_fb.h"
#include "mdss_mdp.h"
#include "mdss_livedisplay.h"

/*
 * LiveDisplay is the display management service in CyanogenMod. It uses
 * various capabilities of the hardware and software in order to
 * optimize the experience for ambient conditions and time of day.
 *
 * This module is initialized by mdss_fb for each panel, and creates
 * several new controls in /sys/class/graphics/fbX based on the
 * configuration in the devicetree.
 *
 * rgb: Always available with MDSS. Used for color temperature and
 *      user-level calibration. Takes a string of "r g b".
 * 
 * Removed everything else as it is part of mDNIe or 
 * Samsung panel driver.
 */

/**
 * simple color temperature interface using polynomial color correction
 *
 * input values are r/g/b adjustments from 0-32768 representing 0 -> 1
 *
 * example adjustment @ 3500K:
 * 1.0000 / 0.5515 / 0.2520 = 32768 / 25828 / 17347
 *
 * reference chart:
 * http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html
 */
static int mdss_livedisplay_set_rgb_locked(struct msm_fb_data_type *mfd)
{
	static struct mdp_pcc_cfg_data pcc_cfg;
	struct mdss_livedisplay_ctx *mlc;

	mlc = get_ctx(mfd);

	if (mlc == NULL)
		return -ENODEV;

	pr_info("%s: r=%d g=%d b=%d\n", __func__, mlc->r, mlc->g, mlc->b);

	memset(&pcc_cfg, 0, sizeof(struct mdp_pcc_cfg_data));

	pcc_cfg.block = mfd->index + MDP_LOGICAL_BLOCK_DISP_0;
	if (mlc->r == 32768 && mlc->g == 32768 && mlc->b == 32768)
		pcc_cfg.ops = MDP_PP_OPS_DISABLE;
	else
		pcc_cfg.ops = MDP_PP_OPS_ENABLE;
	pcc_cfg.ops |= MDP_PP_OPS_WRITE;
	pcc_cfg.r.r = mlc->r;
	pcc_cfg.g.g = mlc->g;
	pcc_cfg.b.b = mlc->b;

	return mdss_mdp_user_pcc_config(&pcc_cfg);
}

/*
 * Update all or a subset of parameters
 */
static int mdss_livedisplay_update_locked(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int types)
{
	int ret = 0;
	struct mdss_panel_info *pinfo = NULL;
	struct mdss_livedisplay_ctx *mlc = NULL;

	if (ctrl_pdata == NULL)
		return -ENODEV;

	pinfo = &(ctrl_pdata->panel_data.panel_info);
	if (pinfo == NULL)
		return -ENODEV;

	mlc = pinfo->livedisplay;
	if (mlc == NULL)
		return -ENODEV;

	if (!mdss_panel_is_power_on_interactive(pinfo->panel_power_state))
		return 0;

	// Restore saved RGB settings
	mdss_livedisplay_set_rgb_locked(mlc->mfd);

	return ret;
}

int mdss_livedisplay_update(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int types)
{
	struct mdss_panel_info *pinfo;
	struct mdss_livedisplay_ctx *mlc;
	int ret = 0;

	pinfo = &(ctrl_pdata->panel_data.panel_info);
	if (pinfo == NULL)
		return -ENODEV;

	mlc = pinfo->livedisplay;
	if (mlc == NULL)
		return -ENODEV;

	if (mlc->mfd == NULL)
		return -ENODEV;

	mutex_lock(&mlc->lock);
	ret = mdss_livedisplay_update_locked(ctrl_pdata, types);
	mutex_unlock(&mlc->lock);

	return ret;
}

static ssize_t mdss_livedisplay_get_rgb(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)fbi->par;
	struct mdss_livedisplay_ctx *mlc;

	if (mfd == NULL)
		return -ENODEV;

	mlc = get_ctx(mfd);

	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			mlc->r, mlc->g, mlc->b);
}

static ssize_t mdss_livedisplay_set_rgb(struct device *dev,
							struct device_attribute *attr,
							const char *buf, size_t count)
{
	uint32_t r = 0, g = 0, b = 0;
	struct fb_info *fbi = dev_get_drvdata(dev);
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)fbi->par;
	struct mdss_panel_data *pdata;
	struct mdss_livedisplay_ctx *mlc;
	int ret = -EINVAL;

	if (mfd == NULL)
		return -ENODEV;

	if (count > 19)
		return -EINVAL;

	mlc = get_ctx(mfd);
	pdata = dev_get_platdata(&mfd->pdev->dev);

	sscanf(buf, "%d %d %d", &r, &g, &b);

	if (r < 0 || r > 32768)
		return -EINVAL;
	if (g < 0 || g > 32768)
		return -EINVAL;
	if (b < 0 || b > 32768)
		return -EINVAL;

	mutex_lock(&mlc->lock);

	mlc->r = r;
	mlc->g = g;
	mlc->b = b;

	if (!mdss_panel_is_power_on_interactive(mfd->panel_power_state) ||
			(mdss_livedisplay_set_rgb_locked(mfd) == 0))
		ret = count;

	mutex_unlock(&mlc->lock);

	return ret;
}

static DEVICE_ATTR(rgb, S_IRUGO | S_IWUSR | S_IWGRP, mdss_livedisplay_get_rgb, mdss_livedisplay_set_rgb);

int mdss_livedisplay_parse_dt(struct device_node *np, struct mdss_panel_info *pinfo)
{
	int rc = 0, i = 0;
	struct mdss_livedisplay_ctx *mlc;

	if (pinfo == NULL)
		return -ENODEV;

	mlc = kzalloc(sizeof(struct mdss_livedisplay_ctx), GFP_KERNEL);
	mutex_init(&mlc->lock);

	mlc->r = mlc->g = mlc->b = 32768;

	pinfo->livedisplay = mlc;
	return 0;
}

int mdss_livedisplay_create_sysfs(struct msm_fb_data_type *mfd)
{
	int rc = 0;
	struct mdss_livedisplay_ctx *mlc = get_ctx(mfd);

	if (mlc == NULL)
		return 0;

	rc = sysfs_create_file(&mfd->fbi->dev->kobj, &dev_attr_rgb.attr);
	if (rc)
		goto sysfs_err;

	mlc->mfd = mfd;

	return rc;

sysfs_err:
	pr_err("%s: sysfs creation failed, rc=%d", __func__, rc);
	return rc;
}

