/* 
 * $Id: alias.c,v 1.4 2004-01-14 21:23:25 tomcollins Exp $
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
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include <vpopmail.h>
#include <vauth.h>
#include "config.h"
#include "qmailadmin.h"
#include "qmailadminx.h"

char* dotqmail_alias_command(char* command);
int bkscandir(const char *dirname,
              struct dirent ***namelist,
            int (*select)(struct dirent *),
            int (*compar)(const void *, const void *));

int show_aliases(void)
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage, "%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template("show_alias.html");
  return 0;
}

int qa_sort(const void * a, const void * b)
{
  return strcasecmp ((*(const struct dirent **) a)->d_name,
                     (*(const struct dirent **) b)->d_name);
}


show_dotqmail_lines(char *user, char *dom, time_t mytime, char *dir)
{
 int moreusers=0;
 DIR *mydir;
 struct dirent *mydirent;
 FILE *fs;
 char alias_user[MAX_BUFF];
 char alias_name[MAX_FILE_NAME];
 char *alias_domain;
 char *alias_name_from_command;
 int i,j,stop,k,startnumber;
 int m,n;
 struct dirent **namelist;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if (atoi(Pagenumber)==0) {
    *Pagenumber='1';
  }

  startnumber = MAXALIASESPERPAGE * (atoi(Pagenumber) - 1);
  k=0;

  if ( (mydir = opendir(".")) == NULL ) {
    fprintf(actout,"<tr><td colspan=\"4\">");
    fprintf(actout,"%s %d", get_html_text("143"), 1);
    fprintf(actout,"</td></tr>");
    return(0);
  }

  n = bkscandir(".", &namelist, 0, qa_sort);
  
  for (m=0; m<n; m++) {
    mydirent=namelist[m];
    /*
     *  don't read files that are really ezmlm-idx listowners,
     *  i.e. .qmail-user-owner
     *   
     */ 
    if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
      if ( strstr(mydirent->d_name, "-owner") != NULL ) continue; 
      if ( strstr(mydirent->d_name, "-default") != NULL ) continue; 

      if ( k < startnumber ) {
        k++; 
        continue;
      }
      if ( k >MAXALIASESPERPAGE + startnumber) {
        moreusers=1;
        break;
      }

      if ( (fs=fopen(mydirent->d_name,"r"))==NULL) {
        fprintf(actout,"<tr><td colspan=4>");
        fprintf(actout,"%s %s", get_html_text("144"), mydirent->d_name);
        fprintf(actout,"</td></tr>\n");
        continue;
      }
      for(i=7,j=0;j<MAX_FILE_NAME-1&&mydirent->d_name[i]!=0;++i,++j) {
        alias_name[j] = mydirent->d_name[i] == ':' ? '.' : mydirent->d_name[i];
      }
      alias_name[j] = 0;
      memset(TmpBuf2, 0, sizeof(TmpBuf2));
      fgets(TmpBuf2, sizeof(TmpBuf2), fs);
      alias_name_from_command = dotqmail_alias_command(TmpBuf2);

      /* Note that the current system fails for multi-line .qmail-user files
         where the first line starts with a '#' or is invalid.
         This is good for mailing lists (since dotqmail_alias_command bails
         on program delivery that contains ezmlm) but bad for people who
         may have complex .qmail-user files that start with a comment. */

      if ( alias_name_from_command != NULL || *TmpBuf2 == '#') {
        stop=0;

        fprintf(actout, "<tr>\n");
        qmail_button (alias_name, "deldotqmail", user, dom, mytime, "trash.png");
        if (*TmpBuf2 == '#')
          fprintf(actout, "<td> </td>");   /* don't allow modify on blackhole */
        else
          qmail_button (alias_name, "moddotqmail", user, dom, mytime, "modify.png");
        fprintf(actout, "<td align=left>%s</td>\n", alias_name);
        fprintf(actout, "<td align=left>");

        if (*TmpBuf2 == '#') {
          /* this is a blackhole account */
          fprintf (actout, "<I>%s</I>", get_html_text("303"));
          stop = 1;
        }
        while (!stop) {
          alias_name_from_command = dotqmail_alias_command(TmpBuf2);
                
          /* check to see if it is an invalid line , 
           * if so skip to next
           */
          if (alias_name_from_command == NULL ) {
            if (fgets(TmpBuf2, sizeof(TmpBuf2), fs)==NULL) { 
              stop=1;
            }
            continue;
          }
                    
          strcpy(alias_user, alias_name_from_command);
          alias_domain=alias_user;
          /* get the domain alone from alias_user */
          for(;*alias_domain != '\0' && *alias_domain != '@'
            && *alias_domain != ' '; alias_domain++);
          alias_domain++;
          if(strcmp(alias_domain, Domain)==0) {
             /* if a local user, exclude the domain */
             strcpy(TmpBuf3, alias_user);
             for(j=0; TmpBuf3[j]!=0 && TmpBuf3[j]!='@';j++);
             TmpBuf3[j]=0;
             if (check_local_user(TmpBuf3)) {
                strcpy(alias_user, TmpBuf3);
             } else {
                /* make it red so it jumps out -- this is no longer a valid forward */
                sprintf(alias_user, "<font color=\"red\">%s</font>", 
                        alias_name_from_command);
             }
          }
                
          if (fgets(TmpBuf2, sizeof(TmpBuf2), fs) == NULL) {
            stop=1;
            fprintf(actout, "%s ", alias_user);
          } else {
            fprintf(actout, "%s, ", alias_user);
          }
        }
        fprintf(actout, "</td>\n");
                
        fprintf(actout, "</tr>\n");
      }
      fclose(fs);
      k++;
    }
  }
  closedir(mydir);
  /* bk: fix memory leak */
  for (m=0; m<n; m++)
    free(namelist[m]);
  free(namelist);

  if (AdminType == DOMAIN_ADMIN) {
    fprintf(actout, "<tr><td align=\"right\" colspan=\"4\">");
    fprintf(actout, "[&nbsp;");
    if(atoi(Pagenumber) > 1 ) {
      fprintf(actout, "<a href=\"%s/com/showforwards?user=%s&dom=%s&time=%d&page=%d\">%s</a>",
        CGIPATH,user,dom,mytime,atoi(Pagenumber)-1?atoi(Pagenumber)-1:atoi(Pagenumber),get_html_text("135"));
      fprintf(actout, "&nbsp;|&nbsp;");
    }
    fprintf(actout, "<a href=\"%s/com/showforwards?user=%s&dom=%s&time=%d&page=%s\">%s</a>",
      CGIPATH,user,dom,mytime,Pagenumber,get_html_text("136"));
    fprintf(actout, "&nbsp;|&nbsp;");
    if (moreusers) {
      fprintf(actout, "<a href=\"%s/com/showforwards?user=%s&dom=%s&time=%d&page=%d\">%s</a>",
        CGIPATH,user,dom,mytime,atoi(Pagenumber)+1,get_html_text("137"));    
      fprintf(actout, "&nbsp;]");
    }
    fprintf(actout, "</td></tr>");                                    
  }
}

