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
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>




#include "libmx5000/mx5000.h"
#include "libmx5000/mx5000screencontent.h"



int main(int argc, char **argv)
{
  int fd;
  struct MX5000ScreenContent *sc = NULL, *sc2=NULL;
  int retval;
  struct passwd *pwd;
  struct utsname utsname;
  char buf[156];
  int l,i;
  int pos = 7;
  const unsigned char image[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7F, 0x41, 0x50, 0x58, 0x5D, 0x58, 0x50, 0x41, 0x7F, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x83, 0xC6, 0x6C, 0x3B, 0x6C, 0xC6, 0x83, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFD, 0x0D, 0x1D, 0x3D, 0x7D, 0x3D, 0x1D, 0x0D, 0xFD, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  fd = mx5000_open();

  if (fd < 0) {
    perror("Opening device");
    return fd;
  }

  

  //   mx5000_sc_add_rolling(sc, "123456789ABCDEF0123456789abcdef0123456789AbCdEf0123456789abCDef", 0x30, 16, 0x11, 1);

#if 0
#if 0

  sc = mx5000_sc_new_scroll("this is a reaaally really long long text that will be shown in a big dip type scroller...  I really really mean it, and its very long for that reason, I hope its long enough", -1, 0x07, 0, 0x11, 0,  0x1b, 0);
  
  mx5000_sc_add_text(sc,"   EXOT               ", 17, STATIC, 0x28, 0);
  mx5000_add_horiz_line(sc, 0x1E, 0, 0x65);

#else
  
  sc = mx5000_sc_new_static();

  mx5000_sc_add_text(sc, "tester", -1, STATIC, 10, 10);


#endif

  mx5000_sc_send(sc, fd);

#endif

#if 0
  sc = _mx5000_sc_alloc();

  mx5000_sc_add_menuline(sc,"BACK       ENTER", "\x03\x00\x00\x00");
  mx5000_sc_add_menuline(sc,"Line1   keys    ", "\x00\x05\x64\x00");
  mx5000_sc_add_menuline(sc,"Line2           ", "\x01\x06\x00\x00");
  mx5000_sc_add_menuline(sc,"Line3           ", "\x02\xFF\xFF\x00");

  mx5000_sc_nextmenu(sc);

  mx5000_sc_add_menuline(sc,"BACK  2  keys   ", "\x04\x00\x00\x00");
  mx5000_sc_add_menuline(sc,"Line1 2         ", "\x03\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line2 2         ", "\x04\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line3 2         ", "\x05\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line4 2         ", "\x06\xFF\xFF\x00");

  mx5000_sc_nextmenu(sc);


  mx5000_sc_send_menus(sc, fd, 5, 1);

  mx5000_sc_free(sc);

  sc = _mx5000_sc_alloc();

  mx5000_sc_add_menuline(sc,"BACK  3     PLAY", "\x05\x00\x00\x00");
  mx5000_sc_add_menuline(sc,"Line1 3         ", "\x2E\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line2 3         ", "\x2F\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line3 3         ", "\x30\xFF\xFF\x00");
  mx5000_sc_add_menuline(sc,"Line4 4         ", "\x31\xFF\xFF\x00");

  
  mx5000_sc_send_menus(sc, fd, 6, 0);


  mx5000_sc_free(sc);

#endif

#if 1
  

sc = mx5000_sc_new_image(image, 0x1D, 0x20, 2, 0);

  mx5000_sc_add_text(sc, "   PLEASE         ", 12, STATIC, 0x0E, 0x1E);
  mx5000_sc_add_text(sc, "    WAIT           ", 12, STATIC, 0x20, 0x1E);

  mx5000_sc_send(sc, fd);
#endif

  //mx5000_set_name(fd, "bobby", -1);
  //mx5000_reset(fd);






 out:
  close(fd);

  return 0;
}
