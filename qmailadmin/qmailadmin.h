/* 
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


#define QMAILADMIN_TEMPLATEDIR "QMAILADMIN_TEMPLATEDIR"

#define SHOW_VERSION_LINK 1

#define MAX_FILE_NAME 100

#define MAX_BUFF 500
#define MAX_BIG_BUFF 5000

#define QMAILADMIN_UNLIMITED -1

#define NO_ADMIN		0

#define DOMAIN_ADMIN	2
#define USER_ADMIN		3


#define NUM_SQL_OPTIONS	  6

char *get_html_text( char *index );
int open_lang( char *lang);

int quota_to_bytes(char[], char*);     //jhopper prototype
int quota_to_megabytes(char[], char*); //jhopper prototype

/* prototypes for sorting functions in util.c */
int sort_init();
int sort_add_entry (char *, char);
char *sort_get_entry (int);
void sort_cleanup();
void sort_dosort();
void str_replace (char *, char, char);

void qmail_button(char *modu, char *command, char *user, char *dom, time_t mytime, char *png);

