/* 
 * $Id: qmailadmin.c,v 1.6.2.6 2005-01-12 15:14:27 tomcollins Exp $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
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
#include <vpopmail_config.h>
/* undef some macros that get redefined in config.h below */
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <vpopmail.h>
#include <vauth.h>
#include <vlimits.h>

#include "alias.h"
#include "auth.h"
#include "autorespond.h"
#include "cgi.h"
#include "command.h"
#include "config.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "qmailadmin.h"
#include "show.h"
#include "user.h"
#include "util.h"

char Username[MAX_BUFF];
char Domain[MAX_BUFF];
char Password[MAX_BUFF];
char Gecos[MAX_BUFF];
char Quota[MAX_BUFF];
char Time[MAX_BUFF];
char ActionUser[MAX_BUFF];
char Newu[MAX_BUFF];
char Password1[MAX_BUFF];
char Password2[MAX_BUFF];
char Crypted[MAX_BUFF];
char Alias[MAX_BUFF];
char AliasType[MAX_BUFF];
char LineData[MAX_BUFF];
char Action[MAX_BUFF];
char Message[MAX_BIG_BUFF];
char StatusMessage[MAX_BIG_BUFF];
int num_of_mailinglist;
int CGIValues[256];
char Pagenumber[MAX_BUFF];
char SearchUser[MAX_BUFF];
time_t Mytime;
char *TmpCGI = NULL;
char TmpBuf[MAX_BIG_BUFF];
char TmpBuf1[MAX_BUFF];
char TmpBuf2[MAX_BUFF];
char TmpBuf3[MAX_BUFF];
char TempBuf[MAX_BUFF];
int Compressed;
FILE *actout;
FILE *lang_fs;
FILE *color_table;

struct vlimits Limits;
int AdminType;
int MaxPopAccounts;
int MaxAliases;
int MaxForwards;
int MaxAutoResponders;
int MaxMailingLists;

int DisablePOP;
int DisableIMAP;
int DisableDialup;
int DisablePasswordChanging;
int DisableWebmail;
int DisableRelay;

int CurPopAccounts;
int CurForwards;
int CurBlackholes;
int CurAutoResponders;
int CurMailingLists;
uid_t Uid;
gid_t Gid;
char RealDir[156];
char Lang[40];

