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

#ifndef MX5000SCREENCONTENT_H
#define MX5000SCREENCONTENT_H


enum line_mode {
  STATIC = 0x10,
  STATICBIG = 0x11,
  STATICHUGE = 0x12
};

enum display_icon {
  PERSON = 0x01,
  CIRCLE = 0x02,
  TRIANGLE_RIGHT = 0x03,
  TRIANGLE_LEFT = 0x04,
  ARROW_UP = 0x05,
  ARROW_DOWN = 0x06,
  SQUARE = 0x07,
  PAUSE1 = 0x09,
  PAUSE2 = 0x0a,
  NEXTTRACK1 = 0x0b,
  NEXTTRACK2 = 0x0c,
  UNDO1 = 0x0d,
  UNDO2 = 0x0e,
  BOX_A1 = 0x10,
  BOX_A2 = 0x11,
  BOX_B1 = 0x12,
  BOX_B2 = 0x13,
  BOX_C1 = 0x12,
  BOX_C2 = 0x13,
  BOX_D1 = 0x16,
  BOX_D2 = 0x17,

  PROGRESS_LEFT_FULL = 0x18,
  PROGRESS_LEFT_EMPTY = 0x19,
  PROGRESS_CENTER_FULL = 0x1A,
  PROGRESS_CENTER_EMPTY = 0x1B,
  PROGRESS_RIGHT_FULL = 0x1C,
  PROGRESS_RIGHT_EMPTY = 0x1D, 
};



struct MX5000ScreenContent;

struct MX5000ScreenContent *mx5000_sc_new_static(void);
struct MX5000ScreenContent *mx5000_sc_new_scroll(const char *text, int len, 
						 unsigned char ypos1, unsigned char xpos1,
						 unsigned char ypos2, unsigned char xpos2,
						 unsigned char ypos3, unsigned char xpos3);
struct MX5000ScreenContent *mx5000_sc_new_rolling(const char *text, int len, 
						  unsigned char visiblelen, 
						  unsigned char ypos, unsigned char xpos);
struct MX5000ScreenContent *mx5000_sc_new_image(const unsigned char *imgdata, unsigned char width, unsigned char height, unsigned char ypos, unsigned char xpos);



int mx5000_sc_add_text(struct MX5000ScreenContent *sc,
		       const char *text, int len, enum line_mode size, 
		       unsigned char ypos, unsigned char xpos);
int mx5000_sc_add_progress_bar_percentage(struct MX5000ScreenContent *sc, 
					  unsigned char perc);
int mx5000_sc_add_progress_bar(struct MX5000ScreenContent *sc, 
			       unsigned char filled_width, 
			       unsigned char width, enum line_mode size, 
			       unsigned char ypos, unsigned char xpos);
int mx5000_add_horiz_line(struct MX5000ScreenContent *sc,
			  unsigned char ypos, unsigned char xposfirst,
			  unsigned char xposlast);
int mx5000_sc_add_icon(struct MX5000ScreenContent *sc, enum display_icon icon,
		       enum line_mode size, unsigned char ypos, 
		       unsigned char xpos);


int mx5000_sc_send(struct MX5000ScreenContent *sc, int fd);
void mx5000_sc_free(struct MX5000ScreenContent *sc);



int mx5000_screen_buffer_updater(int fd, char* string, int len, 
				 int buffer_offset);
int mx5000_reset(int fd);



#endif /* MX5000SCREENCONTENT_H */
