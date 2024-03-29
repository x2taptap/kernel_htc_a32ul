/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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
#include <linux/irqdomain.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/machine.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "core.h"
#include "pinconf.h"
#include "pinctrl-msm.h"
#if defined(CONFIG_HTC_POWER_DEBUG) && defined(CONFIG_PINCTRL_MSM_TLMM)
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/qpnp/pin.h>
#include <soc/qcom/pm.h>
#endif

struct msm_pinctrl_dd {
	void __iomem *base;
	int	irq;
	unsigned int num_pins;
	struct msm_pindesc *msm_pindesc;
	unsigned int num_pintypes;
	struct msm_pintype_info *msm_pintype;
	struct pinctrl_desc pctl;
	struct pinctrl_dev *pctl_dev;
	struct msm_pin_grps *pin_grps;
	unsigned int num_grps;
	struct  msm_pmx_funcs *pmx_funcs;
	unsigned int num_funcs;
	struct device *dev;
};

struct msm_irq_of_info {
	const char *compat;
	int (*irq_init)(struct device_node *np, struct irq_chip *ic);
};

static int msm_pmx_functions_count(struct pinctrl_dev *pctldev)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	return dd->num_funcs;
}

static const char *msm_pmx_get_fname(struct pinctrl_dev *pctldev,
						unsigned selector)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	return dd->pmx_funcs[selector].name;
}

static int msm_pmx_get_groups(struct pinctrl_dev *pctldev,
		unsigned selector, const char * const **groups,
		unsigned * const num_groups)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	*groups = dd->pmx_funcs[selector].gps;
	*num_groups = dd->pmx_funcs[selector].num_grps;
	return 0;
}

static void msm_pmx_prg_fn(struct pinctrl_dev *pctldev, unsigned selector,
					unsigned group, bool enable)
{
	struct msm_pinctrl_dd *dd;
	const unsigned int *pins;
	struct msm_pindesc *pindesc;
	struct msm_pintype_info *pinfo;
	unsigned int pin, cnt, func;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pins = dd->pin_grps[group].pins;
	pindesc = dd->msm_pindesc;

	for (cnt = 0; cnt < dd->pin_grps[group].num_pins; cnt++) {
		pin = pins[cnt];
		pinfo = pindesc[pin].pin_info;
		pin = pin - pinfo->pin_start;
		func = dd->pin_grps[group].func;
		pinfo->prg_func(pin, func, enable, pinfo);
	}
}

static int msm_pmx_enable(struct pinctrl_dev *pctldev, unsigned selector,
					unsigned group)
{
	msm_pmx_prg_fn(pctldev, selector, group, true);
	return 0;
}

static void msm_pmx_disable(struct pinctrl_dev *pctldev,
					unsigned selector, unsigned group)
{
	msm_pmx_prg_fn(pctldev, selector, group, false);
}

static int msm_pmx_gpio_request(struct pinctrl_dev *pctldev,
				struct pinctrl_gpio_range *grange,
				unsigned pin)
{
	struct msm_pinctrl_dd *dd;
	struct msm_pindesc *pindesc;
	struct msm_pintype_info *pinfo;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pindesc = dd->msm_pindesc;
	pinfo = pindesc[pin].pin_info;
	
	pinfo->prg_func(pin, 0, true, pinfo);
	return 0;
}

static struct pinmux_ops msm_pmxops = {
	.get_functions_count	= msm_pmx_functions_count,
	.get_function_name	= msm_pmx_get_fname,
	.get_function_groups	= msm_pmx_get_groups,
	.enable			= msm_pmx_enable,
	.disable		= msm_pmx_disable,
	.gpio_request_enable	= msm_pmx_gpio_request,
};

static int msm_pconf_prg(struct pinctrl_dev *pctldev, unsigned int pin,
				unsigned long *config, bool rw)
{
	struct msm_pinctrl_dd *dd;
	struct msm_pindesc *pindesc;
	struct msm_pintype_info *pinfo;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pindesc = dd->msm_pindesc;
	pinfo = pindesc[pin].pin_info;
	pin = pin - pinfo->pin_start;
	return pinfo->prg_cfg(pin, config, rw, pinfo);
}

static int msm_pconf_set(struct pinctrl_dev *pctldev, unsigned int pin,
				unsigned long config)
{
	return msm_pconf_prg(pctldev, pin, &config, true);
}