int main(argc,argv)
 int argc;
 char *argv[];
{
 const char *ip_addr=getenv("REMOTE_ADDR");
 const char *x_forward=getenv("HTTP_X_FORWARDED_FOR");
 char *pi;
 int i,j;
 char *rm;
 char returnhttp[MAX_BUFF];
 char returntext[MAX_BUFF];

  init_globals();

  if (x_forward) ip_addr = x_forward;
  if (!ip_addr) ip_addr = "127.0.0.1";
  pi=getenv("PATH_INFO");
  if ( pi )  pi = strdup(pi);

  if (pi && strncmp(pi, "/com/", 5) == 0) {
    struct vqpasswd *pw;

    memset(TmpBuf2, 0, sizeof(TmpBuf2));
    for(j=0,i=5;pi[i]!=0&&j<99;++i,++j) TmpBuf2[j] = pi[i];
    rm = getenv("REQUEST_METHOD");
    rm = (rm == NULL ? "" : strdup(rm));

    if ( strncmp(rm , "POST", 4) == 0 ) {
      get_cgi();
    } else {
      TmpCGI = getenv("QUERY_STRING");
      TmpCGI = (TmpCGI == NULL ? "" : strdup(TmpCGI));
    }

    GetValue(TmpCGI, Username, "user=", sizeof(Username));
    GetValue(TmpCGI, Domain, "dom=", sizeof(Domain));
    GetValue(TmpCGI, Time, "time=", sizeof(Time));
    Mytime = atoi(Time);
    pw = vauth_getpw( Username, Domain );

    /* get the real uid and gid and change to that user */
    vget_assign(Domain,RealDir,sizeof(RealDir),&Uid,&Gid);
    if ( geteuid() == 0 ) {
      if ( setgid(Gid) != 0 ) perror("setgid");
      if ( setuid(Uid) != 0 ) perror("setuid");
    }

    if ( chdir(RealDir) < 0 ) {
      fprintf(stderr, "<h2>%s %s</h2>\n", get_html_text("171"), RealDir );
    }
    load_limits();

    set_admin_type();

    if ( AdminType == USER_ADMIN || AdminType == DOMAIN_ADMIN ) {
      auth_user_domain(ip_addr, pw);
    } else {
      auth_system(ip_addr, pw);
    }

    process_commands();

  } else if (pi && strncmp(pi, "/open/", 6) == 0) {
    memset(TmpBuf2, 0, sizeof(TmpBuf2));
    for(j=0,i=6;pi[i]!=0&&j<99;++i,++j) TmpBuf2[j] = pi[i];
    rm = getenv("REQUEST_METHOD");
    rm = (rm == NULL ? "" : strdup(rm));

    if ( strncmp(rm , "POST", 4) == 0 ) {
      get_cgi();
    } else {
      TmpCGI = getenv("QUERY_STRING");
      TmpCGI = (TmpCGI == NULL ? "" : strdup(TmpCGI));
    }

    GetValue(TmpCGI, Username, "user=", sizeof(Username));
    GetValue(TmpCGI, Domain, "dom=", sizeof(Domain));
    GetValue(TmpCGI, Time, "time=", sizeof(Time));
    Mytime = atoi(Time);

    vget_assign(Domain,RealDir,sizeof(RealDir),&Uid,&Gid);
    if ( geteuid() == 0 ) {
      if ( setgid(Gid) != 0 ) perror("setgid");
      if ( setuid(Uid) != 0 ) perror("setuid");
    }
    vclose();
    exit(0);
    

  } else {
   char *rm;
   struct vqpasswd *pw;
   FILE *fs;


     rm = getenv("REQUEST_METHOD");
     if ( rm ) rm = strdup(rm);

     if ( rm && ( strncmp(rm,"POST",4)==0 || strncmp(rm,"GET",3)==0)) {
       if ( strncmp(rm , "POST", 4) == 0 ) {
         get_cgi();
       } else {
         TmpCGI = getenv("QUERY_STRING");
         TmpCGI = (TmpCGI == NULL ? "" : strdup(TmpCGI));
       }

       GetValue(TmpCGI, Username, "username=", sizeof(Username));
       GetValue(TmpCGI, Domain, "domain=", sizeof(Domain));
       GetValue(TmpCGI, Password, "password=", sizeof(Password));

       vget_assign(Domain,RealDir,sizeof(RealDir),&Uid,&Gid);
       if ( geteuid() == 0 ) {
         if ( setgid(Gid) != 0 ) perror("setgid");
         if ( setuid(Uid) != 0 ) perror("setuid");
       }

       /* Authenticate a user and domain admin */
       if ( strlen(Domain) > 0 ) {
         chdir(RealDir);
         load_limits();

         pw = vauth_user( Username, Domain, Password, "" );
         if ( pw == NULL ) { 
           snprintf (StatusMessage, sizeof(StatusMessage), "%s\n", get_html_text("198"));
           show_login();
           vclose();
           exit(0);
         }

         snprintf (TmpBuf, sizeof(TmpBuf), "%s/" MAILDIR, pw->pw_dir);
         del_id_files( TmpBuf);

         Mytime = time(NULL);
         snprintf (TmpBuf, sizeof(TmpBuf), "%s/" MAILDIR "/%u.qw", pw->pw_dir, (unsigned int) Mytime);
         fs = fopen(TmpBuf, "w");
         if ( fs == NULL ) {
           printf ("%s %s<br>\n", get_html_text("144"), TmpBuf);
           vclose();
           exit(0);
         }
         memset(TmpBuf, 0, sizeof(TmpBuf)); 
         /* set session vars */
         GetValue(TmpCGI, returntext, "returntext=", sizeof(returntext));
         GetValue(TmpCGI, returnhttp, "returnhttp=", sizeof(returnhttp));
         snprinth (TmpBuf, sizeof(TmpBuf), "ip_addr=%C&returntext=%C&returnhttp=%C\n",
                 ip_addr, returntext, returnhttp); 
         fputs(TmpBuf,fs); 
        // fputs(ip_addr, fs);
         fclose(fs);
         vget_assign(Domain, TmpBuf1, sizeof(TmpBuf1), &Uid, &Gid);
         set_admin_type();

         /* show the main menu for domain admins, modify user page
            for regular users */
         if (AdminType == DOMAIN_ADMIN) {
           show_menu(Username, Domain, Mytime);
         } else {
           strcpy (ActionUser, Username);
           moduser();
         }
         vclose();
         exit(0);
       }
     }
  }
  show_login();
  vclose();
  
  return 0;
}

