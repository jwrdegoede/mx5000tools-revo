// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility lib for using hidraw devices in libmx5000.
 *
 * Copyright (C) 2019 Hans de Goede <hdegoede@redhat.com>
 */
 
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hidraw_lib.h"

int lib_hidraw_open_device(const char *device,
			   const struct lib_hidraw_id *ids)
{
	struct hidraw_report_descriptor descriptor;
	struct hidraw_devinfo devinfo;
	int i, err, fd;

	fd = open(device, O_RDWR);
	if (fd == -1)
		return -1;

	err = ioctl(fd, HIDIOCGRAWINFO, &devinfo);
	if (err == -1) {
		close(fd);
		return -1;
	}

	descriptor.size = LIB_HIDRAW_DESC_HDR_SZ;
	err = ioctl(fd, HIDIOCGRDESC, &descriptor);
	if (err == -1) {
		close(fd);
		return -1;
	}

	for (i = 0; ids[i].devinfo.bustype; i++) {
		if (memcmp(&devinfo, &ids[i].devinfo, sizeof(devinfo)))
			continue;

		if (ids[i].descriptor_header[0] == 0 ||
		    (descriptor.size >= LIB_HIDRAW_DESC_HDR_SZ &&
		     memcmp(descriptor.value, ids[i].descriptor_header,
			    LIB_HIDRAW_DESC_HDR_SZ) == 0))
			break; /* Found it */
	}
	if (!ids[i].devinfo.bustype) {
		close(fd);
		return -1;
	}

	return fd;
}

int lib_hidraw_find_device(const struct lib_hidraw_id *ids)
{
	char devname[PATH_MAX];
	struct dirent *dirent;
	int fd = -1;
	DIR *dir;

	dir = opendir("/dev");
	if (dir == NULL)
		return -1;

	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_type != DT_CHR ||
		    strncmp(dirent->d_name, "hidraw", 6))
			continue;

		strcpy(devname, "/dev/");
		strcat(devname, dirent->d_name);

		fd = lib_hidraw_open_device(devname, ids);
		if (fd != -1)
			break;
	}

	closedir(dir);

	return fd;
}

int lib_hidraw_send_output_report(int fd, const struct lib_hidraw_id *ids,
				  unsigned char *data, int count)
{
	int result;

	result = write(fd, data, count);
	/*
	 * We may temporary loose access to the device. Possible causes are e.g.:
	 * 1. A temporary connection loss (Bluetooth); or
	 * 2. The device dropping of the bus to re-appear with another prod-id
	 *    (the G510 keyboard does this when (un)plugging the headphones).
	 * If the device was opened by name, we try to re-acquire the device
	 * here to deal with these kinda temporary device losses.
	 */
	if (result == -1 && errno == ENODEV) {
		int new_fd = lib_hidraw_find_device(ids);
		if (new_fd != -1) {
			fprintf(stderr, "Successfully re-opened mx5000 hidraw device\n");
			dup2(new_fd, fd);
			close(new_fd);
			result = write(fd, data, count);
		}
	}

	return result;
}