static int msm_pconf_get(struct pinctrl_dev *pctldev, unsigned int pin,
					unsigned long *config)
{
	return msm_pconf_prg(pctldev, pin, config, false);
}

static int msm_pconf_group_set(struct pinctrl_dev *pctldev,
			unsigned group, unsigned long config)
{
	struct msm_pinctrl_dd *dd;
	const unsigned int *pins;
	unsigned int cnt;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pins = dd->pin_grps[group].pins;

	for (cnt = 0; cnt < dd->pin_grps[group].num_pins; cnt++)
		msm_pconf_set(pctldev, pins[cnt], config);

	return 0;
}

static int msm_pconf_group_get(struct pinctrl_dev *pctldev,
				unsigned int group, unsigned long *config)
{
	struct msm_pinctrl_dd *dd;
	const unsigned int *pins;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pins = dd->pin_grps[group].pins;
	msm_pconf_get(pctldev, pins[0], config);
	return 0;
}

static struct pinconf_ops msm_pconfops = {
	.pin_config_get		= msm_pconf_get,
	.pin_config_set		= msm_pconf_set,
	.pin_config_group_get	= msm_pconf_group_get,
	.pin_config_group_set	= msm_pconf_group_set,
};

static int msm_get_grps_count(struct pinctrl_dev *pctldev)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	return dd->num_grps;
}

static const char *msm_get_grps_name(struct pinctrl_dev *pctldev,
						unsigned selector)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	return dd->pin_grps[selector].name;
}

static int msm_get_grps_pins(struct pinctrl_dev *pctldev,
		unsigned selector, const unsigned **pins, unsigned *num_pins)
{
	struct msm_pinctrl_dd *dd;

	dd = pinctrl_dev_get_drvdata(pctldev);
	*pins = dd->pin_grps[selector].pins;
	*num_pins = dd->pin_grps[selector].num_pins;
	return 0;
}

static struct msm_pintype_info *msm_pgrp_to_pintype(struct device_node *nd,
						struct msm_pinctrl_dd *dd)
{
	struct device_node *ptype_nd;
	struct msm_pintype_info *pinfo = NULL;
	int idx = 0;

	
	ptype_nd = of_parse_phandle(nd, "qcom,pins", 0);
	
	for (idx = 0; idx < dd->num_pintypes; idx++) {
		pinfo = &dd->msm_pintype[idx];
		if (ptype_nd == pinfo->node) {
			of_node_put(ptype_nd);
			break;
		}
	}
	return pinfo;
}

static int msm_dt_node_to_map(struct pinctrl_dev *pctldev,
			struct device_node *cfg_np, struct pinctrl_map **maps,
			unsigned *nmaps)
{
	struct msm_pinctrl_dd *dd;
	struct device_node *parent;
	struct msm_pindesc *pindesc;
	struct msm_pintype_info *pinfo;
	struct pinctrl_map *map;
	const char *grp_name;
	char *fn_name;
	u32 val;
	unsigned long *cfg;
	int cfg_cnt = 0, map_cnt = 0, func_cnt = 0, ret = 0;

	dd = pinctrl_dev_get_drvdata(pctldev);
	pindesc = dd->msm_pindesc;
	
	parent = of_get_parent(cfg_np);
	pinfo = msm_pgrp_to_pintype(parent, dd);
	
	if (of_find_property(parent, "qcom,pin-func", NULL))
		func_cnt++;
	
	ret = pinconf_generic_parse_dt_config(cfg_np, &cfg, &cfg_cnt);
	if (ret) {
		dev_err(dd->dev, "properties incorrect\n");
		return ret;
	}

	map_cnt = cfg_cnt + func_cnt;

	
	map = kzalloc(sizeof(*map) * map_cnt, GFP_KERNEL);
	if (!map)
		return -ENOMEM;
	*nmaps = 0;

	
	of_property_read_string(parent, "label", &grp_name);
	
	map[*nmaps].data.configs.group_or_pin = grp_name;
	map[*nmaps].data.configs.configs = cfg;
	map[*nmaps].data.configs.num_configs = cfg_cnt;
	map[*nmaps].type = PIN_MAP_TYPE_CONFIGS_GROUP;
	*nmaps += 1;

	
	if (func_cnt == 0) {
		*maps = map;
		goto no_func;
	}
	
	of_property_read_u32(parent, "qcom,pin-func", &val);
	fn_name = kzalloc(strlen(grp_name) + strlen("-func"),
						GFP_KERNEL);
	if (!fn_name) {
		ret = -ENOMEM;
		goto func_err;
	}
	snprintf(fn_name, strlen(grp_name) + strlen("-func") + 1, "%s%s",
						grp_name, "-func");
	map[*nmaps].data.mux.group = grp_name;
	map[*nmaps].data.mux.function = fn_name;
	map[*nmaps].type = PIN_MAP_TYPE_MUX_GROUP;
	*nmaps += 1;
	*maps = map;
	of_node_put(parent);
	return 0;

func_err:
	kfree(cfg);
	kfree(map);
no_func:
	of_node_put(parent);
	return ret;
}

