/* 
 * $Id: forward.c,v 1.7 2004-02-07 09:22:36 rwidmer Exp $
 * Copyright (C) 1999-2002 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <vpopmail.h>
#include <vauth.h>
#include "config.h"
#include "qmailadmin.h"
#include "qmailadminx.h"

char* dotqmail_alias_command(char* command);

int show_forwards(char *user, char *dom, time_t mytime, char *dir)
{

  fprintf( stderr, "show_forwards\n" );

  if (AdminType != DOMAIN_ADMIN) {
    fprintf( stderr, "Not an admin\n" );
    sprintf(StatusMessage,"%s", get_html_text(142));
    return(142);
  }

  if(CurForwards == 0) {
    fprintf( stderr, "show_forwards - aint none\n" );
    sprintf(StatusMessage,"%s", get_html_text(232));
    show_menu(Username, Domain, Mytime);
    return(232);
  } else {
    fprintf( stderr, "show_forwards - send template\n" );
    send_template("show_forwards.html");
  }

  return 0;
}
