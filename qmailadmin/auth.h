/*
 * $Id: auth.h,v 1.1.2.1 2004-11-20 01:10:41 tomcollins Exp $
 * Copyright (C) 2009 Inter7 Internet Technologies, Inc.
 */

void auth_system (const char *ip_addr, struct vqpasswd *pw);
void auth_user_domain (const char *ip_addr, struct vqpasswd *pw);
void set_admin_type();

