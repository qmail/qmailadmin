/* 
 * $Id: template.c,v 1.5 2003-12-24 05:22:05 tomcollins Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <vpopmail.h>
#include <vpopmail_config.h>
#include <vauth.h>
#include "config.h"
#include "qmailadmin.h"
#include "qmailadminx.h"

static char dchar[4];
void check_user_forward_vacation(char newchar);
void check_mailbox_flags(char newchar);
void transmit_block(FILE *fs);
void ignore_to_end_tag(FILE *fs);
void get_calling_host();
char *get_session_val(char *session_var);

static char NTmpBuf[500];

/*
 * send an html template to the browser 
 */
int send_template(char *actualfile)
{
  send_template_now("header.html");
  send_template_now(actualfile);
  send_template_now("footer.html");
  return 0;
}

int send_template_now(char *filename)
{
 FILE *fs;
 FILE *fs_qw;
 int i;
 int j;
 int inchar;
 int testint;
 char *tmpstr;
 struct stat mystat;
 char qconvert[11];
 char *qnote = " MB";
 struct vqpasswd *vpw;
 char value[MAX_BUFF];

  if (strstr(filename, "/")!= NULL||strstr(filename,"..")!=NULL) {
    printf("warning: invalid file name %s\n", filename );
    return(-1);
  }


  tmpstr = getenv(QMAILADMIN_TEMPLATEDIR);
  if (tmpstr == NULL ) tmpstr = HTMLLIBDIR; 
  snprintf(TmpBuf2, (sizeof(TmpBuf2) - 1), "%s/html/%s", tmpstr, filename);

  if (lstat(TmpBuf2, &mystat) == -1) {
    printf("Warning: cannot lstat '%s', check permissions.<BR>\n", TmpBuf2);
    return(-1);
  }
  if (S_ISLNK(mystat.st_mode)) {
    printf("Warning: '%s' is a symbolic link.<BR>\n", TmpBuf2);
    return(-1);
  }

  /* open the template */
  fs = fopen( TmpBuf2, "r" );
  if (fs == NULL) {
    fprintf(actout,"%s %s<br>\n", get_html_text("144"), TmpBuf2);
    return 0;
  }

  /* parse the template looking for "##" pattern */
  while ((inchar = fgetc(fs)) >= 0) {
    /* if not '#' then send it */
    if (inchar != '#') {
      fputc(inchar, actout);

    /* found a '#' */
    } else {
      /* look for a second '#' */
      inchar = fgetc(fs);
      if (inchar < 0) {
        break;

      /* found a tag */
      } else if (inchar == '#') {
        inchar = fgetc(fs);
        if (inchar < 0) break;

        /* switch on the tag */
        switch (inchar) {
          /* send the action user parameter */
          case 'A':
            fprintf(actout,"%s", ActionUser);
            break;

          /* send the Alias parameter */
          case 'a':
            fprintf(actout,"%s", Alias);
            break;

          /* show number of pop accounts */
          case 'B':
            load_limits();
            count_users();
            if(MaxPopAccounts > -1) {
              printf("%d/%d", CurPopAccounts, MaxPopAccounts);
            } else {
              printf("%d/%s", CurPopAccounts, get_html_text("229"));
            }
            break;

          /* show the lines inside a alias table */
          case 'b':
            show_dotqmail_lines(Username,Domain,Mytime,RealDir,"alias");
            break;

          /* send the CGIPATH parameter */
          case 'C':
            fprintf(actout,"%s", CGIPATH);
            break;

          /* show the lines inside a mailing list table */
          case 'c':
            show_mailing_list_line2(Username,Domain,Mytime,RealDir);
            break;

          /* send the domain parameter */
          case 'D':
            fprintf(actout,"%s", Domain);
            break;

          /* show the lines inside a forward table */
          case 'd':
            show_dotqmail_lines(Username,Domain,Mytime,RealDir,"forward");
            break;

          /* this will be used to parse mod_mailinglist-idx.html */
          case 'E':
            show_current_list_values();
            break;

          /* show the lines inside a mailing list table */
          case 'e':
            show_mailing_list_line(Username,Domain,Mytime,RealDir);
            break;

          /* display a file (used for mod_autorespond ONLY) */
          /* this code should be moved to a function in autorespond.c */
          case 'F':
            {
             FILE *fs;
              sprintf(TmpBuf, ".qmail-%s", ActionUser);
              for (i=6; TmpBuf[i] != 0; ++i) {
                if (TmpBuf[i] == '.') TmpBuf[i] = ':';
              }
              if ((fs=fopen(TmpBuf, "r")) == NULL) ack("123", 123);
              fgets(TmpBuf2, sizeof(TmpBuf2), fs);

              if (fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {

                /* See if it's a Maildir path rather than address */
                i = strlen(TmpBuf2) - 2;
                if (TmpBuf2[i] == '/') {
                  --i;
                  for(; TmpBuf2[i] != '/'; --i);
                  --i;
                  for(;TmpBuf2[i]!='/';--i);
                  for(++i, j=0; TmpBuf2[i] != '/'; ++j,++i) {
                    TmpBuf3[j] = TmpBuf2[i];
                  }
                  TmpBuf3[j] = '\0';
                  fprintf(actout, "value=\"%s@%s\"><td>\n", TmpBuf3, Domain);
                } else {
                  /* take off newline */
                  i = strlen(TmpBuf2); --i; TmpBuf2[i] = 0;
                  fprintf(actout, "value=\"%s\"><td>\n", &TmpBuf2[1]);
                }
              } 
              fclose(fs);
              upperit(ActionUser);
              sprintf(TmpBuf, "%s/message", ActionUser);

              if ((fs = fopen(TmpBuf, "r")) == NULL) ack("123", 123);

              fgets( TmpBuf2, sizeof(TmpBuf2), fs);
              fgets( TmpBuf2, sizeof(TmpBuf2), fs);
              fprintf(actout, "         <td>&nbsp;</td>\n");
              fprintf(actout, "         </tr>\n");
              fprintf(actout, "         <tr>\n");
              fprintf(actout, "         <td align=right><b>%s</b></td>\n", get_html_text("006"));

              /* take off newline */
              i = strlen(TmpBuf2); --i; TmpBuf2[i] = 0;
              fprintf(actout, "         <td><input type=\"text\" size=40 name=\"alias\" maxlength=128 value=\"%s\"></td>\n",
                &TmpBuf2[9]);
              fprintf(actout, "         <td>&nbsp;</td>\n");
              fprintf(actout, "        </tr>\n");
              fprintf(actout, "       </table>\n");
              fprintf(actout, "       <textarea cols=80 rows=40 name=\"message\">");
                fgets(TmpBuf2, sizeof(TmpBuf2), fs);
              while (fgets(TmpBuf2, sizeof(TmpBuf2), fs)) {
                fprintf(actout, "%s", TmpBuf2);
              }
              fprintf(actout, "</textarea>");
              fclose(fs);
            }
            break;

          /* show the forwards */
          case 'f':
            if (AdminType == DOMAIN_ADMIN) {
              show_forwards(Username,Domain,Mytime,RealDir);
            }
            break;

          /* show the mailing list digest subscribers */
          case 'G':
            if (AdminType == DOMAIN_ADMIN) {
              show_list_group_now(2);
            }
            break;

          /* show the lines inside a autorespond table */
          case 'g':
            show_autorespond_line(Username,Domain,Mytime,RealDir);
            break;

          /* show the counts */
          case 'h':
            show_counts();
            break;

          case 'I':
            show_dotqmail_file(ActionUser);
            break;

          /* check for user forward and forward/store vacation */
          case 'i':
            check_user_forward_vacation(fgetc(fs));
            break;

          /* show mailbox flag status */
          /* James Raftery <james@now.ie> 12 Dec. 2002 */
          case 'J':
            check_mailbox_flags(fgetc(fs));
            break;

          /* show number of mailing lists */
          /* Added by <mbowe@pipeline.com.au> 5th June 2003 */
          case 'j':
            load_limits();
            count_mailinglists();
            if(MaxMailingLists > -1) {
              printf("%d/%d", CurMailingLists, MaxMailingLists);
            } else {
              printf("%d/%s", CurMailingLists, get_html_text("229"));
            }
            break;

          /* show number of forwards */
          /* Added by <mbowe@pipeline.com.au> 5th June 2003 */
          case 'k':
            load_limits();
            count_forwards();
            if(MaxForwards > -1) {
              printf("%d/%d", CurForwards, MaxForwards);
            } else {
              printf("%d/%s", CurForwards, get_html_text("229"));
            }
            break;

          /* show number of autoresponders */
          /* Added by <mbowe@pipeline.com.au> 5th June 2003 */
          case 'K':
            load_limits();
            count_autoresponders();
            if(MaxAutoResponders > -1) {
              printf("%d/%d", CurAutoResponders, MaxAutoResponders);
            } else {
              printf("%d/%s", CurAutoResponders, get_html_text("229"));
            }
            break;

          /* show the aliases stuff */
          case 'l':
            if (AdminType == DOMAIN_ADMIN) {
              show_aliases(Username,Domain,Mytime,RealDir);
            }
            break;

          /* login username */
          case 'L':
            if(strlen(Username)>0) {
               printf("%s", Username);
            } else if(TmpCGI && GetValue(TmpCGI, value, "user=", sizeof(value))==0) {
               printf("%s", value);
            } else
               printf("postmaster");
            break;

          /* show the mailing list subscribers */
          case 'M':
            if (AdminType == DOMAIN_ADMIN) {
              show_list_group_now(0);
            }
            break;

          /* show the mailing lists */
          case 'm':
            if (AdminType == DOMAIN_ADMIN) {
              show_mailing_lists(Username,Domain,Mytime,RealDir);
            }
            break;

          /* parse include files */
          case 'N':
            i=0; 
            TmpBuf[i]=fgetc(fs);
            if (TmpBuf[i] == '/') {
              fprintf(actout, "%s", get_html_text("144"));
            } else {
              for(;TmpBuf[i] != '\0' && TmpBuf[i] != '#' && i < sizeof(TmpBuf)-1;) {
                TmpBuf[++i] = fgetc(fs);
              }
              TmpBuf[i] = '\0';
              if ((strstr(TmpBuf, "../")) != NULL) {
                fprintf(actout, "%s: %s", get_html_text("144"), TmpBuf);
              } else if((strcmp(TmpBuf, filename)) != 0) {
                send_template_now(TmpBuf);
              }
            }
            break;

          /* build a pulldown menu of all POP/IMAP users */
          case 'O':
            {
             struct vqpasswd *pw;

               pw = vauth_getall(Domain,1,1);
               while (pw != NULL) {
                 fprintf(actout, "<option value=\"%s\">%s</option>\n", 
                   pw->pw_name, pw->pw_name);
                 pw = vauth_getall(Domain,0,0);
               }
            }
            break;

          /* show the mailing list moderators */
          case 'o':
            if (AdminType == DOMAIN_ADMIN) {
              show_list_group_now(1);
            }
            break;

          /* display mailing list prefix */
          case 'P':
            {
              TmpBuf3[0] = '\0';
              get_mailinglist_prefix(TmpBuf3);
              fprintf(actout, "%s", TmpBuf3);
            }
            break;

          /* show POP/IMAP users */
          case 'p':
            show_user_lines(Username, Domain, Mytime, RealDir);
            break;

          /* display user's quota (mod user page) */
          case 'q':
            vpw = vauth_getpw(ActionUser, Domain);
            if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) { 
               quota_to_megabytes(qconvert, vpw->pw_shell); 
               printf("%s", qconvert);
            } else {
               if(AdminType == DOMAIN_ADMIN) 
                  printf("NOQUOTA");
               else
                  printf(get_html_text("229"));
            }
            break; 

          /* show the autoresponder stuff */
          case 'r':
            if (AdminType == DOMAIN_ADMIN) {
              show_autoresponders(Username,Domain,Mytime,RealDir);
            }
            break;

          /* send the status message parameter */
          case 'S':
            fprintf(actout,"%s", StatusMessage);
            break;

          /* show the catchall name */
          case 's':
            get_catchall();
            break;

          /* send the time parameter */
          case 'T':
            fprintf(actout,"%d", Mytime);
            break;

          /* transmit block? */
          case 't':
            transmit_block(fs);
            break;
            
          /* send the username parameter */
          case 'U':
            fprintf(actout,"%s", Username);
            break;

          /* show the users */
          case 'u':
            show_users(Username,Domain,Mytime,RealDir);
            break;

          /* show version number */
          case 'V':
            printf("<a href=\"http://sourceforge.net/projects/qmailadmin/\">%s</a> %s<BR>", 
              QA_PACKAGE, QA_VERSION);
            printf("<a href=\"http://www.inter7.com/vpopmail/\">%s</a> %s<BR>", 
              PACKAGE, VERSION);
            break;

          /* display the main menu */
          /* move this to a function... */
          case 'v':
            fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font><br>", 
              Domain);
            fprintf(actout, 
       "<font size=\"2\" color=\"#ff0000\"><b>%s</b></font><br>", 
              get_html_text("001"));
            if (AdminType==DOMAIN_ADMIN){

              if (MaxPopAccounts != 0) {
                fprintf(actout, 
       "<a href=\"%s/com/showusers?user=%s&time=%i&dom=%s&\">",
                  CGIPATH,Username,Mytime,Domain); 
                fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                  get_html_text("061"));
              }

              if (MaxForwards != 0 || MaxAliases != 0) {
                fprintf(actout, 
       "<a href=\"%s/com/showforwards?user=%s&time=%i&dom=%s&\">",
                  CGIPATH,Username,Mytime,Domain);
                fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                  get_html_text("122"));
              }

              if (MaxAutoResponders != 0) {
                fprintf(actout, 
       "<a href=\"%s/com/showautoresponders?user=%s&time=%i&dom=%s&\">",
                  CGIPATH,Username,Mytime,Domain);
                fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></a></font><br>",
                  get_html_text("077"));
              }

              if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
                fprintf(actout, 
       "<a href=\"%s/com/showmailinglists?user=%s&time=%i&dom=%s&\">",
                  CGIPATH, Username,Mytime,Domain);
                fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                  get_html_text("080"));
              }
            } else {
              /* the quota code in here is kinda screwy and could use review
               * then again, with recent changes, the non-admin shouldn't
               * even get to this page.
               */
              long diskquota = 0, maxmsg = 0;
	      char path[256];
              vpw = vauth_getpw(Username, Domain);

               fprintf(actout, 
       "<a href=\"%s/com/moduser?user=%s&time=%i&dom=%s&moduser=%s\">",
                 CGIPATH,Username,Mytime,Domain,Username);
               fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s %s</b></font></a><br><br>",
                 get_html_text("111"), Username);
              if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) {
                quota_to_megabytes(qconvert, vpw->pw_shell);
              } else {
                sprintf(qconvert, get_html_text("229")); qnote = "";
              }
              fprintf(actout, "<font size=\"2\" color=\"#000000\"><b>%s:</b><br>%s %s %s",
                get_html_text("249"), get_html_text("253"), qconvert, qnote);
              fprintf(actout, "<br>%s ", get_html_text("254"));
	      snprintf(path, sizeof(path), "%s/Maildir", vpw->pw_dir);
              readuserquota(path, &diskquota, &maxmsg);
              fprintf(actout, "%-2.2lf MB</font><br>", ((double)diskquota)/1048576.0);  /* Convert to MB */
             }

             if (AdminType == DOMAIN_ADMIN) {
               fprintf(actout, "<br>");
               fprintf(actout, 
       "<font size=\"2\" color=\"#ff0000\"><b>%s</b></font><br>",
                 get_html_text("124"));

               if (MaxPopAccounts != 0) {
                 fprintf(actout, 
       "<a href=\"%s/com/adduser?user=%s&time=%i&dom=%s&\">",
                   CGIPATH,Username,Mytime,Domain);
                 fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                   get_html_text("125"));
               }

               if (MaxForwards != 0) {
                 fprintf(actout, 
       "<a href=\"%s/com/adddotqmail?atype=forward&user=%s&time=%i&dom=%s&\">",
                   CGIPATH, Username,Mytime,Domain);
                 fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                   get_html_text("127"));
               }

               if (MaxAutoResponders != 0) {
                 fprintf(actout, 
       "<a href=\"%s/com/addautorespond?user=%s&time=%i&dom=%s&\">",
                   CGIPATH, Username,Mytime,Domain);
                 fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></a></font><br>",
                   get_html_text("128"));
               }

               if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
                 fprintf(actout, 
       "<a href=\"%s/com/addmailinglist?user=%s&time=%i&dom=%s&\">",
                   CGIPATH, Username,Mytime,Domain);
                 fprintf(actout, 
       "<font size=\"2\" color=\"#000000\"><b>%s</b></font></a><br>",
                   get_html_text("129"));
               }
             }
             break;

          /* dictionary line, we three more chars for the line */
          case 'X':
            for(i=0;i<3;++i) dchar[i] = fgetc(fs);
            dchar[i] = 0;
            printf("%s", get_html_text(dchar));
            break;

          /* exit / logout link/text */
          case 'x':
            printf("<a href=\"");
            strcpy (value, get_session_val("returntext="));
            if(strlen(value) > 0) {
               printf("%s\">%s", 
                      get_session_val("returnhttp="), value);
            } else {
               printf("%s/com/logout?user=%s&dom=%s&time=%d&\">%s",
                      CGIPATH, Username, Domain, Mytime, get_html_text("218"));
            }
            printf("</a>\n");
            break;

          /* returnhttp */
          case 'y':
            printf("%s", get_session_val("returnhttp=")); 
            break; 

          /* returntext */
          case 'Y':
            printf("%s", get_session_val("returntext=")); 
            break;
         
          /* send the image URL directory */
          case 'Z':
            fprintf(actout, "%s", IMAGEURL);
            break;            

          /* display domain on login page (last used, value of dom in URL,
           * or guess from hostname in URL).
           */
          case 'z':
            if( strlen(Domain) > 0 ) {
               printf("%s", Domain);
            } else if(TmpCGI && GetValue(TmpCGI, value, "dom=", sizeof(value))==0) {
               printf("%s", value);
#ifdef DOMAIN_AUTOFILL
            } else {
               get_calling_host();
#endif
            }
            break;

          default:
            break;
        }
        /*
         * didn't find a tag, so send out the first '#' and 
         * the current character
         * 
         */
      } else {
        fputc('#', actout);
        fputc(inchar, actout);
      }
    } 
  }
  fclose(fs);
  fflush(actout);
  vclose();

  return 0;
}

