/* 
 * $Id: autorespond.c,v 1.2 2003-10-10 16:36:24 tomcollins Exp $
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

show_autoresponders(user,dom,mytime,dir)
 char *user;
 char *dom;
 time_t mytime;
 char *dir;
{
  if ( MaxAutoResponders == 0 ) return(0);

  count_autoresponders();

  if(CurAutoResponders == 0) {
    sprintf(StatusMessage,"%s", get_html_text("233"));
    show_menu(Username, Domain, Mytime);
  } else {
    send_template( "show_autorespond.html" );
  }
}

int show_autorespond_line(char *user, char *dom, time_t mytime, char *dir)
{
 DIR *mydir;
 struct dirent *mydirent;
 FILE *fs;
 char *addr;
 int i,j;

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout, "<tr><td colspan=\"3\">%s %d</td></tr>\n", 
      get_html_text("143"), 1);
    return 0;
  }

  sort_init();

  while( (mydirent=readdir(mydir)) != NULL ) {
    /* does name start with ".qmail-" ? */
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
        fprintf(actout, "<tr><td colspan=\"3\">%s %s</td></tr>\n", 
         get_html_text("144"), mydirent->d_name);
        continue;
      }

      fgets( TmpBuf2, sizeof(TmpBuf2), fs);
      fclose(fs);

      if ( strstr( TmpBuf2, "autorespond") != 0 ) {
        sort_add_entry (&mydirent->d_name[7], 0);
      }
    }
  }
  closedir(mydir);
  sort_dosort();

  for (i = 0; addr = sort_get_entry(i); ++i) {
    fprintf(actout, "<tr>");
    
    fprintf(actout, "<td align=\"center\">");
    fprintf(actout, "<a href=\"%s/com/delautorespond?user=%s&dom=%s&time=%d&modu=%s\">",
      CGIPATH,user,dom,mytime,addr);
    fprintf(actout, "<img src=\"%s/trash.png\" border=\"0\"></a>", IMAGEURL);
    fprintf(actout, "</td>");

    fprintf(actout, "<td align=\"center\">");
    fprintf(actout, "<a href=\"%s/com/modautorespond?user=%s&dom=%s&time=%d&modu=%s\">",
      CGIPATH,user,dom,mytime,addr);
    fprintf(actout, "<img src=\"%s/modify.png\" border=\"0\"></a>", IMAGEURL);
    fprintf(actout, "</td>");

    fprintf(actout, "<td align=\"left\">%s@%s</td>", addr, Domain);
    
    fprintf(actout, "</tr>\n");
  }
  sort_cleanup();
}

addautorespond()
{

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  count_autoresponders();
  load_limits();
  if ( MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders ) {
    fprintf(actout, "%s %d\n", get_html_text("158"), MaxAutoResponders);
    show_menu();
    vclose();
    exit(0);
  }

  send_template( "add_autorespond.html" );

}

addautorespondnow()
{
 FILE *fs;
 int i;
 struct vqpasswd *vpw;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  count_autoresponders();
  load_limits();
  if ( MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders ) {
    fprintf(actout, "%s %d\n", get_html_text("158"), MaxAutoResponders);
    show_menu();
    vclose();
    exit(0);
  }

  if ( fixup_local_name(ActionUser) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("174"), ActionUser);
    addautorespond();
    vclose();
    exit(0);
  }


  if ( check_local_user(ActionUser) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("175"), ActionUser);
    addautorespond();
    vclose();
    exit(0);
  }

  if ( strlen(ActionUser) == 0 ) {
    sprintf(StatusMessage, "%s\n", get_html_text("176"));
    addautorespond();
    vclose();
    exit(0);
  }

  if ( strlen(Newu)>0 && check_email_addr(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("177"), Newu);
    addautorespond();
    vclose();
    exit(0);
  }

  if (strlen(Alias) <= 1) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("178"), ActionUser);
    addautorespond();
    vclose();
    exit(0);
  }

  if (strlen(Message) <= 1) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("179"), ActionUser);
    addautorespond();
    vclose();
    exit(0);
  }


  /*
   * Make the autorespodner directory
   */
  memset(TmpBuf2,0,sizeof(TmpBuf2));
  strncpy(TmpBuf2, ActionUser, sizeof(TmpBuf2));
  upperit(TmpBuf2);
  mkdir(TmpBuf2, 0750);

  /*
   * Make the autoresponder .qmail file
   */
  sprintf(TmpBuf, ".qmail-%s", ActionUser);
  for(i=6;TmpBuf[i]!=0;++i) if ( TmpBuf[i] == '.' ) TmpBuf[i] = ':';

  if ( (fs = fopen(TmpBuf, "w")) == NULL ) ack("123", 123);

  fprintf(fs, "|%s/autorespond 10000 5 %s/%s/message %s/%s\n",
    AUTORESPOND_PATH, RealDir, TmpBuf2, RealDir, TmpBuf2);

  if ( strlen(Newu) > 0 ) {
    fprintf(fs, "&%s\n", Newu);
  } 
  fclose(fs);

  /*
    * Make the autoresponder message file
   */
  sprintf(TmpBuf, "%s/message", TmpBuf2);
  if ( (fs = fopen(TmpBuf, "w")) == NULL ) ack("123", 123);
  fprintf(fs, "From: %s@%s\n", ActionUser,Domain);
  fprintf(fs, "Subject: %s\n\n", Alias);
  fprintf(fs, "%s", Message);
  fclose(fs);

  /*
   * Report success
   */
  sprintf(StatusMessage, "%s %s@%s\n", get_html_text("180"),
    ActionUser, Domain);
  show_autoresponders(Username,Domain);

}

