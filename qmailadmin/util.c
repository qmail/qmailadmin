/* 
 * $Id: util.c,v 1.5 2004-01-30 03:28:19 rwidmer Exp $
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

extern FILE *lang_fs;
extern FILE *color_table;

#define SORT_TABLE_ENTRIES 100000

/* pointer to array of pointers */
unsigned char **sort_list;

unsigned char *sort_block[200]; /* memory blocks for sort data */
int memleft, memindex, sort_entry;
char *sort_ptr;


int count_stuff(void)
{


 DIR *mydir;
 struct dirent *mydirent;
 struct stat mystat;
 FILE *fs;
 char *alias_name_from_command;
 char mailinglist_name[MAX_FILE_NAME];
 char Buffer[MAX_BUFF];
 int i,j;

 struct vqpasswd *pw;

  fprintf( stderr, "Count Stuff\n" );

  /*   Count the Pop Accounts   */
  pw = vauth_getall(Domain,1,0);
  while(pw!=NULL){
    ++CurPopAccounts;
    pw = vauth_getall(Domain,0,0);
  }

  fprintf( stderr, "Count Stuff after count pop %d\n", CurPopAccounts );

  /*   Get limits data from vpopmail   */
  vget_limits(Domain, &Limits);
  MaxPopAccounts = Limits.maxpopaccounts;
  MaxAliases = Limits.maxaliases;
  MaxForwards = Limits.maxforwards;
  MaxAutoResponders = Limits.maxautoresponders;
  MaxMailingLists = Limits.maxmailinglists;

  DisablePOP = Limits.disable_pop;
  DisableIMAP = Limits.disable_imap;
  DisableDialup = Limits.disable_dialup;
  DisablePasswordChanging = Limits.disable_passwordchanging;
  DisableWebmail = Limits.disable_webmail;
  DisableRelay = Limits.disable_relay;

  fprintf( stderr, "Count Stuff after set limits\n" );

  /*   scan the domain directory for .qmail files  */
  if ( (mydir = opendir(".")) == NULL ) { 
    /*  If we can't open the domain directory  */
    /*  This is a BIG problem.  Should show error, */
    /*  paint the footer file and die.  */
    fprintf( stderr, "QmailAdmin: Can't open domian directory\n" );
    return(143);
  }

  /*  Scan the directory for .qmail files  */
  while ((mydirent=readdir(mydir)) != NULL) {

    fprintf(stderr,"Checking: %s\n", mydirent->d_name);  

    if ( strncmp(".qmail-", mydirent->d_name, 7) != 0 ) {
      /*  It is not a .qmail file   */
      fprintf(stderr,"not .qmail file\n");   
      continue;
    }

    if (!lstat(mydirent->d_name, &mystat) && S_ISLNK(mystat.st_mode)) {
      /*  It is a symlink, probably a mailing list  */

      if ( strncmp("-owner", mydirent->d_name, 7) != 0 ) {
        /*  If we only count the -owner link, the count  */
        /*  should come out right.  */
        ++CurMailingLists;
      }
      fprintf(stderr,"mail list file\n");   
      continue;
    } 

    if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
      /*  Unable to open a .qmail file  */
      continue;
    } 

    /*  Read the first line of the .qmail file  */
    memset(Buffer, 0, sizeof(Buffer));
    fgets(Buffer, sizeof(Buffer), fs);

    /*  Decide what kind of account it is  */
    if (*Buffer == '#') {
      fprintf(stderr,"Blackhole\n");   
      CurBlackholes++;

    } else if ( strstr( Buffer, "autorespond") != 0 ) {
      fprintf(stderr,"Autoresponder\n");   
      CurAutoResponders++;

    } else if (*Buffer != '|' ) {
      fprintf(stderr,"Forward\n");   
      CurForwards++;

    }

    fclose(fs);
  }

  closedir(mydir);

  /* Get the default catchall box name */
  fprintf( stderr, "This is a test\n" );
  if ((fs=fopen(".qmail-default","r")) == NULL) {
    /* report error opening .qmail-default and exit */
    fprintf( stderr, "show_user_lines can't open default file\n" );
    sprintf( CurCatchall, "Can't open .qmail-default file" );
    

  } else {
    fgets( Buffer, sizeof(Buffer), fs);
    fclose(fs);

    fprintf( stderr, ".qmail-default: %s\n", Buffer);

    if (strstr(Buffer, " bounce-no-mailbox\n") != NULL) {
      sprintf(CurCatchall,"%s", get_html_text("130"));

    } else if (strstr(Buffer, " delete\n") != NULL) {
      sprintf(CurCatchall,"%s", get_html_text("236"));

    } else if ( strstr(Buffer, "@") != NULL ) {
      /* check for local user to forward to */
      if (strstr(Buffer, Domain) != NULL) {
        i = strlen(Buffer); --i; Buffer[i] = 0; /* take off newline */
        for(;Buffer[i]!=' ';--i);
        for(j=0,++i;Buffer[i]!=0 && Buffer[i]!='@';++j,++i) 
           CurCatchall[j] = Buffer[i];
        CurCatchall[j]=0;
      }

    } else {
      /* Maildir type catchall */
      i = strlen(Buffer); --i; Buffer[i] = 0; /* take off newline */
      for(;Buffer[i]!='/';--i);
      for(j=0,++i;Buffer[i]!=0;++j,++i) CurCatchall[j] = Buffer[i];
      CurCatchall[j]=0;
    }

    fprintf( stderr, "CurCatchall: %s\n", CurCatchall);

    return 0;
  }

  fprintf( stderr, "Blackholes: %d Lists; %d Robots: %d Forwards: %d Users %d Catchall: %s\n", CurBlackholes, CurMailingLists, CurAutoResponders, CurForwards, CurPopAccounts, CurCatchall);
           
  return 0;
}


/***********************************************************/

int sort_init ()
{
  sort_entry = memindex = memleft = 0;
  sort_list = malloc(SORT_TABLE_ENTRIES * sizeof(char *));
  if (!sort_list) { return -1; }
  return 0;
}
/* add entry to list for sorting, copies string until it gets to char 
   'end' */
int sort_add_entry (char *entry, char end)
{
  int len;

  if (sort_entry == SORT_TABLE_ENTRIES) {
    return -2;   /* table is full */
  }
  if (memleft < 256)
  {
    /* allocate a 64K block of memory to store table entries */
    memleft = 65536;
    sort_ptr = sort_block[memindex++] = malloc(memleft);
  }
  if (!sort_ptr) { return -1; }
  sort_list[sort_entry++] = sort_ptr;
  len = 1;   /* at least a terminator */
  while (*entry && *entry != end) {
    *sort_ptr++ = *entry++;
    len++;
  }
  *sort_ptr++ = 0;  /* NULL terminator */
  memleft -= len;
}
char *sort_get_entry(int index)
{
  if ((index < 0) || (index >= sort_entry)) { return NULL; }
  return sort_list[index];
}
void sort_cleanup()
{
  while (memindex) { free (sort_block[--memindex]); }
}
/* Comparison routine used in qsort for multiple functions */
static int sort_compare (const void *p1, const void *p2)
{
  return strcasecmp (*(char **)p1, *(char **)p2);
} 
void sort_dosort()
{
  qsort (sort_list, sort_entry, sizeof(char *), sort_compare);
}

void str_replace (char *s, char orig, char repl)
{
  while (*s) {
    if (*s == orig) { *s = repl; }
    s++;
  }
}

char qmail_icon(char *rv, char *png)    
{

  sprintf(rv, "<img src=\"%s/%s\" border=0>", IMAGEURL, png);

}

char qmail_button(char *rv, char *command, char *modu, char *png)    
{
 char Buffer[MAX_BUFF];
 char Image[MAX_BUFF];

  sprintf(Buffer, "<a href=\"%s/com/%s?user=%s&dom=%s&time=%d&modu=%s\">",
    CGIPATH, command, Username, Domain, Mytime, modu);
/*
  sprintf(rv, "<a href=\"%s/com/%s?user=%s&dom=%s&time=%d&modu=%s\"><img src=\"%s/%s\" border=0></a>", 
    CGIPATH, command, Username, Domain, Mytime, modu, IMAGEURL, png);
*/
  qmail_icon(Image, png);
  sprintf(rv, "%s%s</a>", Buffer, Image );

}