void init_globals()
{
  char *accept_lang;
  char *langptr, *qptr;
  char *charset;
  int lang_err;
  float maxq, thisq;

  memset(CGIValues, 0, sizeof(CGIValues));
  CGIValues['0'] = 0;
  CGIValues['1'] = 1;
  CGIValues['2'] = 2;
  CGIValues['3'] = 3;
  CGIValues['4'] = 4;
  CGIValues['5'] = 5;
  CGIValues['6'] = 6;
  CGIValues['7'] = 7;
  CGIValues['8'] = 8;
  CGIValues['9'] = 9;
  CGIValues['A'] = 10;
  CGIValues['B'] = 11;
  CGIValues['C'] = 12;
  CGIValues['D'] = 13;
  CGIValues['E'] = 14;
  CGIValues['F'] = 15;
  CGIValues['a'] = 10;
  CGIValues['b'] = 11;
  CGIValues['c'] = 12;
  CGIValues['d'] = 13;
  CGIValues['e'] = 14;
  CGIValues['f'] = 15;

  actout = stdout;
  memset(Username, 0, sizeof(Username));
  memset(Domain, 0, sizeof(Domain));
  memset(Password, 0, sizeof(Password));
  memset(Quota, 0, sizeof(Quota));
  memset(Time, 0, sizeof(Time));
  memset(ActionUser, 0, sizeof(ActionUser));
  memset(Newu, 0, sizeof(Newu));
  memset(Password1, 0, sizeof(Password1));
  memset(Password2, 0, sizeof(Password2));
  memset(Crypted, 0, sizeof(Crypted));
  memset(Alias, 0, sizeof(Alias));
  memset(Message, 0, sizeof(Message));
  memset(TmpBuf, 0, sizeof(TmpBuf));
  memset(TmpBuf1, 0, sizeof(TmpBuf1));
  memset(TmpBuf2, 0, sizeof(TmpBuf2));
  memset(TmpBuf3, 0, sizeof(TmpBuf3));

  AdminType = NO_ADMIN;

  lang_fs = NULL;

  /* Parse HTTP_ACCEPT_LANGUAGE to find highest preferred language
   * that we have a translation for.  Example setting:
   * de-de, ja;q=0.25, en;q=0.50, de;q=0.75
   * The q= lines determine which is most preferred, defaults to 1.
   * Our routine starts with en at 0.0, and then would try de-de (1.00),
   * de (1.00), ja (0.25), en (0.50), and then de (0.75).
   */

  /* default to English at 0.00 preference */
  maxq = 0.0;
  strcpy (Lang, "en");

  /* read in preferred languages */
  langptr = getenv("HTTP_ACCEPT_LANGUAGE");
  if (langptr != NULL) {
    accept_lang = malloc (strlen(langptr));
    strcpy (accept_lang, langptr);
    langptr = strtok(accept_lang, " ,\n");
    while (langptr != NULL) {
      qptr = strstr (langptr, ";q=");
      if (qptr != NULL) {
        *qptr = '\0';  /* convert semicolon to NULL */
        thisq = (float) atof (qptr+3);
      } else {
        thisq = 1.0;
      }

      /* if this is a better match than our previous best, try it */
      if (thisq > maxq) {
        lang_err = open_lang (langptr);

        /* Remove this next section for strict interpretation of
         * HTTP_ACCEPT_LANGUAGE.  It will try language xx (with the
         * same q value) if xx-yy fails.
         */
        if ((lang_err == -1) && (langptr[2] == '-')) {
          langptr[2] = '\0';
          lang_err = open_lang (langptr);
        }

        if (lang_err == 0) {
          maxq = thisq;
          strcpy (Lang, langptr);
        }
      }
      langptr = strtok (NULL, " ,\n");
    }

    free(accept_lang);
  }

  /* open the best language choice */
  open_lang (Lang);

  /* open the color table */
  open_colortable();

  umask(VPOPMAIL_UMASK);

  charset = get_html_text("000");
  printf ("Content-Type: text/html; charset=%s\n",
    *charset == '\0' ? "iso-8859-1" : charset);
#ifdef NO_CACHE
  printf ("Cache-Control: no-cache\n"); 
  printf ("Cache-Control: no-store\n"); 
  printf ("Pragma: no-cache\n"); 
  printf ("Expires: Thu, 01 Dec 1994 16:00:00 GMT\n");
#endif    
  printf ("\n"); 
}

