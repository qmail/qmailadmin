/* 
 * $Id: mailinglist.c,v 1.5 2003-12-04 15:22:33 tomcollins Exp $
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
 * 
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
#include <errno.h>

char dotqmail_name[MAX_FILE_NAME];
char replyto_addr[256];
int replyto;
int dotnum;
int checkopt[256];    /* used to display mailing list options */

#define REPLYTO_SENDER 1
#define REPLYTO_LIST 2
#define REPLYTO_ADDRESS 3

void set_options();
void default_options();

int show_mailing_lists(char *user, char *dom, time_t mytime)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  /* see if there's anything to display */
  count_mailinglists();
   if ( CurMailingLists == 0 ) {
    sprintf(StatusMessage,"%s", get_html_text("231"));
    show_menu();
    vclose();
    exit(0);
  }

   if ( MaxMailingLists == 0 ) {
    return(0);
  }
  send_template( "show_mailinglist.html" );
}

int show_mailing_list_line(char *user, char* dom, time_t mytime, char *dir)
{
  DIR *mydir;
  struct dirent *mydirent;
  FILE *fs;
  char *addr;
  char testfn[MAX_FILE_NAME];
  int i,j;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

   if ( MaxMailingLists == 0 ) {
    return(0);
  }

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout,"<tr><td>%s %d</tr><td>", get_html_text("143"), 1);
    return(0);
  }

  /* First display the title row */
  fprintf(actout, "<tr bgcolor=\"#cccccc\">");
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("072"));
#ifdef EZMLMIDX
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("071"));  
#endif
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("081"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("083"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("084"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("085"));
#ifdef EZMLMIDX
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("086"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("087"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("088"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("237"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("238"));
  fprintf(actout, "<th align=center><font size=2>%s</font></th>", get_html_text("239"));
#endif
  fprintf(actout, "</tr>\n");
 
  sort_init();

  /* Now, display each list */
  while( (mydirent=readdir(mydir)) != NULL ) {
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
#ifdef EZMLMIDX
        fprintf(actout, "<tr><td colspan=12>%s %s</td></tr>\n", get_html_text("144"), mydirent->d_name);
#else
        fprintf(actout, "<tr><td colspan=5>%s %s</td></tr>\n", get_html_text("144"), mydirent->d_name);
#endif
        continue;
      }
      fgets(TmpBuf2, sizeof(TmpBuf2), fs);
      fclose(fs);
      if ( strstr( TmpBuf2, "ezmlm-reject") != 0 ) {
        sort_add_entry (&mydirent->d_name[7], 0);
      }
    }
  }
  closedir(mydir);
  sort_dosort();

  for (i = 0; addr = sort_get_entry(i); ++i) {
    sprintf (testfn, ".qmail-%s-digest-owner", addr);
    /* convert ':' in addr to '.' */
    str_replace (addr, ':', '.');

    fprintf(actout,"<tr>");
    qmail_button(addr, "delmailinglist", user, dom, mytime, "trash.png");

#ifdef EZMLMIDX
    qmail_button(addr, "modmailinglist", user, dom, mytime, "modify.png");
#endif
    fprintf(actout,"<td align=left>%s</td>\n", addr); 

    qmail_button(addr, "addlistuser", user, dom, mytime, "delete.png");
    qmail_button(addr, "dellistuser", user, dom, mytime, "delete.png");
    qmail_button(addr, "showlistusers", user, dom, mytime, "delete.png");

#ifdef EZMLMIDX
    qmail_button(addr, "addlistmod", user, dom, mytime, "delete.png");
    qmail_button(addr, "dellistmod", user, dom, mytime, "delete.png");
    qmail_button(addr, "showlistmod", user, dom, mytime, "delete.png");

    /* Is it a digest list? */
    if ( (fs=fopen(testfn,"r"))==NULL) {
      /* not a digest list */
      fprintf (actout, "<TD COLSPAN=3> </TD>");
    } else {
      qmail_button(addr, "addlistdig", user, dom, mytime, "delete.png");
      qmail_button(addr, "dellistdig", user, dom, mytime, "delete.png");
      qmail_button(addr, "showlistdig", user, dom, mytime, "delete.png");
      fclose(fs);
    }
#endif
    fprintf(actout, "</tr>\n");
  }
  sort_cleanup();
}

int is_mailing_list(FILE *fs)
{
       while (!feof(fs)) {
               fgets( TmpBuf2, sizeof(TmpBuf2), fs);
               if ( strstr( TmpBuf2, "ezmlm-reject") != 0 ||
                    strstr( TmpBuf2, "ezmlm-send")   != 0 )
                       return -1;
       }
       return 0;
}

/* mailing list lines on the add user page */
int show_mailing_list_line2(char *user, char *dom, time_t mytime, char *dir)
{
 DIR *mydir;
 struct dirent *mydirent;
 FILE *fs;
 char *addr;
 int i,j;
 int listcount;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if (*EZMLMDIR == 'n' || MaxMailingLists == 0 ) {
    return(0);
  }

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout,"%s %d<BR>\n", get_html_text("143"), 1);
    return(0);
  }

  listcount = 0;
  sort_init();

  while( (mydirent=readdir(mydir)) != NULL ) {
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
        fprintf(actout,"%s %s<br>\n",
          get_html_text("144"), mydirent->d_name);
        continue;
      }
      fgets( TmpBuf2, sizeof(TmpBuf2), fs);
      fclose(fs);
      if ( strstr( TmpBuf2, "ezmlm-reject") != 0 ) {
        sort_add_entry (&mydirent->d_name[7], 0);
        listcount++;
      }
    }
  }
  closedir(mydir);

  /* if there aren't any lists, don't display anything */
  if (listcount == 0) {
    sort_cleanup();
    return 0;
  }

  fprintf(actout,"<hr><table width=100%% cellpadding=1 cellspacing=0 border=0");
  fprintf(actout," align=center bgcolor=\"#000000\"><tr><td>");
  fprintf(actout,"<table width=100%% cellpadding=0 cellspacing=0 border=0 bgcolor=\"#e6e6e6\">");
  fprintf(actout,"<tr><th bgcolor=\"#000000\" colspan=2>");
  fprintf(actout,"<font color=\"#ffffff\">%s</font></th>\n", 
    get_html_text("095"));

  sort_dosort();

  fprintf(actout, "<INPUT NAME=number_of_mailinglist TYPE=hidden VALUE=%d>\n", listcount);
  for (i = 0; i < listcount; ++i)
  {
    addr = sort_get_entry(i);
    str_replace (addr, ':', '.');
    fprintf(actout,"<TR><TD ALIGN=RIGHT><INPUT NAME=\"subscribe%d\" TYPE=checkbox VALUE=%s></TD>", i, addr);
    fprintf(actout,"<TD align=LEFT>%s@%s</TD></TR>", addr, Domain);
  }
  fprintf(actout,"</table></td></tr></table>\n");
  sort_cleanup();
}