delautorespond()
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template( "del_autorespond_confirm.html" );
}

delautorespondnow()
{
 int i;
 int pid;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  for(i=0;ActionUser[i]!=0;++i) if (ActionUser[i]=='.') ActionUser[i] = ':';
  sprintf(TmpBuf2, ".qmail-%s", ActionUser);
  if ( unlink(TmpBuf2) != 0 ) ack( get_html_text("181"), 345);

  memset(TmpBuf2,0,sizeof(TmpBuf2));
  for(i=0;ActionUser[i]!=0;++i) {
    if(islower(ActionUser[i])) {
      TmpBuf2[i]=toupper(ActionUser[i]);
    } else {
      TmpBuf2[i]=ActionUser[i];
    }
  }

        for(i=0;TmpBuf2[i]!=0;++i) if (TmpBuf2[i]==':') TmpBuf2[i] = '.';
        for(i=0;ActionUser[i]!=0;++i) if (ActionUser[i]==':') ActionUser[i] = '.';
  sprintf(TmpBuf, "%s/%s", RealDir, TmpBuf2);
  vdelfiles(TmpBuf);
  sprintf(StatusMessage, "%s %s\n", get_html_text("182"), ActionUser);

  count_autoresponders();

  if(CurAutoResponders == 0) {
    show_menu(Username, Domain, Mytime);
  } else {
    send_template( "show_autorespond.html" );
  }
}

modautorespond()
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template( "mod_autorespond.html" );
}


modautorespondnow()
{
 FILE *fs;
 int i;
 struct vqpasswd *vpw;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if ( fixup_local_name(ActionUser) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("174"), ActionUser);
    modautorespond();
    vclose();
    exit(0);
  }

  if ( strlen(Newu)>0 && check_email_addr(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("177"), Newu);
    modautorespond();
    vclose();
    exit(0);
  }

  if (strlen(Alias) <= 1) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("177"), ActionUser);
    modautorespond();
    vclose();
    exit(0);
  }

  if (strlen(Message) <= 1) {
    sprintf(StatusMessage, get_html_text("BODY_EMPTY"), ActionUser);
    modautorespond();
    vclose();
    exit(0);
  }


  /*
   * Make the autoresponder directory
   */
        strcpy(TmpBuf2,ActionUser);
        upperit(TmpBuf2);
        mkdir(TmpBuf2, 0750);

  /*
   * Make the autoresponder .qmail file
   */
  sprintf(TmpBuf, ".qmail-%s", ActionUser);
  for(i=6;TmpBuf[i]!=0;++i) if ( TmpBuf[i] == '.' ) TmpBuf[i] = ':';
  if ( (fs = fopen(TmpBuf, "w")) == NULL ) ack("123", 123);

  fprintf(fs, "|%s/autorespond 10000 5 %s/%s/message %s/%s\n",
    AUTORESPOND_PATH, RealDir, TmpBuf2, RealDir, TmpBuf2);

  if ( strlen(Newu) > 0 ) {
    for(i=0;Newu[i]!='@';TmpBuf3[i] = Newu[i],++i);
    if((vpw=vauth_getpw(TmpBuf3, Domain))!=NULL && (strstr(Newu,Domain)!= 0)){
      fprintf(fs, "%s/Maildir/\n", vpw->pw_dir);
    } else {
      fprintf(fs, "&%s\n", Newu);
    }
  } else if ( (vpw = vauth_getpw(ActionUser, Domain)) != NULL) {
    fprintf(fs, "%s/Maildir/\n", vpw->pw_dir);
  }
  fclose(fs);

  /*
   * Make the autoresponder message file
   */
  sprintf(TmpBuf, "%s/message", TmpBuf2);
  if ( (fs = fopen(TmpBuf, "w")) == NULL ) ack("123", 123);
  fprintf(fs, "From: %s@%s\n", ActionUser,Domain);
  fprintf(fs, "Subject: %s\n\n", Alias);
  fprintf(fs, "%s", Message);

  /*
   * Report success
   */
  sprintf(StatusMessage, "%s %s@%s\n", get_html_text("183"),ActionUser,Domain);
  show_autoresponders(Username, Domain, Mytime);
}

count_autoresponders()
{
 DIR *mydir;
 struct dirent *mydirent;
 FILE *fs;
 char mailinglist_name[MAX_FILE_NAME];
 int i,j;

  if ( (mydir = opendir(".")) == NULL ) { 
    fprintf(actout,"%s %d<BR>\n", get_html_text("143"), 1);
    fprintf(actout,"</table>");
    return(0);
  }

  CurAutoResponders = 0;
  while( (mydirent=readdir(mydir)) != NULL ) {
    /* does name start with ".qmail-" ? */
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
        fprintf(actout,"%s %s<br>\n", get_html_text("144"),
          mydirent->d_name);
        continue;
      }
      fgets( TmpBuf2, sizeof(TmpBuf2), fs);
      if ( strstr( TmpBuf2, "autorespond") != 0 ) {
        ++CurAutoResponders;
      }
      fclose(fs);
    }
  }
  closedir(mydir);

}