static void msm_dt_free_map(struct pinctrl_dev *pctldev,
			     struct pinctrl_map *map, unsigned num_maps)
{
	int idx;

	for (idx = 0; idx < num_maps; idx++) {
		if (map[idx].type == PIN_MAP_TYPE_CONFIGS_GROUP)
			kfree(map[idx].data.configs.configs);
		else if (map[idx].type == PIN_MAP_TYPE_MUX_GROUP)
			kfree(map[idx].data.mux.function);
	};

	kfree(map);
}

static struct pinctrl_ops msm_pctrlops = {
	.get_groups_count	= msm_get_grps_count,
	.get_group_name		= msm_get_grps_name,
	.get_group_pins		= msm_get_grps_pins,
	.dt_node_to_map		= msm_dt_node_to_map,
	.dt_free_map		= msm_dt_free_map,
};

static int msm_pinctrl_request_gpio(struct gpio_chip *gc, unsigned offset)
{
	return pinctrl_request_gpio(gc->base + offset);
}

static void msm_pinctrl_free_gpio(struct gpio_chip *gc, unsigned offset)
{
	pinctrl_free_gpio(gc->base + offset);
}

static int msm_of_get_pin(struct device_node *np, int index,
				struct msm_pinctrl_dd *dd, uint *pin)
{
	struct of_phandle_args pargs;
	struct msm_pintype_info *pinfo, *pintype;
	int num_pintypes;
	int ret, i;

	ret = of_parse_phandle_with_args(np, "qcom,pins", "#qcom,pin-cells",
								index, &pargs);
	if (ret)
		return ret;
	pintype = dd->msm_pintype;
	num_pintypes = dd->num_pintypes;
	for (i = 0; i < num_pintypes; i++)  {
		pinfo = &pintype[i];
		
		if (pargs.np != pinfo->node)
			continue;
		
		if (pargs.args[0] > pinfo->num_pins) {
			ret = -EINVAL;
			dev_err(dd->dev, "Invalid pin number for type %s\n",
								pinfo->name);
			goto out;
		}
		*pin = pargs.args[0] + pinfo->pin_start;
	}
out:
	of_node_put(pargs.np);
	return ret;
}

static int msm_pinctrl_dt_parse_pins(struct device_node *dev_node,
						struct msm_pinctrl_dd *dd)
{
	struct device *dev;
	struct device_node *pgrp_np;
	struct msm_pin_grps *pin_grps, *curr_grp;
	struct msm_pmx_funcs *pmx_funcs, *curr_func;
	char *func_name;
	const char *grp_name;
	int ret, i, grp_index = 0, func_index = 0;
	uint pin = 0, *pins, num_grps = 0, num_pins = 0, len = 0;
	uint num_funcs = 0;
	u32 func = 0;

	dev = dd->dev;
	for_each_child_of_node(dev_node, pgrp_np) {
		if (!of_find_property(pgrp_np, "qcom,pins", NULL))
			continue;
		if (of_find_property(pgrp_np, "qcom,pin-func", NULL))
			num_funcs++;
		num_grps++;
	}

