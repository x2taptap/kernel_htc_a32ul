/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#ifndef __QPNP_LINEAR_CHARGER_H
#define __QPNP_LINEAR_CHARGER_H

#include <linux/errno.h>
#include <linux/power_supply.h>
#ifdef CONFIG_HTC_BATT_8960
#include <linux/power/htc_charger.h>
#endif

#define FALSE       0
#define TRUE        1

struct ext_usb_chg_pm8916 {
	const char	*name;
	void		*ctx;
	int		(*start_charging) (void *ctx);
	int		(*stop_charging) (void *ctx);
	bool		(*is_trickle) (void *ctx);
	struct htc_charger	*ichg;
};

#ifdef CONFIG_QPNP_LINEAR_CHARGER
#ifdef CONFIG_HTC_BATT_8960
int pm8916_is_pwr_src_plugged_in(void);
int pm8916_get_batt_temperature(int *result);
int pm8916_get_batt_voltage(int *result);
int pm8916_get_batt_current(int *result);
int pm8916_get_batt_id(int *result);
int pm8916_get_batt_soc(int *result);
int pm8916_get_batt_cc(int *result);
int pm8916_get_batt_present(void);
int pm8916_get_batt_status(void);
int pm8916_vm_bms_report_eoc(void);
int pm8916_is_batt_temp_fault_disable_chg(int *result);
int pm8916_is_batt_temperature_fault(int *result);
int pm8916_set_pwrsrc_and_charger_enable(enum htc_power_source_type src,
			bool chg_enable, bool pwrsrc_enable);
int pm8916_charger_enable(bool enable);
int pm8916_charger_disable(bool enable);
int pm8916_pwrsrc_enable(bool enable);
int pm8916_get_battery_status(void);

int pm8916_get_charging_source(int *result);
int pm8916_get_charging_enabled(int *result);
int pm8916_get_charge_type(void);
int pm8916_get_chg_usb_iusbmax(void);
int pm8916_get_chg_vinmin(void);
int pm8916_get_input_voltage_regulation(void);
int pm8916_set_chg_vin_min(int val);
int pm8916_set_chg_iusbmax(int val);
int pm8916_is_charger_ovp(int* result);
int pm8916_set_hsml_target_ma(int target_ma);
int pm8916_dump_all(void);
int pm8916_is_batt_full(int *result);
int pm8916_is_batt_full_eoc_stop(int *result);
int pm8916_charger_get_attr_text(char *buf, int size);
int pm8916_gauge_get_attr_text(char *buf, int size);
int pm8916_fake_chg_gone_irq_handler(void);
int pm8916_fake_usbin_valid_irq_handler(void);
int pm8916_fake_coarse_det_usb_irq_handler(void);
int pm8916_get_usb_temperature(int *result);
int pm8916_mfg_get_charging_state(int *result);
bool pm8916_get_batt_sw_control(void);
#ifdef CONFIG_DUTY_CYCLE_LIMIT
int pm8916_limit_charge_enable(int chg_limit_reason,
			 int chg_limit_timer_sub_mask,
			 int limit_charge_timer_ma);
#else
int pm8916_limit_charge_enable(bool enable);
#endif
int pm8916_is_chg_safety_timer_timeout(int *result);
#endif
#else 
#ifdef CONFIG_HTC_BATT_8960
static inline int pm8916_is_pwr_src_plugged_in(void)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_temperature(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_voltage(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_current(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_id(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_soc(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_cc(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_status(void)
{
	return -ENXIO;
}
static inline int pm8916_is_batt_temp_fault_disable_chg(int *result)
{
	return -ENXIO;
}
static inline int pm8916_vm_bms_report_eoc(void)
{
	return -ENXIO;
}
static inline int pm8916_is_batt_temperature_fault(int *result)
{
	return -ENXIO;
}
static inline int pm8916_set_pwrsrc_and_charger_enable(enum htc_power_source_type src,
			bool chg_enable, bool pwrsrc_enable)
{
	return -ENXIO;
}
static inline int pm8916_charger_enable(bool enable)
{
	return -ENXIO;
}
static inline int pm8916_charger_disable(bool enable)
{
	return -ENXIO;
}
static inline int pm8916_is_charger_ovp(int* result)
{
	return -ENXIO;
}
static inline int pm8916_pwrsrc_enable(bool enable)
{
	return -ENXIO;
}
static inline int pm8916_get_battery_status(void)
{
	return -ENXIO;
}
static inline int pm8916_get_batt_present(void)
{
	return -ENXIO;
}
static inline int pm8916_get_charging_source(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_charging_enabled(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_charge_type(void)
{
	return -ENXIO;
}
static inline int pm8916_get_chg_usb_iusbmax(void)
{
	return -ENXIO;
}
static inline int pm8916_get_chg_vinmin(void)
{
	return -ENXIO;
}
static inline int pm8916_get_input_voltage_regulation(void)
{
	return -ENXIO;
}
static inline int pm8916_set_chg_vin_min(int val)
{
	return -ENXIO;
}
static inline int pm8916_set_chg_iusbmax(int val)
{
	return -ENXIO;
}
static inline int pm8916_set_hsml_target_ma(int target_ma)
{
	return -ENXIO;
}
static inline int pm8916_dump_all(void)
{
	return -ENXIO;
}
static inline int pm8916_is_batt_full(int *result)
{
	return -ENXIO;
}
static inline int pm8916_is_batt_full_eoc_stop(int *result)
{
	return -ENXIO;
}
static inline int pm8916_charger_get_attr_text(char *buf, int size)
{
	return -ENXIO;
}
static inline int pm8916_fake_chg_gone_irq_handler(void)
{
	return -ENXIO;
}
static inline int pm8916_fake_usbin_valid_irq_handler(void)
{
	return -ENXIO;
}
static inline int pm8916_fake_coarse_det_usb_irq_handler(void)
{
	return -ENXIO;
}
static inline int pm8916_is_chg_safety_timer_timeout(int *result)
{
	return -ENXIO;
}
static inline int pm8916_get_usb_temperature(int *result)
{
	return -ENXIO;
}
static inline int pm8916_mfg_get_charging_state(int *result)
{
	return -ENXIO;
}

static inline bool pm8916_get_batt_sw_control(void)
{
	return -ENXIO;
}

#ifdef CONFIG_DUTY_CYCLE_LIMIT
static inline int pm8916_limit_charge_enable(int chg_limit_reason,
			 int chg_limit_timer_sub_mask,
			 int limit_charge_timer_ma)
#else
static inline int pm8916_limit_charge_enable(bool enable)
{
	return -ENXIO;
}
#endif
#endif 
#endif 
#endif 

