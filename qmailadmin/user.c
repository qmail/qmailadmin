/* 
 * $Id: user.c,v 1.15 2004-02-01 00:50:28 rwidmer Exp $
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
#include <errno.h>
#include "vpopmail_config.h"
/* undef some macros that get redefined in config.h below */
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include "config.h"
#include "qmailadmin.h"
#include "qmailadminx.h"
#include "vpopmail.h"
#include "vauth.h"


#define HOOKS 1

#ifdef HOOKS
/* note that as of December 2003, only the first three hooks are 
implemented */
#define HOOK_ADDUSER     "adduser"
#define HOOK_DELUSER     "deluser"
#define HOOK_MODUSER     "moduser"
#define HOOK_ADDMAILLIST "addmaillist"
#define HOOK_DELMAILLIST "delmaillist"
#define HOOK_MODMAILLIST "modmaillist"
#define HOOK_LISTADDUSER "addlistuser"
#define HOOK_LISTDELUSER "dellistuser"
#endif

int show_users(char *Username, char *Domain, time_t Mytime)
{
  if (MaxPopAccounts == 0) return 0;
  send_template("show_users.html");
}


int show_user_lines(char *user, char *dom, time_t mytime, char *dir)
{
 int  i,j,k,startnumber,moreusers = 1;
 FILE *fs;
 struct vqpasswd *pw;
 int totalpages;
 int colspan = 7;
 int allowdelete;
 char qconvert[11];

  if (MaxPopAccounts == 0) return 0;

  if (*SearchUser) {
    pw = vauth_getall(dom,1,1);
    for (k=0; pw!=NULL; k++) {
      if ((!SearchUser[1] && *pw->pw_name >= *SearchUser) ||
          ((!strcmp(SearchUser, pw->pw_name)))) {
        break;
      }

      pw = vauth_getall(dom,0,0);
    }

    if (k == 0) sprintf(Pagenumber, "1");
    else sprintf(Pagenumber, "%d", (k/MAXUSERSPERPAGE)+1);
  }

  /* Determine number of pages */
  pw = vauth_getall(dom,1,1);
  for (k=0; pw!=NULL; k++) pw = vauth_getall(dom, 0, 0);

  if (k == 0) totalpages = 1;
  else totalpages = ((k/MAXUSERSPERPAGE)+1);

  /* End determine number of pages */
  if (atoi(Pagenumber)==0) *Pagenumber='1';

  startnumber = MAXUSERSPERPAGE * (atoi(Pagenumber) - 1);

  /*
   * check to see if there are any users to list, 
   * otherwise repeat previous page
   *  
   */
  pw = vauth_getall(dom,1,1);
  if ( AdminType==DOMAIN_ADMIN ||
       (AdminType==USER_ADMIN && strcmp(pw->pw_name,Username)==0)){

    for (k = 0; k < startnumber; ++k) { 
      pw = vauth_getall(dom,0,0); 
    }
  }

  if (pw == NULL) {
    /*  No more users to view  */
    sprintf(uBufA, "%i", colspan);
    sprintf(uBufB, "%s", get_html_text("131"));
    send_template("show_error_line.html");
    moreusers = 0;

  } else {
    /*  Display users  */
    char path[256];
    while ((pw != NULL) && ((k < MAXUSERSPERPAGE + startnumber) ||  
            (AdminType!=DOMAIN_ADMIN || AdminType!=DOMAIN_ADMIN || 
            (AdminType==USER_ADMIN && strcmp(pw->pw_name,Username)==0)))) {

      if (AdminType==DOMAIN_ADMIN || 
          (AdminType==USER_ADMIN && strcmp(pw->pw_name,Username)==0)) {
        long diskquota = 0, maxmsg = 0;

        /* display account name and user name */
        sprintf(uBufA, "%s", pw->pw_name);
        sprintf(uBufB, "%s", pw->pw_gecos);

        /* display user's quota */
        snprintf(path, sizeof(path), "%s/Maildir", pw->pw_dir);
        readuserquota(path, &diskquota, &maxmsg);
        /* Convert to MB */
        sprintf(uBufD, "%-2.2lf", ((double)diskquota)/1048576.0);  
        if (strncmp(pw->pw_shell, "NOQUOTA", 2) != 0) {
            if(quota_to_megabytes(qconvert, pw->pw_shell)) {
              sprintf(uBufC, "%s&nbsp;/&nbsp;(BAD)", uBufD);

            } else { 
              sprintf(uBufC, "%s&nbsp/&nbsp;%s", uBufD, qconvert); 
            }

          } else { 
            sprintf(uBufC, "%s&nbsp;/&nbsp;%s", uBufD, get_html_text("229"));
          }

        /* display button to modify user */
        qmail_button(uBufD, "moduser", pw->pw_name, "modify.png" );
          
        /* if the user has admin privileges and pw->pw_name is not 
         * the user or postmaster, allow deleting 
         */
        if ((1 == 1 || AdminType==DOMAIN_ADMIN ) && 
             strcmp(pw->pw_name, Username) != 0 &&
             strcmp(pw->pw_name, "postmaster") != 0) {
          allowdelete = 1;

        /* else, don't allow deleting */
        } else {
          allowdelete = 0;
        }

        /* display trashcan for delete, or nothing if delete not allowed */
        if (allowdelete) {
          qmail_button(uBufE, "deluser", pw->pw_name, "trash.png" );
        } else {
          strcpy(uBufE, "&nbsp;");
        }

        /* display button in the 'set catchall' column */
        if (strncmp(pw->pw_name,CurCatchall,sizeof(CurCatchall)) == 0) {
          /*  Catchall name matches account name being displayed  */
          qmail_icon(uBufF, "radio-on.png");

        } else if (1==1 || AdminType==DOMAIN_ADMIN) {
          /*  User is admin, so can edit / delete anyone  */
          qmail_button(uBufF, "setdefault", pw->pw_name, "radio-off.png" );

        } else {
          /*  User can't delete  */
          qmail_icon(uBufF, "disabled.png");

        }

        send_template("show_users_line.html" );
        }        

        pw = vauth_getall(dom,0,0);
        ++k;
      }
    }

    return 0;
  }

  adduser()
  {

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    return(142);
  }
                                                
  if ( MaxPopAccounts != -1 && CurPopAccounts >= MaxPopAccounts ) {
    sprintf(StatusMessage, "%s %d\n", get_html_text("199"),
      MaxPopAccounts);
    show_menu();
    return(199);
  }

  send_template( "add_user.html" );

}

