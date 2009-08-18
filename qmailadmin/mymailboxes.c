/* 
 * $Id$
 * Copyright (C) 1999-2009 Inter7 Internet Technologies, Inc. 
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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <vpopmail_config.h>
/* undef some macros that get redefined in config.h below */
#undef PACKAGE_NAME  
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include "alias.h"
#include "cgi.h"
#include "config.h"
#include "limits.h"
#include "printh.h"
#include "qmailadmin.h"
#include "qmailadminx.h"
#include "show.h"
#include "template.h"
#include "user.h"
#include "mymailboxes.h"
#include "util.h"
#include "vpopmail.h"
#include "vauth.h"
#include "group.h"

#define MM_MAX_BUFF 500

/*
   From user.c
*/

extern int makevacation(FILE *, char *);

/*
   Logged in user
*/

extern struct vqpasswd user;

/*
   Search response user list
*/

typedef struct __suser_ {
   char *name;
   struct __suser_ *next;
} suser_t;

static suser_t *susers = NULL;

static int search_user(char *, const char *);

/*
   Search against username and GECOS
   Update linked list
   Returns number of matches
*/

static int search_user(char *dom, const char *search)
{
   suser_t *s = NULL;
   char *p = NULL;
   char user[255] = { 0 };
   struct vqpasswd *pw = NULL;
   int num = 0, len = 0, matched = 0, i = 0, ret = 0;
   group_t g;

   num = 0;
   susers = NULL;

   if ((search == NULL) || (!(*search)))
	  return 0;

   len = strlen(search);
   if (len == 1)
	  return 0;

   ret = group_init(&g);
   if (!ret)
	  return 0;

   ret = group_load(Username, dom, &g);
   if (!ret)
	  return 0;

   for (i = 0; i < g.n_members; i++) {
	  for (p = g.member[i]; *p; p++) {
		 if (*p == '@')
			break;
	  }

	  if (!(*p))
		 continue;

	  if (strcasecmp(p + 1, dom))
		 continue;

	  memset(user, 0, sizeof(user));
	  len = (p - g.member[i]);
	  if (len >= sizeof(user))
		 continue;

	  memcpy(user, g.member[i], len);
	  
	  pw = vauth_getpw(user, dom);
	  if (!pw)
		 continue;

	  len = strlen(search);

	  for (p = pw->pw_name; *p; p++) {
		  if (!(strncasecmp(p, search, len))) {
			s = malloc(sizeof(suser_t));
			if (s == NULL)
			   break;

			memset(s, 0, sizeof(suser_t));

			s->name = strdup(user);
			s->next = susers;
			susers = s;

			num++;
			matched = 1;
			break;
		 }
	  }

	  if (!matched) {
		 for (p = pw->pw_gecos; *p; p++) {
			if (!(strncasecmp(p, search, len))) {
			   s = malloc(sizeof(suser_t));
			   if (s == NULL)
				  break;

			   memset(s, 0, sizeof(suser_t));

			   s->name = strdup(pw->pw_name);
			   s->next = susers;
			   susers = s;

			   num++;
			   matched = 1;
			   break;
			}
		 }
	  }
   }

   return num;
}

void show_mymailboxes(char *Username, char *Domain, time_t Mytime)
{
  send_template("show_mymailboxes.html");
}

