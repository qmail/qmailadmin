/* 
 * $Id: autorespond.c,v 1.3.2.1 2004-02-02 00:39:47 tomcollins Exp $
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
 char *addr;
 char alias_name[MAX_FILE_NAME];
 char *alias_line;
 int i,j;

  sort_init();

  alias_line = valias_select_all (alias_name, Domain);
  while( alias_line != NULL ) {
    if ( strstr( alias_line, "/autorespond ") != 0 ) {
      sort_add_entry (alias_name, 0);
    }
    alias_line = valias_select_all_next (alias_name);
  }

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
   * Make the autoresponder directory
   */
  memset(TmpBuf2,0,sizeof(TmpBuf2));
  strncpy(TmpBuf2, ActionUser, sizeof(TmpBuf2));
  upperit(TmpBuf2);
  mkdir(TmpBuf2, 0750);

  /*
   * Make the autoresponder .qmail file
   */
  valias_delete (ActionUser, Domain);
  sprintf(TmpBuf, "|%s/autorespond 10000 5 %s/%s/message %s/%s",
    AUTORESPOND_PATH, RealDir, TmpBuf2, RealDir, TmpBuf2);
  valias_insert (ActionUser, Domain, TmpBuf);
  if ( strlen(Newu) > 0 ) {
    sprintf(TmpBuf, "&%s", Newu);
    valias_insert (ActionUser, Domain, TmpBuf);
  } 

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
  show_autoresponders(Username, Domain, Mytime);
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

  /* delete the alias */
  valias_delete (ActionUser, Domain);

  memset(TmpBuf2,0,sizeof(TmpBuf2));
  for(i=0;ActionUser[i]!=0;++i) {
    if(islower(ActionUser[i])) {
      TmpBuf2[i]=toupper(ActionUser[i]);
    } else {
      TmpBuf2[i]=ActionUser[i];
    }
  }

  /* delete the autoresponder directory */
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


/* addautorespondnow and modautorespondnow should be merged into a single function */
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
    sprintf(StatusMessage, "%s %s\n", get_html_text("178"), ActionUser);
    modautorespond();
    vclose();
    exit(0);
  }

  if (strlen(Message) <= 1) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("179"), ActionUser);
    modautorespond();
    vclose();
    exit(0);
  }


  /*
   * Make the autoresponder directory
   */
  memset(TmpBuf2,0,sizeof(TmpBuf2));
  strncpy(TmpBuf2, ActionUser, sizeof(TmpBuf2));
  upperit(TmpBuf2);
  mkdir(TmpBuf2, 0750);

  /*
   * Make the autoresponder .qmail file
   */
  valias_delete (ActionUser, Domain);
  sprintf(TmpBuf, "|%s/autorespond 10000 5 %s/%s/message %s/%s",
    AUTORESPOND_PATH, RealDir, TmpBuf2, RealDir, TmpBuf2);
  valias_insert (ActionUser, Domain, TmpBuf);
  if ( strlen(Newu) > 0 ) {
    sprintf(TmpBuf, "&%s", Newu);
    valias_insert (ActionUser, Domain, TmpBuf);
  } 

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
  sprintf(StatusMessage, "%s %s@%s\n", get_html_text("183"),
    ActionUser, Domain);
  show_autoresponders(Username, Domain, Mytime);
}

count_autoresponders()
{
 char alias_name[MAX_FILE_NAME];
 char *alias_line;

  CurAutoResponders = 0;

  alias_line = valias_select_all (alias_name, Domain);
  while( alias_line != NULL ) {
    if ( strstr( alias_line, "/autorespond ") != 0 ) {
      CurAutoResponders++;
    }
    alias_line = valias_select_all_next (alias_name);
  }

}