int moduser()
{

  if (!( AdminType==DOMAIN_ADMIN ||
        (AdminType==USER_ADMIN && strcmp(ActionUser,Username)==0))){
    sprintf(StatusMessage,"%s", get_html_text("142"));
    return(142);
  }

  send_template( "mod_user.html" );
} 

int addusernow()
{
 char pw[50];
 int cnt=0, num;
 char *c_num;
 char **mailingListNames;
 char *tmp;
 char *email;
 char **arguments;
 char tmpstr[MAX_BUFF];
 char qconvert[11];
 int pid;
 int i;
 int tmpint;
 int error;
 struct vqpasswd *mypw;
 char pw_shell[256];
 char spamvalue[50];
 static char NewBuf[156];
 FILE *fs;
 char Buffer1[MAX_BUFF];
 char Buffer2[MAX_BUFF];

  c_num = malloc(MAX_BUFF);
  email = malloc(128);
  tmp = malloc(MAX_BUFF);
  arguments = (char **)malloc(MAX_BUFF);

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    return(142);
  }

  if ( MaxPopAccounts != -1 && CurPopAccounts >= MaxPopAccounts ) {
    sprintf(StatusMessage, "%s %d\n", get_html_text("199"),
      MaxPopAccounts);
    show_menu();
    return(199);
  }
 
  GetValue(TmpCGI,Newu, "newu=", sizeof(Newu));

  if ( fixup_local_name(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("148"), Newu);
    adduser();
    return(148);
  } 

  if ( check_local_user(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("175"), Newu);
    adduser();
    return(175);
  } 
  // Coded added by jhopper