	pin_grps = (struct msm_pin_grps *)devm_kzalloc(dd->dev,
						sizeof(*pin_grps) * num_grps,
						GFP_KERNEL);
	if (!pin_grps) {
		dev_err(dev, "Failed to allocate grp desc\n");
		return -ENOMEM;
	}
	pmx_funcs = (struct msm_pmx_funcs *)devm_kzalloc(dd->dev,
						sizeof(*pmx_funcs) * num_funcs,
						GFP_KERNEL);
	if (!pmx_funcs) {
		dev_err(dev, "Failed to allocate grp desc\n");
		return -ENOMEM;
	}
	for_each_child_of_node(dev_node, pgrp_np) {
		if (!of_find_property(pgrp_np, "qcom,pins", NULL))
			continue;
		curr_grp = pin_grps + grp_index;
		
		ret = of_property_read_string(pgrp_np, "label", &grp_name);
		if (ret) {
			dev_err(dev, "Unable to allocate group name\n");
			return ret;
		}
		ret = of_property_read_u32(pgrp_np, "qcom,num-grp-pins",
								&num_pins);
		if (ret) {
			dev_err(dev, "pin count not specified for groups %s\n",
								grp_name);
			return ret;
		}
		pins = devm_kzalloc(dd->dev, sizeof(unsigned int) * num_pins,
						GFP_KERNEL);
		if (!pins) {
			dev_err(dev, "Unable to allocte pins for %s\n",
								grp_name);
			return -ENOMEM;
		}
		for (i = 0; i < num_pins; i++) {
			ret = msm_of_get_pin(pgrp_np, i, dd, &pin);
			if (ret) {
				dev_err(dev, "Pin grp %s does not have pins\n",
								grp_name);
				return ret;
			}
			pins[i] = pin;
		}
		curr_grp->pins = pins;
		curr_grp->num_pins = num_pins;
		curr_grp->name = grp_name;
		grp_index++;
		
		if (!of_find_property(pgrp_np, "qcom,pin-func", NULL))
			continue;
		curr_func = pmx_funcs + func_index;
		len = strlen(grp_name) + strlen("-func") + 1;
		func_name = devm_kzalloc(dev, len, GFP_KERNEL);
		if (!func_name) {
			dev_err(dev, "Cannot allocate func name for grp %s",
								grp_name);
			return -ENOMEM;
		}
		snprintf(func_name, len, "%s%s", grp_name, "-func");
		curr_func->name = func_name;
		curr_func->gps = devm_kzalloc(dev, sizeof(char *), GFP_KERNEL);
		if (!curr_func->gps) {
			dev_err(dev, "failed to alloc memory for group list ");
			return -ENOMEM;
		}
		of_property_read_u32(pgrp_np, "qcom,pin-func", &func);
		curr_grp->func = func;
		curr_func->gps[0] = grp_name;
		curr_func->num_grps = 1;
		func_index++;
	}
	dd->pin_grps = pin_grps;
	dd->num_grps = num_grps;
	dd->pmx_funcs = pmx_funcs;
	dd->num_funcs = num_funcs;
	return 0;
}

static void msm_populate_pindesc(struct msm_pintype_info *pinfo,
					struct msm_pindesc *msm_pindesc)
{
	int i;
	struct msm_pindesc *pindesc;

	for (i = 0; i < pinfo->num_pins; i++) {
		pindesc = &msm_pindesc[i + pinfo->pin_start];
		pindesc->pin_info = pinfo;
		snprintf(pindesc->name, sizeof(pindesc->name),
					"%s-%d", pinfo->name, i);
	}
}

static bool msm_pintype_supports_gpio(struct msm_pintype_info *pinfo)
{
	struct device_node *pt_node;

	if (!pinfo->node)
		return false;

	for_each_child_of_node(pinfo->node, pt_node) {
		if (of_find_property(pt_node, "gpio-controller", NULL)) {
			pinfo->gc.of_node = pt_node;
			pinfo->supports_gpio = true;
			return true;
		}
	}
	return false;
}

static bool msm_pintype_supports_irq(struct msm_pintype_info *pinfo)
{
	struct device_node *pt_node;

	if (!pinfo->node)
		return false;

	for_each_child_of_node(pinfo->node, pt_node) {
		if (of_find_property(pt_node, "interrupt-controller", NULL)) {
			pinfo->irq_chip->node = pt_node;
			return true;
		}
	}
	return false;
}

static int msm_pinctrl_dt_parse_pintype(struct device_node *dev_node,
						struct msm_pinctrl_dd *dd)
{
	struct device_node *pt_node;
	struct msm_pindesc *msm_pindesc;
	struct msm_pintype_info *pintype, *pinfo;
	u32 num_pins, pinfo_entries, curr_pins;
	int i, ret;
	uint total_pins = 0;

	pinfo = dd->msm_pintype;
	pinfo_entries = dd->num_pintypes;
	curr_pins = 0;