/* Display status of mailbox flags */
/* James Raftery <james@now.ie> 12 Dec. 2002 / 15 Apr. 2003 */
void check_mailbox_flags(char newchar)
{
 static struct vqpasswd *vpw = NULL;

  if (vpw==NULL) vpw = vauth_getpw(ActionUser, Domain); 

  switch (newchar) {

    /* 1: "checked" if V_USER0 is set */
    /* 2: "checked" if V_USER0 is unset */
    case '1':
      if (vpw->pw_gid & V_USER0)
        printf("checked");
      break;

    case '2':
      if (!(vpw->pw_gid & V_USER0))
        printf("checked");
      break;

    /* 3: "checked" if V_USER1 is set */
    /* 4: "checked" if V_USER1 is unset */
    case '3':
      if (vpw->pw_gid & V_USER1)
        printf("checked");
      break;

    case '4':
      if (!(vpw->pw_gid & V_USER1))
        printf("checked");
      break;

    /* 5: "checked" if V_USER2 is set */
    /* 6: "checked" if V_USER2 is unset */
    case '5':
      if (vpw->pw_gid & V_USER2)
        printf("checked");
      break;

    case '6':
      if (!(vpw->pw_gid & V_USER2))
        printf("checked");
      break;

    /* 7: "checked" if V_USER3 is set */
    /* 8: "checked" if V_USER3 is unset */
    case '7':
      if (vpw->pw_gid & V_USER3)
        printf("checked");
      break;

    case '8':
      if (!(vpw->pw_gid & V_USER3))
        printf("checked");
      break;

    default:
      break;
  }

  return;
}