#ifdef MODIFY_QUOTA
  GetValue(TmpCGI, Quota, "quota=", sizeof(Quota));
#endif
  GetValue(TmpCGI,Password1, "password1=", sizeof(Password1));
  GetValue(TmpCGI,Password2, "password2=", sizeof(Password2));
  if ( strcmp( Password1, Password2 ) != 0 ) {
    sprintf(StatusMessage, "%s\n", get_html_text("200"));
    adduser();
    return(200);
  }

#ifndef ENABLE_LEARN_PASSWORDS
  if ( strlen(Password1) <= 0 ) {
    sprintf(StatusMessage, "%s\n", get_html_text("234"));
    adduser();
    return(234);
  }
#endif

  snprintf (email, 128, "%s@%s", Newu, Domain);
    
  GetValue(TmpCGI,Gecos, "gecos=", sizeof(Gecos));
  if ( strlen( Gecos ) == 0 ) {
    strcpy(Gecos, Newu);
  }

  GetValue(TmpCGI, c_num, "number_of_mailinglist=", MAX_BUFF);
  num = atoi(c_num);
  if(!(mailingListNames = malloc(sizeof(char *) * num))) {
    sprintf(StatusMessage, "%s\n", get_html_text("201"));
    return(201);

  } else {
    for(cnt = 0; cnt < num; cnt++) {
      if(!(mailingListNames[cnt] = malloc(MAX_BUFF))) {
        sprintf(StatusMessage, "%s\n", get_html_text("201"));
        return(201);
      }
    }

    for(cnt = 0; cnt < num; cnt++) {
      sprintf(tmp, "subscribe%d=", cnt);
      error = GetValue(TmpCGI, mailingListNames[cnt], tmp, MAX_BUFF);
      if( error != -1 ) {
        pid=fork();

        if (pid==0) {
          sprintf(Buffer1, "%s/ezmlm-sub", EZMLMDIR);
          sprintf(Buffer2, "%s/%s", RealDir, mailingListNames[cnt]);
          execl(Buffer1, "ezmlm-sub", Buffer2, email, NULL);
          exit(127);
        } else {
          wait(&pid);
        }
      } 
    }
  }

  /* add the user then get the vpopmail password structure */
  if ( vadduser( Newu, Domain, Password1, Gecos, USE_POP ) == 0 && 
#ifdef MYSQL_REPLICATION
    !sleep(2) &&
#endif
    (mypw = vauth_getpw( Newu, Domain )) != NULL ) {

    /* vadduser() in vpopmail 5.3.29 and later sets the default
     * quota, so we only need to change it if the user enters
     * something in the Quota field.
     */

#ifdef MODIFY_QUOTA
    if (strcmp (Quota, "NOQUOTA") == 0) {
      vsetuserquota (Newu, Domain, "NOQUOTA");
    } else if ( Quota[0] != 0 ) {
      if(quota_to_bytes(qconvert, Quota)) { 
        sprintf(StatusMessage, get_html_text("314"));
      } else {
        vsetuserquota (Newu, Domain, qconvert);
      }
    }
#endif

#ifdef MODIFY_SPAM
    GetValue(TmpCGI, spamvalue, "spamcheck=", sizeof(spamvalue));
    if(strcmp(spamvalue, "on") == 0) {
       snprintf(NewBuf, sizeof(NewBuf), "%s/.qmail", mypw->pw_dir);
       fs = fopen(NewBuf, "w+");
       fprintf(fs, "%s\n", SPAM_COMMAND);
       fclose(fs);
    }
#endif

    /* report success */
    sprintf(StatusMessage, "%s %s@%s (%s) %s",
      get_html_text("002"), Newu, Domain, Gecos,
      get_html_text("119"));

  } else {
    /* otherwise, report error */
    sprintf(StatusMessage, "<font color=\"red\">%s %s@%s (%s) %s</font>", 
      get_html_text("002"), Newu, Domain, Gecos, get_html_text("120"));
  }

  call_hooks(HOOK_ADDUSER, Newu, Domain, Password1, Gecos);

  CurPopAccounts++;

  /* After we add the user, show the user page
   * people like to visually verify the results
   */
  show_users(Username, Domain, Mytime);

}