check_local_user( user )
 char *user;
{
 struct stat buf;
 int i,j;

  strcpy(TmpBuf, ".qmail-");
  for(i=0,j=7;user[i]!=0;++i,++j){
    if ( user[i] == '.' ) TmpBuf[j] = ':'; 
    else TmpBuf[j] = user[i];
  }
  TmpBuf[j] = 0;

  if ( stat(TmpBuf, &buf) == 0 ) return(-1);
  if ( vauth_getpw(user, Domain)) return(-1);

  return(0);
}

show_counts()
{
  fprintf(stderr, "show_counts\n" );

  sprintf(uBufA, "%d", CurPopAccounts);
  sprintf(uBufB, "%d", CurForwards);
  sprintf(uBufC, "%d", CurAutoResponders);
  sprintf(uBufD, "%d", CurMailingLists);
}

check_email_addr( addr )
 char *addr;
{
 char *taddr = addr;


  if(strlen(taddr)<0) return(1);
  for(taddr=addr;*taddr!=0;++taddr) {
    if(!isalnum(*taddr) && !ispunct(*taddr)) {
      return(1);
    }
  }

  /* force to lower */
  lowerit(addr);

  for(taddr=addr;*taddr!='@'&&*taddr!=0;++taddr) {
    if ( isspace(*taddr) ) return(1);
    if(ispunct(*taddr) && 
       *taddr!='.' && *taddr!='-' && *taddr!='+' && *taddr!='=' &&
       *taddr!='_') {
      return(1);
    }
  }

  /* if just a user name with no @domain.com then bad */
  if (*taddr==0) return(1);

  /* Look for a sub domain */
  for(;*taddr!='.'&&*taddr!=0;++taddr);

  if (*taddr==0) return(1);
  return(0);
}

fixup_local_name( addr )
 char *addr;
{
 char *taddr = addr;

  /* don't allow zero length user names */
  if(strlen(taddr)<=0) return(1);

  /* force it to lower case */
  lowerit(addr);

  /* check for valid email address */
  for(taddr=addr;*taddr!=0;++taddr) {

    if(!isalnum(*taddr) && !ispunct(*taddr)) return(1);
    if(isspace(*taddr)) return(1);

    if(ispunct(*taddr)&&*taddr!='-'&&*taddr!='.'&&*taddr!='_' &&
               *taddr!='+' && *taddr!='=') {
      if(*taddr!='.') return(1);
    }
  }

  /* if we made it here, everything is okay */
  return(0);
}

/*    Delete this because we are always going to send the footer.   */
ack(msg, c)
 char *msg;
 int c;
{
  fprintf(actout,"%s\n", msg);
  fprintf(actout,"[/BODY][/HTML]\n", msg);
  vclose();
  exit(0);
}


upperit( instr )
 char *instr;
{
  while(*instr!=0) {
    if ( islower(*instr) ) *instr = toupper(*instr);
    ++instr;
  }
}

char *safe_getenv(char *var)
{
 char *s;

  s = getenv(var);
  if ( s == NULL ) return("");
  return(s);
}

char *strstart(sstr, tstr)
 char *sstr;
 char *tstr;
{
 char *ret_str;

  ret_str = sstr;
  if ( sstr == NULL || tstr == NULL ) return(NULL);

  while ( *sstr != 0 && *tstr != 0 ) {
    if ( *sstr != *tstr ) return(NULL);
    ++sstr;
    ++tstr;
  }

  if ( *tstr == 0 ) return(ret_str);
  return(NULL);

}

int open_lang(char *lang)
{
  char langfile[200];
  static char *langpath = NULL;
  struct stat mystat;

  /* do not read lang files with path control characters */
  if ( strstr(lang,".")!=NULL || strstr(lang,"/")!=NULL ) return(-1);

  /* convert to lower case (using lowerit() from libvpopmail) */
  lowerit(lang);

  /* close previous language if still open */
  if (lang_fs != NULL) fclose (lang_fs);

  if (langpath == NULL) {
    langpath = getenv(QMAILADMIN_TEMPLATEDIR);
    if (langpath == NULL ) langpath = HTMLLIBDIR;
  }

  snprintf(langfile, sizeof(langfile), "%s/lang/%s", langpath, lang);

  /* do not open symbolic links */
  if (lstat(langfile, &mystat)==-1 || S_ISLNK(mystat.st_mode)) return(-1);

  if ( (lang_fs=fopen(langfile, "r"))==NULL) return(-1);

  return(0);
}