void check_user_forward_vacation(char newchar)
{
 static struct vqpasswd *vpw = NULL;
 static FILE *fs1=NULL; /* for the .qmail file */
 static FILE *fs2=NULL; /* for the vacation message file */
 int i;

  if (vpw==NULL) vpw = vauth_getpw(ActionUser, Domain); 
  if (fs1== NULL) {
    snprintf(NTmpBuf, sizeof(NTmpBuf), "%s/.qmail", vpw->pw_dir);
    fs1 = fopen(NTmpBuf,"r");
  }

  if ( newchar=='7') {
    fprintf(actout, "%s", vpw->pw_gecos);
    return;
  }

  if (fs1 == NULL) {
    if (newchar=='0'){
      printf("checked ");
    }
    return;
  } 

  /* start at the begingin if second time thru */
  rewind(fs1);

  if (fgets(NTmpBuf,sizeof(NTmpBuf),fs1)!=NULL) {
                           
    /* if it is a forward to a program */
    if (strstr(NTmpBuf, "autorespond")!=NULL ) {
      if (newchar=='4') {
        printf("checked ");
      } else if (newchar=='5') {
        if (fs2 == NULL) { 
          snprintf(NTmpBuf, sizeof(NTmpBuf), "%s/vacation/message", vpw->pw_dir);
          fs2 = fopen(NTmpBuf,"r");
        }
        if (fs2 != NULL) {
          rewind(fs2);
          /*
           *  it's a hack, the second line always has
           *  the subject
           */ 
          fgets(NTmpBuf,sizeof(NTmpBuf),fs2);
          fgets(NTmpBuf,sizeof(NTmpBuf),fs2);
          printf("%s", &NTmpBuf[9]);
        }
      } else if (newchar=='6') {
        if (fs2 == NULL) { 
          snprintf(NTmpBuf, sizeof(NTmpBuf), "%s/vacation/message", vpw->pw_dir);
          fs2 = fopen(NTmpBuf,"r");
        }
        if (fs2 != NULL) {
          rewind(fs2);
          for(i = 0; i < 3 && fgets(NTmpBuf,sizeof(NTmpBuf),fs2) != NULL; ++i);
          while (fgets(NTmpBuf,sizeof(NTmpBuf),fs2) != NULL) {
            printf("%s", NTmpBuf);
          }
        }
      }

      i = 0;
      do {
        if (newchar == '3' && (NTmpBuf[0] == '/' ||
                               strstr(NTmpBuf, SPAM_COMMAND)!=NULL) ) {
          printf("checked ");
          return;
        }
        if ( newchar == '2' ) {
          if (NTmpBuf[0]=='/') continue;
          if (NTmpBuf[0]=='|') continue;
          if ( i>0 ) printf(", ");
          if (NTmpBuf[0]=='&') {
            printf("%s", strtok(&NTmpBuf[1], "\n"));
          } else {
            printf("%s", NTmpBuf[0]);
          } 
          ++i;
        }
        /* Jeff Hedlund (jeff.hedlund@matrixsi.com) 28 May 2003 */
        /* i9: "checked" if spam filtering on */
        if ( newchar == '9' && strstr(NTmpBuf, SPAM_COMMAND)!=NULL ) {
           printf("checked ");
           return;
        }
      } while(fgets(NTmpBuf,sizeof(NTmpBuf),fs1) != NULL );
                   
      
      return;
    } else {

      /* James Raftery <james@now.ie>, 21 May 2003 */
      /* i8: "checked" if blackhole */
      if (strstr(NTmpBuf, " delete\n") != NULL) {
        if (newchar == '8') {
           printf("checked ");
        }
        return;
      }

      if (newchar == '1' || newchar == '0') {
        if(strstr(NTmpBuf, SPAM_COMMAND)!=NULL) {
          if(newchar == '0') printf("checked ");
        } else {
          if(newchar == '1') printf("checked ");
        }
        return;
      }

      i = 0;
      do { 
        if (newchar == '3' && (NTmpBuf[0] == '/' ||
                               strstr(NTmpBuf, SPAM_COMMAND)!=NULL)) {
          printf("checked "); 
          return;
        }

        if (newchar == '9' && strstr(NTmpBuf, SPAM_COMMAND)!=NULL ) {
          printf("checked ");
          return;
        }
        
        if (newchar == '2') {
          if (NTmpBuf[0]=='/') continue;
          if (strstr(NTmpBuf, SPAM_COMMAND)!=NULL) continue;
          if (i > 0) printf(", ");
          if (NTmpBuf[0] == '&') {
            printf("%s", strtok(&NTmpBuf[1], "\n"));
          } else {
            printf("%s", NTmpBuf);
          } 
          ++i;
        }
      } while (fgets(NTmpBuf,sizeof(NTmpBuf),fs1) != NULL);
    }
  }
  return;
}