int addmailinglist(void)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  count_mailinglists();
  load_limits();
  if ( MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists ) {
    fprintf(actout, "%s %d\n", get_html_text("184"), 
      MaxMailingLists);
    show_menu();
    vclose();
    exit(0);
  }
  
  /* set up default options for new list */
  default_options();

#ifdef EZMLMIDX
  send_template( "add_mailinglist-idx.html" );
#else
  send_template( "add_mailinglist-no-idx.html" );
#endif

}

int delmailinglist(void)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  send_template( "del_mailinglist_confirm.html" );
}

int delmailinglistnow(void)
{
 int pid;
 DIR *mydir;
 struct dirent *mydirent;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout,"%s %d<BR>\n", get_html_text("143"), 1);
    fprintf(actout,"</table>");
    return 0;
  }
 
  /* make dotqmail name */
  strcpy(dotqmail_name, ActionUser);
  for(dotnum=0;dotqmail_name[dotnum]!='\0';dotnum++) {
    if(dotqmail_name[dotnum]=='.') dotqmail_name[dotnum] = ':';
  }

  sprintf(TmpBuf2, ".qmail-%s", dotqmail_name);
  sprintf(TmpBuf3, ".qmail-%s-", dotqmail_name);
  while( (mydirent=readdir(mydir)) != NULL ) {

    /* delete the main .qmail-"list" file */
    if ( strcmp(TmpBuf2, mydirent->d_name) == 0 ) {
      if ( unlink(mydirent->d_name) != 0 ) {
        ack(get_html_text("185"), TmpBuf2);
      }

    /* delete secondary .qmail-"list"-* files */
    } else if ( strncmp(TmpBuf3, mydirent->d_name, strlen(TmpBuf3)) == 0 ) {
      if ( unlink(mydirent->d_name) != 0 ) {
        ack(get_html_text("185"), TmpBuf2);
      }
    }
  }
  closedir(mydir);


  sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
  vdelfiles(TmpBuf2);

    count_mailinglists();
  sprintf(StatusMessage, "%s %s\n", get_html_text("186"), ActionUser);
    if ( CurMailingLists == 0 ) {
        show_menu();
    } else {
    show_mailing_lists(Username, Domain, Mytime);
  }

}

/* sets the Reply-To header in header* files based on form fields
 * designed to be called by ezmlm_make() (after calling ezmlm-make)
 * Replaces the "Reply-To" line in <filename> with <newtext>.
 */
void ezmlm_setreplyto (char *filename, char *newtext)
{
  FILE *headerfile, *temp;
  char realfn[256];
  char tempfn[256];
  char buf[256];

  sprintf (realfn, "%s/%s/%s", RealDir, ActionUser, filename);
  sprintf (tempfn, "%s.tmp", realfn);

  headerfile = fopen(realfn, "r");
  if (!headerfile) return;
  temp = fopen(tempfn, "w");
  if (!temp) { fclose (headerfile); return; }

  /* copy contents to new file, except for Reply-To header */
  while (fgets (buf, sizeof(buf), headerfile) != NULL) {
    if (strncasecmp ("Reply-To", buf, 8) != 0) {
      fputs (buf, temp);
    }
  }

  fputs (newtext, temp);

  fclose (headerfile);
  fclose (temp);
  unlink (realfn);
  rename (tempfn, realfn);
}

ezmlm_make (int newlist)
{
  FILE * file;
  int pid;

#ifdef EZMLMIDX
  char list_owner[MAX_BUFF];
  char owneremail[MAX_BUFF+5];  
#endif
  char options[MAX_BUFF];
  char *arguments[MAX_BUFF];
  int argc;
  int i=0;
  char tmp[64];
  char *tmpstr;
  char loop_ch[64];
  int  loop;
  
  /* Initialize listopt to be a string of the characters A-Z, with each one
   * set to the correct case (e.g., A or a) to match the expected behavior
   * of not checking any checkboxes.  Leave other letters blank.
   * NOTE: Leave F out, since we handle it manually.
   */
  char listopt[] = "A  D   hIj L N pQRST      ";
  
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if ( fixup_local_name(ActionUser) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("188"), ActionUser);
    addmailinglist();
    vclose();
    exit(0);
  }

  /* update listopt based on user selections */
  for (loop = 0; loop < 20; loop++) {
    sprintf(tmp, "opt%d=", loop);
    GetValue(TmpCGI, loop_ch, tmp, sizeof(loop_ch));
    for (tmpstr = loop_ch; *tmpstr; tmpstr++)
    {
      if ((*tmpstr >= 'A') && (*tmpstr <= 'Z')) {
        listopt[*tmpstr-'A'] = *tmpstr;
      } else if ((*tmpstr >= 'a') && (*tmpstr <= 'z')) {
        listopt[*tmpstr-'a'] = *tmpstr;
      }
    }
  }
  
  /* don't allow option c, force option e if modifying existing list */
  listopt[2] = ' ';
  listopt[4] = newlist ? ' ' : 'e';
  
  argc=0;
  arguments[argc++] = "ezmlm-make";

#ifdef EZMLMIDX
  /* check the list owner entry */
  GetValue(TmpCGI, list_owner, "listowner=", sizeof(list_owner)); // Get the listowner
  if ( strlen(list_owner) > 0 ) {
    sprintf (owneremail, "-5%s", list_owner);
    arguments[argc++] = owneremail;
  }
#endif

  /* build the option string */
  tmpstr = options;
  arguments[argc++] = tmpstr;
  *tmpstr++ = '-';
