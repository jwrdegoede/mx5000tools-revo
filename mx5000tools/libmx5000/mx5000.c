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

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <asm/types.h>
#include <linux/hiddev.h>

#include "libmx5000/mx5000.h"

#define HIDDEVDIR "/dev/usb/"

int mx5000_open_path(const char *path)
{
  int fd = -1;
  int err = 0;
  char buf[256];

  if (!path)
    return -1;

  fd = open(path, O_RDWR);
  if (fd<0)
    return fd;

  err = ioctl(fd, HIDIOCGNAME(256) , buf);
  if (err < 0)
    goto close;
  if (strcmp("Logitech Logitech BT Mini-Receiver", buf))
    goto close;

  return fd;

 close:
  close(fd);

  return -1;
}

int mx5000_open(void)
{
  int fd = -1;
  struct dirent *dirent;
  DIR *dir;
  char devname[PATH_MAX];

  dir = opendir(HIDDEVDIR);
  if (dir == NULL)
    return -1;

  while((dirent = readdir(dir)) != NULL) {
    if (dirent->d_type != DT_CHR)
      continue;

    strcpy(devname, HIDDEVDIR);
    strcat(devname, dirent->d_name);

    fd = mx5000_open_path(devname);

    if (fd >= 0)
      break;
  }

  closedir(dir);

  return fd;
}




int mx5000_send_report(int fd, const char *_buf, __u32 reportid)
{
  struct hiddev_report_info rinfo;
  struct hiddev_usage_ref uref;
  const unsigned char *buf = (const void *)_buf;
  int size;
  __u32 usage_code;
  int i, err;
  
  switch(reportid) {
  case 0x10:
    size = 6;
    usage_code = 0xff000001;
    break;
  case 0x11:
    size = 19;
    usage_code = 0xff000002;
    break;
  case 0x12:
    size = 45;
    usage_code = 0xff000003;
    break;
  default:
    return -1;
  }


  for (i = 0; i < size; i++) {
    memset(&uref, 0, sizeof(uref));
    uref.report_type = HID_REPORT_TYPE_OUTPUT;
    uref.report_id   = reportid;
    uref.field_index = 0;
    uref.usage_index = i;
    uref.usage_code = usage_code;
    uref.value       = buf[i];
    err = ioctl(fd, HIDIOCSUSAGE, &uref);
    if (err < 0) {
      fprintf(stderr, "error %d, errno %d\n", err, errno);
      return err;
    }
  }

  memset(&rinfo, 0, sizeof(rinfo));
  rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
  rinfo.report_id   = reportid;
  rinfo.num_fields  = 1;
  err = ioctl(fd, HIDIOCSREPORT, &rinfo);

  return err;
}

void mx5000_set_icons(int fd, enum iconstatus email, enum iconstatus messenger, 
		    enum  iconstatus mute, enum iconstatus walkie )
{
  char icons[] = { 0x01, 0x82, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  
  icons[3] = email;
  icons[4] = messenger;
  icons[5] = mute;
  icons[7] = walkie;

  mx5000_send_report(fd, icons, 0x11);
}

void mx5000_set_temp_unit(int fd, int isfarenheight )
{
  const char tempunit1[] = { 0x01, 0x81, 0x30, 0x00, 0x00, 0x00 };
  char tempunit2[] =       { 0x01, 0x80, 0x30, 0xDC, 0x00, 0x00 };

  tempunit2[5] = (isfarenheight)?(0x01):(0x00);

  mx5000_send_report(fd, tempunit1, 0x10);
  mx5000_send_report(fd, tempunit2, 0x10);

}



void mx5000_set_kbd_opts(int fd, enum kbdopts opts)
{
  const char keyopts1[] = { 0x01, 0x81, 0x01, 0x00, 0x00, 0x00 }; 
  char keyopts2[] =       { 0x01, 0x80, 0x01, 0x14, 0x00, 0x00 };

  keyopts2[5] = opts;


  mx5000_send_report(fd, keyopts1, 0x10);
  usleep(50*1000);
  mx5000_send_report(fd, keyopts2, 0x10);

}

void mx5000_set_time(int fd, time_t mytime)
{
  int a,b;
  struct tm mytm;
  
  char daymonth[] = {   0x01, 0x80, 0x32, 0x00, 0x00, 0x00 };
  char hourminute[] = { 0x01, 0x80, 0x31, 0x19, 0x00, 0x00 };

  localtime_r(&mytime, &mytm);


  daymonth[4]= mytm.tm_mday;
  daymonth[5]= mytm.tm_mon;

  hourminute[4] = mytm.tm_min;
  hourminute[5] = mytm.tm_hour;
  
  a = mx5000_send_report(fd, daymonth, 0x10);
  b = mx5000_send_report(fd, hourminute, 0x10);

}


void mx5000_beep(int fd)
{
  const char beep1[] = { 0x01, 0x81, 0x50, 0x00, 0x00, 0x00 };
  const char beep2[] = { 0x01, 0x80, 0x50, 0x02, 0x00, 0x00 };

  mx5000_send_report(fd, beep1, 0x10);
  mx5000_send_report(fd, beep2, 0x10);

}



void mx5000_set_name(int fd, char buf[14], int len)
{
  char line2[19] = { 0x01, 0x82, 0x34, 0x04, 0x01, 
		     0x00, 0x00, 0x00, 0x00, 0x00, 
		     0x00, 0x00, 0x80, 0x00, 0x00, 
		     0x00, 0xFB, 0x12, 0x00 };
  
  if (len < 0)
    len = strlen(buf);

  if (len > 11)
    len = 11;

  line2[3] = len+1;

  memcpy(line2+5, buf, len);



  mx5000_send_report(fd, line2, 0x11);

}