int call_hooks(char *hook_type, char *p1, char *p2, char *p3, char *p4)
{
 FILE *fs = NULL;
 int pid;
 char *hooks_path;
 char *cmd = NULL;
 char *tmpstr;
 int error;
 char Buffer[MAX_BUFF];
    
  hooks_path = malloc(MAX_BUFF);

  /* first look in directory for domain */
  sprintf(hooks_path, "%s/.qmailadmin-hooks", RealDir);
  if((fs = fopen(hooks_path, "r")) == NULL) {
    /* then try ~vpopmail/etc */
    sprintf(hooks_path, "%s/etc/.qmailadmin-hooks", VPOPMAILDIR);
    if((fs = fopen(hooks_path, "r")) == NULL) {
      return (0);
    }
  }

  while(fgets(Buffer, sizeof(Buffer), fs) != NULL) {
    if ( (*Buffer == '#') || (*Buffer == '\0')) continue;
    tmpstr = strtok(Buffer, " :\t\n");
    if (tmpstr == NULL) continue;

    if ( strcmp(tmpstr, hook_type) == 0) {
      tmpstr = strtok(NULL, " :\t\n");

      if ((tmpstr == NULL) || (*tmpstr == '\0')) continue;

      cmd = strdup(tmpstr);        

      break;
    }
  }

  fclose(fs);

  if (cmd == NULL) return 0;   /* don't have a hook for this type */
    
  pid = fork();

  if (pid == 0) {
    /* Second param to execl should actually be just the program name,
       without the path information.  Add a pointer to point into cmd
       at the start of the program name only.    BUG 2003-12 */
    error = execl(cmd, cmd, p1, p2, p3, p4, NULL);
    fprintf(actout, "Error %d %s \"%s\", %s, %s, %s, %s, %s\n",
      errno, get_html_text("202"), cmd, hook_type, p1, p2, p3, p4);
    /* if (error == -1) return (-1); */
    exit(127);
  } else {
    wait(&pid);
  }

  return (0);
}

deluser()
{
  send_template( "del_user_confirm.html" );
}

int delusergo()
{
 static char forward[200] = "";
 static char forwardto[200] = "";
 FILE *fs;
 int i;
 struct vqpasswd *pw;
     
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    return(142);
  }

  vdeluser( ActionUser, Domain );

  sprintf(StatusMessage, "%s %s", ActionUser, get_html_text("141"));

  /* Start create forward when delete - 
   * Code added by Eugene Teo 6 June 2000 
   * Modified by Jeff Hedlund (jeff.hedlund@matrixsi.com) 4 June 2003 
   */

  GetValue(TmpCGI,forward, "forward=", sizeof(forward));
  if (strcmp(forward, "on") == 0) {
    GetValue(TmpCGI, forwardto, "forwardto=", sizeof(forwardto));    
    if(adddotqmail_shared(ActionUser, forwardto, -1)!=0) {
       sprintf(StatusMessage, get_html_text("315"), forwardto);
    }
  } 

  call_hooks(HOOK_DELUSER, ActionUser, Domain, forwardto, "");
  show_users(Username, Domain, Mytime);
}

setremotecatchall() 
{
  send_template("setremotecatchall.html");
}