int show_mailbox_lines(char *user, char *dom, time_t mytime, char *dir)
{
 int  k,startnumber,moreusers = 1;
 struct vqpasswd *pw;
 int totalpages;
 int colspan = 4;
 suser_t *s = NULL;
 int ret = 0;
 group_t g;
 char userb[255] = { 0 }, *p = NULL;

 k = 0;
 pw = NULL;

 ret = group_init(&g);
 if (!ret)
	return 0;

 ret = group_load(user, dom, &g);
 if (!ret)
	return 0;

 s = susers = NULL;

  if (*SearchUser) {
	 k = search_user(dom, SearchUser);

	 if (susers == NULL)
		k = g.n_members;

    if (k == 0) strcpy (Pagenumber, "1");
    else if (atoi(Pagenumber) < 1)
	   sprintf(Pagenumber, "%d", (k/MAXUSERSPERPAGE)+1);
  }

  /* Determine number of pages */
  if (susers == NULL)
	 k = g.n_members;

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
  s = susers;

  if ((susers == NULL) && (k)) {
	 k = startnumber;
	 if (k >= g.n_members)
		k = g.n_members-1;

      memset(userb, 0, sizeof(userb));

	  for (p = g.member[k]; *p; p++) {
		 if (*p == '@')
			break;
	  }

	  if (!(*p)) {
		 return 0;
	  }

	  ret = (p - g.member[k]);
	  if (ret >= sizeof(userb))
		 return 0;

	  memcpy(userb, g.member[k], ret);
	  pw = vauth_getpw(userb, p + 1);
	  k++;
  }

  else {
	 for (k = 0; k < (startnumber - 1); ++k)
		s = s->next;

	 if (s) {
	    pw = vauth_getpw(s->name, dom);
		s = s->next;
     }
  }

  if (pw == NULL) {
    printf ("<tr><td colspan=\"%i\" bgcolor=%s>%s</td></tr>\n", 
      colspan, get_color_text("000"), html_text[131]);
      moreusers = 0;
    } else {
      while ((pw != NULL) && ((k < MAXUSERSPERPAGE + startnumber))) {
          /* display account name and user name */
          printf ("<tr bgcolor=%s>", get_color_text("000"));
          printh ("<td align=\"left\">%H</td>", pw->pw_name);
          printh ("<td align=\"left\">%H</td>", pw->pw_gecos);

          /* display button to modify user */
          printf ("<td align=\"center\">");
          printh ("<a href=\"%s/com/modmymailbox?user=%C&dom=%C&time=%d&moduser=%C\">",
            CGIPATH,user,dom,mytime,pw->pw_name);
          printf ("<img src=\"%s/modify.png\" border=\"0\"></a>", IMAGEURL);
          printf ("</td>");
            
          /* display trashcan for delete */
          printf ("<td align=\"center\">");
            printh ("<a href=\"%s/com/delmymailbox?user=%C&dom=%C&time=%d&deluser=%C\">",
              CGIPATH,user,dom,mytime,pw->pw_name);
            printf ("<img src=\"%s/trash.png\" border=\"0\"></a>", IMAGEURL);

          printf ("</td>");
          printf ("</tr>\n");

		pw = NULL;
		if (s) {
		   pw = vauth_getpw(s->name, dom);
		   s = s->next;
	    }

		if ((susers == NULL) && (g.member[k]) && (k < g.n_members)) {
			memset(userb, 0, sizeof(userb));

			for (p = g.member[k]; *p; p++) {
			   if (*p == '@')
				  break;
			}

			if (!(*p)) {
			   return 0;
			}

			ret = (p - g.member[k]);
			if (ret >= sizeof(userb))
			   return 0;

			memcpy(userb, g.member[k], ret);
			pw = vauth_getpw(userb, p + 1);
		 }

        ++k;
     } 
   }

      printf ("<tr bgcolor=%s>", get_color_text("000"));
      printf ("<td colspan=\"%i\" align=\"center\">", colspan);
      printf ("<hr>");
      printf ("<b>%s</b>", html_text[133]);
      printf ("<br>");
      for (k = 'a'; k <= 'z'; k++) {
        printh ("<a href=\"%s/com/showmymailboxes?user=%C&dom=%C&time=%d&searchuser=%c\">%c</a>\n",
          CGIPATH,user,dom,mytime,k,k);
      }
      printf ("<br>");
      for (k = 0; k < 10; k++) {
        printh ("<a href=\"%s/com/showmymailboxes?user=%C&dom=%C&time=%d&searchuser=%d\">%d</a>\n",
          CGIPATH,user,dom,mytime,k,k);
      }
      printf ("</td>");
      printf ("</tr>\n");

      printf ("<tr bgcolor=%s>", get_color_text("000"));
      printf ("<td colspan=%i>", colspan);
      printf ("<table border=0 cellpadding=3 cellspacing=0 width=\"100%%\"><tr><td align=\"center\"><br>");
      printf ("<form method=\"get\" action=\"%s/com/showmymailboxes\">", CGIPATH);
      printh ("<input type=\"hidden\" name=\"user\" value=\"%H\">", user);
      printh ("<input type=\"hidden\" name=\"dom\" value=\"%H\">", dom);
      printf ("<input type=\"hidden\" name=\"time\" value=\"%u\">", (unsigned int) mytime);
      printh ("<input type=\"text\" name=\"searchuser\" value=\"%H\">&nbsp;", SearchUser);
      printf ("<input type=\"submit\" value=\"%s\">", html_text[204]);
      printf ("</form>");
      printf ("</td></tr></table>");

	  
	  if (totalpages > 1) {
		 printf ("<hr>");
		 printf ("</td></tr>\n");

		 printf ("<tr bgcolor=%s>", get_color_text("000"));
		 printf ("<td colspan=\"%i\" align=\"right\">", colspan);

	   }

      /* only display "previous page" if pagenumber > 1 */
      if (atoi(Pagenumber) > 1) {
		 printf ("<font size=\"2\"><b>");
		 printf ("[&nbsp;");
		 printh ("<a href=\"%s/com/showmymailboxes?user=%C&dom=%C&time=%d&page=%d%s%s\">%s</a>",
          CGIPATH,user,dom,mytime,
          atoi(Pagenumber)-1 ? atoi(Pagenumber)-1 : atoi(Pagenumber), 
		  susers ? "&searchuser=" : "",
		  susers ? SearchUser : "",
          html_text[135]);
        printf ("&nbsp;]&nbsp</font>");
      }

      if (moreusers && atoi(Pagenumber) < totalpages) {
		 printf ("<font size=\"2\"><b>");
		 printf ("[&nbsp;");
        printh ("<a href=\"%s/com/showmymailboxes?user=%C&dom=%C&time=%d&page=%d%s%s\">%s</a>",
          CGIPATH,user,dom,mytime,atoi(Pagenumber)+1,
		  susers ? "&searchuser=" : "",
		  susers ? SearchUser : "",
          html_text[137]);
        printf ("&nbsp;]&nbsp</font>");
      }

      printf ("</b></font>");
      printf ("</td></tr>\n");
  
  return 0;
}