	for_each_child_of_node(dev_node, pt_node) {
		for (i = 0; i < pinfo_entries; i++) {
			pintype = &pinfo[i];
			if (strcmp(pt_node->name, pintype->name))
				continue;
			of_node_get(pt_node);
			pintype->node = pt_node;
			
			ret = of_property_read_u32(pt_node, "qcom,num-pins",
								&num_pins);
			if (ret) {
				dev_err(dd->dev, "num pins not specified\n");
				goto fail;
			}
			
			pintype->num_pins = num_pins;
			pintype->pin_start = curr_pins;
			pintype->pin_end = curr_pins + num_pins;
			pintype->set_reg_base(dd->base, pintype);
			total_pins += num_pins;
			curr_pins += num_pins;
		}
	}
	dd->msm_pindesc = devm_kzalloc(dd->dev,
						sizeof(struct msm_pindesc) *
						total_pins, GFP_KERNEL);
	if (!dd->msm_pindesc) {
		dev_err(dd->dev, "Unable to allocate msm pindesc");
		goto fail;
	}

	dd->num_pins = total_pins;
	msm_pindesc = dd->msm_pindesc;
	for (i = 0; i < pinfo_entries; i++) {
		pintype = &pinfo[i];
		
		if (!pintype->node)
			continue;
		msm_populate_pindesc(pintype, msm_pindesc);
	}
	return 0;
fail:
	for (i = 0; i < pinfo_entries; i++) {
		pintype = &pinfo[i];
		if (pintype->node)
			of_node_put(pintype->node);
	}
	return -ENOMEM;
}

#if defined(CONFIG_HTC_POWER_DEBUG) && defined(CONFIG_PINCTRL_MSM_TLMM)
static struct dentry *debugfs_base;
struct gpio_chip *g_chip;

int msm_dump_gpios(struct seq_file *m, int curr_len, char *gpio_buffer)
{
        unsigned int i, len;
        struct msm_gpio_dump_info data;
        char list_gpio[100];
        char *title_msg = "------------ MSM GPIO -------------";

        if (m) {
                seq_printf(m, "%s\n", title_msg);
        } else {
                pr_info("%s\n", title_msg);
                curr_len += sprintf(gpio_buffer + curr_len,
                "%s\n", title_msg);
        }

        for (i = 0; i < g_chip->ngpio; i++) {
                memset(list_gpio, 0 , sizeof(list_gpio));
                len = 0;
                __msm_gpio_get_dump_info(g_chip, i, &data);
                len += sprintf(list_gpio + len, "GPIO[%3d]: ", i);

                len += sprintf(list_gpio + len, "[FS]0x%x, ", data.func_sel);

                if (data.dir)
                        len += sprintf(list_gpio + len, "[DIR]OUT, [VAL]%s ", data.value ? "HIGH" : " LOW");
                else
                        len += sprintf(list_gpio + len, "[DIR] IN, [VAL]%s ", data.value ? "HIGH" : " LOW");

                switch (data.pull) {
                case 0x0:
                        len += sprintf(list_gpio + len, "[PULL]NO, ");
                        break;
                case 0x1:
                        len += sprintf(list_gpio + len, "[PULL]PD, ");
                        break;
                case 0x2:
                        len += sprintf(list_gpio + len, "[PULL]KP, ");
                        break;
                case 0x3:
                        len += sprintf(list_gpio + len, "[PULL]PU, ");
                        break;
                default:
                        break;
                }

                len += sprintf(list_gpio + len, "[DRV]%2dmA, ", 2*(data.drv+1));

                if (!data.dir) {
                        len += sprintf(list_gpio + len, "[INT]%s, ", data.int_en ? "YES" : " NO");
                        if (data.int_en) {
                                switch (data.int_owner) {
                                case 0x0:
                                        len += sprintf(list_gpio + len, " WC_PROC, ");
                                        break;
                                case 0x1:
                                        len += sprintf(list_gpio + len, "SPS_PROC, ");
                                        break;
                                case 0x2:
                                        len += sprintf(list_gpio + len, " LPA_DSP, ");
                                        break;
                                case 0x3:
                                        len += sprintf(list_gpio + len, "RPM_PROC, ");
                                        break;
                                case 0x4:
                                        len += sprintf(list_gpio + len, " KP_PROC, ");
                                        break;
                                case 0x5:
                                        len += sprintf(list_gpio + len, "MSS_PROC, ");
                                        break;
                                case 0x6:
                                        len += sprintf(list_gpio + len, " TZ_PROC, ");
                                        break;
                                case 0x7:
                                        len += sprintf(list_gpio + len, "    NONE, ");
                                        break;
                                default:
                                        break;
                                }
                        }
                }

                list_gpio[99] = '\0';
                if (m) {
                        seq_printf(m, "%s\n", list_gpio);
                } else {
                        pr_info("%s\n", list_gpio);
                        curr_len += sprintf(gpio_buffer +
                        curr_len, "%s\n", list_gpio);
                }
        }
        return curr_len;
}