int setremotecatchallnow() 
{
  GetValue(TmpCGI,Newu, "newu=", sizeof(Newu));

  if (check_email_addr(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("148"), Newu);
    setremotecatchall();
    return(148);
  }
  set_remote_catchall_now();
}

int set_remote_catchall_now()
{
 FILE *fs;

  if ( (fs = fopen(".qmail-default", "w")) == NULL ) {
    fprintf(stderr,"QmailAdmin: SRCN %s %s\n", 
            get_html_text("144"), ".qmail-default");

  } else {
    fprintf(fs,"| %s/bin/vdelivermail '' %s\n",VPOPMAILDIR,Newu);
    fclose(fs);
    strcpy(CurCatchall,Newu);
  }
  show_users(Username, Domain, Mytime);
  return(0);
}

int bounceall()
{
 FILE *fs;

  if ( (fs = fopen(".qmail-default", "w")) == NULL ) {
    fprintf(stderr,"QmailAdmin: BA %s %s\n", 
            get_html_text("144"), ".qmail-default");
  } else {
    fprintf(fs,"| %s/bin/vdelivermail '' bounce-no-mailbox\n",VPOPMAILDIR);
    fclose(fs);
  }
  strcpy(CurCatchall, get_html_text("130"));
  show_users(Username, Domain, Mytime);
  return(130);
}

int deleteall()
{
 FILE *fs;

  if ( (fs = fopen(".qmail-default", "w")) == NULL ) {
    fprintf(stderr,"QmailAdmin: DA %s %s\n", 
            get_html_text("144"), ".qmail-default");
  } else {
    fprintf(fs,"| %s/bin/vdelivermail '' delete\n",VPOPMAILDIR);
    fclose(fs);
  }
  strcpy(CurCatchall, get_html_text("303"));
  show_users(Username, Domain, Mytime);
  return(303);
}

