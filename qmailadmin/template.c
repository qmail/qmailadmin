/* 
 * $Id: template.c,v 1.11 2004-02-07 09:22:36 rwidmer Exp $
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

//#define debug

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
/* undef some macros that get redefined in config.h below */
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
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

/*
 * send an html template to the browser 
 */
int send_template(char *filename)
{
 FILE *fs;
 FILE *fs_qw;
 int i;
 int j;
 int inchar;
 int testint;
 int target;
 char *tmpstr;
 struct stat mystat;
 char qconvert[11];
 char *qnote = " MB";
 struct vqpasswd *vpw;
 char value[MAX_BUFF];
 char Buffer[MAX_BUFF];
 char fqfn[MAX_BUFF];

  if (strstr(filename, "/")!= NULL||strstr(filename,"..")!=NULL) {
    fprintf(stderr, "Qmailadmin: invalid template name %s\n", filename );
    return(-1);
  }

  #ifdef debug
  fprintf(stderr, "Send Template %s\n", filename);
  #endif

  /*  Try to get the template path from ENVIRONMENT  */
  tmpstr = getenv(QMAILADMIN_TEMPLATEDIR);

  /*  If not use the compiled in value  */
  if (tmpstr == NULL ) tmpstr = HTMLLIBDIR; 

  /*  Build the fully qualified file name  */
  snprintf(fqfn, (sizeof(fqfn) - 1), "%s/html/%s", tmpstr, filename);

  #ifdef debug
  fprintf(stderr, "Final template filename: %s\n", fqfn);
  #endif

  /*  Does file exist  */
  if (lstat(fqfn, &mystat) == -1) {
    fprintf(stderr, "Qmailadmin: Can't stat '%s', check permissions.\n", fqfn);
    return(-2);
  }

  /*  Don't allow symlinks in template directories  */
  if (S_ISLNK(mystat.st_mode)) {
    fprintf(stderr, "Qmailadmin: symlink not allowed '%s'\n", fqfn);
    return(-3);
  }

  /* open the template */
  fs = fopen( fqfn, "r" );
  if (fs == NULL) {
    fprintf(stderr,"STN1 %s %s\n", get_html_text(144), fqfn);
    return 144;
  }

  /* parse the template looking for "##" pattern */
  while ((inchar = fgetc(fs)) >= 0) {

    /* if not '#' then send it */
    if (inchar != '#') {
      fputc(inchar, actout);

    } else {
      /* found a '#' - look for a second '#' */
      inchar = fgetc(fs);
      if (inchar < 0) {
        break;

      } else if (inchar == '#') {
        /* found a tag */
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
            if(MaxPopAccounts > -1) {
              printf("%d/%d", CurPopAccounts, MaxPopAccounts);
            } else {
              printf("%d/%s", CurPopAccounts, get_html_text(229));
            }
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

          /* show the mailing list digest subscribers */
          case 'G':
            show_list_group_now(2);
            break;

          /* show the lines inside a autorespond table */
          case 'g':
            show_autorespond_line(Username,Domain,Mytime,RealDir);
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
            if(MaxMailingLists > -1) {
              printf("%d/%d", CurMailingLists, MaxMailingLists);
            } else {
              printf("%d/%s", CurMailingLists, get_html_text(229));
            }
            break;

          /* show number of forwards */
          /* Added by <mbowe@pipeline.com.au> 5th June 2003 */
          case 'k':
            if(MaxForwards > -1) {
              printf("%d/%d", CurForwards, MaxForwards);
            } else {
              printf("%d/%s", CurForwards, get_html_text(229));
            }
            break;

          /* show number of autoresponders */
          /* Added by <mbowe@pipeline.com.au> 5th June 2003 */
          case 'K':
            if(MaxAutoResponders > -1) {
              printf("%d/%d", CurAutoResponders, MaxAutoResponders);
            } else {
              printf("%d/%s", CurAutoResponders, get_html_text(229));
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
            show_list_group_now(0);
            break;

          /* parse include files */
          case 'N':
            i=0; 
            Buffer[i]=fgetc(fs);
            if (Buffer[i] == '/') {
              fprintf(actout, "STN2 %s", get_html_text(144));
            } else {
              for(;Buffer[i] != '\0' && Buffer[i] != '#' && i < sizeof(Buffer)-1;) {
                Buffer[++i] = fgetc(fs);
              }
              Buffer[i] = '\0';
              if ((strstr(Buffer, "../")) != NULL) {
                fprintf(actout, "STN3 %s: %s", get_html_text(144), Buffer);
              } else if((strcmp(Buffer, filename)) != 0) {
                send_template(Buffer);
              }
            }
            break;

          /* show the mailing list moderators */
          case 'o':
            show_list_group_now(1);
            break;

          /* display mailing list prefix */
          case 'P':
            {
              Buffer[0] = '\0';
              get_mailinglist_prefix(Buffer);
              fprintf(actout, "%s", Buffer);
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
                  printf(get_html_text(229));
            }
            break; 

          /* send the status message parameter */
          case 'S':
            fprintf(actout,"%s", StatusMessage);
            break;

          /* show the catchall name */
          case 's':
            fprintf(actout,"%s", CurCatchall);
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
            switch(fgetc(fs)) {
              case 'A':
                fprintf(actout, "%s", uBufA);
                break;
   
              case 'B':
                fprintf(actout, "%s", uBufB);
                break;
   
              case 'C':
                fprintf(actout, "%s", uBufC);
                break;
   
              case 'D':
                fprintf(actout, "%s", uBufD);
                break;
   
              case 'E':
                fprintf(actout, "%s", uBufE);
                break;
   
              case 'F':
                fprintf(actout, "%s", uBufF);
                break;
   
              case 'G':
                fprintf(actout, "%s", uBufG);
                break;
   
              case 'H':
                fprintf(actout, "%s", uBufH);
                break;
   
              case 'I':
                fprintf(actout, "%s", uBufI);
                break;
   
              case 'J':
                fprintf(actout, "%s", uBufJ);
                break;
   
              case 'K':
                fprintf(actout, "%s", uBufK);
                break;
   
              case 'L':
                fprintf(actout, "%s", uBufL);
                break;
    
            } 
            break;

          /* show version number */
          case 'V':
            sprintf( uBufA, "%s", QA_PACKAGE );
            sprintf( uBufB, "%s", QA_VERSION );
            sprintf( uBufC, "%s", PACKAGE );
            sprintf( uBufD, "%s", VERSION );
            send_template("version.html");
            break;

          /* send the common URL parms */
          case 'W':
            fprintf(actout,"user=%s&dom=%s&time=%d&page=%s", 
                    Username, Domain, Mytime, Pagenumber);
            break;

          /* dictionary line, we three more chars for the line */
          case 'X':
            for(i=0;i<3;++i) dchar[i] = fgetc(fs);
            dchar[i] = 0;
            target = atoi( dchar );
            #ifdef debug
            fprintf(stderr, "X - dchar: %s target: %d\n", dchar, target);
            fflush(stderr);
            #endif
            printf("%s", get_html_text(target));
            break;

          /* logout link/text */
          case 'x':
            printf("<a href=\"");
            strcpy (value, get_session_val("returntext="));
            if(strlen(value) > 0) {
               printf("%s\">%s</a>", 
                      get_session_val("returnhttp="), value);
            } else {
               printf("%s/com/logout?user=%s&dom=%s&time=%d&\">%s</a>",
                      CGIPATH, Username, Domain, Mytime, get_html_text(218));
            }
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
 static char IsStandard[9];                /* ##i0 */
 static char IsForward[9];                 /* ##i1 */
 static char ForwardAddr[MAX_BUFF];        /* ##i2 */
 static char IsLocalCopy[9];               /* ##i3 */
 static char IsVacation[9];                /* ##i4 */
 static char VacationSubject[MAX_BUFF];    /* ##i5 */
 static char IsBlackHole[9];               /* ##i8 */
 static char IsSpamCheck[9];               /* ##i9 */
 static int Initialized=0;
 static int ResetVacation=1;
 int i;
 char Buffer[MAX_BUFF];


  /*********************************************************/
  
  if( 0==Initialized ) {    /*  Not Initialized  */
    fprintf( stderr, "Not initialized user: %s Domain %s\n", 
             ActionUser, Domain );
    /*  Get data on user from vpopmail  */
    vpw = vauth_getpw(ActionUser, Domain); 

    /*  Open the user's .qmail file   */
    snprintf(Buffer, sizeof(Buffer), "%s/.qmail", vpw->pw_dir);
    fprintf( stderr, "Try to open user .qmail file: %s", Buffer );
    if( (fs1 = fopen(Buffer,"r"))==NULL) {
      fprintf( stderr, "Unable to open user .qmail file: %s", Buffer );
    }

    /*  Initialize display values   */
    strcpy(IsStandard, "");
    strcpy(IsForward, "");
    strcpy(IsLocalCopy, "");
    strcpy(IsVacation, "");
    strcpy(IsBlackHole, "");
    strcpy(IsSpamCheck, "");

    if (fs1 == NULL) {   
      /* No .qmail file - standard delivery  */  
      strcpy(IsStandard, "checked ");

    } else if (fgets(Buffer,sizeof(Buffer),fs1)!=NULL) {
      /*  Parse the first line of the .qmail file  */
                           
      if( '&'==Buffer[0] ) {  
        /*   It is a forward   */
        strcpy(IsForward, "checked ");
        strcpy(ForwardAddr, strtok(&Buffer[1], "\n"));

      } else if (strstr(Buffer, "autorespond")!=NULL ) {
        /* It is a mail robot */
        strcpy(IsVacation, "checked ");
 
      } else if (strstr(Buffer, "|/bin/true delete")!=NULL ) {
        /*  It is a black hole  */
        strcpy(IsBlackHole, "checked ");

      } else if (strstr(Buffer, "#")!=NULL ) {
        /*  It is a better black hole */
        strcpy(IsBlackHole, "checked ");

      } else if (strstr(Buffer, "preline")!=NULL ) {
        /*  It is a spam checked simple delivery  */
        strcpy(IsSpamCheck, "checked ");
 
      } else {
        /*  Nothing identified from first line  */
      }

      if (fgets(Buffer,sizeof(Buffer),fs1)!=NULL) {
        /*  Parse the second line (if any) */

        if (strstr(IsForward, "checked ")!=NULL ) {
          /*  It is a forward with a saved copy */
          strcpy(IsLocalCopy, "checked ");

          if(strstr(Buffer, SPAM_COMMAND)!=NULL) {
            /*  It is spam checked  */
            strcpy(IsSpamCheck, "checked ");
          }
        }

        if (strstr(IsVacation, "checked ")!=NULL ) {
          /*  It is a Vacation  check for spam */
          if(strstr(Buffer, SPAM_COMMAND)!=NULL) {
            /*  It is spam checked  */
            strcpy(IsSpamCheck, "checked ");
          }
        }

      } else { /*  There is no second line  */
        if (strstr(IsSpamCheck, "checked ")!=NULL ) {
          /*  One line, spam checked is Standard  */
          strcpy(IsStandard, "checked ");
        }
      }

      if (strstr(IsVacation, "checked ")!=NULL ) {
        snprintf(Buffer, sizeof(Buffer), "%s/vacation/message", vpw->pw_dir);
        fs2 = fopen(Buffer,"r");
        /* Eat first line */
        fgets(Buffer,sizeof(Buffer),fs2);

        /* Second line is the subject */
        fgets(VacationSubject,sizeof(VacationSubject),fs2);
        /* Leave the file pointing at the start of the message */
        ResetVacation=0;
      }

    } else {  /*  There are no lines in the .qmail file  */
      strcpy(IsStandard, "checked ");
    }

    Initialized=1;
  }  /*  end of Not Initialized  */

/**********************************************************/

  switch(newchar) {
    case '0':
    fprintf(actout, "%s", IsStandard);
    break;

    case '1':
    fprintf(actout, "%s", IsForward);
    break;

    case '2':
    fprintf(actout, "%s", ForwardAddr);
    break;

    case '3':
    fprintf(actout, "%s", IsLocalCopy);
    break;

    case '4':
    fprintf(actout, "%s", IsVacation);
    break;

    case '5':
    fprintf(actout, "%s", VacationSubject);
    break;

    case '6':
      if (strstr(IsVacation, "checked ")!=NULL ) {
        if( 1==ResetVacation ) {
           rewind(fs2);
           for(i=0; i<3 && fgets(Buffer,sizeof(Buffer),fs2)!=NULL; i++);
           }

        while (fgets(Buffer,sizeof(Buffer),fs2)!=NULL) {
          fprintf(actout, "%s", Buffer );
          }

        ResetVacation=1;
      }
   break;

    case '7':
    fprintf(actout, "%s", vpw->pw_gecos);
    break;

    case '8':
    fprintf(actout, "%s", IsBlackHole);
    break;

    case '9':
    fprintf(actout, "%s", IsSpamCheck);
    break;
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

      case 'A':
         /* user (not domain admin) */
         if(AdminType == DOMAIN_ADMIN) { ignore_to_end_tag(fs); }
         break;

      case 'h':
#ifndef HELP
         ignore_to_end_tag(fs);
#endif
         break;

      case 'H':
#ifdef HELP
         ignore_to_end_tag(fs);
#endif
         break;

      case 'i':
#ifndef USER_INDEX
         ignore_to_end_tag(fs);
#endif
         break;

      case 'I':
#ifdef USER_INDEX
         ignore_to_end_tag(fs);
#endif
         break;

      case 'm':
#ifndef ENABLE_MYSQL
         ignore_to_end_tag(fs);
#endif
         break;

      case 'M':
#ifdef ENABLE_MYSQL
         ignore_to_end_tag(fs);
#endif
         break;

      case 'q':
#ifndef MODIFY_QUOTA
         ignore_to_end_tag(fs);
#endif
         break;

      case 'Q':
#ifdef MODIFY_QUOTA
         ignore_to_end_tag(fs);
#endif
         break;

      case 's':
#ifndef MODIFY_SPAM
         ignore_to_end_tag(fs);
#endif
         break;

      case 'S':
#ifdef MODIFY_SPAM
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

      case 'U':
         /* administrative commands */
         if(AdminType == USER_ADMIN) { ignore_to_end_tag(fs); }
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
   char Buffer[MAX_BUFF];

   sprintf(dombuf, "%s/control/virtualdomains", QMAILDIR);
   
   fs = fopen(dombuf, "r");
   if( fs != NULL ) {
      if ((srvnam = getenv("HTTP_HOST")) != NULL) {
         lowerit(srvnam);
         while( fgets(Buffer, sizeof(Buffer), fs) != NULL && count==0 ) {
            /* strip off newline */
            l = strlen(Buffer); l--; Buffer[l] = 0;
            /* virtualdomains format:  "domain:domain", get to second one */
            domptr = Buffer;
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
   char Buffer[MAX_BUFF];
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
         memset(Buffer, 0, sizeof(Buffer));
         if ( fgets(Buffer, sizeof(Buffer), fs_qw) != NULL ) {
            if(GetValue(Buffer, value, session_var, sizeof(value))==0)
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