void del_id_files( char *dirname )
{
 DIR *mydir;
 struct dirent *mydirent;

  mydir = opendir(dirname);
  if ( mydir == NULL ) return;

  while((mydirent=readdir(mydir))!=NULL){
    if ( strstr(mydirent->d_name,".qw")!=0 ) {
      snprintf (TmpBuf3, sizeof(TmpBuf3), "%s/%s", dirname, mydirent->d_name);
      unlink(TmpBuf3);
    }
  }
  closedir(mydir);
}

void quickAction (char *username, int action)
{
  /* This feature sponsored by PinkRoccade Public Sector, Sept 2004 */

  struct stat fileinfo;
  char dotqmailfn[MAX_BUFF];
  char *space, *ar, *ez;
  char *aliasline;

  /* Note that all of the functions called from quickAction() assume
   * that the username to modify is in a global called "ActionUser"
   * It would be better to pass this information as a parameter, but
   * that's how it was originally done.  The code in command.c that
   * calls quickAction() passes ActionUser as the username parameter
   * in hopes that someday we'll remove the globals and pass parameters.
   */

  /* first check for alias/forward, autorepsonder (or even mailing list) */
  aliasline = valias_select (username, Domain);
  if (aliasline != NULL) {
    /* Autoresponder/Mailing List detection algorithm:
     * We're looking for either '/autorespond ' or '/ezmlm-reject ' to
     * appear in the first line, before a space appears
     */
    space = strstr (aliasline, " ");
    ar = strstr (aliasline, "/autorespond ");
    ez = strstr (aliasline, "/ezmlm-reject ");
    if (ar && space && (ar < space)) {
      /* autorepsonder */
      if (action == ACTION_MODIFY) modautorespond();
      else if (action == ACTION_DELETE) delautorespond();
    } else if (ez && space && (ez < space)) {
      /* mailing list (cdb-backend only) */
      if (action == ACTION_MODIFY) modmailinglist();
      else if (action == ACTION_DELETE) delmailinglist();
    } else {
      /* it's just a forward/alias of some sort */
      if (action == ACTION_MODIFY) moddotqmail();
      else if (action == ACTION_DELETE) deldotqmail();
    }
  } else if (vauth_getpw (username, Domain)) {
    /* POP/IMAP account */
    if (action == ACTION_MODIFY) moduser();
    else if (action == ACTION_DELETE) deluser();
  } else {
    /* check for mailing list on SQL backend (not in valias_select) */
    snprintf (dotqmailfn, sizeof(dotqmailfn), ".qmail-%s", username);
    str_replace (dotqmailfn+7, '.', ':');
    if (stat (dotqmailfn, &fileinfo) == 0) {
      /* mailing list (MySQL backend) */
      if (action == ACTION_MODIFY) modmailinglist();
      else if (action == ACTION_DELETE) delmailinglist();
    } else {
      /* user does not exist */
      snprinth (StatusMessage, sizeof(StatusMessage), "%s (%H@%H)", 
        get_html_text("153"), username, Domain);
      show_menu(Username, Domain, Mytime);
      vclose();
    }
  }
}