#ifndef EZMLMIDX
  /* non idx list, only allows options A and P */
  *tmpstr++ = listopt[0];        /* a or A */
  *tmpstr++ = listopt['p'-'a'];  /* p or P */
  *tmpstr++ = 0;   /* add NULL terminator */
#else
  /* ignore options v-z, but test a-u */
  for (i = 0; i <= ('u'-'a'); i++)
  {
    if (listopt[i] != ' ') {
      *tmpstr++ = listopt[i];
    }
  }
  *tmpstr++ = 0;   /* add NULL terminator */

  /* check for sql support */
  GetValue(TmpCGI, tmp, "sqlsupport=", sizeof(tmp));
  if( strlen(tmp) > 0 ) {
    arguments[argc++] = tmpstr;
    tmpstr += sprintf (tmpstr, "%s", tmp) + 1;
    arguments[argc++] = tmpstr;
    for(loop = 1; loop <= NUM_SQL_OPTIONS; loop++) {  
      sprintf(tmp, "sql%d=", loop);
      GetValue(TmpCGI, loop_ch, tmp, sizeof(loop_ch));
      tmpstr += sprintf (tmpstr, "%s:", loop_ch);
    }
    /* remove trailing : */
    tmpstr--;
    *tmpstr++ = 0;
  }
#endif

  /* make dotqmail name */
  strcpy(dotqmail_name, ActionUser);
  for(dotnum=0;dotqmail_name[dotnum]!='\0';dotnum++) {
    if(dotqmail_name[dotnum]=='.') dotqmail_name[dotnum] = ':';
  }
  pid=fork();
  if (pid==0) {
    sprintf(TmpBuf1, "%s/ezmlm-make", EZMLMDIR);
    sprintf(TmpBuf2, "%s/%s", RealDir, ActionUser);
    sprintf(TmpBuf3, "%s/.qmail-%s", RealDir, dotqmail_name);

    arguments[argc++]=TmpBuf2;
    arguments[argc++]=TmpBuf3;
    arguments[argc++]=ActionUser;
    arguments[argc++]=Domain;
    arguments[argc]=NULL;

    execv(TmpBuf1, arguments);
    exit(127);
  } else {
    wait(&pid);
  }

  /* 
   * ezmlm-make -e leaves .qmail-listname-(accept||reject) links for some reason.
   * (causing file permission errors in "show mailing lists") Also, it doesn't 
   * delete dir/digest/ when turning off digests.  This section cleans up...
   */
  if(listopt['M'-'A'] == 'M') { /* moderation off */
    sprintf(tmp, "%s/.qmail-%s-accept-default", RealDir, dotqmail_name);
    unlink (tmp);
    sprintf(tmp, "%s/.qmail-%s-reject-default", RealDir, dotqmail_name);
    unlink (tmp);
  }
  if(listopt['D'-'A'] == 'D') { /* digest off */
    sprintf(tmp, "%s/.qmail-%s-digest-return-default", RealDir, dotqmail_name);
    unlink (tmp);
    sprintf(tmp, "%s/.qmail-%s-digest-owner", RealDir, dotqmail_name);
    unlink (tmp);

    /* delete the digest directory */
    sprintf(tmp, "%s/%s/digest", RealDir, ActionUser);
    vdelfiles(tmp);
    chdir(RealDir);
  }

  /* Check for prefix setting */
  GetValue(TmpCGI, tmp, "prefix=", sizeof(tmp));
  
  /* strip leading '[' and trailing ']' from tmp */
  tmpstr = strchr (tmp, ']');
  if (tmpstr != NULL) *tmpstr = '\0';
  tmpstr = tmp;
  while (*tmpstr == '[') tmpstr++;

  /* Create (or delete) the file as appropriate */
  sprintf(TmpBuf, "%s/%s/prefix", RealDir, ActionUser);
  if (strlen(tmp) > 0)
  {
    file=fopen(TmpBuf , "w");
    if (file)
    {
      fprintf(file, "[%s]", tmpstr);
      fclose(file);
    }
  }
  else
  {
    unlink (TmpBuf);
  }

  /* set Reply-To header */
  GetValue (TmpCGI, TmpBuf, "replyto=", sizeof(TmpBuf));
  replyto = atoi(TmpBuf);
  if (replyto == REPLYTO_SENDER) {
    /* ezmlm shouldn't remove/add Reply-To header */
    ezmlm_setreplyto ("headeradd", "");
    ezmlm_setreplyto ("headerremove", "");
  } else {
    if (replyto == REPLYTO_ADDRESS) {
      GetValue (TmpCGI, replyto_addr, "replyaddr=", sizeof(replyto_addr));
      sprintf (TmpBuf, "Reply-To: %s\n", replyto_addr);
    } else {  /* REPLYTO_LIST */
      strcpy (TmpBuf, "Reply-To: <#l#>@<#h#>\n");
    }
    ezmlm_setreplyto ("headeradd", TmpBuf);
    ezmlm_setreplyto ("headerremove", "Reply-To");
  }

  /* update inlocal file */
  sprintf(TmpBuf, "%s/%s/inlocal", RealDir, ActionUser);
  if (file=fopen(TmpBuf, "w")) {
    fprintf(file, "%s-%s", Domain, ActionUser);
    fclose(file);
  }
}

int addmailinglistnow(void)
{
  count_mailinglists();
  load_limits();
  if ( MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists ) {
    fprintf(actout, "%s %d\n", get_html_text("184"),
      MaxMailingLists);
    show_menu();
    vclose();
    exit(0);
  }

  if ( check_local_user(ActionUser) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("175"), ActionUser);
    addmailinglist();
    vclose();
    exit(0);
  }

  ezmlm_make(1);

  sprintf(StatusMessage, "%s %s@%s\n", get_html_text("187"),
          ActionUser, Domain);
  show_mailing_lists(Username, Domain, Mytime);
}

