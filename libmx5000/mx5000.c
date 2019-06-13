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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <unistd.h>
#include "libmx5000/mx5000.h"
#include "libmx5000/hidraw_lib.h"

typedef unsigned short u16;

static const struct lib_hidraw_id hidraw_ids[] = {
	/* MX5000 */
	{ { BUS_USB,       0x046d, 0xb305 } },
	{ { BUS_BLUETOOTH, 0x046d, 0xb305 } },
	/* MX5500 */
	{ { BUS_USB,       0x046d, 0xb30b } },
	{ { BUS_BLUETOOTH, 0x046d, 0xb30b } },
	/* Terminator */
	{}
};

int mx5000_open_path(const char *path)
{
  /*
   * Backward compat workaround, if we are asked to open an old style usb
   * hiddev device, fallback to automatically selecting the first mx5000/mx5500
   * hidraw device.
   */
  if (strstr(path, "hiddev"))
    return lib_hidraw_find_device(hidraw_ids);

  return lib_hidraw_open_device(path, hidraw_ids);
}

int mx5000_open(void)
{
  return lib_hidraw_find_device(hidraw_ids);
}

int mx5000_send_report(int fd, const char *_buf, __u32 reportid)
{
  unsigned char buf[46];
  int err, size;
  
  switch(reportid) {
  case 0x10:
    size = 6;
    break;
  case 0x11:
    size = 19;
    break;
  case 0x12:
    size = 45;
    break;
  default:
    return -1;
  }

  buf[0] = reportid;
  memcpy(buf + 1, _buf, size);
  err = lib_hidraw_send_output_report(fd, hidraw_ids, buf, size + 1);
  return (err == (size + 1)) ? 0 : err;
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
  struct tm mytm;
  
  char hourminute[] = { 0x01, 0x80, 0x31, 0x00, 0x00, 0x00 };
  char daymonth[] = {   0x01, 0x80, 0x32, 0x00, 0x00, 0x00 };
  char year[] = {       0x01, 0x80, 0x33, 0x00, 0x00, 0x00 };

  localtime_r(&mytime, &mytm);

  daymonth[3]= (mytm.tm_wday + 6) % 7;
  daymonth[4]= mytm.tm_mday;
  daymonth[5]= mytm.tm_mon;

  hourminute[3] = mytm.tm_sec;
  hourminute[4] = mytm.tm_min;
  hourminute[5] = mytm.tm_hour;

  year[3] = mytm.tm_year % 100;

  mx5000_send_report(fd, hourminute, 0x10);
  mx5000_send_report(fd, daymonth, 0x10);
  mx5000_send_report(fd, year, 0x10);
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