static int list_gpios_show(struct seq_file *m, void *unused)
{
        msm_dump_gpios(m, 0, NULL);
	qpnp_pin_dump(m, 0, NULL);
        return 0;
}

static int list_sleep_gpios_show(struct seq_file *m, void *unused)
{
        print_gpio_buffer(m);
        return 0;
}

static int list_sleep_gpios_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_sleep_gpios_show, inode->i_private);
}

static int list_sleep_gpios_release(struct inode *inode, struct file *file)
{
        free_gpio_buffer();
        return single_release(inode, file);
}

static int list_gpios_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_gpios_show, inode->i_private);
}

static const struct file_operations list_gpios_fops = {
        .open           = list_gpios_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = seq_release,
};

static const struct file_operations list_sleep_gpios_fops = {
        .open           = list_sleep_gpios_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = list_sleep_gpios_release,
};
#endif

static void msm_pinctrl_cleanup_dd(struct msm_pinctrl_dd *dd)
{
	int i;
	struct msm_pintype_info *pintype;

	pintype = dd->msm_pintype;
	for (i = 0; i < dd->num_pintypes; i++) {
		if (pintype->node)
			of_node_put(dd->msm_pintype[i].node);
	}
}

static int msm_pinctrl_get_drvdata(struct msm_pinctrl_dd *dd,
						struct platform_device *pdev)
{
	int ret;
	struct device_node *node = pdev->dev.of_node;

	ret = msm_pinctrl_dt_parse_pintype(node, dd);
	if (ret)
		goto out;

	ret = msm_pinctrl_dt_parse_pins(node, dd);
	if (ret)
		msm_pinctrl_cleanup_dd(dd);
out:
	return ret;
}

static int msm_register_pinctrl(struct msm_pinctrl_dd *dd)
{
	int i;
	struct pinctrl_pin_desc *pindesc;
	struct msm_pintype_info *pinfo, *pintype;
	struct pinctrl_desc *ctrl_desc = &dd->pctl;

	ctrl_desc->name = "msm-pinctrl";
	ctrl_desc->owner = THIS_MODULE;
	ctrl_desc->pmxops = &msm_pmxops;
	ctrl_desc->confops = &msm_pconfops;
	ctrl_desc->pctlops = &msm_pctrlops;

	pindesc = devm_kzalloc(dd->dev, sizeof(*pindesc) * dd->num_pins,
							 GFP_KERNEL);
	if (!pindesc) {
		dev_err(dd->dev, "Failed to allocate pinctrl pin desc\n");
		return -ENOMEM;
	}

	for (i = 0; i < dd->num_pins; i++) {
		pindesc[i].number = i;
		pindesc[i].name = dd->msm_pindesc[i].name;
	}
	ctrl_desc->pins = pindesc;
	ctrl_desc->npins = dd->num_pins;
	dd->pctl_dev = pinctrl_register(ctrl_desc, dd->dev, dd);
	if (!dd->pctl_dev) {
		dev_err(dd->dev, "could not register pinctrl driver\n");
		return -EINVAL;
	}

	pinfo = dd->msm_pintype;
	for (i = 0; i < dd->num_pintypes; i++) {
		pintype = &pinfo[i];
		if (!pintype->supports_gpio)
			continue;
		pintype->grange.name = pintype->name;
		pintype->grange.id = i;
		pintype->grange.pin_base = pintype->pin_start;
		pintype->grange.base = pintype->gc.base;
		pintype->grange.npins = pintype->gc.ngpio;
		pintype->grange.gc = &pintype->gc;
		pinctrl_add_gpio_range(dd->pctl_dev, &pintype->grange);
	}
	return 0;
}