int show_list_group_now(int mod)
{
  /* mod = 0 for subscribers, 1 for moderators, 2 for digest users */
  
 FILE *fs;
 int i,handles[2],pid,a,x,y,z = 0,z1=1,subuser_count = 0; 
 char buf[256];
 char *addr;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  lowerit(ActionUser);
  pipe(handles);

  pid=fork();
  if (pid==0) {
    close(handles[0]);
    dup2(handles[1],fileno(stdout));
    sprintf(TmpBuf1, "%s/ezmlm-list", EZMLMDIR);
    if(mod == 1) {
        sprintf(TmpBuf2, "%s/%s/mod", RealDir, ActionUser);
    } else if(mod == 2) {
        sprintf(TmpBuf2, "%s/%s/digest", RealDir, ActionUser);
    } else {
        sprintf(TmpBuf2, "%s/%s/", RealDir, ActionUser);
    }
    execl(TmpBuf1, "ezmlm-list", TmpBuf2, NULL);
    exit(127);
  } else {
    close(handles[1]);
    fs = fdopen(handles[0],"r");

    /* Load subscriber/moderator list */

    sort_init();
    while( (fgets(buf, sizeof(buf), fs)!= NULL)) {
      sort_add_entry (buf, '\n');   /* don't copy newline */
      subuser_count++;
    }

    sort_dosort();

    /* Display subscriber/moderator/digest list, along with delete button */
    if(mod == 1) {
        strcpy(TmpBuf, "228"); strcpy(TmpBuf1, "220");
        /* strcpy(TmpBuf2, "087"); */
    } else if(mod == 2) {
        strcpy(TmpBuf, "244"); strcpy(TmpBuf1, "246");
        /* strcpy(TmpBuf2, "245"); */
    } else {
        strcpy(TmpBuf, "230"); strcpy(TmpBuf1, "222");
        /* strcpy(TmpBuf2, "084"); */
    }
    strcpy(TmpBuf2, "072");
    fprintf(actout,"<TABLE border=0 width=\"100%%\">\n");
    fprintf(actout," <TR>\n");
    fprintf(actout,"  <TH align=left COLSPAN=4><B>%s</B> %d<BR><BR></TH>\n", get_html_text(TmpBuf), subuser_count);
    fprintf(actout," </TR>\n");
    fprintf(actout," <TR align=center bgcolor=%s>\n", get_color_text("002"));
    fprintf(actout,"  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf2));
    fprintf(actout,"  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf1));
    fprintf(actout,"  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf2));
    fprintf(actout,"  <TH align=center><b><font size=2>%s</font></b></TH>\n", get_html_text(TmpBuf1));
    fprintf(actout," </TR>\n");

    if(mod == 1) {
        strcpy(TmpBuf, "dellistmodnow");
    } else if(mod == 2) {
        strcpy(TmpBuf, "dellistdignow");
    } else {
        strcpy(TmpBuf, "dellistusernow");
    }
    for(z = 0; addr = sort_get_entry(z); ++z) {
      fprintf(actout," <TR align=center>");
      fprintf(actout,"  <TD align=right><A href=\"%s/com/%s?modu=%s&newu=%s&dom=%s&user=%s&time=%d\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
        CGIPATH, TmpBuf, ActionUser, addr, Domain, Username, Mytime, IMAGEURL);
      fprintf(actout,"  <TD align=left>%s</TD>\n", addr);
      ++z;
      if(addr = sort_get_entry(z)) {
        fprintf(actout,"  <TD align=right><A href=\"%s/com/%s?modu=%s&newu=%s&dom=%s&user=%s&time=%d\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
          CGIPATH, TmpBuf, ActionUser, addr, Domain, Username, Mytime, IMAGEURL);
      fprintf(actout,"  <TD align=left>%s</TD>\n", addr);
      } else {
        fprintf(actout,"  <TD COLSPAN=2> </TD>");
      }
      fprintf(actout," </TR>");
    }

    sort_cleanup();

    fprintf(actout,"</TABLE>");
    fclose(fs); close(handles[0]);
    wait(&pid);
    sprintf(StatusMessage, "%s\n", get_html_text("190"));
    fprintf(actout, get_html_text("END_LIST_NAMES"));

  }
}

/*
int show_list_users_now(void) { return show_list_group_now(0); }
int show_list_moderators_now(void) { return show_list_group_now(1); }
int show_list_digest_users_now(void) { return show_list_group_now(2); }
*/

int show_list_group(char *template)
{
  if (AdminType != DOMAIN_ADMIN) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  
  if (MaxMailingLists == 0) {
    return 0;
  }
  
  send_template(template);
}

/*
int show_list_users(void) { return show_list_group("show_subscribers.html"); }
int show_list_digest_users(void) { return show_list_group("show_digest_subscribers.html"); }
int show_list_moderators(void) { return show_list_group("show_moderators.html"); }
*/

addlistgroup (char *template)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template(template);
}

addlistuser() { addlistgroup( "add_listuser.html" ); }
addlistmod() { addlistgroup( "add_listmod.html" ); }
addlistdig() { addlistgroup( "add_listdig.html" ); }

addlistgroupnow (int mod)
{
  // mod = 0 for subscribers, 1 for moderators, 2 for digest subscribers

 int i, result;
 int pid;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  lowerit(ActionUser);

  if ( check_email_addr(Newu) ) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("148"), Newu);
    if (mod == 1) {
      addlistmod();
    } else if (mod == 2) {
      addlistdig();
    } else {
      addlistuser();
    }
    vclose();
    exit(0);
  }

  pid=fork();
  if (pid==0) {
    sprintf(TmpBuf1, "%s/ezmlm-sub", EZMLMDIR);
    if(mod == 1) {
        sprintf(TmpBuf2, "%s/%s/mod", RealDir, ActionUser);
    } else if(mod == 2) {
        sprintf(TmpBuf2, "%s/%s/digest", RealDir, ActionUser);
    } else {
        sprintf(TmpBuf2, "%s/%s/", RealDir, ActionUser);
    }
    execl(TmpBuf1, "ezmlm-sub", TmpBuf2, Newu, NULL);
    exit(127);
  } else wait(&pid);

  if(mod == 1 ) {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, 
        get_html_text("194"), ActionUser, Domain);
    send_template( "add_listmod.html" );
  } else if(mod == 2) {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, 
        get_html_text("240"), ActionUser, Domain);
    send_template( "add_listdig.html" );
  } else {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, 
        get_html_text("193"), ActionUser, Domain);
    send_template( "add_listuser.html" );
  }
  vclose();
  exit(0);
}

