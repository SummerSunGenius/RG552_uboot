/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/arch/periph.h>
#include <power/regulator.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <spl.h>
DECLARE_GLOBAL_DATA_PTR;

int rk_board_init(void)
{
	struct udevice *pinctrl, *regulator;
	int ret;

	/*
	 * The PWM does not have decicated interrupt number in dts and can
	 * not get periph_id by pinctrl framework, so let's init them here.
	 * The PWM2 and PWM3 are for pwm regulators.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto out;
	}

	/* Enable pwm0 for panel backlight */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM0);
	if (ret) {
		debug("%s PWM0 pinctrl init fail! (ret=%d)\n", __func__, ret);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM2);
	if (ret) {
		debug("%s PWM2 pinctrl init fail!\n", __func__);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM3);
	if (ret) {
		debug("%s PWM3 pinctrl init fail!\n", __func__);
		goto out;
	}

	ret = regulator_get_by_platname("vcc5v0_host", &regulator);
	if (ret) {
		debug("%s vcc5v0_host init fail! ret %d\n", __func__, ret);
		goto out;
	}

	ret = regulator_set_enable(regulator, true);
	if (ret) {
		debug("%s vcc5v0-host-en set fail!\n", __func__);
		goto out;
	}

out:
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xfe800000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.usb2_phyif_utmi_width = 16,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif
