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

get_cgi()
{
 int count;
 int i,j;

  count = atoi( safe_getenv("CONTENT_LENGTH"));

  TmpCGI = malloc(count+1);
  memset(TmpCGI,0,count+1);

  i = 0;
  do {
    j = fread( &TmpCGI[i], 1, count - i, stdin);
    if ( j >= 0 ) i += j;
    else break;
  } while (j > 0 && i < count );
}

int GetValue(source,dest,name,dest_max)
 char *source;
 char *dest;
 char *name;
 int   dest_max;
{
 int i,j,k;


  memset(dest,0,dest_max);
  for(i=0; strstart(&source[i],name)!=&source[i] && source[i]!=0; ++i);

  if( source[i] != 0 ) {
    i+=strlen(name);
  } else {
    return( -1 );
  }

  for(k=0,j=i; source[j]!='&' && k<dest_max-1 && source[j]!=0;++k,++j) {
    if ( source[j] == '%' ) {
      if ( source[j+1] == '0' && source[j+2]=='D'){
        --k;
      } else if (source[j+1]=='0'&&source[j+2]=='A'){
        dest[k] = '\n';
      } else {
        dest[k] = (CGIValues[source[j+1]]<<4) + 
            CGIValues[source[j+2]];
      }
      j += 2;
    } else   if ( source[j] == '+' ) {
      dest[k] = ' ';
    } else {
      dest[k] = source[j];
    }
  }
  dest[k] = 0; 
  --k;
  while(isspace(dest[k])) {
    dest[k] = 0;
    --k;
  }
  return(0);
}