/*
addlistusernow() { addlistgroupnow(0); }
addlistmodnow() { addlistgroupnow(1); }
addlistdignow() { addlistgroupnow(2); }
*/

dellistgroup(char *template)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  send_template(template);
}

dellistuser() { dellistgroup ( "del_listuser.html" ); }
dellistmod() { dellistgroup ( "del_listmod.html" ); }
dellistdig() { dellistgroup ( "del_listdig.html" ); }

dellistgroupnow(int mod)
{
 int i;
 int pid;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  lowerit(Newu);

  pid=fork();
  if (pid==0) {
    sprintf(TmpBuf1, "%s/ezmlm-unsub", EZMLMDIR);
    if(mod == 1) {
        sprintf(TmpBuf2, "%s/%s/mod", RealDir, ActionUser);
    } else if(mod == 2 ) {
        sprintf(TmpBuf2, "%s/%s/digest", RealDir, ActionUser);
    } else {
        sprintf(TmpBuf2, "%s/%s/", RealDir, ActionUser);
    }
    execl(TmpBuf1, "ezmlm-unsub", TmpBuf2, Newu, NULL);
    exit(127);
  } else wait(&pid);

  if(mod == 1) {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, get_html_text("197"),
        ActionUser, Domain);
  } else if(mod == 2) {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, get_html_text("242"),
        ActionUser, Domain);
  } else {
    sprintf(StatusMessage, "%s %s %s@%s\n", Newu, get_html_text("203"),
        ActionUser, Domain);
  }
  show_mailing_lists(Username, Domain, Mytime);
  vclose();
  exit(0);
}

/*
dellistusernow() { dellistgroupnow(0); }
dellistmodnow() { dellistgroupnow(1); }
dellistdignow() { dellistgroupnow(2); }
*/

count_mailinglists()
{
 DIR *mydir;
 struct dirent *mydirent;
 FILE *fs;

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout,"%s %d<BR>\n", get_html_text("143"), 1);
    fprintf(actout,"</table>");
    return(0);
  }
 

  CurMailingLists = 0;
  while( (mydirent=readdir(mydir)) != NULL ) {
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
        fprintf(actout, get_html_text("144"), 
          mydirent->d_name);
        continue;
      }
      fgets( TmpBuf2, sizeof(TmpBuf2), fs);
      if ( strstr( TmpBuf2, "ezmlm-reject") != 0 ) {
        ++CurMailingLists;
      }
      fclose(fs);
    }
  }
  closedir(mydir);

}

modmailinglist()
{
  /* name of list to modify is stored in ActionUser */
 int i;
 FILE *fs;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  strcpy (Alias, "");  /* initialize Alias (list owner) to empty string */

  /* get the current listowner and copy it to Alias */
  strcpy (dotqmail_name, ActionUser);
  str_replace (dotqmail_name, '.', ':');
  sprintf(TmpBuf, ".qmail-%s-owner", dotqmail_name);
  if((fs=fopen(TmpBuf, "r"))!=NULL) {
    while(fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
      if(strstr(TmpBuf2, "@")!=NULL) {
        /* strip leading & if present */
        sprintf(Alias, "%s", (*TmpBuf2 == '&' ? (TmpBuf2 + 1) : TmpBuf2) );
        i = strlen(Alias); --i; Alias[i] = '\0'; /* strip newline */
      }
    }
    fclose(fs);
  }

  /* set default to "replies go to original sender" */
  replyto = REPLYTO_SENDER;  /* default */
  *replyto_addr = '\0';
  sprintf(TmpBuf, "%s/headeradd", ActionUser);
  /* get the Reply-To setting for the list */
  if ((fs = fopen (TmpBuf, "r")) != NULL) {
    while (fgets (TmpBuf2, sizeof(TmpBuf2), fs)) {
      if (strncasecmp ("Reply-To: ", TmpBuf2, 10) == 0) {
        i = strlen(TmpBuf2); --i; TmpBuf2[i] = '\0'; /* strip newline */
        if (strcmp ("<#l#>@<#h#>", TmpBuf2 + 10) == 0) {
          replyto = REPLYTO_LIST;
        } else {
          replyto = REPLYTO_ADDRESS;
          strcpy (replyto_addr, TmpBuf2 + 10);
        }
      }
    }
    fclose(fs);
  }

  /* read in options for the current list */
  set_options();

#ifdef EZMLMIDX
  send_template( "mod_mailinglist-idx.html" );
#else
  send_template( "show_mailinglists.html" );
#endif

}

modmailinglistnow()
{
  ezmlm_make(0);
  
  sprintf(StatusMessage, "%s %s@%s\n", get_html_text("226"),
    ActionUser, Domain);
  show_mailing_lists(Username, Domain, Mytime);
}

build_list_value(char *param, char *color, char *opt1, char *desc1, char *opt2, char *desc2, int checked)
{
  fprintf(actout, "<tr bgcolor=%s>\n", get_color_text(color));
  fprintf(actout, "  <td>\n");
  fprintf(actout, "    <input type=radio name=%s value=%s%s></td>\n", param, opt1, checked ? "" : " CHECKED");
  fprintf(actout, "  <td>%s</td>\n", get_html_text(desc1));
  fprintf(actout, "  <td>\n");
  fprintf(actout, "    <input type=radio name=%s value=%s%s></td>\n", param, opt2, checked ? " CHECKED" : "");
  fprintf(actout, "  <td>%s</td>\n", get_html_text(desc2));
  fprintf(actout, "</tr>\n");
}

build_option_str (char *type, char *param, char *options, char *description)
{
  int selected;
  char *optptr;
  
  selected = 1;
  for (optptr = options; *optptr; optptr++)
  {
    selected = selected && checkopt[*optptr];
  }
  /* selected is now true if all options for this radio button are true */

  fprintf(actout, "<INPUT TYPE=%s NAME=\"%s\" VALUE=\"%s\"%s> %s\n", 
    type, param, options, selected ? " CHECKED" : "", description);
}

int file_exists (char *filename)
{
  FILE *fs;
  if( (fs=fopen(filename, "r")) !=NULL ) {
    fclose(fs);
    return 1;
  } else {
    return 0;
  }
}