char *get_html_text( char *index )
{
 static char *tmpbuf;
 char *tmpstr;

  tmpbuf = malloc(400);

  if (lang_fs == NULL) return("");

  rewind(lang_fs);
  while(fgets(tmpbuf,400,lang_fs)!=NULL){
    tmpstr = strtok(tmpbuf, " ");
    if (strcmp(tmpstr, index) == 0 ) {
      tmpstr = strtok(NULL, "\n");
      return(tmpstr);
    }    
  }
  return("");
}

int open_colortable()
{
 static char tmpbuf[200];
 char *tmpstr;

  tmpstr = getenv(QMAILADMIN_TEMPLATEDIR);
  if (tmpstr == NULL ) tmpstr = HTMLLIBDIR;

  snprintf(tmpbuf, sizeof(tmpbuf), "%s/html/colortable", tmpstr);
  if ( (color_table=fopen(tmpbuf, "r"))==NULL) return(-1);
  return(0);
}

char *get_color_text( char *index )
{
 static char tmpbuf[400];
 char *tmpstr;

  if (color_table == NULL) return("");

  rewind(color_table);
  while(fgets(tmpbuf,sizeof(tmpbuf),color_table)!=NULL){
    tmpstr = strtok(tmpbuf, " ");
    if (strcmp(tmpstr, index) == 0 ) {
      tmpstr = strtok(NULL, "\n");
      return(tmpstr);
    }    
  }
  return("");
}
/* bk - use maildir++ quotas now
char *get_quota_used(char *dir) {
    char *tmpstr;
    static char tmpbuff[MAX_BUFF];
    double size;

    size = get_du(dir);
    if (size > 0) {
        size = size / 1048576; 
    }
    sprintf(tmpbuff, "%.2lf", size);
    tmpstr = tmpbuff;
    return tmpstr;
}
*/
/* quota_to_bytes: used to convert user entered quota (given in MB)
                   back to bytes for vpasswd file
   return value: 0 for success, 1 for failure
*/
int quota_to_bytes(char returnval[], char *quota) {
    char *tmpstr;
    double tmp;

    if (quota == NULL) { return 1; }
    if (tmp = atof(quota)) {
        tmp *= 1048576;
        sprintf(returnval, "%.0lf", tmp);
        return 0;
    } else {
	strcpy (returnval, "");
	return 1;
    }
}
/* quota_to_megabytes: used to convert vpasswd representation of quota
                       to number of megabytes.
   return value: 0 for success, 1 for failure
*/
int quota_to_megabytes(char *returnval, char *quota) {
    char *tmpstr;
    double tmp;
    int i;

    if (quota == NULL) { return 1; }
    i = strlen(quota);
    if ((quota[i-1] == 'M') || (quota[i-1] == 'm')) {
        tmp = atol(quota);  /* already in megabytes */
    } else if ((quota[i-1] == 'K') || (quota[i-1] == 'k')) {
	tmp = atol(quota) * 1024;  /* convert kilobytes to megabytes */
    } else if (tmp = atol(quota)) {
        tmp /= 1048576.0;
    } else {
	strcpy (returnval, "");
	return 1;
    }
    sprintf(returnval, "%.2lf", tmp);
    return 0;
}

/*
 * Brian Kolaci
 * updated function that doesn't require fts_*
 */
/* bk - use maildir++ quotas now
off_t get_du(const char *dir_name)
{
  DIR *dirp;
  struct dirent *dp;
  struct stat statbuf;
  off_t file_size = 0;
  char *tmpstr;

  if (dir_name == NULL)
    return 0;

  if (chdir(dir_name) == -1)
    return 0;

  if ((dirp = opendir(".")) == NULL)
    return 0;

  while ((dp=readdir(dirp)) != NULL) {
    if (!strcmp(dp->d_name, "..") || !strcmp(dp->d_name, "."))
      continue;
    if ((tmpstr=strstr(dp->d_name, ",S=")) != NULL) {
      file_size += atol(tmpstr+3);
    } else if (stat(dp->d_name,&statbuf)==0 && (statbuf.st_mode & S_IFDIR) ) {
      file_size += get_du(dp->d_name);
    }
  }
  closedir(dirp);
  if (dir_name != NULL && strcmp(dir_name, ".." ) && strcmp(dir_name, "." ))
    chdir("..");
  return(file_size);
}
*/