int modusergo()
{
 char crypted[20]; 
 char *tmpstr;
 int i;
 int ret_code;
 int password_updated = 0;
 struct vqpasswd *vpw=NULL;
 static char box[500];
 static char NewBuf[156];
 char *quotaptr;
 char qconvert[11];
 int count;
 FILE *fs;
 int spam_check = 0;

  if (!( AdminType==DOMAIN_ADMIN ||
         (AdminType==USER_ADMIN && strcmp(ActionUser,Username)==0))){
    sprintf(StatusMessage,"%s", get_html_text("142"));
    return(142);
  }

  if (strlen(Password1)>0 && strlen(Password2)>0 ) {
    if ( strcmp( Password1, Password2 ) != 0 ) {
      sprintf(StatusMessage, "%s\n", get_html_text("200"));
      moduser();
      return(200);
    }
    ret_code = vpasswd( ActionUser, Domain, Password1, USE_POP);
    if ( ret_code != VA_SUCCESS ) {
      sprintf(StatusMessage, "%s (%s)", get_html_text("140"), 
        verror(ret_code));
    } else {
      /* sprintf(StatusMessage,"%s %s@%s.", get_html_text("139"), 
ActionUser, Domain ); */
      strcpy (StatusMessage, get_html_text("139"));
    }
  }

#ifdef MODIFY_QUOTA
  /* strings used: 307 = "Invalid Quota", 308 = "Quota set to unlimited",
   * 309 = "Quota set to %s bytes"
   */
  if (AdminType == DOMAIN_ADMIN) {
    GetValue(TmpCGI, Quota, "quota=", sizeof(Quota));
    vpw = vauth_getpw(ActionUser, Domain); 
    if ((strlen(Quota) == 0) || (strcmp (vpw->pw_shell, Quota) == 0)) {
      /* Blank or no change, do nothing */
    } else if (strncmp(Quota, "NOQUOTA", 2)==0) {
      if (vsetuserquota( ActionUser, Domain, Quota )) {
        sprintf(StatusMessage, get_html_text("307"));  /* invalid quota */
      } else {
        sprintf(StatusMessage, get_html_text("308"));
      }
    } else if (atoi(Quota)) {
      quotaptr = Quota;
      if (quota_to_bytes(qconvert, quotaptr)) {
        sprintf(StatusMessage, get_html_text("307"));
      } else if(strcmp(qconvert, vpw->pw_shell)==0) {
        /* unchanged, do nothing */
      } else if(vsetuserquota( ActionUser, Domain, qconvert )) {
        sprintf(StatusMessage, get_html_text("307"));
      } else { 
        sprintf(StatusMessage, get_html_text("309"), qconvert);
      }
    } else {
      sprintf(StatusMessage, get_html_text("307"));
    }
  }
#endif

  GetValue(TmpCGI,Gecos, "gecos=", sizeof(Gecos));
  if ( strlen( Gecos ) != 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gecos = Gecos;
    vauth_setpw(vpw, Domain);
  }

  /* check for the V_USERx flags and set accordingly */
  /* James Raftery {james@now.ie}, 12 Dec. 2002 */
  GetValue(TmpCGI,box, "zeroflag=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid |= V_USER0;
    vauth_setpw(vpw, Domain);
  } else if ( strcmp(box,"off") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid &= ~V_USER0;
    vauth_setpw(vpw, Domain);
  }
  GetValue(TmpCGI,box, "oneflag=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid |= V_USER1;
    vauth_setpw(vpw, Domain);
  } else if ( strcmp(box,"off") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid &= ~V_USER1;
    vauth_setpw(vpw, Domain);
  }
  GetValue(TmpCGI,box, "twoflag=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid |= V_USER2;
    vauth_setpw(vpw, Domain);
  } else if ( strcmp(box,"off") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid &= ~V_USER2;
    vauth_setpw(vpw, Domain);
  }
  GetValue(TmpCGI,box, "threeflag=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid |= V_USER3;
    vauth_setpw(vpw, Domain);
  } else if ( strcmp(box,"off") == 0 ) {
    vpw = vauth_getpw(ActionUser, Domain); 
    vpw->pw_gid &= ~V_USER3;
    vauth_setpw(vpw, Domain);
  }

  /* get value of the spam filter box */
  GetValue(TmpCGI,box, "spamcheck=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) spam_check = 1;

  /* get the value of the cforward radio button */
  GetValue(TmpCGI,box, "cforward=", sizeof(box));
  
  /* if they want to disable everything */
  if ( strcmp(box,"disable") == 0 ) {

    /* unlink the .qmail file */
    if ( vpw == NULL ) vpw = vauth_getpw(ActionUser, Domain); 
    snprintf(NewBuf,sizeof(NewBuf),"%s/.qmail", vpw->pw_dir);
    unlink(NewBuf);

    /* if the mail is to be checked for spam, rewrite the file with command */
    if( spam_check == 1 ) {
       fs = fopen(NewBuf, "w+");
       fprintf(fs, "%s\n", SPAM_COMMAND);
       fclose(fs);
    }

    /* delete any vacation directory */
    snprintf(NewBuf,sizeof(NewBuf),"%s/vacation", vpw->pw_dir);
    vdelfiles(NewBuf);

  /* if they want to forward */
  } else if (strcmp(box,"forward") == 0 ) {

    /* get the value of the foward */
    GetValue(TmpCGI,box, "nforward=", sizeof(box));

    /* If nothing was entered, error */
    if ( box[0] == 0 ) {
      sprintf(StatusMessage, "%s\n", get_html_text("215"));
      moduser();
      return(215);

    /* check it for a valid email address
    } else if ( check_email_addr( box ) == 1 )  {
      sprintf(StatusMessage, "%s\n", get_html_text("148"));
      moduser();
    */
    }

    /* everything looks good, open the file */
    if ( vpw == NULL ) {
      vpw = vauth_getpw(ActionUser, Domain); 
    }
    snprintf(NewBuf,sizeof(NewBuf),"%s/.qmail", vpw->pw_dir);

    fs = fopen(NewBuf,"w+");
    tmpstr = strtok(box," ,;\n");

    /* tmpstr points to first non-token */

    count=0;
    while( tmpstr != NULL && count < 5) {
      if ((*tmpstr != '|') && (*tmpstr != '/')) {
        fprintf(fs,"&%s\n", tmpstr);
        ++count;
      }
      tmpstr = strtok(NULL," ,;\n");
    }

    /* if they want to save a copy */
    GetValue(TmpCGI,box, "fsaved=", sizeof(box));
    if ( strcmp(box,"on") == 0 ) {
      if( spam_check == 1 ) {
        /* if spam check is enabled, that will save the message*/
        fprintf(fs, "%s\n", SPAM_COMMAND);
      } else {
        fprintf(fs,"%s/Maildir/\n", vpw->pw_dir);
      }
    } 
    fclose(fs);

  /* they want vacation */
  } else if (strcmp(box,"vacation") == 0 ) {

    /* get the subject */
    GetValue(TmpCGI,box, "vsubject=", sizeof(box));

    /* if no subject, error */
    if ( box[0] == 0 ) {
      sprintf(StatusMessage, "%s\n", get_html_text("216"));
      moduser();
      return(216);
    }
 
    /* make the vacation directory */
    if ( vpw == NULL ) vpw = vauth_getpw(ActionUser, Domain); 
    snprintf(NewBuf,sizeof(NewBuf),"%s/vacation", vpw->pw_dir);
    mkdir(NewBuf, 0750);

    /* open the .qmail file */
    snprintf(NewBuf,sizeof(NewBuf),"%s/.qmail", vpw->pw_dir);
    fs = fopen(NewBuf,"w+");
    fprintf(fs, "| %s/autorespond 86400 3 %s/vacation/message %s/vacation\n",
      AUTORESPOND_PATH, vpw->pw_dir, vpw->pw_dir );

    /* save a copy for the user (if checking for spam, it will keep a copy)*/
    if(spam_check==1)
       fprintf(fs, "%s\n", SPAM_COMMAND);
    else
      fprintf(fs,"%s/Maildir/\n", vpw->pw_dir);
    fclose(fs);

    /* set up the message file */
    snprintf(NewBuf,sizeof(NewBuf),"%s/vacation/message", vpw->pw_dir);
    GetValue(TmpCGI,Message, "vmessage=",sizeof(Message));

    if ( (fs = fopen(NewBuf, "w")) == NULL ) ack("123", 123);
    fprintf(fs, "From: %s@%s\n", ActionUser,Domain);
    fprintf(fs, "Subject: %s\n\n", box);
    fprintf(fs, "%s", Message);
    fclose(fs);

    /* save the forward for vacation too */
    GetValue(TmpCGI,box,"nforward=", sizeof(box));
    snprintf(NewBuf, sizeof(NewBuf), "%s/.qmail", vpw->pw_dir);
    fs = fopen(NewBuf, "a+");
    tmpstr = strtok(box, " ,;\n");
    count = 0;
    while( tmpstr != NULL && count < 2 ) {
      fprintf(fs, "&%s\n", tmpstr);
      tmpstr = strtok(NULL, " ,;\n");
      ++count;
    }
    fclose(fs);

  /* they want to blackhole */
  /* James Raftery {james@now.ie}, 21 May 2003 */
  } else if (strcmp(box,"blackhole") == 0 ) {

    /* open the .qmail file */
    snprintf(NewBuf,sizeof(NewBuf),"%s/.qmail", vpw->pw_dir);
    fs = fopen(NewBuf,"w");

    /* for now we use /bin/true, eventually switch this to '# delete' */
    fprintf(fs, "|%s/true delete\n", TRUE_PATH);

    fclose(fs);

  } else {
    /* is this an error conditition? none of radios selected? */
    printf("nothing\n");
  }

  call_hooks(HOOK_MODUSER, ActionUser, Domain, Password1, Gecos);
  moduser();
}
