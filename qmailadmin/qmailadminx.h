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
#include <vauth.h>
#include <vlimits.h>

extern char Username[MAX_BUFF];
extern char Domain[MAX_BUFF];
extern char Password[MAX_BUFF];
extern char Gecos[MAX_BUFF];
extern char Quota[MAX_BUFF];
extern char Time[MAX_BUFF];
extern char Action[MAX_BUFF];
extern char ActionUser[MAX_BUFF];
extern char Newu[MAX_BUFF];
extern char Password1[MAX_BUFF];
extern char Password2[MAX_BUFF];
extern char Crypted[MAX_BUFF];
extern char Alias[MAX_BUFF];
extern char AliasType[MAX_BUFF];
extern char LineData[MAX_BUFF];
extern char Action[MAX_BUFF];
extern char Message[MAX_BIG_BUFF];
extern char StatusMessage[MAX_BIG_BUFF];
extern int CGIValues[256];
extern char Pagenumber[MAX_BUFF];
extern char SearchUser[MAX_BUFF];
extern time_t Mytime;
extern char *TmpCGI;
extern int Compressed;
extern FILE *actout;

extern struct vlimits Limits;
extern int num_of_mailinglist;
extern int AdminType;
extern int MaxPopAccounts;
extern int MaxAliases;
extern int MaxForwards;
extern int MaxAutoResponders;
extern int MaxMailingLists;

extern int DisablePOP;
extern int DisableIMAP;
extern int DisableDialup;
extern int DisablePasswordChanging;
extern int DisableWebmail;
extern int DisableRelay;

extern int CurPopAccounts;
extern int CurForwards;
extern int CurAutoResponders;
extern int CurMailingLists;
extern int CurBlackholes;
extern char CurCatchall[MAX_BUFF];

extern int Uid;
extern int Gid;
extern char RealDir[156];

extern char Lang[40];

extern char uBufA[MAX_BUFF];
extern char uBufB[MAX_BUFF];
extern char uBufC[MAX_BUFF];
extern char uBufD[MAX_BUFF];
extern char uBufE[MAX_BUFF];
extern char uBufF[MAX_BUFF];
extern char uBufG[MAX_BUFF];
extern char uBufH[MAX_BUFF];
extern char uBufI[MAX_BUFF];
extern char uBufJ[MAX_BUFF];
extern char uBufK[MAX_BUFF];
extern char uBufL[MAX_BUFF];

void del_id_files( char *);
int open_lang();

extern char *strstart();
extern char *safe_getenv();

extern int count_stuff(void); 

/*   Not found in any other QmailAdmin source file    */
/*  extern int CallVmoduser;  */

