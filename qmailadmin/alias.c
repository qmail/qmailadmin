/* 
 * $Id: alias.c,v 1.7 2004-01-30 08:30:58 rwidmer Exp $
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
 char Buffer1[MAX_BUFF];
 char Buffer2[MAX_BUFF];
 char Buffer3[MAX_BUFF];
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
    strcpy(uBufA, "4");
    sprintf(uBufB, "%s %d", get_html_text("143"), 1);
    send_template_now("show_error_line.html");
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
        strcpy(uBufA, "4");
        sprintf(uBufB, "SDQL %s %s", get_html_text("144"), mydirent->d_name);
        send_template_now("show_error_line.html");
        continue;
      }
      for(i=7,j=0;j<MAX_FILE_NAME-1&&mydirent->d_name[i]!=0;++i,++j) {
        alias_name[j] = mydirent->d_name[i] == ':' ? '.' : mydirent->d_name[i];
      }
      alias_name[j] = 0;
      memset(Buffer2, 0, sizeof(Buffer2));
      fgets(Buffer2, sizeof(Buffer2), fs);
      alias_name_from_command = dotqmail_alias_command(Buffer2);

      /* Note that the current system fails for multi-line .qmail-user files
         where the first line starts with a '#' or is invalid.
         This is good for mailing lists (since dotqmail_alias_command bails
         on program delivery that contains ezmlm) but bad for people who
         may have complex .qmail-user files that start with a comment. */

      if ( alias_name_from_command != NULL || *Buffer2 == '#') {
        stop=0;

        qmail_button(uBufA, "deldotqmail", alias_name, "trash.png");

        if (*Buffer2 == '#')
          strcpy( uBufB, "&nbsp;");
        else
          qmail_button(uBufB, "moddotqmail", alias_name, "modify.png");

        sprintf(uBufC, "%s", alias_name);

        if (*Buffer2 == '#') {
          /* this is a blackhole account */
          sprintf (Buffer1, "%s", get_html_text("303"));
          stop = 1;
        }
        while (!stop) {
          strcpy(Buffer1, "");

          alias_name_from_command = dotqmail_alias_command(Buffer2);
                
          /* check to see if it is an invalid line , 
           * if so skip to next
           */
          if (alias_name_from_command == NULL ) {
            if (fgets(Buffer2, sizeof(Buffer2), fs)==NULL) { 
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
             strcpy(Buffer3, alias_user);
             for(j=0; Buffer3[j]!=0 && Buffer3[j]!='@';j++);
             Buffer3[j]=0;
             if (check_local_user(Buffer3)) {
                strcpy(alias_user, Buffer3);
             } else {
                /* make it red so it jumps out -- this is no longer a valid forward */
                sprintf(alias_user, "<font color=\"red\">%s</font>", 
                        alias_name_from_command);
             }
          }
                
          if (fgets(Buffer2, sizeof(Buffer2), fs) == NULL) {
            stop=1;
            sprintf(Buffer1, "%s%s ", Buffer1, alias_user);
          } else {
            sprintf(Buffer1, "%s%s, ", Buffer1, alias_user);
          }

        }
        strcpy(uBufD, Buffer1);    
        strcpy(Buffer1, "");   
        send_template_now("show_forwards_line.html");
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
 char Buffer2[MAX_BUFF];
 char Buffer3[MAX_BUFF];
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
    sprintf(StatusMessage,"SDQF %s %s<br>\n", get_html_text("144"), dot_file);
    return(144);
  }
                
  memset(Buffer2, 0, sizeof(Buffer2));

  while (fgets( Buffer2, sizeof(Buffer2), fs) != NULL) {
    alias_name_from_command = dotqmail_alias_command(Buffer2);
                        
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
       strcpy(Buffer3, alias_user);
       for(j=0; Buffer3[j]!=0 && Buffer3[j]!='@';j++);
       Buffer3[j]=0;
       if (check_local_user(Buffer3)) {
          strcpy(alias_user, Buffer3);
       } else {
          /* make it red so it jumps out -- this is no longer a valid forward */
          sprintf(alias_user, "<font color=\"red\">%s</font>", 
                  alias_name_from_command);
       }
    }
  }
  fclose(fs);
}

int onevalidonly(char *user) {
 FILE *fs;
 char *alias_name_from_command;
 char *dot_file;
 char Buffer[MAX_BUFF];
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
  while( fgets( Buffer, sizeof(Buffer), fs) != NULL ) {
    alias_name_from_command = dotqmail_alias_command(Buffer);
    /* check to see if it is an invalid line , if so skip to next */
    if (alias_name_from_command == NULL ) continue;
        
    j++;
  }
  fclose(fs);

  if (j <2 ) return (1);
  else return (0);

}



void moddotqmail()
{

  if ( AdminType!=DOMAIN_ADMIN ) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

  send_template("mod_dotqmail.html");
}

void moddotqmailnow() 
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
 char Buffer[MAX_BUFF];
 struct vqpasswd *pw;

  if (AdminType!=DOMAIN_ADMIN && 
      !(AdminType==USER_ADMIN && strcmp(ActionUser, Username)==0)) {
    sprintf(StatusMessage,"%s", get_html_text("142"));
    vclose();
    exit(0);
  }

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
 char Buffer[MAX_BUFF];

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

  sprintf(Buffer, "&%s\n", dest);
  if (dotqmail_add_line(forwardname, Buffer)) {
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
