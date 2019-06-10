// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility lib for using hidraw devices in LCDd drivers.
 *
 * Copyright (C) 2017 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef HIDRAW_LIB_H
#define HIDRAW_LIB_H

#include <asm/types.h>
#include <linux/hidraw.h>
#include <linux/input.h>

#define LIB_HIDRAW_DESC_HDR_SZ		16

struct lib_hidraw_id {
	/* An entry entirely filled with zeros terminates the list of ids */
	struct hidraw_devinfo devinfo;
	/*
	 * Optional, may be used on devices with multiple USB interfaces to
	 * pick the right interface.
	 */
	unsigned char descriptor_header[LIB_HIDRAW_DESC_HDR_SZ];
};

int lib_hidraw_find_device(const struct lib_hidraw_id *ids);
int lib_hidraw_open_device(const char *device,
			   const struct lib_hidraw_id *ids);
int lib_hidraw_send_output_report(int fd, const struct lib_hidraw_id *ids,
				  unsigned char *data, int count);
#endif
