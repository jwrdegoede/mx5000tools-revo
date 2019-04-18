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
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "libmx5000/mx5000.h"
#include "libmx5000/mx5000screencontent.h"
#include "libmx5000/mx5000image.h"

#include "config.h"


void help(const char *prog)
{

  printf(PACKAGE_NAME " " PACKAGE_VERSION "  by Olivier Crete <tester@tester.ca>\n"
	 "Usage: %s <-d <path> <other options>\n"
	 " -d, --device <path>   Path to the hiddev device\n"
	 " -r, --reset           Resets screen back to menu system\n"
	 " -b, --beep            Makes the keyboard beep twice\n"
	 " -n, --name <name>     Sets the name displayed on the screen (max length: 11)\n"
	 " -t, --time [time]     Sets the displayed to [time] (in seconds since epoch)\n"
	 "                       or now if parameter is passed\n"
	 " -c, --celcius         Sets the temperature unit to celcius\n"
	 " -f, --farenheight     Sets the temperature unit to farenheights\n"
	 " -i, --icons <values>  Sets the four icons left side (email, IM, mute, phone)\n" 
	 "                       off (0), on (1) or blink (2)\n"
	 "                       ex.: \"2010\" for email blink, mute on and the rest off\n"
	 " -k, --keybdopts <val> Set keyboard options:\n"
	 "                             Enable both (0)  Disable beeps on special keys (1) \n"
	 "                             Disable media keys (2)  Disable both (3)\n"
	 " -u, --update <ref> <text> Update the text added with --add-text\n"
	 "             MAIN SCREEN UPDATE MODE\n"
	 "             One of the 4 following options must be specified for this mode\n"
	 "             xpos and ypos are the position in pixels of the bottom left corner\n"
	 "     --static          Just put static content\n"
	 "     --rolling  <text> <vlen> <xpos> <ypos>  maximum text length: 48\n"
	 "                       Displays one line of \"rolling\" text\n"
	 "                       vlen is how many characters to display at once (max: 16) \n"
	 "     --image <file.pbm> <xpos> <ypos>  Displays a pbm file\n"
	 "                       xpos/ypos are the position of the top left corner\n"
	 "                       Max size is 29x40\n"
	 "     --scrolling <text> <xpos1> <ypos1> <xpos2> <ypos2> <xpos3> <ypos3>\n"
	 "                       Displays three lines of scrolling text\n"
	 "     --add-text <text> <size> <xpos> <ypos> Adds a line of static text\n"
	 "                       size is: reg, big or huge\n"
	 "     --add-progressbar <width> <filled> <size> <xpos> <ypos>\n"
	 "                       Displays a progress bar size is the same as for text\n"
	 "                       width and filled are in characters\n"
	 "     --add-horizline <xpos start> <xpos end> <ypos>\n"
	 "                       Display an horizontal line\n"
	 "     --add-icon <icon> <size> <xpos> <ypos>\n"
	 "                       Display an icon\n"
	 "     --help-icons      Display a list of possible icons\n"
 "     --ref             Print the reference of the last updatable item\n",
	 prog);
}
	 


char *getonearg(int argc, char **argv) {
  
  if (optind >= argc)
    return NULL;

  if (argv[optind][0] == '-') 
    return NULL;
  
  return argv[optind++];
}

enum line_mode sizefromstr(char *str)
{
  if (!strcmp("big", str)) {
    return STATICBIG;
  } else if (!strcmp("huge", str)) {
    return STATICHUGE;
  } else {
    return STATIC;
  }
}

