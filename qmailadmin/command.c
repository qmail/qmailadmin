/* 
 * $Id: command.c,v 1.6 2004-01-31 11:08:00 rwidmer Exp $
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
 int rv=0;   // return value
 char Buffer[MAX_BUFF];

  fprintf( stderr, "Process_commands: %s\n", command );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                            No rights                          */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  if (strcmp(command, "logout") == 0 ) {
    sprintf(Buffer, "%s/%s/Maildir", RealDir, Username );
    del_id_files(Buffer);
    show_login();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                           User  rights                        */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  } else if (AdminType < USER_ADMIN ) {
    rv = show_login();

  } else if (strcmp(command, "showmenu") == 0 ) {
    rv = show_menu(Username, Domain, Mytime);

  } else if (strcmp(command, "adddotqmail") == 0 ) {
    rv = adddotqmail();

  } else if (strcmp(command, "adddotqmailnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    rv = adddotqmailnow();

  } else if (strcmp(command, "deldotqmail") == 0 ) {
    rv = deldotqmail();

  } else if (strcmp(command, "deldotqmailnow") == 0 ) {
    rv = deldotqmailnow();

  } else if (strcmp(command, "moddotqmail") == 0 ) {
    rv = moddotqmail();

  } else if (strcmp(command, "moddotqmailnow") == 0 ) {
    GetValue(TmpCGI, Action, "action=", sizeof(Action));
    rv = moddotqmailnow();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                        Domain rights                          */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  } else if (AdminType < DOMAIN_ADMIN ) {
    rv = show_menu(Username, Domain, Mytime);

/*                A U T O R E S P O N D E R S                     */ 

  } else if (strcmp(command, "showautoresponders") == 0) {
    rv = show_autoresponders(Username, Domain, Mytime, command);

  } else if (strcmp(command, "addautorespond") == 0 ) {
    rv = addautorespond();

  } else if (strcmp(command, "addautorespondnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "owner=", sizeof(Newu));
    rv = addautorespondnow();

  } else if (strcmp(command, "delautorespond") == 0 ) {
    rv = delautorespond();

  } else if (strcmp(command, "delautorespondnow") == 0 ) {
    rv = delautorespondnow();

  } else if (strcmp(command, "modautorespond") == 0 ) {
    rv = modautorespond();

  } else if (strcmp(command, "modautorespondnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    GetValue(TmpCGI, Newu, "owner=", sizeof(Newu));
    rv = modautorespondnow();

/*                       F O R W A R D S                          */

  } else if (strcmp(command, "showforwards") == 0) {
    rv = show_forwards(Username, Domain, Mytime, command);

/*                     M A I L I N G   L I S T S                  */ 

  } else if (strcmp(command, "showmailinglists") == 0) {
    rv = show_mailing_lists(Username, Domain, Mytime, command);

  } else if (strcmp(command, "addlistmodnow") == 0 ) {
    rv = addlistgroupnow(1);

  } else if (strcmp(command, "dellistmod") == 0 ) {
    rv = dellistgroup("del_listmod.html");

  } else if (strcmp(command, "dellistmodnow") == 0 ) {
    rv = dellistgroupnow(1);

  } else if (strcmp(command, "addlistmod") == 0 ) {
    rv = addlistgroup("add_listmod.html");

  } else if (strcmp(command, "showlistmod") == 0 ) {
    rv = show_list_group("show_moderators.html");

  } else if (strcmp(command, "addlistdig") == 0 ) {
    rv = addlistgroup("add_listdig.html");

  } else if (strcmp(command, "addlistdignow") == 0 ) {
    rv = addlistgroupnow(2);

  } else if (strcmp(command, "dellistdig") == 0 ) {
    rv = dellistgroup("del_listdig.html");

  } else if (strcmp(command, "dellistdignow") == 0 ) {
    rv = dellistgroupnow(2);

  } else if (strcmp(command, "showlistdig") == 0 ) {
    rv = show_list_group("show_digest_subscribers.html");

  } else if (strcmp(command, "addmailinglist") == 0 ) {
    rv = addmailinglist();

  } else if (strcmp(command, "delmailinglist") == 0 ) {
    rv = delmailinglist();

  } else if (strcmp(command, "delmailinglistnow") == 0 ) {
    rv = delmailinglistnow();

  } else if (strcmp(command, "addlistusernow") == 0 ) {
    rv = addlistgroupnow(0);

  } else if (strcmp(command, "dellistuser") == 0 ) {
    rv = dellistgroup("del_listuser.html");

  } else if (strcmp(command, "dellistusernow") == 0 ) {
    rv = dellistgroupnow(0);

  } else if (strcmp(command, "addlistuser") == 0 ) {
    rv = addlistgroup("add_listuser.html");

  } else if (strcmp(command, "addmailinglistnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    rv = addmailinglistnow();

  } else if (strcmp(command, "modmailinglist") == 0 ) {
    rv = modmailinglist();

  } else if (strcmp(command, "modmailinglistnow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "newu=", sizeof(ActionUser));
    rv = modmailinglistnow();

  } else if (strcmp(command, "showlistusers") == 0 ) {
    rv = show_list_group("show_subscribers.html");

  } else if (strcmp(command, "setdefault") == 0 ) {
    rv = setdefaultaccount();

/*                            U S E R S                           */ 

  } else if (strcmp(command, "showusers") == 0) {
    rv = show_users(Username, Domain, Mytime, command);

  } else if (strcmp(command, "adduser") == 0 ) {
    rv = adduser();

  } else if (strcmp(command, "addusernow") == 0 ) {
    rv = addusernow();

  } else if (strcmp(command, "deluser") == 0 ) {
    GetValue(TmpCGI, ActionUser, "deluser=", sizeof(ActionUser));
    rv = deluser();

  } else if (strcmp(command, "delusernow") == 0 ) {
    GetValue(TmpCGI, ActionUser, "deluser=", sizeof(ActionUser));
    rv = delusergo();

  } else if (strcmp(command, "moduser") == 0 ) {
    rv = moduser();

  } else if (strcmp(command, "modusernow") == 0 ) {
    rv = modusergo();

  } else if (strcmp(command, "bounceall") == 0 ) {
    rv = bounceall();

  } else if (strcmp(command, "deleteall") == 0 ) {
    rv = deleteall();

  } else if (strcmp(command, "setremotecatchall") == 0 ) {
    rv = setremotecatchall();

  } else if (strcmp(command, "setremotecatchallnow") == 0 ) {
    rv = setremotecatchallnow();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                        System rights                          */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  } else {
    fprintf(stderr, "Invalid command: %s\n", command);

  }

  if( rv > -1 ) {
    printf( "Return Value: %d<BR>\n", rv );
    fprintf(stderr, "Return Value: %d<BR>\n", rv );
  }
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
