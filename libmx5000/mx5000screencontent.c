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
#include <time.h>
#include <stdlib.h>

#include <asm/types.h>
#include <linux/hiddev.h>

#include "libmx5000/mx5000screencontent.h"
#include "libmx5000/mx5000.h"

// static const char PLEASEWAIT_TEXT_END_BYTES[] = { 0x50, 0xFF, 0x01, 0x30, 0x00, 0x01, 0x00, 0x00 };

#define MX5000_MAXDATA (12*(45-3))

struct MX5000ScreenContent {
  unsigned char textdata[MX5000_MAXDATA];
  unsigned int textlen;
  unsigned char layoutdata[MX5000_MAXDATA];
  unsigned int layoutlen;
};

static void _mx5000_sc_send_datareports(int fd, const char *data, int len);
struct MX5000ScreenContent *_mx5000_sc_alloc(void);

struct MX5000ScreenContent *_mx5000_sc_alloc(void)
{
  struct MX5000ScreenContent *sc;

  sc = malloc(sizeof(struct MX5000ScreenContent));
  if (sc == NULL)
    return sc;

  memset(sc, 0, sizeof(struct MX5000ScreenContent));
  return sc;
}

struct MX5000ScreenContent *mx5000_sc_new_static(void)
{
  struct MX5000ScreenContent *sc;
  const char STATIC_TEXT_END_BYTES[] = { 0x50, 0x03, 0x00, 0x01 };

  sc = _mx5000_sc_alloc();

  if(sc == NULL)
    return NULL;

  memcpy(sc->layoutdata, STATIC_TEXT_END_BYTES, sizeof(STATIC_TEXT_END_BYTES));
  sc->layoutlen += sizeof(STATIC_TEXT_END_BYTES);

  return sc;
}

struct MX5000ScreenContent *mx5000_sc_new_scroll(const char *text, int len, 
						 unsigned char ypos1, unsigned char xpos1,
						 unsigned char ypos2, unsigned char xpos2,
						 unsigned char ypos3, unsigned char xpos3)
{
  struct MX5000ScreenContent *sc;
  const char SCROLL_TEXT_END_BYTES[] =  { 0x50, 0x03, 0x00, 0x51 };
  unsigned char *buf;
  int i = 0;

  sc = _mx5000_sc_alloc();

  if(sc == NULL)
    return NULL;

  memcpy(sc->layoutdata, SCROLL_TEXT_END_BYTES, sizeof(SCROLL_TEXT_END_BYTES));
  sc->layoutlen += sizeof(SCROLL_TEXT_END_BYTES);

  buf = sc->layoutdata + sc->layoutlen;
  
 if (len < 0)
    len = strlen(text);

  memcpy(sc->textdata+sc->textlen, text, len);
  sc->textlen += len;
 
  if (len % 16)
    memset(sc->textdata+sc->textlen, ' ', 16-len%16);
  sc->textlen += 16-len%16;

  buf[i++] = 0;
  buf[i++] = 0;
  buf[i++] = (len%16)?(1+(len/16)):(len/16);
  buf[i++] = 0x33;
  buf[i++] = 0x01;

  buf[i++] = 0x60;
  buf[i++] = xpos1;
  buf[i++] = ypos1;

  buf[i++] = 0x60;
  buf[i++] = xpos2;
  buf[i++] = ypos2;

  buf[i++] = 0x60;
  buf[i++] = xpos3;
  buf[i++] = ypos3;


  sc->layoutlen += i;


  return sc;
}



struct MX5000ScreenContent *mx5000_sc_new_rolling(const char *text, int len, 
						  unsigned char visiblelen, 
						  unsigned char ypos, unsigned char xpos)
{
  unsigned char *buf;
  const int offset = 0;
  struct MX5000ScreenContent *sc;
  const char MUSIC_TEXT_END_BYTES[] = { 0x50, 0x03, 0x00, 0x52 };

  if (!text)
    return NULL;

  sc = _mx5000_sc_alloc();

  if(sc == NULL)
    return NULL;
  
  memcpy(sc->layoutdata, MUSIC_TEXT_END_BYTES, sizeof(MUSIC_TEXT_END_BYTES));
  sc->layoutlen += sizeof(MUSIC_TEXT_END_BYTES);

  buf = sc->layoutdata+sc->layoutlen;
  

  if (len < 0)
    len = strlen(text);
  
  if (len > 0x30)
    len = 0x30;

  if (visiblelen > 16)
    visiblelen = 16;
  
  memcpy(sc->textdata+sc->textlen, text, len);
  sc->textlen += len;

  if (len%2) {
    sc->textdata[sc->textlen++] = 0;
  }
  
  buf[0] = 0;
  buf[1] = offset/2;
  buf[2] = len-1;
  buf[3] = visiblelen;
  buf[4] = 1;

  // the second part can be separated from the first (dont ask why)

  buf[5] = 0x21;
  buf[6] = xpos;  // rolling xpos
  buf[7] = ypos; // rolling ypos
  buf[8] = 0x00; // ignored ?? 

  sc->layoutlen += 9;

  return sc; 

}