void transmit_block(FILE *fs) {
   /* tests to see if text between ##tX and ##tt should be transmitted */
   /* where X is a letter corresponding to one of the below values     */
   /* jeff.hedlund@matrixsi.com                                        */
   char inchar;
   
   inchar = fgetc(fs);
   switch(inchar) {
      case 'a':
         /* administrative commands */
         if(AdminType != DOMAIN_ADMIN) { ignore_to_end_tag(fs); }
         break;
      case 'h':
#ifndef HELP
         ignore_to_end_tag(fs);
#endif
         break;
      case 'm':
#ifndef ENABLE_MYSQL
         ignore_to_end_tag(fs);
#endif
         break;
      case 'q':
#ifndef MODIFY_QUOTA
         ignore_to_end_tag(fs);
#endif
         break;
      case 's':
#ifndef MODIFY_SPAM
         ignore_to_end_tag(fs);
#endif
         break;
      case 't':
         /* explicitly break if it was ##tt, that's an end tag */
         break;
      case 'u':
         /* user (not administrative) */
         if(AdminType != USER_ADMIN) { ignore_to_end_tag(fs); }
         break;
   }
}

void ignore_to_end_tag(FILE *fs) {
   /* simply looks for the ending tag (##tt) ignoring everything else */
   /* jeff.hedlund@matrixsi.com                                       */
   int nested = 0;
   int inchar;

   inchar = fgetc(fs);
   while(inchar >= 0) {
      if(inchar=='#') {
         inchar = fgetc(fs);
         if(inchar=='#') {
            inchar = fgetc(fs);
            if(inchar=='t') {
               inchar = fgetc(fs);
               if(inchar=='t' && nested==0)
                  return;
               else if(inchar!='t')
                  nested++;
               else
                  nested--;
            }
         }
      }
      inchar=fgetc(fs);
   }
}