/* 
 * This Function shows the inside of a .qmail file, 
 * with the edit mode
 *
 */
int show_dotqmail_file(char *user) 
{
 FILE *fs;
 char alias_user[MAX_BUFF];
 char *alias_domain;
 char *alias_name_from_command;
 char *dot_file;
 int l,j;

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
    
  l = strlen(user);
  dot_file= strcat(strcpy(malloc(8 + l), ".qmail-"), user);

  for(j=8;dot_file[j]!=0;j++) {
    if (dot_file[j]=='.') {
      dot_file[j] = ':';
    }
  }

  if ( (fs=fopen(dot_file,"r"))==NULL) {
    sprintf(StatusMessage,"%s %s<br>\n", get_html_text("144"), dot_file);
    vclose();
    exit(0);
  }
                
  fprintf(actout, "<tr>");
  fprintf(actout, "<td align=\"center\" valign=\"top\"><b>%s</b></td>", user);
    
  memset(TmpBuf2, 0, sizeof(TmpBuf2));

  while (fgets( TmpBuf2, sizeof(TmpBuf2), fs) != NULL) {
    alias_name_from_command = dotqmail_alias_command(TmpBuf2);
                        
    /* check to see if it is an invalid line , if so skip to next*/
    if (alias_name_from_command == NULL ) continue;

    strcpy(alias_user, alias_name_from_command);
    /* get the domain alone from alias_user */
    alias_domain = alias_user;
    for(;*alias_domain != '\0' && *alias_domain != '@'
      && *alias_domain != ' '; alias_domain++);
    alias_domain++;
    if(strcmp(alias_domain, Domain)==0) {
       /* if a local user, exclude the domain */
       strcpy(TmpBuf3, alias_user);
       for(j=0; TmpBuf3[j]!=0 && TmpBuf3[j]!='@';j++);
       TmpBuf3[j]=0;
       if (check_local_user(TmpBuf3)) {
          strcpy(alias_user, TmpBuf3);
       } else {
          /* make it red so it jumps out -- this is no longer a valid forward */
          sprintf(alias_user, "<font color=\"red\">%s</font>", 
                  alias_name_from_command);
       }
    }
    fprintf(actout, "<td align=\"center\" valign=\"top\">%s</td>\n", alias_user);
    fprintf(actout, "<td align=\"center\" valign=\"top\">\n");
    fprintf(actout, "<form method=\"post\" name=\"moddotqmail\" action=\"%s/com/moddotqmailnow\">\n", CGIPATH);
    fprintf(actout, "<input type=\"hidden\" name=\"user\" value=\"%s\">\n",
      Username);
    fprintf(actout, "<input type=\"hidden\" name=\"dom\" value=\"%s\">\n",
      Domain);
    fprintf(actout, "<input type=\"hidden\" name=\"time\" value=\"%i\">\n",
      Mytime);
    fprintf(actout, "<input type=\"hidden\" name=\"modu\" value=\"%s\">\n",
      user);
    fprintf(actout, "<input type=\"hidden\" name=\"linedata\" value=\"%s\">\n",
      TmpBuf2);
    fprintf(actout, "<input type=\"hidden\" name=\"action\" value=\"delentry\">\n");
    fprintf(actout, "<input type=\"image\" border=\"0\" src=\"%s/delete.png\">\n",
      IMAGEURL);
    fprintf(actout, "</form>\n");


    fprintf(actout, "</td>\n");
    fprintf(actout, "</tr>\n");
    fprintf(actout, "<tr>\n");
    fprintf(actout, "<td align=\"left\">&nbsp;</td>\n");
  }
  /* finish up the last line (all empty) */
  fprintf(actout, "<td align=\"left\">&nbsp;</td>");
  fprintf(actout, "<td align=\"left\">&nbsp;</td>");
  fprintf(actout, "</tr>");
  fclose(fs);
}

