/*                                                                  */
/* This was taken from BinkD and modified a bit for hpt -- ml, 2001 */
/*                                                                  */

#if defined(__NT__) || defined (NT) || defined (WINNT)
/*--------------------------------------------------------------------*/
/*       Copyright (c) 1997 by Fydodor Ustinov                        */
/*                             FIDONet 2:5020/79                      */
/*                                                                    */
/*  This program is  free software;  you can  redistribute it and/or  */
/*  modify it  under  the terms of the GNU General Public License as  */
/*  published  by the  Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version. See COPYING.  */
/*--------------------------------------------------------------------*/

#ifdef __WATCOMC__
#include <direct.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN NAME_MAX
#endif
#elif defined (_MSC_VER)
#include <direct.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN _MAX_PATH
#endif
#endif

#include <windows.h>
#include <stdio.h>
#include <limits.h>

extern void w_log (char key, char *logString, ...);

unsigned long getfree (char *path) {
char RPN[MAXPATHLEN];	// root path
char *pRPN;             // Pointer to Root path
DWORD SPC;				// sectors per cluster
DWORD BPS;				// bytes per sector
DWORD FC;				// number of free clusters
DWORD TNC;				// total number of clusters
BOOL rc;

  pRPN = RPN;
  if (isalpha(path[0]) && path[1] == ':' ) {
	  // Drive letter
	  RPN[0] = path[0];
	  RPN[1] = ':';
	  RPN[2] = '\\';
	  RPN[3] = '\0';
  } else if (path[0] == '\\' && path[1] == '\\') {
	  // UNC path
	  int i;
      RPN[0] = '\\';
	  RPN[1] = '\\';
	  i = 2;
	  // copy server name....
      do {
		  RPN[i] = path[i];
	  } while (path[i++] != '\\');
      // .... and share name
      do {
		  RPN[i] = path[i];
	  } while (path[i++] != '\\');

      RPN[i] = '\0';

  } else {
	  // Current Drive
	  pRPN = NULL;
  }
  rc = GetDiskFreeSpace(pRPN,&SPC,&BPS,&FC,&TNC);
  if (rc != TRUE) {
    w_log ('9', "GetDiskFreeSpace error: return code = %lu", GetLastError());
    return ULONG_MAX;		    /* Assume enough disk space */
  } else {
    return (unsigned long) (BPS * SPC * FC);
  }
}
#elif defined(__OS2__) || defined(OS2)

#ifdef __WATCOMC__
#define __IBMC__ 0
#define __IBMCPP__ 0
#endif

#define INCL_DOS
#include <os2.h>
#include <ctype.h>
#include <limits.h>

extern void w_log (char key, char *logString, ...);

unsigned long getfree (char *path)
{
  FSALLOCATE fsa;
  ULONG disknum = 0;
  APIRET rc;

  if (isalpha (path[0]) && path[1] == ':')
    disknum = toupper (path[0]) - 'A' + 1;

  rc = DosQueryFSInfo (disknum, /* Drive number            */
		       FSIL_ALLOC,	    /* Level 1 allocation info */
		       (PVOID) & fsa,	/* Buffer                  */
		       sizeof (fsa));	/* Size of buffer          */

  if (rc)
  {
    w_log ('9', "DosQueryFSInfo error: return code = %u", rc);
    return ULONG_MAX;		    /* Assume enough disk space */
  }
  else
  {
    return fsa.cSectorUnit * fsa.cUnitAvail * fsa.cbSector;
  }
}
#elif defined(UNIX) || defined (__linux__)
/*
   This was taken from ifmail, and modified a bit for binkd -- mff, 1997

   Copyright (c) 1993-1995 Eugene G. Crosser

   ifcico is a FidoNet(r) compatible mailer for U*IX platforms.

   You may do virtually what you wish with this software, as long as the
   explicit reference to its original author is retained:

   Eugene G. Crosser <crosser@pccross.msk.su>, 2:5020/230@FidoNet

   THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY KIND,
   EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE COPYRIGHT HOLDER BE
   LIABLE FOR ANY DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#ifndef __BEOS__
#if defined( __svr4__ ) || defined( __SVR4 )
#include <sys/statvfs.h>
#else
#include <sys/statfs.h>
#endif
#endif
#include <limits.h>

extern void w_log (char key, char *logString, ...);

#if defined(_SYS_STATFS_H) || defined(_SYS_STATVFS_H)
unsigned long getfree (char *path)
{
#ifdef _SYS_STATVFS_H
  struct statvfs sfs;

  if (statvfs (path, &sfs) != 0)
#else
  struct statfs sfs;

  if (statfs (path, &sfs) != 0)
#endif
  {
    w_log ('9', "cannot statfs \"%s\", assume enough space", path);
    return ULONG_MAX;
  }
  else
	  /* return (sfs.f_bsize * sfs.f_bfree); */
	  return (sfs.f_bsize * sfs.f_bavail);
}

#else
unsigned long getfree (char *path)
{
  return ULONG_MAX;
}

#endif /* defined(HAVE_STATFS) | defined(HAVE_STATVFS) */

#else

#error "unknown system!"

#endif
