/* 
 * $Id: qmailadmin.c,v 1.11 2004-02-01 02:13:56 rwidmer Exp $
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
#include <vpopmail_config.h>
/* undef some macros that get redefined in config.h below */
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include "config.h"
#include "qmailadmin.h"
#include <vpopmail.h>
#include <vauth.h>
#include <vlimits.h>

char Username[MAX_BUFF];
char Domain[MAX_BUFF];
char Password[MAX_BUFF];
char Gecos[MAX_BUFF];
char Quota[MAX_BUFF];
char Time[MAX_BUFF];
char Action[MAX_BUFF];
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
int CGIValues[256];
char Pagenumber[MAX_BUFF]="";
char SearchUser[MAX_BUFF];
time_t Mytime;
char *TmpCGI = NULL;
int Compressed;
FILE *actout;

struct vlimits Limits;
int num_of_mailinglist;
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

int CurPopAccounts=0;
int CurForwards=0;
int CurAutoResponders=0;
int CurMailingLists=0;
int CurBlackholes=0;
char CurCatchall[MAX_BUFF];

uid_t Uid;
gid_t Gid;
char RealDir[156];

char Lang[40];

char uBufA[MAX_BUFF];
char uBufB[MAX_BUFF];
char uBufC[MAX_BUFF];
char uBufD[MAX_BUFF];
char uBufE[MAX_BUFF];
char uBufF[MAX_BUFF];
char uBufG[MAX_BUFF];
char uBufH[MAX_BUFF];
char uBufI[MAX_BUFF];
char uBufJ[MAX_BUFF];
char uBufK[MAX_BUFF];
char uBufL[MAX_BUFF];

/*   not in qmailadminx.h   */
FILE *lang_fs;

void del_id_files( char *);
int create_session_file( char *ip_addr, char *domaindir);

get_parms()
{
 char *rm;

/* Get the REQUEST_METHOD  */
  rm = getenv("REQUEST_METHOD");
  rm = (rm == NULL ? "" : strdup(rm));

/*  Decide how to retrieve the data   */
  if ( strncmp(rm , "POST", 4) == 0 ) {
    get_cgi();
  } else {
    TmpCGI = getenv("QUERY_STRING");
    TmpCGI = (TmpCGI == NULL ? "" : strdup(TmpCGI));
  }

/*   Now get common values out of TmpCGI   */
  GetValue(TmpCGI, Username, "user=", sizeof(Username));
  GetValue(TmpCGI, Domain, "dom=", sizeof(Domain));
  GetValue(TmpCGI, Time, "time=", sizeof(Time));
  GetValue(TmpCGI, Action, "action=", sizeof(Time));
  GetValue(TmpCGI, Password, "password=", sizeof(Password));

  Mytime = atoi(Time);

  GetValue(TmpCGI, Pagenumber, "page=", sizeof(Pagenumber));
  GetValue(TmpCGI, SearchUser, "searchuser=", sizeof(SearchUser));
  GetValue(TmpCGI, ActionUser, "modu=", sizeof(ActionUser));
  GetValue(TmpCGI, Password1, "password1=", sizeof(Password1));
  GetValue(TmpCGI, Password2, "password2=", sizeof(Password2));
  GetValue(TmpCGI, Gecos, "gecos=", sizeof(Gecos));
  GetValue(TmpCGI, AliasType, "atype=", sizeof(AliasType));
  GetValue(TmpCGI, Alias, "alias=", sizeof(Alias));
  GetValue(TmpCGI, LineData, "linedata=", sizeof(LineData));
  GetValue(TmpCGI, Message, "message=", sizeof(Message));
  GetValue(TmpCGI, Newu, "newu=", sizeof(Newu));

  fprintf(stderr, "GetParms: Username: %s  Password: %s  Domain: %s\n", 
          Username, Password, Domain);

}

setuidgid(char *Domain)
{
  /* get the real uid and gid and change to that user */
  vget_assign(Domain,RealDir,sizeof(RealDir),&Uid,&Gid);

  if ( geteuid() == 0 ) {
    if ( setgid(Gid) != 0 ) perror("setgid");
    if ( setuid(Uid) != 0 ) perror("setuid");
  }

  if ( chdir(RealDir) < 0 ) {
    fprintf(stderr, "MAIN %s %s\n", get_html_text("171"), RealDir );
  }
}