int onevalidonly(char *user) {
 FILE *fs;
 char *alias_name_from_command;
 char *dot_file;
 int l,j;

  l = strlen(user);
  dot_file= strcat(strcpy(malloc(8 + l), ".qmail-"), user);
  for(j=8;dot_file[j]!=0;j++) {
    if (dot_file[j]=='.') {
      dot_file[j] = ':';
    }
  }

  if ( (fs=fopen(dot_file,"r"))==NULL) {
    sprintf(StatusMessage,"%s %s<br>\n", get_html_text("144"),
    dot_file);
    vclose();
    exit(0);
  }
            
  j=0;
  while( fgets( TmpBuf2, sizeof(TmpBuf2), fs) != NULL ) {
    alias_name_from_command = dotqmail_alias_command(TmpBuf2);
    /* check to see if it is an invalid line , if so skip to next */
    if (alias_name_from_command == NULL ) continue;
        
    j++;
  }
  fclose(fs);

  if (j <2 ) return (1);
  else return (0);

}



moddotqmail()
{
  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template("mod_dotqmail.html");
}

moddotqmailnow() 
{
 struct vqpasswd *pw;

  if ( strcmp(ActionUser,"default")==0) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  if (strcmp(Action,"delentry")==0) {
    if (onevalidonly(ActionUser) ) {
      sprintf(StatusMessage, "%s\n", get_html_text("149"));
      moddotqmail();
      vclose();
      exit(0);
    }
        
    if (dotqmail_del_line(ActionUser,LineData) ) {
      sprintf(StatusMessage, "%s %d\n", get_html_text("150"), 1);
      moddotqmail();
      vclose();
      exit(0);
    }
    sprintf(StatusMessage, "%s\n", get_html_text("151") );
    moddotqmail();
    vclose();
    exit(0);
    
  } else if (strcmp(Action,"add")==0) {
    if( adddotqmail_shared(ActionUser, Newu, 0)) {
      moddotqmail();
      vclose();
      exit(0);
    } else {
      sprintf(StatusMessage,"%s %s\n", get_html_text("152"), Newu);
      moddotqmail();
      vclose();
      exit(0);
    }
  } else {
    sprintf(StatusMessage, "%s\n", get_html_text("155"));
    vclose();
    exit(0);
  }
}