void modmymailbox()
{
   int ret = 0;
   group_t g;
   char b[500] = { 0 };

    group_init(&g);

  ret = group_load(ActionUser, Domain, &g);
  if (!ret) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[142]);
    vclose();
    exit(0);
   }

  memset(b, 0, sizeof(b));
  snprintf(b, sizeof(b), "%s@%s", Username, Domain);

  if (strcasecmp(g.owner, b)) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[142]);
    vclose();
	exit(0);
   }

  send_template( "mod_mymailbox.html" );
} 


void addmymailbox()
{
   int ret = 0;
   group_t g;

   group_init(&g);

   ret = group_load(Username, Domain, &g);
   if (!ret) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[331]);
    vclose();
    exit(0);
   }

   if (g.n_members >= g.max_members) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[330]);
    vclose();
	send_template("show_mymailboxes.html");
    exit(0);
   }
   
  send_template( "add_mymailbox.html" );
}


void addmymailboxnow()
{
 char *c_num;
 char *tmp;
 char *email;
 char **arguments;
 struct vqpasswd *mypw;
 static char NewBuf[156];
 int ret = 0;
   group_t g;

   group_init(&g);

   ret = group_load(Username, Domain, &g);
   if (!ret) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[331]);
    vclose();
    exit(0);
   }

   if (g.n_members >= g.max_members) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s", html_text[330]);
    vclose();
    exit(0);
   }

  c_num = malloc(MM_MAX_BUFF);
  email = malloc(128);
  tmp = malloc(MM_MAX_BUFF);
  arguments = (char **)malloc(MM_MAX_BUFF);

  GetValue(TmpCGI,Newu, "newu=", sizeof(Newu));

  if ( fixup_local_name(Newu) ) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s %s\n", html_text[148], Newu);
    addmymailbox();
    vclose();
    exit(0);
  } 

  if ( check_local_user(Newu) ) {
    snprinth (StatusMessage, sizeof(StatusMessage), "%s %H\n", html_text[175], Newu);
    addmymailbox();
    vclose();
    exit(0);
  } 

  GetValue(TmpCGI,Password1, "password1=", sizeof(Password1));
  GetValue(TmpCGI,Password2, "password2=", sizeof(Password2));
  if ( strcmp( Password1, Password2 ) != 0 ) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s\n", html_text[200]);
    addmymailbox();
    vclose();
    exit(0);
  }

  if ( strlen(Password1) <= 0 ) {
    snprintf (StatusMessage, sizeof(StatusMessage), "%s\n", html_text[234]);
    addmymailbox();
    vclose();
    exit(0);
  }

  snprintf (email, 128, "%s@%s", Newu, Domain);
    
  GetValue(TmpCGI,Gecos, "gecos=", sizeof(Gecos));
  if ( strlen( Gecos ) == 0 ) {
	snprintf(StatusMessage, sizeof(StatusMessage), "%s\n", html_text[318]);
	addmymailbox();
	vclose();
	exit(0);
  }

  /* add the user then get the vpopmail password structure */
  if ( vadduser( Newu, Domain, Password1, Gecos, USE_POP ) == 0 && 
#ifdef MYSQL_REPLICATION
    !sleep(2) &&
#endif
    (mypw = vauth_getpw( Newu, Domain )) != NULL ) {

	 /*
	    Set group default quota
	  */

	 if (g.quota == 0)
        vsetuserquota (Newu, Domain, "NOQUOTA");

     else {

		snprintf(NewBuf, sizeof(NewBuf), "%d", g.quota);
	    vsetuserquota (Newu, Domain, NewBuf);
     }

	/*
		 Add to group
    */

    ret = group_add(&g, Newu, Domain);
	if (!ret) {
	   vdeluser(Newu, Domain);
	   snprinth (StatusMessage, sizeof(StatusMessage), "<font color=\"red\">%s %H@%H (%H) %s</font>", 
          html_text[2], Newu, Domain, Gecos, html_text[120]);
    }

	else {
	   ret = group_write(&g);
	   if (!ret) {
		  group_remove(&g, Newu, Domain);
		  group_write(&g);
		  vdeluser(Newu, Domain);
	   }

	   else {
		 /* report success */
		 snprinth (StatusMessage, sizeof(StatusMessage), "%s %H@%H (%H) %s",
			html_text[2], Newu, Domain, Gecos,
			html_text[119]);
	   }
    }

  } else {
    /* otherwise, report error */
    snprinth (StatusMessage, sizeof(StatusMessage), "<font color=\"red\">%s %H@%H (%H) %s</font>", 
      html_text[2], Newu, Domain, Gecos, html_text[120]);
  }

  /* After we add the user, show the user page
   * people like to visually verify the results
   */
  show_mymailboxes(Username, Domain, Mytime);
}