static void msm_register_gpiochip(struct msm_pinctrl_dd *dd)
{

	struct gpio_chip *gc;
	struct msm_pintype_info *pintype, *pinfo;
	int i, ret = 0;

	pinfo = dd->msm_pintype;
	for (i = 0; i < dd->num_pintypes; i++) {
		pintype = &pinfo[i];
		if (!msm_pintype_supports_gpio(pintype))
			continue;
		gc = &pintype->gc;
		gc->request = msm_pinctrl_request_gpio;
		gc->free = msm_pinctrl_free_gpio;
		gc->dev = dd->dev;
		gc->ngpio = pintype->num_pins;
		gc->base = -1;
		ret = gpiochip_add(gc);
		if (ret) {
			dev_err(dd->dev, "failed to register gpio chip\n");
			pinfo->supports_gpio = false;
		}
	}
#if defined(CONFIG_HTC_POWER_DEBUG) && defined(CONFIG_PINCTRL_MSM_TLMM)
        g_chip = gc;
#endif

}

static int msm_register_irqchip(struct msm_pinctrl_dd *dd)
{
	struct msm_pintype_info *pintype, *pinfo;
	int i, ret = 0;

	pinfo = dd->msm_pintype;
	for (i = 0; i < dd->num_pintypes; i++) {
		pintype = &pinfo[i];
		if (!msm_pintype_supports_irq(pintype))
			continue;
		ret = pintype->init_irq(dd->irq, pintype, dd->dev);
		return ret;
	}
	return 0;
}

int msm_pinctrl_probe(struct platform_device *pdev,
					struct msm_tlmm_desc *tlmm_info)
{
	struct msm_pinctrl_dd *dd;
	struct device *dev = &pdev->dev;
	int ret;

	dd = devm_kzalloc(dev, sizeof(*dd), GFP_KERNEL);
	if (!dd) {
		dev_err(dev, "Alloction failed for driver data\n");
		return -ENOMEM;
	}
	dd->dev = dev;
	dd->msm_pintype = tlmm_info->pintypes;
	dd->base = tlmm_info->base;
	dd->irq = tlmm_info->irq;
	dd->num_pintypes = tlmm_info->num_pintypes;
	ret = msm_pinctrl_get_drvdata(dd, pdev);
	if (ret) {
		dev_err(&pdev->dev, "driver data not available\n");
		return ret;
	}
	msm_register_gpiochip(dd);
	ret = msm_register_pinctrl(dd);
	if (ret) {
		msm_pinctrl_cleanup_dd(dd);
		return ret;
	}
	msm_register_irqchip(dd);
	platform_set_drvdata(pdev, dd);
#if defined(CONFIG_HTC_POWER_DEBUG) && defined(CONFIG_PINCTRL_MSM_TLMM)
        debugfs_base = debugfs_create_dir("htc_gpio", NULL);
        if (!debugfs_base)
                return -ENOMEM;

        if (!debugfs_create_file("list_gpios", S_IRUGO, debugfs_base,
                        &g_chip, &list_gpios_fops))
                return -ENOMEM;

        if (!debugfs_create_file("list_sleep_gpios", S_IRUGO, debugfs_base,
                       &g_chip, &list_sleep_gpios_fops))
                return -ENOMEM;
#endif
	return 0;
}
EXPORT_SYMBOL(msm_pinctrl_probe);

#ifdef CONFIG_USE_PINCTRL_IRQ
struct irq_chip mpm_tlmm_irq_extn = {
	.irq_eoi	= NULL,
	.irq_mask	= NULL,
	.irq_unmask	= NULL,
	.irq_retrigger	= NULL,
	.irq_set_type	= NULL,
	.irq_set_wake	= NULL,
	.irq_disable	= NULL,
};

struct msm_irq_of_info msm_tlmm_irq[] = {
#ifdef CONFIG_PINCTRL_MSM_TLMM
	{
		.compat = "qcom,msm-tlmm-gp",
		.irq_init = msm_tlmm_of_gp_irq_init,
	},
#endif
};

int __init msm_tlmm_of_irq_init(struct device_node *controller,
						struct device_node *parent)
{
	int rc, i;
	const char *compat;

	rc = of_property_read_string(controller, "compatible", &compat);
	if (rc)
		return rc;

	for (i = 0; i < ARRAY_SIZE(msm_tlmm_irq); i++) {
		struct msm_irq_of_info *tlmm_info = &msm_tlmm_irq[i];

		if (!of_compat_cmp(tlmm_info->compat, compat, strlen(compat)))
				return tlmm_info->irq_init(controller,
							&mpm_tlmm_irq_extn);

	}
	return -EIO;
}
#endif
