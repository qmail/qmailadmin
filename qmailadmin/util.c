/* 
 * $Id: util.c,v 1.12 2004-02-20 06:15:00 tomcollins Exp $
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

#define SORT_TABLE_ENTRIES 100000

#define DEFAULT_LANG "en"
#define MAX_LANG_ENTRY 4000
#define MAX_LANG_FILESIZE 20000
char *lang_entry[MAX_LANG_ENTRY];
char *langpath = NULL;

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

  /*   Count the Pop Accounts   */
  pw = vauth_getall(Domain,1,0);
  while(pw!=NULL){
    ++CurPopAccounts;
    pw = vauth_getall(Domain,0,0);
  }

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

    if ( strncmp(".qmail-", mydirent->d_name, 7) != 0 ) {
      /*  It is not a .qmail file   */
      continue;
    }

    if (!lstat(mydirent->d_name, &mystat) && S_ISLNK(mystat.st_mode)) {
      /*  It is a symlink, probably a mailing list  */

      if ( strstr(mydirent->d_name, "-owner") != 0 &&
           strstr(mydirent->d_name, "-digest-owner") == 0 )  {
        /*  If we only count the -owner link, the count  */
        /*  should come out right.  */
        ++CurMailingLists;
      }
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
      CurBlackholes++;

    } else if ( strstr( Buffer, "autorespond") != 0 ) {
      CurAutoResponders++;

    } else if (*Buffer != '|' ) {
      CurForwards++;

    }

    fclose(fs);
  }

  closedir(mydir);

  /* Get the default catchall box name */
  if ((fs=fopen(".qmail-default","r")) == NULL) {
    /* report error opening .qmail-default and exit */
    fprintf( stderr, "show_user_lines can't open default file\n" );
    sprintf( CurCatchall, "Can't open .qmail-default file" );
    

  } else {
    fgets( Buffer, sizeof(Buffer), fs);
    fclose(fs);

    fprintf( stderr, ".qmail-default: %s\n", Buffer);

    if (strstr(Buffer, " bounce-no-mailbox\n") != NULL) {
      sprintf(CurCatchall,"%s", get_html_text(130));

    } else if (strstr(Buffer, " delete\n") != NULL) {
      sprintf(CurCatchall,"%s", get_html_text(236));

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
 char Buffer[MAX_BUFF];

  strcpy(Buffer, ".qmail-");
  for(i=0,j=7;user[i]!=0;++i,++j){
    if ( user[i] == '.' ) Buffer[j] = ':'; 
    else Buffer[j] = user[i];
  }
  Buffer[j] = 0;

  if ( stat(Buffer, &buf) == 0 ) return(-1);
  if ( vauth_getpw(user, Domain)) return(-1);

  return(0);
}

show_counts()
{
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

/* initialize language table with NULL entries */
void init_lang_table()
{
  int i;
  
  for (i = 0; i < MAX_LANG_ENTRY; i++)
  	lang_entry[i] = NULL;
}

/* This function works by reading an entire "language" file into memory and
 * parsing out the table entries.  A table entries are comprised of a numeric
 * index, followed by a space and then the text of the entry.  For example:
 *
 * 123 Main Menu
 *
 * "123" is the index for the string "Main Menu"
 *
 * Previously, index had to be zero padded and three characters long, followed
 * by a single space and then the text.  Index no longer requires the zero padding
 * and there can be any number of spaces and tabs between the index and its text.
 *
 * load_lang_table scans through the file, finding and decoding each line,
 * saving a pointer to the text of the entry and converting the trailing newline
 * to a NULL.  To display an entry in the table, it's a simple matter of
 * referencing lang_entry[index] which is a pointer to the NULL-terminated string.
 */
int load_lang_table(char *lang)
{
  /* load entries from translation "lang" into the lang_entry table */
  char langfile[200];
  char buf[512];
  char *lang_filedata;
  int i;
  char *p;
  FILE *f;
  
  snprintf(langfile, sizeof(langfile), "%s/lang/%s", langpath, lang);
  f = fopen (langfile, "r");
  if (f == NULL) return -1;
  
  lang_filedata = malloc(MAX_LANG_FILESIZE);
  if (lang_filedata == NULL) return -2;

  fread (lang_filedata, 1, MAX_LANG_FILESIZE, f);
  fclose (f);
  p = lang_filedata;
  while (*p != '\0') {
    /* skip leading zeros, otherwise atol does octal conversion */
    if (*p == '0') p++;  /* skip over leading zero */
    if (*p == '0') p++;  /* skip over second leading zero */

    /* convert index value */
    i = (int) atol (p);

    /* find delimiter and advance pointer to text of entry */
    while (*p != ' ' && *p != '\t' && *p != '\0') p++;
    while (*p == ' ' || *p == '\t') p++;

    if ((i > 0) && (i < MAX_LANG_ENTRY) && (lang_entry[i] == NULL))
      lang_entry[i] = p;

    /* find end of line and convert to NULL when found */
    while (*p != '\n' && *p != '\0') p++;
    if (*p != '\0') *p++ = '\0';  /* convert to NULL and point to next entry */
  }
  
  /* Don't free lang_filedata!  It's a static buffer with the language entries! */
  return 0;
}

int open_lang_file(char *lang)
{
  char langfile[200];
  FILE *lang_fs;
  
  /* do not read lang files with path control characters */
  if ( strstr(lang,".")!=NULL || strstr(lang,"/")!=NULL ) return(-1);

  /* convert to lower case (using lowerit() from libvpopmail) */
  lowerit(lang);

  if (langpath == NULL) {
    langpath = getenv(QMAILADMIN_TEMPLATEDIR);
    if (langpath == NULL ) langpath = HTMLLIBDIR;
  }

  snprintf(langfile, sizeof(langfile), "%s/lang/%s", langpath, lang);

  /* make sure language file exists and we can read it */
  if ( (lang_fs=fopen(langfile, "r"))==NULL) return(-1);

  fclose (lang_fs);  
  return(0);
}

int open_lang()
{
  char langfile[200];
  static char *langpath = NULL;
  struct stat mystat;

  int i,j;
  struct vqpasswd *pw;
  char *accept_lang;
  char *langptr, *qptr;
  int lang_err;
  float maxq, thisq;

  /* Parse HTTP_ACCEPT_LANGUAGE to find highest preferred language
   * that we have a translation for.  Example setting:
   * de-de, ja;q=0.25, en;q=0.50, de;q=0.75
   * The q= lines determine which is most preferred, defaults to 1.
   * Our routine starts with en at 0.0, and then would try de-de (1.00),
   * de (1.00), ja (0.25), en (0.50), and then de (0.75).
   */

  /* default to DEFAULT_LANG at 0.00 preference */
  maxq = 0.0;
  strcpy (Lang, DEFAULT_LANG);

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
        lang_err = open_lang_file(langptr);

        /* Remove this next section for strict interpretation of
         * HTTP_ACCEPT_LANGUAGE.  It will try language xx (with the
         * same q value) if xx-yy fails.
         */
        if ((lang_err == -1) && (langptr[2] == '-')) {
          langptr[2] = '\0';
          lang_err = open_lang_file(langptr);
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
  
  /* now load the language file entries */
  init_lang_table();
  load_lang_table(Lang);
  if (strcmp (Lang, DEFAULT_LANG) != 0) {
    /* if any entries are missing, fill with DEFAULT_LANG equivalents */
    load_lang_table(DEFAULT_LANG);
  }
}

char *get_html_text( int target )
{
  #ifdef DEBUG_GET_TEXT
  fprintf( stderr, "get_html_text target: %d\n", target);
  fflush( stderr );
  #endif

  if ((target >= 0) && (target < MAX_LANG_ENTRY) && (lang_entry[target] != NULL))
  	return lang_entry[target];
  else
  	return "";
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