int get_ezmlmidx_line_arguments(char *line, char *program, char argument)
{
  char *begin; 
  char *end;
  char *arg;

  // does line contain program name?
  if ((strstr(line, program)) != NULL) {
    // find the options
    begin=strchr(line, ' ');
    begin++;
    if (*begin == '-') {
      end=strchr(begin, ' ');
      arg=strchr(begin, argument);
      // if arg is found && it's in the options (before the trailing space), return 1
      if (arg && (arg < end)) return 1;
    }       
  }       
  return 0;
}

void set_options() {
 char c;
 FILE *fs;

  /*
   * Note that with ezmlm-idx it might be possible to replace most
   * of this code by reading the config file in the list's directory.
   */

  /* make dotqmail name (ActionUser with '.' replaced by ':') */
  strcpy(dotqmail_name, ActionUser);
  for(dotnum=0;dotqmail_name[dotnum]!='\0';dotnum++) {
    if(dotqmail_name[dotnum]=='.') dotqmail_name[dotnum] = ':';
  }

  // default to false for lowercase letters
  for (c = 'a'; c <= 'z'; checkopt[c++] = 0);

  // figure out some options in the -default file
  sprintf(TmpBuf, ".qmail-%s-default", dotqmail_name);
  if( (fs=fopen(TmpBuf, "r")) !=NULL ) {
    while(fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-get", 'P')) > 0) {
        checkopt['b'] = 1;
      }
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-get", 's')) > 0) {
        checkopt['g'] = 1;
      }
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'S')) > 0) {
        checkopt['h'] = 1;
      }
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'U')) > 0) {
        checkopt['j'] = 1;
      }
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'l')) > 0) {
        checkopt['l'] = 1;
      }
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-manage", 'e')) > 0) {
        checkopt['n'] = 1;
      }
      if((strstr(TmpBuf2, "ezmlm-request")) != 0) {
        checkopt['q'] = 1;
      }
    }
    fclose(fs);
  }

  // figure out some options in the -accept-default file
  sprintf(TmpBuf, ".qmail-%s-accept-default", dotqmail_name);
  if( (fs=fopen(TmpBuf, "r")) !=NULL ) {
    while(fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
      if(strstr(TmpBuf2, "ezmlm-archive") !=0) {
        checkopt['i'] = 1;
      }
    }
    fclose(fs);
  }

  // figure out some options in the qmail file
  sprintf(TmpBuf, ".qmail-%s", dotqmail_name);
  if( (fs=fopen(TmpBuf, "r")) !=NULL ) {
    while(fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
      if((get_ezmlmidx_line_arguments(TmpBuf2, "ezmlm-store", 'P')) > 0) {
        checkopt['o'] = 1;
      }
      if((strstr(TmpBuf2, "ezmlm-gate")) != 0 || (strstr(TmpBuf2, "ezmlm-issubn")) != 0) {
        checkopt['u'] = 1;
      }
      if(strstr(TmpBuf2, "ezmlm-archive") !=0) {
        checkopt['i'] = 1;
      }
    }
    fclose(fs);
  }

  sprintf(TmpBuf, ".qmail-%s-accept-default", dotqmail_name);
  checkopt['m'] = file_exists(TmpBuf);

  sprintf(TmpBuf, "%s/archived", ActionUser);
  checkopt['a'] = file_exists(TmpBuf);
  
  sprintf(TmpBuf, "%s/digest/bouncer", ActionUser);
  checkopt['d'] = file_exists(TmpBuf);
  
  sprintf(TmpBuf, "%s/prefix", ActionUser);
  checkopt['f'] = file_exists(TmpBuf);

  sprintf(TmpBuf, "%s/public", ActionUser);
  checkopt['p'] = file_exists(TmpBuf);
  
  sprintf(TmpBuf, "%s/remote", ActionUser);
  checkopt['r'] = file_exists(TmpBuf);
  
  sprintf(TmpBuf, "%s/modsub", ActionUser);
  checkopt['s'] = file_exists(TmpBuf);
  
  sprintf(TmpBuf, "%s/text/trailer", ActionUser);
  checkopt['t'] = file_exists(TmpBuf);
  
  /* update the uppercase option letters (just the opposite of the lowercase) */
  for (c = 'A'; c <= 'Z'; c++)
  {
    checkopt[c] = !checkopt[(c - 'A' + 'a')];
  }
}

void default_options() {
  char c;
  
  *dotqmail_name = '\0';
  replyto = REPLYTO_SENDER;
  *replyto_addr = '\0';

  /* These are currently set to defaults for a good, generic list.
   * Basically, make it safe/friendly and don't turn anything extra on.
   */
   
  /* for the options below, use 1 for "on" or "yes" */
  checkopt['a'] = 1; /* Archive */
  checkopt['b'] = 1; /* Moderator-only access to archive */
  checkopt['c'] = 0; /* ignored */
  checkopt['d'] = 0; /* Digest */
  checkopt['e'] = 0; /* ignored */
  checkopt['f'] = 1; /* Prefix */
  checkopt['g'] = 1; /* Guard Archive */
  checkopt['h'] = 0; /* Subscribe doesn't require conf */
  checkopt['i'] = 0; /* Indexed */
  checkopt['j'] = 0; /* Unsubscribe doesn't require conf */
  checkopt['k'] = 0; /* Create a blocked sender list */
  checkopt['l'] = 0; /* Remote admins can access subscriber list */
  checkopt['m'] = 0; /* Moderated */
  checkopt['n'] = 0; /* Remote admins can edit text files */
  checkopt['o'] = 0; /* Others rejected (for Moderated lists only */
  checkopt['p'] = 1; /* Public */
  checkopt['q'] = 1; /* Service listname-request */
  checkopt['r'] = 0; /* Remote Administration */
  checkopt['s'] = 0; /* Subscriptions are moderated */
  checkopt['t'] = 0; /* Add Trailer to outgoing messages */
  checkopt['u'] = 1; /* Only subscribers can post */
  checkopt['v'] = 0; /* ignored */
  checkopt['w'] = 0; /* special ezmlm-warn handling (ignored) */
  checkopt['x'] = 0; /* enable some extras (ignored) */
  checkopt['y'] = 0; /* ignored */
  checkopt['z'] = 0; /* ignored */

  /* update the uppercase option letters (just the opposite of the lowercase) */
  for (c = 'A'; c <= 'Z'; c++)
  {
    checkopt[c] = !checkopt[(c - 'A' + 'a')];
  }
}

