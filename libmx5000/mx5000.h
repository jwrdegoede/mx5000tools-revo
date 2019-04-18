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

#ifndef MX5000_H
#define MX5000_H

#include <asm/types.h>
#include <time.h>

enum kbdopts {
  ENABLE_EVERYTHING = 0x00,
  DISABLE_BEEP_ON_SPECIAL_KEYS = 0x01,
  DISABLE_MEDIA_KEYS = 0x02,
};

enum iconstatus {
  ICON_OFF = 0x00,
  ICON_ON = 0x01,
  ICON_BLINK = 0x02
};


int mx5000_open(void);
int mx5000_open_path(const char *path);

int mx5000_send_report(int fd, const char *buf, __u32 reportid);


void mx5000_set_icons(int fd, enum iconstatus email, enum iconstatus messenger, 
		     enum  iconstatus mute, enum iconstatus walkie);
void mx5000_set_temp_unit(int fd, int isfarenheight );
void mx5000_set_kbd_opts(int fd, enum kbdopts opts);
void mx5000_set_time(int fd, time_t mytime);
void mx5000_set_name(int fd, char buf[14], int len);
void mx5000_beep(int fd);


#endif /* MX5000_H */