const char *iconstrfromstr(char *str)
{
  if (!strcmp(str, "person")) {
    static const char buf[] = { PERSON, 0};
    return buf;
  } else if (!strcmp(str, "circle")) {
    static const char buf[] = { CIRCLE, 0};
    return buf;
  } else if (!strcmp(str, "right_triangle")) {
    static const char buf[] = { TRIANGLE_RIGHT, 0};
    return buf;
  } else if (!strcmp(str, "left_triangle")) {
    static const char buf[] = { TRIANGLE_LEFT, 0};
    return buf;
  } else if (!strcmp(str, "arrow_up")) {
    static const char buf[] = { ARROW_UP, 0};
    return buf;
  } else if (!strcmp(str, "arrow_down")) {
    static const char buf[] = { ARROW_DOWN, 0};
    return buf;
  } else if (!strcmp(str, "square")) {
    static const char buf[] = { SQUARE, 0};
    return buf;
  } else if (!strcmp(str, "pause")) {
    static const char buf[] = { PAUSE1, PAUSE2, 0};
    return buf;
  } else if (!strcmp(str, "next_track")) {
    static const char buf[] = { NEXTTRACK1, NEXTTRACK2, 0};
    return buf;
  } else if (!strcmp(str, "undo")) {
    static const char buf[] = { UNDO1, UNDO2, 0};
    return buf;
  } else if (!strcmp(str, "a")) {
    static const char buf[] = { BOX_A1, BOX_A2, 0};
    return buf;
  } else if (!strcmp(str, "b")) {
    static const char buf[] = { BOX_B1, BOX_B2, 0};
    return buf;
  } else if (!strcmp(str, "c")) {
    static const char buf[] = { BOX_C1, BOX_C2, 0};
    return buf;
  } else if (!strcmp(str, "d")) {
    static const char buf[] = { BOX_D1, BOX_D2, 0};
    return buf;
  }

  
  return NULL;
}




