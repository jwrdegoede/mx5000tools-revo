/* mx5000tools
 * Copyright (C) 2006 Olivier Crete
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <asm/types.h>
#include <linux/hiddev.h>


int main(int argc, char **argv)
{

  int fd = -1;
  const char devname[] = "/dev/usb/hiddev0";
  struct hiddev_devinfo dinfo;
  struct hiddev_report_info rinfo;
  struct hiddev_field_info finfo;
  struct hiddev_usage_ref uref;
  int i,j;
  int ret = 0, ret2 = 0;
  

  int err = 0;
  int flags;
  char setbutton[] =  { 0x01, 0x80, 0x00, 0x09, 0x01, 0x00 };
  char resetbutton[] ={ 0x01, 0x80, 0x00, 0x00, 0x01, 0x00 };

  fd = open(devname, O_RDWR);
  if (fd < 0) {
    perror("opening devfile");
    return 1;
  }


  memset(&dinfo, 0, sizeof(dinfo));
  err = ioctl(fd, HIDIOCGDEVINFO, &dinfo);

  if (ioctl < 0) {
    perror("gdevinfo");
    goto out;
  }

  printf("bustype: %u busnum %u devnum %u ifnum %u vendor %x product %x version %x numapp: %u\n",
	 dinfo.bustype,
	 dinfo.busnum,
	 dinfo.devnum,
	 dinfo.ifnum,
	 dinfo.vendor,
	 dinfo.product,
	 dinfo.version,
	 dinfo.num_applications);


  
  rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
  rinfo.report_id = HID_REPORT_ID_FIRST;
  ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
  if (ret < 0) printf("getreport (ret: %d)\n", ret);
  printf("reportinfo report_type: %u report_id: 0x%x numfields:%d\n", rinfo.report_type, rinfo.report_id, rinfo.num_fields);
 

  while (ret >= 0) {
    for (i = 0; i < rinfo.num_fields; i++) { 
      finfo.report_type = rinfo.report_type;
      finfo.report_id = rinfo.report_id;
      finfo.field_index = i;
      ioctl(fd, HIDIOCGFIELDINFO, &finfo);
      if (ret<0) printf("getfieldinfo (ret: %d errno: %d)\n", ret, errno);
      printf("fieldinfo report_type: %u reportid: 0x%x field_index: %u, maxusage: %u flags: 0x%x physical 0x%x logical %u application 0x%x logicalmin %d logicalmax %d physicalmin %d physicalmax %d unit_exponent %u unit %u\n",
	     finfo.report_type,
	     finfo.report_id,
	     finfo.field_index,
	     finfo.maxusage,
	     finfo.flags,
	     finfo.physical,		/* physical usage for this field */
	     finfo.logical,		/* logical usage for this field */
	     finfo.application,		/* application usage for this field */
	     finfo.logical_minimum,
	     finfo.logical_maximum,
	     finfo.physical_minimum,
	     finfo.physical_maximum,
	     finfo.unit_exponent,
	     finfo.unit);
#if 1
      for (j = 0; j < finfo.maxusage; j++) {
	uref.report_type = rinfo.report_type;
	uref.report_id = rinfo.report_id;
	uref.field_index = i;
	uref.usage_index = j;
	ret2 = ioctl(fd, HIDIOCGUCODE, &uref);
	if (ret2<0) printf("getucode (ret2: %d errno: %d\n", ret2, errno);
	ret2 = ioctl(fd, HIDIOCGUSAGE, &uref);
	if (ret2<0) printf("getusage (ret2: %d errno: %d)\n", ret2, errno);
	printf("usageinfo usage_index: %d usage_code 0x%x value: 0x%x\n", uref.usage_index, uref.usage_code, uref.value);
      }
#endif
    }
    rinfo.report_id |= HID_REPORT_ID_NEXT;
    ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
    if (ret<0) printf("getreport (ret: %d)\n", ret);
    printf("reportinfo report_type: %u report_id: 0x%x numfields:%d\n", rinfo.report_type, rinfo.report_id, rinfo.num_fields);

  }


 out:
  close(fd);

  return err;
}
