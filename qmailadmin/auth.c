/* 
 * $Id: auth.c,v 1.7 2004-02-01 00:50:28 rwidmer Exp $
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

void del_id_files( char *dirname )
{
 DIR *mydir;
 struct dirent *mydirent;
 struct stat statbuf;
 char DirName[MAX_BIG_BUFF];
 char FileName[MAX_BIG_BUFF];

  sprintf(DirName, "%s/Maildir", dirname);

  mydir = opendir(DirName);
  if ( mydir == NULL ) return;

  while((mydirent=readdir(mydir))!=NULL){
    if ( strstr(mydirent->d_name,".qw")!=0 ) {
      sprintf(FileName, "%s/%s", DirName, mydirent->d_name);
      unlink(FileName);
    }
  }
  closedir(mydir);
}
int create_session_file( char *ip_addr, char *domaindir)
{
 FILE *fs;
 char Buffer[MAX_BIG_BUFF];
 char returnhttp[MAX_BUFF];
 char returntext[MAX_BUFF];
 struct vqpasswd *vpw=NULL;

  del_id_files( domaindir );

  memset(Buffer, 0, sizeof(Buffer)); 

  Mytime = time(NULL);
  sprintf(Buffer, "%s/Maildir/%d.qw", domaindir, Mytime);
  fs = fopen(Buffer, "w");
  if ( fs == NULL ) {
    fprintf(actout,"MAIN %s %s\n", get_html_text("144"), Buffer);
    fprintf(stderr,"MAIN %s %s\n", get_html_text("144"), Buffer);
    return(1);
  }

  vpw = vauth_getpw(Username, Domain);
  AdminType = NO_ADMIN;
  if ( strlen(Domain) > 0 ) {
    if ( strcmp(Username,"postmaster")==0 ) {
      AdminType = DOMAIN_ADMIN;
    } else if ( vpw->pw_gid & QA_ADMIN ) {
      AdminType = DOMAIN_ADMIN;
    } else {
      AdminType = USER_ADMIN;
    }
  }
  memset(Buffer, 0, sizeof(Buffer)); 

  /* set session vars */
  GetValue(TmpCGI, returntext, "returntext=", sizeof(returntext));
  GetValue(TmpCGI, returnhttp, "returnhttp=", sizeof(returnhttp));
  sprintf(Buffer, 
          "ip_addr=%s&returntext=%s&returnhttp=%s&admintype=%d\n",
          ip_addr, returntext, returnhttp, AdminType); 
  fputs(Buffer,fs); 
  fclose(fs);
  return(0);
}

int get_session_data( Username, Domain, ip_addr)
 char *Username;
 char *Domain;
 char *ip_addr;
{
 FILE *fs;
 time_t time1; 
 time_t time2;
 char ip_value[MAX_BUFF];
 char FileName[MAX_BUFF];
 char Buffer2[MAX_BUFF];
 struct vqpasswd *pw;

  pw = vauth_getpw( Username, Domain );

  if ( chdir(RealDir) < 0 ) {
    /*  can't cd to domain directory  */
    return( 171 );
  }

  /*  Create filename for session file  */
  sprintf(FileName, "%s/Maildir/%s.qw", pw->pw_dir, Time);
  fprintf(stderr, "Session file name: %s\n", FileName );

  /*  Try to open file  */
  if ( NULL == (fs = fopen(FileName, "r"))) {
    fprintf(stderr, "Session file open failed\n" );
    return(172);
  } 


  if ( NULL == ( fgets(Buffer2, sizeof(Buffer2), fs))) {
    fprintf(stderr, "Session file read failed\n" );
    return(150);
  }

  fclose(fs);

#ifdef IPAUTH
  GetValue(Buffer2, ip_value, "ip_addr=", sizeof(ip_value)); 
  if ( strcmp(ip_addr, ip_value) != 0 ) {
    fprintf(stderr, "Session IP address check failed\n" );
    unlink(FileName);
    return(150);
  }
#endif

  time1 = atoi(Time); time2 = time(NULL);
  if ( time2 > time1 + 7200 ) {
    unlink(FileName);
    return(173);
  }

return(0);
}
