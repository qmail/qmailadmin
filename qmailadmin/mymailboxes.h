/*
 * $Id$
 * Copyright (C) 2009 Inter7 Internet Technologies, Inc.
 */
 
void show_mymailboxes(char *Username, char *Domain, time_t Mytime);
int show_mailbox_lines(char *user, char *dom, time_t mytime, char *dir);
void addmymailbox();
void addmymailboxnow();
void delmymailbox();
void delmymailboxgo();
void modmymailbox();
void modmymailboxgo();