void get_calling_host() {
   /* printout the virtual host that matches the HTTP_HOST */
   char *srvnam;
   char *domptr;
   int count = 0;
   int l;
   FILE *fs;
   char dombuf[255];

   sprintf(dombuf, "%s/control/virtualdomains", QMAILDIR);
   
   fs = fopen(dombuf, "r");
   if( fs != NULL ) {
      if ((srvnam = getenv("HTTP_HOST")) != NULL) {
         lowerit(srvnam);
         while( fgets(TmpBuf, sizeof(TmpBuf), fs) != NULL && count==0 ) {
            /* strip off newline */
            l = strlen(TmpBuf); l--; TmpBuf[l] = 0;
            /* virtualdomains format:  "domain:domain", get to second one */
            domptr = TmpBuf;
            for(; *domptr != 0 && *domptr !=':'; domptr++);
            domptr++;
            lowerit(domptr);
            if( strstr(srvnam, domptr) != NULL ) {
               printf("%s", domptr);
               count=1;
            }
         }
         fclose(fs);
      }
   }
}

char *get_session_val(char *session_var) {
   /* returns the value of session_var, first checking the .qw file for saved */
   /* value, or the TmpCGI if it's not yet been saved                         */
   char value[MAX_BUFF];
   char dir[MAX_BUFF];
   char *retval;
   FILE *fs_qw;
   struct vqpasswd *vpw;
   memset(value, 0, sizeof(value));
   memset(dir, 0, sizeof(dir));
   retval = "";
   if ( (vpw = vauth_getpw(Username, Domain)) != NULL ) {
      sprintf(dir, "%s/Maildir/%d.qw", vpw->pw_dir, Mytime);
      fs_qw = fopen(dir, "r");
      if ( fs_qw != NULL ) {
         memset(TmpBuf, 0, sizeof(TmpBuf));
         if ( fgets(TmpBuf, sizeof(TmpBuf), fs_qw) != NULL ) {
            if(GetValue(TmpBuf, value, session_var, sizeof(value))==0)
               retval=value;
         }
         fclose(fs_qw);
      }
   }
   else if (TmpCGI) {
      if (GetValue(TmpCGI, value, session_var, sizeof(value))==0)
         retval=value;
   }
   return(retval);
}