adddotqmail()
{
  count_forwards();
  load_limits();
  if ( MaxForwards != -1 && CurForwards >= MaxForwards ) {
    sprintf(StatusMessage, "%s %d\n", 
    get_html_text("157"), MaxForwards);
    show_menu();
    vclose();
    exit(0);
  }
  send_template( "add_forward.html" );
}


adddotqmailnow()
{
 struct vqpasswd *pw;

  if (AdminType!=DOMAIN_ADMIN && 
      !(AdminType==USER_ADMIN && strcmp(ActionUser, Username)==0)) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  count_forwards();
  load_limits();
  if ( MaxForwards != -1 && CurForwards >= MaxForwards ) {
    sprintf(StatusMessage, "%s %d\n", get_html_text("157"), MaxForwards);
    send_template( "add_forward.html" );
    vclose();
    exit(0);
  }



  if (adddotqmail_shared(Alias, ActionUser, -1)) {
    adddotqmail();
    vclose();
    exit(0);
  } else {

    sprintf(StatusMessage, "%s\n", get_html_text("152"));
    show_forwards(Username,Domain,Mytime,RealDir);
  }
}

int adddotqmail_shared(char *forwardname, char *dest, int create) {
   /* adds line to .qmail for forwardname to dest             */
   /* (if create is 0, this is modifying an existing forward) */
   /* returns -1 if error orccured, 0 if successful           */
   /* fills StatusMessage if error occurs                     */
   /* jeff.hedlund@matrixsi.com                               */

  if (strlen(forwardname)<=0) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("163"), forwardname);
    return(-1);
    
  /* make sure forwardname is valid */
  } else if (fixup_local_name(forwardname)) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("163"), forwardname);
    return(-1);
    
  /* check to see if we already have a user with this name (only for create) */
  } else if (create != 0 && check_local_user(forwardname)) {
    sprintf(StatusMessage, "%s %s\n", get_html_text("175"), forwardname);
    return(-1);
  }

  /* see if forwarding to a local user */
  if (strstr(dest, "@") == NULL) {
    if (check_local_user(dest) == 0) {
       sprintf(StatusMessage, "%s\n", get_html_text("161"));
       return(-1);
    } else {
       /* make it an email address */
       sprintf(dest, "%s@%s", dest, Domain);
    }
  }
  
  /* check that it's a valid email address */
  if (check_email_addr(dest)) {
     sprintf(StatusMessage, "%s %s\n", get_html_text("162"), dest);
     return(-1);
  }

  sprintf(TmpBuf2, "&%s\n", dest);
  if (dotqmail_add_line(forwardname, TmpBuf2)) {
     sprintf(StatusMessage, "%s %d\n", get_html_text("150"), 2);
     return(-1);
  }

  return(0);
}

deldotqmail()
{

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }
  send_template( "del_forward_confirm.html" );

}

deldotqmailnow()
{

  if (AdminType!=DOMAIN_ADMIN && 
       !(AdminType==USER_ADMIN && !strcmp(ActionUser, Username))) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    show_menu(Username, Domain, Mytime);
    vclose();
    exit(0);
  }


  /* check to see if we already have a user with this name */
  if (fixup_local_name(ActionUser)) {
    sprintf(StatusMessage,"%s %s\n", get_html_text("160"), Alias);
    deldotqmail();
    vclose();
    exit(0);
  }

  if (!(dotqmail_delete_files(ActionUser))) {
    sprintf(StatusMessage, "%s %s %s\n", get_html_text("167"), 
      Alias, ActionUser);
  } else {
    sprintf(StatusMessage, "%s %s %s\n", get_html_text("168"), 
      Alias, ActionUser);
  }

  /* don't display aliases/forwards if we just deleted the last one */
  count_forwards();
  if(CurForwards == 0) {
    show_menu(Username, Domain, Mytime);
  } else {
    show_forwards(Username,Domain,Mytime,RealDir);
  }
}

