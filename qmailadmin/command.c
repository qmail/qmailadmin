/* 
 * $Id: command.c,v 1.5 2004-01-30 08:30:58 rwidmer Exp $
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
#include "qmailadmin.h"
#include "qmailadminx.h"
#include <vpopmail_config.h>



process_commands(char *command)
{
 int pid;
 char Buffer[MAX_BUFF];

  if (strcmp(command, "showmenu") == 0 ) {
    show_menu(Username, Domain, Mytime);

  } else if (strcmp(command, "showusers") == 0) {
    GetValue(TmpCGI, Pagenumber, "page=", sizeof(Pagenumber));
    GetValue(TmpCGI, SearchUser, "searchuser=", sizeof(SearchUser));
    show_users(Username, Domain, Mytime, command);

  } else if (strcmp(command, "showforwards") == 0) {
    GetValue(TmpCGI, Pagenumber, "page=", sizeof(Pagenumber));
    show_forwards(Username, Domain, Mytime, command);

  } else if (strcmp(command, "showmailinglists") == 0) {
    show_mailing_lists(Username, Domain, Mytime, command);

  } else if (strcmp(command, "showautoresponders") == 0) {
    show_autoresponders(Username, Domain, Mytime, command);

  } else if (strcmp(command, "adduser") == 0 ) {
    adduser();

  } else if (strcmp(command, "addusernow") == 0 ) {
    addusernow();

  } else if (strcmp(command, "bounceall") == 0 ) {
    bounceall();

  } else if (strcmp(command, "deleteall") == 0 ) {
    deleteall();

  } else if (strcmp(command, "setremotecatchall") == 0 ) {
    setremotecatchall();

  } else if (strcmp(command, "setremotecatchallnow") == 0 ) {
    setremotecatchallnow();

  } else if (strcmp(command, "addlistmodnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    addlistgroupnow(1);

  } else if (strcmp(command, "dellistmod") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    dellistgroup("del_listmod.html");

  } else if (strcmp(command, "dellistmodnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    dellistgroupnow(1);

  } else if (strcmp(command, "addlistmod") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    addlistgroup("add_listmod.html");

  } else if (strcmp(command, "showlistmod") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    show_list_group("show_moderators.html");

  } else if (strcmp(command, "addlistdig") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    addlistgroup("add_listdig.html");

  } else if (strcmp(command, "addlistdignow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    addlistgroupnow(2);

  } else if (strcmp(command, "dellistdig") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    dellistgroup("del_listdig.html");

  } else if (strcmp(command, "dellistdignow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    dellistgroupnow(2);

  } else if (strcmp(command, "showlistdig") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    show_list_group("show_digest_subscribers.html");

  } else if (strcmp(command, "moduser") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    moduser();

  } else if (strcmp(command, "modusernow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Password1, "password1=", sizeof(Password1));
    GetValue(TmpCGI, Password2, "password2=", sizeof(Password2));
    GetValue(TmpCGI, Gecos, "gecos=", sizeof(Gecos));
    modusergo();

  } else if (strcmp(command, "deluser") == 0 ) {
    GetValue(TmpCGI, ActionUser, "deluser=", sizeof(ActionUser));
    deluser();

  } else if (strcmp(command, "delusernow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "deluser=", sizeof(ActionUser));
    delusergo();

  } else if (strcmp(command, "moddotqmail") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    moddotqmail();

  } else if (strcmp(command, "moddotqmailnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    GetValue(TmpCGI, LineData, "linedata=", sizeof(LineData));
    GetValue(TmpCGI, Action, "action=", sizeof(Action));
    moddotqmailnow();

  } else if (strcmp(command, "deldotqmail") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    deldotqmail();

  } else if (strcmp(command, "deldotqmailnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    deldotqmailnow();

  } else if (strcmp(command, "adddotqmail") == 0 ) {
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    adddotqmail();

  } else if (strcmp(command, "adddotqmailnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    GetValue(TmpCGI, Alias, "alias=", sizeof(Alias));
    GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
    adddotqmailnow();

  } else if (strcmp(command, "addmailinglist") == 0 ) {
    addmailinglist();

  } else if (strcmp(command, "delmailinglist") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    delmailinglist();

  } else if (strcmp(command, "delmailinglistnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    delmailinglistnow();

  } else if (strcmp(command, "addlistusernow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    addlistgroupnow(0);

  } else if (strcmp(command, "dellistuser") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    dellistgroup("del_listuser.html");

  } else if (strcmp(command, "dellistusernow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));
    dellistgroupnow(0);

  } else if (strcmp(command, "addlistuser") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    addlistgroup("add_listuser.html");

  } else if (strcmp(command, "addmailinglistnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    addmailinglistnow();

  } else if (strcmp(command, "modmailinglist") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    modmailinglist();

  } else if (strcmp(command, "modmailinglistnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    modmailinglistnow();

  } else if (strcmp(command, "modautorespond") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    modautorespond();

  } else if (strcmp(command, "addautorespond") == 0 ) {
    addautorespond();

  } else if (strcmp(command, "addautorespondnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    GetValue(TmpCGI, Alias, "alias=", sizeof(Alias));
    GetValue(TmpCGI, Message, "message=", sizeof(Message));
    GetValue(TmpCGI, Newu, "owner=", sizeof(Newu));
    addautorespondnow();

  } else if (strcmp(command, "modautorespondnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    GetValue(TmpCGI, Alias, "alias=", sizeof(Alias));
    GetValue(TmpCGI, Message, "message=", sizeof(Message));
    GetValue(TmpCGI, Newu, "owner=", sizeof(Newu));
    modautorespondnow();

  } else if (strcmp(command, "showlistusers") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    show_list_group("show_subscribers.html");

  } else if (strcmp(command, "setdefault") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    GetValue(TmpCGI, Pagenumber, "page=", sizeof(Pagenumber));
    setdefaultaccount();

  } else if (strcmp(command, "delautorespond") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    delautorespond();

  } else if (strcmp(command, "delautorespondnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
    delautorespondnow();

  } else if (strcmp(command, "logout") == 0 ) {
    sprintf(Buffer, "%s/%s/Maildir", RealDir, Username );
    del_id_files(Buffer);
    show_login();
  }

  vclose();
  exit(0);
}

setdefaultaccount()
{
 struct vqpasswd *pw;
 FILE *fs;
 int i;
 int j;

  if ( (fs = fopen(".qmail-default", "w")) == NULL ) {
    sprintf(StatusMessage,"%s", get_html_text("082"));
  } else {
    if ((pw = vauth_getpw( ActionUser, Domain )) == NULL) {
      sprintf(StatusMessage,"%s %s@%s", get_html_text("223"), ActionUser, Domain);
    } else {
      fprintf(fs, "| %s/bin/vdelivermail '' %s\n", VPOPMAILDIR, pw->pw_dir);
      sprintf(CurCatchall, ActionUser);
    }
    fclose(fs);
  }
  show_users(Username, Domain, Mytime);
    vclose();
  exit(0);
}