int get_command_parms( char *commandparms, int parmsize )
{
 char *pi;
 int i,j;

  fprintf(stderr, "get_command_parms started\n" );

  pi=getenv("PATH_INFO");
  if ( pi )  pi = strdup(pi);

  fprintf(stderr, "get_command_parms pi: %s\n", pi );

  memset(commandparms, 0, parmsize);
  fprintf(stderr, "sizeof commandparms: %d", parmsize);

  /*  Cut off the first five characters...  '/com/'   */
  if (pi && strncmp(pi, "/com/", 5) == 0) {
    for(j=0,i=5;pi[i]!=0&&j<99;++i,++j) commandparms[j] = pi[i];
    fprintf(stderr, "get command parms - found it: %s\n", commandparms );
    return( 0 );
  }

  fprintf(stderr, "get command parms - nothing\n" );
  return(1);
}

get_my_ip( char *ip_address )
{

 const char *ip_addr=getenv("REMOTE_ADDR");
 const char *x_forward=getenv("HTTP_X_FORWARDED_FOR");

  if (x_forward) ip_addr = x_forward;
  if (!ip_addr) ip_addr = "127.0.0.1";

  strcpy(ip_address, ip_addr);
}

serious_error_abort( int errors )
{
fprintf(stderr, "Serious internal error: %d\n", errors);

printf("<HTML><BODY><H1>%s - %d</H1></BODY></HTML>", 
       "Serious internal error", errors );
vclose();
exit(0);
}

get_pathinfo()
{


}

main(argc,argv)
 int argc;
 char *argv[];
{
 char CommandParms[MAX_BUFF];
 char ip_addr[MAX_BUFF];
 struct vqpasswd *pw;
 int errors = 0;
 char err_code[4];

  umask(VPOPMAIL_UMASK);

  init_globals();
  if( open_lang()) errors += 1;
  get_parms();  
  get_my_ip( ip_addr );

  paint_headers();

  /*  If there are errors starting up, report them and exit  */
  if( errors > 0 ) serious_error_abort( errors );

  send_template("header.html");  

  if (!get_command_parms(CommandParms, sizeof(CommandParms))) {

    /*  Have command - prepare to execute it  */
    fprintf( stderr, "\nMystery if case #1\n" );
    setuidgid( Domain );
    set_admin_type();
    count_stuff();

    if( errors = get_session_data( Username, Domain, ip_addr )) {
 
      /*  Error with session data - need to login  */
      sprintf(err_code, "%3d\n", errors );
      fprintf( stderr, "Error code returned: %s", err_code);
      sprintf(StatusMessage, "%s\n", get_html_text( err_code ));
      show_login();

    } else {

      /*  Command requested, and session valid - do it  */
      process_commands(CommandParms);

    }


  } else if ( strlen(Action) == 0) {    

    /*  No button pressed...   Show login page   */
    show_login();

  } else if (0 == strlen(Username) || 0==strlen(Password)) {

    /*  If they left anything blank, don't bother to authenticate  */
    sprintf(StatusMessage, "%s\n", get_html_text("316"));
    show_login();

  } else if (NULL == (pw = vauth_user( Username, Domain, Password, "" ))) { 

    /*  Invalid user/domain/password - show error message */
    sprintf(StatusMessage, "%s\n", get_html_text("198"));
    show_login();

  } else {

    /*  Just logged in   */
    setuidgid( Domain );
    if( create_session_file( ip_addr, pw->pw_dir )) {

      printf( "Unable to create session file" );

    } else {

      load_limits();

      if (AdminType == DOMAIN_ADMIN) {

        /* show the main menu for domain admins  */
        show_menu(Username, Domain, Mytime);

      } else {

        /*  show the modify user page for regular users  */
        strcpy (ActionUser, Username);
        moduser();

      }
    }
  }

  send_template("footer.html");
  vclose();
}

init_globals()
{

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
  memset(Action, 0, sizeof(Action));
  memset(ActionUser, 0, sizeof(ActionUser));
  memset(Newu, 0, sizeof(Newu));
  memset(Password1, 0, sizeof(Password1));
  memset(Password2, 0, sizeof(Password2));
  memset(Crypted, 0, sizeof(Crypted));
  memset(Alias, 0, sizeof(Alias));
  memset(Message, 0, sizeof(Message));

  AdminType = NO_ADMIN;
}


paint_headers()
{
  fprintf(actout,"Content-Type: text/html\n");
#ifdef NO_CACHE
  fprintf(actout,"Cache-Control: no-cache\n"); 
  fprintf(actout,"Cache-Control: no-store\n"); 
  fprintf(actout,"Pragma: no-cache\n"); 
  fprintf(actout,"Expires: Thu, 01 Dec 1994 16:00:00 GMT\n");
#endif    
  fprintf(actout,"\n"); 
}