struct MX5000ScreenContent *mx5000_sc_new_image(const unsigned char *imgdata, unsigned char width, unsigned char height, unsigned char ypos, unsigned char xpos)
{
  struct MX5000ScreenContent *sc;
  const char IMAGE_TEXT_END_BYTES[] =  { 0x50, 0xFF, 0x00, 0x01 };
  unsigned char *buf;
  int i = 0;
  int size;

  if (height%8)
    return NULL;

  if (width > 0x1D)
    return NULL;

  if (height > 0x28)
    height = 0x28;

  sc = _mx5000_sc_alloc();

  if(sc == NULL)
    return NULL;

  memcpy(sc->layoutdata, IMAGE_TEXT_END_BYTES, sizeof(IMAGE_TEXT_END_BYTES));
  sc->layoutlen += sizeof(IMAGE_TEXT_END_BYTES);

  sc->textdata[1] = width;
  sc->textdata[2] = height;
  sc->textlen += 3;

  size = (height*width)/8;
  
  memcpy(sc->textdata+3, imgdata, size);

  sc->textlen += size;

  buf = sc->layoutdata + sc->layoutlen;

  buf[i++] = 0x30;
  buf[i++] = xpos;
  buf[i++] = ypos;
  buf[i++] = 0x00;
  buf[i++] = 0x00;

  sc->layoutlen += i;


  return sc;
}





int mx5000_sc_add_text(struct MX5000ScreenContent *sc, 
		       const char *text, int len, enum line_mode size, 
		       unsigned char ypos, unsigned char xpos)
{
  int offset = sc->textlen;
  unsigned char *buf =  (unsigned char*) sc->layoutdata + sc->layoutlen;

  if (!sc || !text)
    return -1;
  //  if (!(size == STATIC || size == STATICBIG))
  //    return -1;

  if (len < 0)
    len = strlen(text);

  if (len > 17)
    len = 17;
  if (ypos < 7)
    return -1;
  

  memcpy(sc->textdata+sc->textlen, text, len);
  sc->textlen += len;

  if (len%2) {
    sc->textdata[sc->textlen] = ' ';
    sc->textlen++;
  }

  buf[0] = size;
  buf[1] = xpos;
  buf[2] = ypos;
  buf[3] = 0;
  buf[4] = offset/2;
  buf[5] = len;
  
  sc->layoutlen += 6;

  return offset;
}






int mx5000_sc_add_progress_bar(struct MX5000ScreenContent *sc, 
			       unsigned char filled_width, 
			       unsigned char width, enum line_mode size,
			       unsigned char ypos, unsigned char xpos)
{
  unsigned char i=0;
  char buf[17];
  
  if (width > 17)
    width = 17;
  if (width < 3)
    width = 3;
  
  if (filled_width > 0)
    buf[0] = PROGRESS_LEFT_FULL;
  else
    buf[0] = PROGRESS_LEFT_EMPTY;
  
  for (i=1;i<width;i++) {
    if (filled_width >= i+1) {
      buf[i] = PROGRESS_CENTER_FULL;
    } else {
      buf[i] = PROGRESS_CENTER_EMPTY;
    }
  }

  if (filled_width >= width) 
    buf[width-1] = PROGRESS_RIGHT_FULL;
  else
    buf[width-1] = PROGRESS_RIGHT_EMPTY;

  
  return mx5000_sc_add_text(sc, buf, width, size, ypos, xpos);

}

int mx5000_sc_add_progress_bar_percentage(struct MX5000ScreenContent *sc, 
					  unsigned char perc)
{
  if (perc > 100)
    return -1;
  return mx5000_sc_add_progress_bar(sc, perc * 12 / 100, 12, STATICBIG, 
				    0x27, 0);
}

int mx5000_add_horiz_line(struct MX5000ScreenContent *sc,
			  unsigned char ypos, unsigned char xposfirst,
			  unsigned char xposlast)
{
  unsigned char *buf = (unsigned char*) sc->layoutdata + sc->layoutlen;
  
  if (xposlast > 101)
    xposlast = 101;
  if (xposfirst > 101)
    return -1;

  buf[0] = 0x20;
  buf[1] = xposfirst;
  buf[2] = ypos;
  buf[3] = xposlast;

  sc->layoutlen += 4;

  return 0;
}


int mx5000_sc_add_icon(struct MX5000ScreenContent *sc, enum display_icon icon,
		       enum line_mode size, unsigned char ypos,
		       unsigned char xpos)
{
  char mychar = icon;

  return mx5000_sc_add_text(sc, &mychar, 1, size, ypos, xpos);
}



static void _mx5000_sc_send_datareports(int fd, const char *data, int len)
{

  int pos = 0;
  int r = 0, i = 0;
  char temp[45];


  for(r=0; r < 13; r++) {
    temp[0] = 0x01;
    if (r == 0)
      temp[1] = 0x90;
    else if ( r == 12 )
      temp[1] = 0x93;
    else
      temp[1] = 0x91;
    temp[2] = r;
    
    for (i = 3; i< 45; i++) {
      if (pos < len) {
	temp[i] = data[pos];
      } else {
	temp[i] = 0;
      }
      pos++;
    }
    

    mx5000_send_report(fd, temp, 0x12);
  }
}


