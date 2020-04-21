/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#include <linux/regmap.h>
#include <linux/device.h>
#include "wsa881x-registers.h"

const u8 wsa881x_reg_readable[WSA881X_CACHE_SIZE] = {
	[WSA881X_CHIP_ID0] = 1,
	[WSA881X_CHIP_ID1] = 1,
	[WSA881X_CHIP_ID2] = 1,
	[WSA881X_CHIP_ID3] = 1,
	[WSA881X_BUS_ID] = 1,
	[WSA881X_CDC_RST_CTL] = 1,
	[WSA881X_CDC_TOP_CLK_CTL] = 1,
	[WSA881X_CDC_ANA_CLK_CTL] = 1,
	[WSA881X_CDC_DIG_CLK_CTL] = 1,
	[WSA881X_CLOCK_CONFIG] = 1,
	[WSA881X_ANA_CTL] = 1,
	[WSA881X_SWR_RESET_EN] = 1,
	[WSA881X_RESET_CTL] = 1,
	[WSA881X_TADC_VALUE_CTL] = 1,
	[WSA881X_TEMP_DETECT_CTL] = 1,
	[WSA881X_TEMP_MSB] = 1,
	[WSA881X_TEMP_LSB] = 1,
	[WSA881X_TEMP_CONFIG0] = 1,
	[WSA881X_TEMP_CONFIG1] = 1,
	[WSA881X_CDC_CLIP_CTL] = 1,
	[WSA881X_SDM_PDM9_LSB] = 1,
	[WSA881X_SDM_PDM9_MSB] = 1,
	[WSA881X_CDC_RX_CTL] = 1,
	[WSA881X_DEM_BYPASS_DATA0] = 1,
	[WSA881X_DEM_BYPASS_DATA1] = 1,
	[WSA881X_DEM_BYPASS_DATA2] = 1,
	[WSA881X_DEM_BYPASS_DATA3] = 1,
	[WSA881X_OTP_CTRL0] = 1,
	[WSA881X_OTP_CTRL1] = 1,
	[WSA881X_HDRIVE_CTL_GROUP1] = 1,
	[WSA881X_INTR_MODE] = 1,
	[WSA881X_INTR_MASK] = 1,
	[WSA881X_INTR_STATUS] = 1,
	[WSA881X_INTR_CLEAR] = 1,
	[WSA881X_INTR_LEVEL] = 1,
	[WSA881X_INTR_SET] = 1,
	[WSA881X_INTR_TEST] = 1,
	[WSA881X_PDM_TEST_MODE] = 1,
	[WSA881X_ATE_TEST_MODE] = 1,
	[WSA881X_PIN_CTL_MODE] = 1,
	[WSA881X_PIN_CTL_OE] = 1,
	[WSA881X_PIN_WDATA_IOPAD] = 1,
	[WSA881X_PIN_STATUS] = 1,
	[WSA881X_DIG_DEBUG_MODE] = 1,
	[WSA881X_DIG_DEBUG_SEL] = 1,
	[WSA881X_DIG_DEBUG_EN] = 1,
	[WSA881X_SWR_HM_TEST1] = 1,
	[WSA881X_SWR_HM_TEST2] = 1,
	[WSA881X_TEMP_DETECT_DBG_CTL] = 1,
	[WSA881X_TEMP_DEBUG_MSB] = 1,
	[WSA881X_TEMP_DEBUG_LSB] = 1,
	[WSA881X_SAMPLE_EDGE_SEL] = 1,
	[WSA881X_IOPAD_CTL] = 1,
	[WSA881X_SPARE_0] = 1,
	[WSA881X_SPARE_1] = 1,
	[WSA881X_SPARE_2] = 1,
	[WSA881X_OTP_REG_0] = 1,
	[WSA881X_OTP_REG_1] = 1,
	[WSA881X_OTP_REG_2] = 1,
	[WSA881X_OTP_REG_3] = 1,
	[WSA881X_OTP_REG_4] = 1,
	[WSA881X_OTP_REG_5] = 1,
	[WSA881X_OTP_REG_6] = 1,
	[WSA881X_OTP_REG_7] = 1,
	[WSA881X_OTP_REG_8] = 1,
	[WSA881X_OTP_REG_9] = 1,
	[WSA881X_OTP_REG_10] = 1,
	[WSA881X_OTP_REG_11] = 1,
	[WSA881X_OTP_REG_12] = 1,
	[WSA881X_OTP_REG_13] = 1,
	[WSA881X_OTP_REG_14] = 1,
	[WSA881X_OTP_REG_15] = 1,
	[WSA881X_OTP_REG_16] = 1,
	[WSA881X_OTP_REG_17] = 1,
	[WSA881X_OTP_REG_18] = 1,
	[WSA881X_OTP_REG_19] = 1,
	[WSA881X_OTP_REG_20] = 1,
	[WSA881X_OTP_REG_21] = 1,
	[WSA881X_OTP_REG_22] = 1,
	[WSA881X_OTP_REG_23] = 1,
	[WSA881X_OTP_REG_24] = 1,
	[WSA881X_OTP_REG_25] = 1,
	[WSA881X_OTP_REG_26] = 1,
	[WSA881X_OTP_REG_27] = 1,
	[WSA881X_OTP_REG_28] = 1,
	[WSA881X_OTP_REG_29] = 1,
	[WSA881X_OTP_REG_30] = 1,
	[WSA881X_OTP_REG_31] = 1,
	[WSA881X_OTP_REG_63] = 1,
	
	[WSA881X_BIAS_REF_CTRL] = 1,
	[WSA881X_BIAS_TEST] = 1,
	[WSA881X_BIAS_BIAS] = 1,
	[WSA881X_TEMP_OP] = 1,
	[WSA881X_TEMP_IREF_CTRL] = 1,
	[WSA881X_TEMP_ISENS_CTRL] = 1,
	[WSA881X_TEMP_CLK_CTRL] = 1,
	[WSA881X_TEMP_TEST] = 1,
	[WSA881X_TEMP_BIAS] = 1,
	[WSA881X_TEMP_ADC_CTRL] = 1,
	[WSA881X_TEMP_DOUT_MSB] = 1,
	[WSA881X_TEMP_DOUT_LSB] = 1,
	[WSA881X_ADC_EN_MODU_V] = 1,
	[WSA881X_ADC_EN_MODU_I] = 1,
	[WSA881X_ADC_EN_DET_TEST_V] = 1,
	[WSA881X_ADC_EN_DET_TEST_I] = 1,
	[WSA881X_ADC_SEL_IBIAS] = 1,
	[WSA881X_ADC_EN_SEL_IBAIS] = 1,
	[WSA881X_SPKR_DRV_EN] = 1,
	[WSA881X_SPKR_DRV_GAIN] = 1,
	[WSA881X_SPKR_DAC_CTL] = 1,
	[WSA881X_SPKR_DRV_DBG] = 1,
	[WSA881X_SPKR_PWRSTG_DBG] = 1,
	[WSA881X_SPKR_OCP_CTL] = 1,
	[WSA881X_SPKR_CLIP_CTL] = 1,
	[WSA881X_SPKR_BBM_CTL] = 1,
	[WSA881X_SPKR_MISC_CTL1] = 1,
	[WSA881X_SPKR_MISC_CTL2] = 1,
	[WSA881X_SPKR_BIAS_INT] = 1,
	[WSA881X_SPKR_PA_INT] = 1,
	[WSA881X_SPKR_BIAS_CAL] = 1,
	[WSA881X_SPKR_BIAS_PSRR] = 1,
	[WSA881X_SPKR_STATUS1] = 1,
	[WSA881X_SPKR_STATUS2] = 1,
	[WSA881X_BOOST_EN_CTL] = 1,
	[WSA881X_BOOST_CURRENT_LIMIT] = 1,
	[WSA881X_BOOST_PS_CTL] = 1,
	[WSA881X_BOOST_PRESET_OUT1] = 1,
	[WSA881X_BOOST_PRESET_OUT2] = 1,
	[WSA881X_BOOST_FORCE_OUT] = 1,
	[WSA881X_BOOST_LDO_PROG] = 1,
	[WSA881X_BOOST_SLOPE_COMP_ISENSE_FB] = 1,
	[WSA881X_BOOST_RON_CTL] = 1,
	[WSA881X_BOOST_LOOP_STABILITY] = 1,
	[WSA881X_BOOST_ZX_CTL] = 1,
	[WSA881X_BOOST_START_CTL] = 1,
	[WSA881X_BOOST_MISC1_CTL] = 1,
	[WSA881X_BOOST_MISC2_CTL] = 1,
	[WSA881X_BOOST_MISC3_CTL] = 1,
	[WSA881X_BOOST_ATEST_CTL] = 1,
	[WSA881X_SPKR_PROT_FE_GAIN] = 1,
	[WSA881X_SPKR_PROT_FE_CM_LDO_SET] = 1,
	[WSA881X_SPKR_PROT_FE_ISENSE_BIAS_SET1] = 1,
	[WSA881X_SPKR_PROT_FE_ISENSE_BIAS_SET2] = 1,
	[WSA881X_SPKR_PROT_ATEST1] = 1,
	[WSA881X_SPKR_PROT_ATEST2] = 1,
	[WSA881X_SPKR_PROT_FE_VSENSE_VCM] = 1,
	[WSA881X_SPKR_PROT_FE_VSENSE_BIAS_SET1] = 1,
	[WSA881X_BONGO_RESRV_REG1] = 1,
	[WSA881X_BONGO_RESRV_REG2] = 1,
	[WSA881X_SPKR_PROT_SAR] = 1,
	[WSA881X_SPKR_STATUS3] = 1,
};