char* dotqmail_alias_command(char* line)
{
 int len;
 static char user[501];
 static char command[MAX_BUFF];
 char *s;
 char *b;

  if (line == NULL) return NULL;   /* null pointer */
  if (*line == 0) return NULL;     /* blank line */
  if (*line == '#') return NULL;   /* comment */

  /* copy everything up to the first whitespace */
  for (len = 0; line[len] != 0 && isspace(line[len]) == 0; len++ ) {
    command[len] = line[len];
  }
  command[len] = 0;

  /* If it ends with a slash and starts with a / or . then
   * this is a Maildir delivery, local alias
   */
  if( (command[len - 1]=='/') && ((command[0] =='/') || (command[0] =='.')) ) { 
    strcpy(user, command); user[len - 1] = 0;
    b = NULL;  /* pointer to mailbox name */

    if ((s = strrchr(user, '/')) == NULL) return NULL;
    if (strcmp(s, "/Maildir") != 0) {
      b = s + 2;  /* point to name of submailbox */
      *s = '\0';  /* add NULL */
      if ((s = strrchr(user, '/')) == NULL) return NULL;
      if (strcmp(s, "/Maildir") != 0) return NULL;
    }

    *s = '\0';
    if ((s = strrchr(user, '/')) == NULL) return NULL;
    if (b != NULL) { sprintf (user, "%s <I>(%s)</I>", s+1, b); }
    else { strcpy (user, s+1); }

    return (user);
        
  /* if it's an email address then display the forward */
  } else if ( !check_email_addr( (command+1) ) ){
    if (*command == '&' ) return (&command[1]);
    else return (command);

  /* if it is a program then */
  } else if ( command[0] == '|' ) {

    /* do not display ezmlm programs */
    if ( strstr(command, "ezmlm" ) != 0 ) return(NULL);

    /* do not display autorespond programs */
    if ( strstr(command, "autorespond" ) != 0 ) return(NULL);

    /* otherwise, display the program */

    /* back up to pipe or first slash to remove path */
    while (line[len] != '/' && line[len] != '|') len--;
    len++;   /* len is now first char of program name */
    sprintf (command, "<I>%s</I>", &line[len]);
    return(command);

  } else {

    /* otherwise just report nothing */
    return(NULL);
  }
}

/*
 * Brian Kolaci
 * quick implementation of the scandir() BSD function
 */
int bkscandir(const char *dirname,
              struct dirent ***namelist,
            int (*select)(struct dirent *),
            int (*compar)(const void *, const void *))
{
  int i;
  int entries;
  int esize;
  struct dirent* dp;
  struct dirent* dent;
  DIR * dirp;

  *namelist = NULL;
  entries = esize = 0;

  /* load the names */
  if ((dirp = opendir(dirname)) == NULL)
    return -1;

  while ((dp = readdir(dirp)) != NULL) {
    if (select == NULL || (*select)(dp)) {
      if (entries >= esize) {
        void* mem;
        esize += 10;
        if ((mem = realloc(*namelist, esize * sizeof(struct dirent*))) == NULL) {
          for (i = 0; i < entries; i++)
            free((*namelist)[i]);
          free(*namelist);
          closedir(dirp);
          return -1;
        }
        *namelist = (struct dirent**)mem;
      }
      if ((dent = (struct dirent*)malloc(sizeof(struct dirent)+MAX_FILE_NAME)) == NULL) {
        for (i = 0; i < entries; i++)
          free((*namelist)[i]);
        free(*namelist);
        closedir(dirp);
        return -1;
      }
      memcpy(dent, dp, sizeof(*dp)+MAX_FILE_NAME);
      (*namelist)[entries] = dent;
      entries++;
    }
  }
  closedir(dirp);

  /* sort them */
  if (compar)
    qsort((void*)*namelist, entries, sizeof(struct dirent*), compar);
  return entries;
}