show_current_list_values() {
 FILE *fs;
 int sqlfileok = 0;
 int usesql = 0;
 int i,j;
 char checked1[MAX_BUFF] = "";
 char listname[128];
 int checked;
   
  /* Note that we do not support the following list options:
   *   k - posts from addresses in listname/deny are rejected
   *   x - strip annoying MIME parts (spreadsheets, rtf, html, etc.)
   *   0 - make the list a sublist of another list
   *   3 - replace the From: header with another address
   *   4 - digest options (limits related to sending digest out)
   *   7, 8, 9 - break moderators up into message, subscription and admin
   */
  
  /* IMPORTANT: If you change the behavior of the checkboxes, you need
   * to update the default settings in modmailinglistnow so qmailadmin
   * will use the proper settings when a checkbox isn't checked.
   */
    
  if (*dotqmail_name) { /* modifying an existing list */
    strcpy (listname, dotqmail_name);
    str_replace (listname, ':', '.');
  } else {
    sprintf (listname, "<I>%s</I>", get_html_text("261"));
  }

  /* Posting Messages */
  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("262"));
  build_option_str ("RADIO", "opt1", "MU", get_html_text("263"));
  fprintf(actout, "<BR>\n");
  build_option_str ("RADIO", "opt1", "Mu", get_html_text("264"));
  fprintf(actout, "<BR>\n");
  build_option_str ("RADIO", "opt1", "mu", get_html_text("265"));
  fprintf(actout, "<BR>\n");
  build_option_str ("RADIO", "opt1", "mUo", get_html_text("266"));
  fprintf(actout, "<BR>\n");
  build_option_str ("RADIO", "opt1", "mUO", get_html_text("267"));
  fprintf(actout, "</P>\n");

  /* List Options */
  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("268"));
  /* this next option isn't necessary since we use the edit box to
   * set/delete the prefix
  sprintf (TmpBuf, get_html_text("269"), listname);
  build_option_str ("CHECKBOX", "opt3", "f", TmpBuf);
  fprintf(actout, "<BR>\n");
  */
  fprintf(actout, "<TABLE><TR><TD ROWSPAN=3 VALIGN=TOP>%s</TD>",
    get_html_text("310"));
  fprintf(actout, "<TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s</TD></TR>\n",
    REPLYTO_SENDER, (replyto == REPLYTO_SENDER) ? " CHECKED" : "", get_html_text("311"));
  fprintf(actout, "<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s</TD></TR>\n",
    REPLYTO_LIST, (replyto == REPLYTO_LIST) ? " CHECKED" : "", get_html_text("312"));
  fprintf(actout, "<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"%d\"%s>%s ",
    REPLYTO_ADDRESS, (replyto == REPLYTO_ADDRESS) ? " CHECKED" : "", get_html_text("313"));
  fprintf(actout, "<INPUT TYPE=TEXT NAME=\"replyaddr\" VALUE=\"%s\" SIZE=30></TD></TR>\n",
    replyto_addr);
  fprintf(actout, "</TABLE><BR>\n");
  build_option_str ("CHECKBOX", "opt4", "t", get_html_text("270"));
  fprintf(actout, "<BR>\n");
  build_option_str ("CHECKBOX", "opt5", "d", get_html_text("271"));
  sprintf (TmpBuf, get_html_text("272"), listname);
  fprintf(actout, "<SMALL>(%s)</SMALL>", TmpBuf);
  fprintf(actout, "<BR>\n");
  sprintf (TmpBuf, get_html_text("273"), listname);
  build_option_str ("CHECKBOX", "opt6", "q", TmpBuf);
  fprintf(actout, "<BR>\n");
  sprintf (TmpBuf, get_html_text("274"), listname, listname, listname);
  fprintf(actout, "&nbsp; &nbsp; <SMALL>(%s)</SMALL></P>", TmpBuf);

  /* Remote Administration */
  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("275"));
  build_option_str ("CHECKBOX", "opt7", "r", get_html_text("276"));
  fprintf(actout, "<BR>\n");
  build_option_str ("CHECKBOX", "opt8", "P", get_html_text("277"));
  fprintf(actout, "<SMALL>(%s)</SMALL><BR>", get_html_text("278"));
  fprintf(actout, "<TABLE><TR><TD ROWSPAN=2 VALIGN=TOP>%s</TD>",
    get_html_text("279"));
  fprintf(actout, "<TD>");
  build_option_str ("CHECKBOX", "opt9", "l", get_html_text("280"));
  fprintf(actout, "</TD>\n</TR><TR>\n<TD>");
  build_option_str ("CHECKBOX", "opt10", "n", get_html_text("281"));
  fprintf(actout, "<SMALL>(%s)</SMALL>.</TD>\n", get_html_text("282"));
  fprintf(actout, "</TR></TABLE>\n</P>\n");

  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("283"));
  fprintf(actout, "%s<BR>\n&nbsp; &nbsp; ", get_html_text("284"));
  build_option_str ("CHECKBOX", "opt11", "H", get_html_text("285"));
  fprintf(actout, "<BR>\n&nbsp; &nbsp; ");
  build_option_str ("CHECKBOX", "opt12", "s", get_html_text("286"));
  fprintf(actout, "<BR>\n%s<BR>\n&nbsp; &nbsp; ", get_html_text("287"));
  build_option_str ("CHECKBOX", "opt13", "J", get_html_text("285"));
  fprintf(actout, "<BR>\n");
  fprintf(actout, "<SMALL>%s</SMALL>\n</P>\n", get_html_text("288"));

  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("289"));
  build_option_str ("CHECKBOX", "opt14", "a", get_html_text("290"));
  fprintf(actout, "<BR>\n");
  /* note that if user doesn't have ezmlm-cgi installed, it might be
     a good idea to default to having option i off. */
  build_option_str ("CHECKBOX", "opt15", "i", get_html_text("291"));
  fprintf(actout, "<BR>\n%s\n<SELECT NAME=\"opt15\">", get_html_text("292"));
  fprintf(actout, "<OPTION VALUE=\"BG\"%s>%s\n",
  	checkopt['B'] && checkopt['G'] ? " SELECTED" : "", get_html_text("293"));
  fprintf(actout, "<OPTION VALUE=\"Bg\"%s>%s\n",
  	checkopt['B'] && checkopt['g'] ? " SELECTED" : "", get_html_text("294"));
  fprintf(actout, "<OPTION VALUE=\"b\"%s>%s\n",
  	checkopt['b'] ? " SELECTED" : "", get_html_text("295"));
  fprintf(actout, "</SELECT>.</P>\n");

  /***********************/
  /* begin MySQL options */
  /***********************/

  /* See if sql is turned on */
  checked = 0;
  sprintf(TmpBuf, "%s/sql", ActionUser);
  if( (fs=fopen(TmpBuf, "r")) !=NULL ) {
    checked = 1;
    while(fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
      strcpy(TmpBuf1, TmpBuf2);
      i = strlen(TmpBuf1); --i; TmpBuf1[i] = 0; /* take off newline */
      if((strstr(TmpBuf1, ":")) != NULL) { 
        sqlfileok = 1;
      }
    }
    usesql = 1;
    fclose(fs);
  }