void delmymailbox()
{
  send_template( "del_mymailbox.html" );
}

void delmymailboxgo()
{
   int ret = 0;
   group_t g;
   char b[500] = { 0 };
     
   /*
	  Remove user from group
	  If user is owner, then stop
   */
 
   group_init(&g);
   ret = group_load(ActionUser, Domain, &g);
   if (!ret) {
	  vclose();
	  exit(0);
   }
			
   memset(b, 0, sizeof(b));
   snprintf(b, sizeof(b), "%s@%s", ActionUser, Domain);

   if (!(strcasecmp(g.owner, b))) {
	  vclose();
	  exit(0);
   }

   group_remove(&g, ActionUser, Domain);
   group_write(&g);

   vdeluser( ActionUser, Domain );
   snprinth (StatusMessage, sizeof(StatusMessage), "%H %s", ActionUser, html_text[141]);
   show_mymailboxes(Username, Domain, Mytime);
}

void modmymailboxgo()
{
 char *tmpstr;
 int ret_code;
 struct vqpasswd *vpw=NULL;
 static char box[500];
 char cforward[50];
 static char NewBuf[156];
 char dotqmailfn[156];
#ifdef MODIFY_QUOTA
 char *quotaptr;
 char qconvert[11];
#endif
 int count;
 FILE *fs;
 int spam_check = 0;
 int vacation = 0;
 int saveacopy = 0;
 int emptydotqmail;
 char *olddotqmail = NULL;
 char *dotqmailline;
 struct stat sb;
 int err;
 int ret = 0;
 group_t g;
 char b[500] = { 0 };

 const char *flagfields[] = { "zeroflag=", "oneflag=", "twoflag=", "threeflag=" };
 const gid_t gidflags[] = { V_USER0, V_USER1, V_USER2, V_USER3 };
 gid_t orig_gid;
 int i;
 
   /*
	  Make sure ActionUser is a member of this group
   */

   group_init(&g);
   ret = group_load(ActionUser, Domain, &g);
   if (!ret) {
	  vclose();
	  exit(0);
   }

   memset(b, 0, sizeof(b));
   snprintf(b, sizeof(b), "%s@%s", Username, Domain);

   if (strcasecmp(g.owner, b)) {
	  vclose();
	  exit(0);
   }

   if (strlen(Password1)>0 && strlen(Password2)>0 ) {
    if ( strcmp( Password1, Password2 ) != 0 ) {
      snprintf (StatusMessage, sizeof(StatusMessage), "%s\n", html_text[200]);
      modmymailbox();
      vclose();
      exit(0);
    }
    ret_code = vpasswd( ActionUser, Domain, Password1, USE_POP);
    if ( ret_code != VA_SUCCESS ) {
      snprintf (StatusMessage, sizeof(StatusMessage), "%s (%s)", html_text[140], 
        verror(ret_code));
    } else {
      /* snprinth (StatusMessage, sizeof(StatusMessage), "%s %H@%H.", html_text[139], 
ActionUser, Domain ); */
      strcpy (StatusMessage, html_text[139]);
    }
  }

  GetValue(TmpCGI,Gecos, "gecos=", sizeof(Gecos));

  vpw = vauth_getpw (ActionUser, Domain);

  /* check for the V_USERx flags and set accordingly */
  /* new code by Tom Collins <tom@tomlogic.com>, Dec 2004 */
  /* replaces code by James Raftery <james@now.ie>, 12 Dec. 2002 */

  orig_gid = vpw->pw_gid;
  for (i = 0; i < 4; i++) {
    GetValue (TmpCGI, box, (char *) flagfields[i], sizeof (box));
    if (strcmp (box, "on") == 0)
      vpw->pw_gid |= gidflags[i];
    else if (strcmp (box, "off") == 0)
      vpw->pw_gid &= ~gidflags[i];
  }

  /* we're trying to cut down on unnecessary updates to the password entry
   * we accomplish this by only updating if the pw_gid or gecos changed
   */

  if ((*Gecos != '\0') && (strcmp (Gecos, vpw->pw_gecos) != 0)) {
    vpw->pw_gecos = Gecos;
    vauth_setpw(vpw, Domain);  
  } else if (vpw->pw_gid != orig_gid) vauth_setpw (vpw, Domain);

  /* get value of the spam filter box */
  GetValue(TmpCGI, box, "spamcheck=", sizeof(box));
  if ( strcmp(box, "on") == 0 ) spam_check = 1;

  /* get the value of the vacation checkbox */
  GetValue(TmpCGI, box, "vacation=", sizeof(box));
  if ( strcmp(box, "on") == 0 ) vacation = 1;
    
  /* if they want to save a copy */
  GetValue(TmpCGI, box, "fsaved=", sizeof(box));
  if ( strcmp(box,"on") == 0 ) saveacopy = 1;

  /* get the value of the cforward radio button */
  GetValue(TmpCGI, cforward, "cforward=", sizeof(cforward));
  if ( strcmp(cforward, "vacation") == 0 ) vacation = 1;
  
  /* open old .qmail file if it exists and load it into memory */
  snprintf (dotqmailfn, sizeof(dotqmailfn), "%s/.qmail", vpw->pw_dir);
  err = stat (dotqmailfn, &sb);
  if (err == 0) {
    olddotqmail = malloc (sb.st_size);
    if (olddotqmail != NULL) {
      fs = fopen (dotqmailfn, "r");
      if (fs != NULL) {
        fread (olddotqmail, sb.st_size, 1, fs);
        fclose (fs);
      }
    }
  }
  
  fs = fopen (dotqmailfn, "w");
  
  /* Scan through old .qmail and write out any unrecognized program delivery
   * lines to the new .qmail file.
   */
  emptydotqmail = 1;
  if (olddotqmail != NULL) {
    dotqmailline = strtok (olddotqmail, "\n");
    while (dotqmailline) {
      if ( (*dotqmailline == '|') &&
          (strstr (dotqmailline, "/true delete") == NULL) &&
          (strstr (dotqmailline, "/autorespond ") == NULL) &&
          (strstr (dotqmailline, SPAM_COMMAND) == NULL) ) {
        fprintf (fs, "%s\n", dotqmailline);
        emptydotqmail = 0;
      }
      dotqmailline = strtok (NULL, "\n");
    }
    free (olddotqmail);
  }

  /* Decide on what to write to the new .qmail file after any old program
   * delivery lines are written.
   */

  err = 0;
  
  /* note that we consider a .qmail file with just Maildir delivery to be empty
   * since it can be removed.
   */
   
  /* if they want to forward */
  if (strcmp (cforward, "forward") == 0 ) {

    /* get the value of the foward */
    GetValue(TmpCGI, box, "nforward=", sizeof(box));

    /* If nothing was entered, error */
    if ( box[0] == 0 ) {
      snprintf (StatusMessage, sizeof(StatusMessage), "%s\n", html_text[215]);
      err = 1;
    } else {

      tmpstr = strtok(box," ,;\n");

      /* tmpstr points to first non-token */

      count=0;
      while( tmpstr != NULL && count < MAX_FORWARD_PER_USER) {
        if ((*tmpstr != '|') && (*tmpstr != '/')) {
          fprintf(fs, "&%s\n", tmpstr);
          emptydotqmail = 0;
          ++count;
        }
        tmpstr = strtok(NULL," ,;\n");
      }

    }
  }

  if ( (strcmp (cforward, "forward") != 0) || saveacopy ) {

    if (strcmp (cforward, "blackhole") == 0) {
      fprintf (fs, "# delete\n");
      emptydotqmail = 0;
    } else if (spam_check == 1) {
      fprintf (fs, "%s\n", SPAM_COMMAND);
      emptydotqmail = 0;
    } else {
      fprintf (fs, "%s/" MAILDIR "/\n", vpw->pw_dir);
      /* this isn't enough to consider the .qmail file non-empty */
    }
  }

  if (vacation) {
    err = makevacation (fs, vpw->pw_dir);
    emptydotqmail = 0;
  } else {
    /* delete old vacation directory */
    snprintf (NewBuf, sizeof(NewBuf), "%s/vacation", vpw->pw_dir);
    vdelfiles (NewBuf);
  }

  fclose (fs);
  if (emptydotqmail) unlink (dotqmailfn);

  if (err) {
    modmymailbox();
    vclose();
    exit(0);
  }
  
  modmymailbox();
}