int main(int argc, char **argv)
{
  int fd = -1;
  static struct option myopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"reset", no_argument, NULL, 'r'},
    {"beep", no_argument, NULL, 'b'},
    {"name", required_argument, NULL, 'n'},
    {"time", optional_argument, NULL, 't'},
    {"celcius", no_argument, NULL, 'c'},
    {"farenheight", no_argument, NULL, 'f'},
    {"icons", required_argument, NULL, 'i'},
    {"keybdopts", required_argument, NULL, 'k'},
    {"update", required_argument, NULL, 'u'},
    {"static", no_argument, NULL, 1},
    {"rolling", required_argument, NULL, 2},
    {"scrolling", required_argument, NULL, 3},
    {"image", required_argument, NULL, 4},
    {"add-text", required_argument, NULL, 5},
    {"add-progressbar", required_argument, NULL, 6},
    {"add-horizline", required_argument, NULL, 7},
    {"add-icon", required_argument, NULL, 8},
    {"help-icons", no_argument, NULL, 9},
    {"ref", no_argument, NULL, 10},
    {NULL, 0, NULL, 0}
  };
  char c;
  struct MX5000ScreenContent *sc = NULL;
  int ref = -1;


  if (argc == 1) {
    help(argv[0]);
    return 1;
  }

  while ((c = getopt_long(argc, argv, "hd:rbn:t::cfi:k:u:", myopts, NULL)) > 0) {
   
    if (fd < 0) {
      if (c == 'd') {
	fd = mx5000_open_path(optarg);
	if (fd < 0) {
	  fprintf(stderr, "Could not open device or improper device (%s)\n",
		  strerror(errno));
	  return -1;
	}
	continue;
      }

      fd = mx5000_open();
      if (fd < 0) {
	fprintf(stderr, "Could not open device or improper device (%s)\n",
		strerror(errno));
	return -1;
      }
    }

    switch(c) {
    case 'h':
      help(argv[0]);
      break;

    case 'd':
      fprintf(stderr, "Device not the first option, ignored\n");
      break;

    case 'r':
      mx5000_reset(fd);
      break;

    case 'b':
      mx5000_beep(fd);
      break;

    case 'n':
      mx5000_set_name(fd, optarg, -1);
      break;

    case 't': {
      time_t mytime = 0;
      if (optarg) {
	mytime = atol(optarg);
	if (!mytime) {
	  fprintf(stderr, "Time should should be the number of seconds since epoch (1-1-1970)\n");
	  continue;
	}
      } else if (optind < argc && (mytime = atol(argv[optind]))) {
	optind++;
      } else {
	mytime = time(NULL);
      }
      mx5000_set_time(fd, mytime);
    }
      break;

    case 'c':
      mx5000_set_temp_unit(fd, 0);
      break;

    case 'f':
      mx5000_set_temp_unit(fd, 1);
      break;

    case 'i':
      if (strlen(optarg) != 4) {
	fprintf(stderr,"You need to specify all 4 icons\n");
	continue;
      }
      if (optarg[0] < '0' || optarg[0] > '2' ||
	  optarg[1] < '0' || optarg[1] > '2' ||
	  optarg[2] < '0' || optarg[2] > '2' ||
	  optarg[3] < '0' || optarg[3] > '2') {
	fprintf(stderr, "Invalid icon settings\n");
	continue;
      }
      
      mx5000_set_icons(fd, optarg[0]-'0', optarg[1]-'0', optarg[2]-'0', optarg[3]-'0');

      break;

    case 'k': {
      int arg = atoi(optarg);
      if (arg < 0 || arg > 3) {
	fprintf(stderr, "Invalid keyboard option\n");
	continue;
      }
      mx5000_set_kbd_opts(fd, arg);
    } 
      break;
     

    case 'u': {
      char *myrefstr=NULL, *text=NULL;
      int myref = 0;
      
      myrefstr = optarg;
      text = getonearg(argc, argv);
      
      if (!myrefstr || !text) {
	fprintf(stderr, "Not enough arguments for --update\n");
	continue;	  
      }
      myref = atoi(myrefstr);
      
      mx5000_screen_buffer_updater(fd, text, -1, myref);
    }
      break;

      
    case 1:
      if (sc) {
	fprintf(stderr, "One only buffer update command can be used (--static, --rolling or --scrolling)\n");
      } else {
	sc = mx5000_sc_new_static();
      }
      break;


    case 2: {
      char *text, *vlenstr, *yposstr, *xposstr;
      int vlen=0, xpos=0, ypos=0;

      if (sc) {
	fprintf(stderr, "One only buffer update command can be used (--static, --rolling, --scrolling or --image)\n");
	continue;
      }

      
      text = optarg;
      vlenstr = getonearg(argc, argv);
      xposstr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);
      
      if (!text || !vlenstr || !yposstr || !xposstr) {
	fprintf(stderr, "Not enough arguments for --rolling\n");
	continue;	  
      }
      
      vlen = atoi(vlenstr);
      xpos = atoi(xposstr);
      ypos = atoi(yposstr);

      if (vlen < 7) {
	fprintf(stderr, "Argument vlen must be at least 7\n");
	continue;
      }

      sc = mx5000_sc_new_rolling(text, -1, vlen, ypos, xpos);
    }
      break;


    case 3: {
      char *text, *yposstr1, *xposstr1, *yposstr2, *xposstr2, *yposstr3, *xposstr3;
      int xpos1=0, ypos1=0, xpos2=0, ypos2=0, xpos3=0, ypos3=0;
      
     if (sc) {
	fprintf(stderr, "One only buffer update command can be used (--static, --rolling, --scrolling or --image)\n");
	continue;
      }


      text = optarg;
      xposstr1 = getonearg(argc, argv);
      yposstr1 = getonearg(argc, argv);
      xposstr2 = getonearg(argc, argv);
      yposstr2 = getonearg(argc, argv);
      xposstr3 = getonearg(argc, argv);
      yposstr3 = getonearg(argc, argv);
      
      if (!text || !yposstr1 || !xposstr1 ||
	  !yposstr2 || !xposstr2 ||
	  !yposstr3 || !xposstr3) {
	fprintf(stderr, "Not enough arguments for --scrolling\n");
	continue;	  
      }
      
      xpos1 = atoi(xposstr1);
      ypos1 = atoi(yposstr1);
      xpos2 = atoi(xposstr2);
      ypos2 = atoi(yposstr2);
      xpos3 = atoi(xposstr3);
      ypos3 = atoi(yposstr3);

      sc = mx5000_sc_new_scroll(text, -1, ypos1, xpos1, ypos2, xpos2, ypos3, xpos3);
    }
      break;

   case 4: {
      char *file, *yposstr, *xposstr;
      int xpos=0, ypos=0;
      unsigned char *image;
      int width, height;

      if (sc) {
	fprintf(stderr, "One only buffer update command can be used (--static, --rolling, --scrolling or --image)\n");
	continue;
      }

      
      file = optarg;
      xposstr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);
      
      if (!file  || !yposstr || !xposstr) {
	fprintf(stderr, "Not enough arguments for --image\n");
	continue;	  
      }
      
      xpos = atoi(xposstr);
      ypos = atoi(yposstr);

      image = pbm_to_mx5000image(file, &width, &height);
      if (!image) {
	fprintf(stderr, "Could not read image: %s\n", file);
	continue;
      }

      printf("width: %x height: %x\n", width, height);
      sc = mx5000_sc_new_image(image, width, height,  ypos, xpos);

      free(image);
    }
      break;



    case 5: {
      char *text, *sizestr, *xposstr, *yposstr;
      int xpos=0, ypos=0;
      enum line_mode size;
      
      if (!sc) {
	fprintf(stderr, "You need to call --static, --rolling or --scrolling before other main screen update options\n");
	continue;
      }

      
      text = optarg;
      sizestr = getonearg(argc, argv);
      xposstr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);

      if (!text || !sizestr || !yposstr || !xposstr) {
	  fprintf(stderr, "Not enough arguments for --add-text\n");
	continue;	  
      }
      
      size = sizefromstr(sizestr);
      xpos = atoi(xposstr);
      ypos = atoi(yposstr);

      ref = mx5000_sc_add_text(sc, text, -1, size, ypos, xpos);
    }
      break;
 

    case 6: {
      char *sizestr, *filledstr, *widthstr, *xposstr, *yposstr;
      int xpos=0, ypos=0, width=0, filled=0;
      enum line_mode size;
      
      if (!sc) {
	fprintf(stderr, "You need to call --static, --rolling or --scrolling before other main screen update options\n");
	continue;
      }

      
      widthstr = optarg;
      filledstr = getonearg(argc, argv);
      sizestr = getonearg(argc, argv);
      xposstr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);

      if (!filledstr || !widthstr || !sizestr || !yposstr || !xposstr) {
	fprintf(stderr, "Not enough arguments for --add-progressbar\n");
	continue;	  
      }
      
      size = sizefromstr(sizestr);
      width = atoi(widthstr);
      filled = atoi(filledstr);
      xpos = atoi(xposstr);
      ypos = atoi(yposstr);

      mx5000_sc_add_progress_bar(sc, filled, width, size, ypos, xpos);
    }
      break;
 


    case 7: {
      char *xposstr, *yposstr, *xposlaststr;
      int xpos=0, ypos=0, xposlast=0;

      
      if (!sc) {
	fprintf(stderr, "You need to call --static, --rolling or --scrolling before other main screen update options\n");
	continue;
      }

      
      xposstr = optarg;
      xposlaststr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);

      if (!yposstr || !xposstr || !xposlaststr) {
	fprintf(stderr, "Not enough arguments for --add-horizline\n");
	continue;	  
      }
      

      xpos = atoi(xposstr);
      xposlast = atoi(xposlaststr);
      ypos = atoi(yposstr);

      printf("hline ypos %d xpos %d xposl %d\n", ypos, xpos, xposlast);
      mx5000_add_horiz_line(sc, ypos, xpos, xposlast);
    }
      break;
 

    case 8: {
      char *iconstr, *sizestr, *xposstr, *yposstr;
      const char *icon;
      int xpos=0, ypos=0, size=0;
      
      if (!sc) {
	fprintf(stderr, "You need to call --static, --rolling or --scrolling before other main screen update options\n");
	continue;
      }

      
      iconstr = optarg;
      sizestr = getonearg(argc, argv);
      xposstr = getonearg(argc, argv);
      yposstr = getonearg(argc, argv);

      if (!yposstr || !xposstr || !iconstr) {
	fprintf(stderr, "Not enough arguments for --add-icon\n");
	continue;	  
      }
      

      ypos = atoi(yposstr);
      xpos = atoi(xposstr);
      size = sizefromstr(sizestr);
      icon = iconstrfromstr(iconstr);

      if (!icon) {
	fprintf(stderr, "Invalid icon name\n");
	continue;
      }

      ref = mx5000_sc_add_text(sc, icon, -1, size, ypos, xpos);
    }
      break;
      
    case 9:
      printf("Possible icons are: person, circle, right_triangle, left_triangle, arrow_up,\n"
	     "arrow_down, square, pause, next_track, undo, a, b, c, d\n");
      break;

    case 10:
      if (ref < 0) {
	fprintf(stderr, "Can't print ref before adding text\n");
      } else {
	printf("%d\n", ref);
      }
      break;
    }

  }


  if (sc) { 
    mx5000_sc_send(sc, fd);
    mx5000_sc_free(sc);
  }
    


  if (fd >= 0) {
    close(fd);
  }

  return 0;
}