#ifdef ENABLE_MYSQL
  fprintf(actout, "<P><B><U>%s</U></B><BR>\n", get_html_text("099"));
  fprintf(actout, "<input type=checkbox name=\"sqlsupport\" value=\"-6\"%s> %s",
    checked ? " CHECKED" : "", get_html_text("053"));

  /* parse dir/sql file for SQL settings */
  fprintf(actout, "    <table cellpadding=0 cellspacing=2 border=0>\n");
#else
  if (checked)
    fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sqlsupport VALUE=\"-6\">\n");
#endif

  /* get hostname */
  strcpy(checked1, "localhost");
  if(usesql == 1 && sqlfileok == 1) {
    strncpy(TmpBuf3, TmpBuf1, 1);       
    if((strstr(TmpBuf3, ":")) == NULL) { 
      for(i=0,j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0'; 
    }       
  }       

#ifdef ENABLE_MYSQL
  fprintf(actout, "      <tr>\n");
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("054"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text name=sql1 value=\"%s\"></td>\n", checked1);
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql1 VALUE=\"%s\">\n", checked1);
#endif

  /* get port */
  strcpy(checked1, "3306");
  if(usesql == 1 && sqlfileok == 1) {
    strncpy(TmpBuf3, &TmpBuf1[++i], 1);       
    if((strstr(TmpBuf3, ":")) == NULL) { 
      for(j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0'; 
    }       
  }       
#ifdef ENABLE_MYSQL
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("055"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text size=7 name=sql2 value=\"%s\"></td>\n", checked1);
  fprintf(actout, "      </tr>\n");
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql2 VALUE=\"%s\">\n", checked1);
#endif

  /* get user */
  strcpy(checked1, "");
  if(usesql == 1 && sqlfileok == 1) {
    strncpy(TmpBuf3, &TmpBuf1[++i], 1);       
    if((strstr(TmpBuf3, ":")) == NULL) { 
      for(j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0'; 
    }       
  }       
#ifdef ENABLE_MYSQL
  fprintf(actout, "      <tr>\n");
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("056"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text name=sql3 value=\"%s\"></td>\n", checked1);
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql3 VALUE=\"%s\">\n", checked1);
#endif

  /* get password */
  strcpy(checked1, "");
  if(usesql == 1 && sqlfileok == 1) {
    strncpy(TmpBuf3, &TmpBuf1[++i], 1);
    if((strstr(TmpBuf3, ":")) == NULL) {
      for(j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0';
    }
  }
#ifdef ENABLE_MYSQL
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("057"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text name=sql4 value=\"%s\"></td>\n", checked1);
  fprintf(actout, "      </tr>\n");
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql4 VALUE=\"%s\">\n", checked1);
#endif

  /* get database name */
  strcpy(checked1, "");
  if(usesql == 1 && sqlfileok == 1) {
    strncpy(TmpBuf3, &TmpBuf1[++i], 1);       
    if((strstr(TmpBuf3, ":")) == NULL) { 
      for(j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0'; 
    }       
  }       
#ifdef ENABLE_MYSQL
  fprintf(actout, "      <tr>\n");
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("058"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text name=sql5 value=\"%s\"></td>\n", checked1);
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql5 VALUE=\"%s\">\n", checked1);
#endif

  /* get table name */
  strcpy(checked1, "ezmlm");
  if(usesql == 1 && sqlfileok == 1) {
    ++i;
    if(strlen(TmpBuf1) != i) {
      for(j=0;TmpBuf1[i]!=':'&&TmpBuf1[i]!='\0';++j,++i) checked1[j] = TmpBuf1[i];
      checked1[j] = '\0'; 
    }       
  }       
#ifdef ENABLE_MYSQL
  fprintf(actout, "        <td ALIGN=RIGHT>%s:\n", get_html_text("059"));
  fprintf(actout, "          </td><td>\n");
  fprintf(actout, "          <input type=text name=\"sql6\" value=\"%s\"></td>\n", checked1);
  fprintf(actout, "      </tr>\n");
  fprintf(actout, "    </table>\n");
#else
  fprintf(actout, "<INPUT TYPE=HIDDEN NAME=sql6 VALUE=\"%s\">\n", checked1);
#endif

}

int get_mailinglist_prefix(char* prefix)
{
  char buffer[MAX_BUFF];
  char *b, *p;
  FILE* file;

  sprintf(buffer, "%s/%s/prefix", RealDir, ActionUser);
  file=fopen(buffer , "r");

  if (file)
  {
    fgets(buffer, sizeof(buffer), file);
    fclose(file);

    b = buffer;
    p = prefix;
    while (*b == '[') b++;
    while ((*b != ']') && (*b != '\n') && (*b != '\0')) *p++ = *b++;
    *p++ = '\0';
  }
  else
  {
    return 1;
  }
  return 0;
}

