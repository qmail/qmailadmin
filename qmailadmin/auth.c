/* 
 * $Id: auth.c,v 1.3.2.2 2004-10-19 15:44:40 tomcollins Exp $
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
#include "config.h"
#include "qmailadmin.h"
#include "qmailadminx.h"
#include <vpopmail.h>
#include <vauth.h>

extern char *crypt();

auth_system(ip_addr, pw)
 char *ip_addr;
 struct vqpasswd *pw;
{
 FILE *fs;
 time_t time1; 
 time_t time2;
 char ip_value[MAX_BUFF];
 
  if( chdir(RealDir) < 0 ){
    sprintf(StatusMessage, "%s %s\n", get_html_text("171"), RealDir);
    show_login();
    vclose();
    exit(0);
  }

  sprintf(TmpBuf1, "%s/" MAILDIR "/%s.qw", pw->pw_dir, Time);

  fs = fopen(TmpBuf1, "r");
  if ( fs == NULL ) {
    sprintf(StatusMessage, "%s\n", get_html_text("172"));
    show_login();
    vclose();
    exit(0);
  } 

  if ( fgets(TmpBuf, sizeof(TmpBuf), fs) == NULL ) {
    sprintf(StatusMessage, "%s %d\n", get_html_text("150"), 4);
    vclose();
    exit(0);
  }
  fclose(fs);

#ifdef IPAUTH
  GetValue(TmpBuf, ip_value, "ip_addr=", sizeof(ip_value)); 
  if ( strcmp(ip_addr, ip_value) != 0 ) {
    unlink(TmpBuf1);
    sprintf(StatusMessage,"invalid\n");
    show_login();
    vclose();
    exit(0);
  }
#endif

  time1 = atoi(Time); time2 = time(NULL);
  if ( time2 > time1 + 7200 ) {
    unlink(TmpBuf1);
    sprintf(StatusMessage, "%s\n", get_html_text("173"));
    show_login();
    vclose();
    exit(0);
  }
}

auth_user_domain(ip_addr,pw)
 char *ip_addr;
 struct vqpasswd *pw;
{
 FILE *fs;
 time_t time1; 
 time_t time2;
 char ip_value[MAX_BUFF];

  if ( chdir(RealDir) < 0 ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("171"), RealDir );
    show_login();
    vclose();
    exit(0);
  }

  sprintf(TmpBuf1, "%s/" MAILDIR "/%s.qw", pw->pw_dir, Time);

  fs = fopen(TmpBuf1, "r");
  if ( fs == NULL ) {
    sprintf(StatusMessage, "%s\n", get_html_text("172"));
    show_login();
    vclose();
    exit(0);
  } 

  if ( fgets(TmpBuf, sizeof(TmpBuf), fs) == NULL ) {
    sprintf(StatusMessage, "%s %d\n", get_html_text("150"), 5);
    vclose();
    exit(0);
  }
  fclose(fs);

#ifdef IPAUTH
  GetValue(TmpBuf, ip_value, "ip_addr=", sizeof(ip_value)); 
  if ( strcmp(ip_addr, ip_value) != 0 ) {
    unlink(TmpBuf1);
    sprintf(StatusMessage, "%s %d (%s != %s)\n", get_html_text("150"), 6, ip_addr, ip_value);
    show_login();
    vclose();
    exit(0);
  }
#endif

  time1 = atoi(Time); time2 = time(NULL);
  if ( time2 > time1 + 7200 ) {
    unlink(TmpBuf1);
    sprintf(StatusMessage, "%s\n", get_html_text("173"));
    show_login();
    vclose();
    exit(0);
  }

}

set_admin_type()
{
 struct vqpasswd *vpw=NULL;

  vpw = vauth_getpw(Username, Domain);
  AdminType = NO_ADMIN;
  if ( strlen(Domain) > 0 ) {
    if ( strcmp(Username,"postmaster")==0 ) {
      AdminType = DOMAIN_ADMIN;
#ifdef VQPASSWD_HAS_PW_FLAGS
    } else if ( vpw->pw_flags & QA_ADMIN ) {
#else
    } else if ( vpw->pw_gid & QA_ADMIN ) {
#endif
      AdminType = DOMAIN_ADMIN;
    } else {
      AdminType = USER_ADMIN;
    }
  }
}