int mx5000_sc_send(struct MX5000ScreenContent *sc, int fd)
{
  char totaldata[550]; 
  int len = 0;
  const char STATIC_TEXT_START_BYTES[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };

  const char initdata[6] = { 0x01, 0x80, 0xA1, 0x01, 0x00, 0x00 };
  char enddata[19] = { 0x01, 0x82, 0xA0, 0x01, 0x00, 0x00, 0x00 /*tlen/2*/, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


  memcpy(totaldata+len, STATIC_TEXT_START_BYTES,
	 sizeof(STATIC_TEXT_START_BYTES));
  len += sizeof(STATIC_TEXT_START_BYTES);

  memcpy(totaldata+len, sc->textdata, sc->textlen);
  len += sc->textlen;

  memcpy(totaldata+len, sc->layoutdata, sc->layoutlen);
  len += sc->layoutlen;

  mx5000_send_report(fd, initdata, 0x10);
  _mx5000_sc_send_datareports(fd, totaldata, len);
  enddata[6] = sc->textlen/2;
  mx5000_send_report(fd, enddata, 0x11);
  
  return 0;
}


int mx5000_sc_add_menuline(struct MX5000ScreenContent *sc, char text[16], char quadruplet[4])
{

  memcpy(sc->textdata+sc->textlen, text, 16);
  sc->textlen += 16;

  memcpy(sc->textdata+sc->textlen, quadruplet, 4);
  sc->textlen += 4;

  return 0;
}

int mx5000_sc_nextmenu(struct MX5000ScreenContent *sc)
{
  printf("len: %d adding %d", sc->textlen,  200-(sc->textlen%200));
  sc->textlen += 200-(sc->textlen%200);
  printf(" total %d\n", sc->textlen);

  return sc->textlen;
}


int mx5000_sc_send_menus(struct MX5000ScreenContent *sc, int fd, int pageid, int first)
{
  char totaldata[550]; 
  int len = 0;
  const char STATIC_TEXT_START_BYTES[] = { 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };

  const char initdata[6] = { 0x01, 0x80, 0xA1, 0x01, 0x00, 0x00 };
  char enddata[19] = { 0x01, 0x82, 0xA0, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  char data1[19] = { 0x11, 0x01, 0x82, 0xA0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 , 0, 0, 0, 0 , 0 ,0 };




  memcpy(totaldata+len, STATIC_TEXT_START_BYTES,
	 sizeof(STATIC_TEXT_START_BYTES));
  len += sizeof(STATIC_TEXT_START_BYTES);

  memcpy(totaldata+len, sc->textdata, sc->textlen);
  len += sc->textlen;

  /*
  if (first) {
    totaldata[509] = 0x43;
    len = 512;
  } else {
    totaldata[509] = 0x98;
    totaldata[510] = 0x67;
  }
  */
  //  mx5000_send_report(fd, data1, 0x11);

  mx5000_send_report(fd, initdata, 0x10);
  _mx5000_sc_send_datareports(fd, totaldata, len);

  enddata[9] = pageid;

  /*
  enddata[3] = 2;
  mx5000_send_report(fd, enddata, 0x11);
  */

  enddata[3] = 3;
  enddata[5] = 1;
  enddata[13] = 2;
  mx5000_send_report(fd, enddata, 0x11);

  return 0;
}





void mx5000_sc_free(struct MX5000ScreenContent *sc)
{
  free(sc);
}


int mx5000_reset(int fd)
{
  const char initdata[6] = { 0x01, 0x80, 0xA1, 0x01, 0x00, 0x00 };
  const char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x01}; 
  const char enddata[19] = { 0x01, 0x82, 0xA0, 0x01, 0x00, 0x00, 0x01 /*tlen/2*/, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };



  mx5000_send_report(fd, initdata, 0x10);
  _mx5000_sc_send_datareports(fd, data, sizeof(data));
  //enddata[6] = 1;
  mx5000_send_report(fd, enddata, 0x11);
  
  return 0;
}



int mx5000_screen_buffer_updater(int fd, char* string, int len, 
				 int buffer_offset)
{
  const char line1[6] = {0x01, 0x80, 0xA1, 0x01, 0x00, 0x00 };
  char line2[45] = { 0x01, 0x92, 0x00,
		     0x01, 0x00, 0x00 /*offset*/, 0x00,
		     0x00, 0x00, 0x00 /*Length*/, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00 };
  int ret;

  if (len < 0)
    len = strlen(string);

  if (len >= 0x18)
    len = 0x18;

  ret = mx5000_send_report(fd, line1, 0x10);

  memcpy(line2+12, string, len);
  
  line2[5] = buffer_offset/2;
  line2[9]=len;


  ret = mx5000_send_report(fd, line2, 0x12);

  return ret;
}
