/*:ts=8*/
/*****************************************************************************
 * Post for HPT (FTN NetMail/EchoMail Tosser)
 *****************************************************************************
 * Copyright (C) 1998-99
 *
 * Kolya Nesterov
 *
 * Fido:     2:463/7208.53
 * Kiev, Ukraine
 *
 * This file is part of HPT.
 *
 * HPT is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * HPT is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HPT; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/
/* Revision log:
   16.12.98 - first version, written at ~1:30, in the middle of doing 
              calculation homework on Theoretical Electrics, you understood ;)
   18.12.98 - woops forgot copyright notice, minor fixes
	      tearline generation added
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(MSDOS) || defined(__DJGPP__)
#include <fidoconfig.h>
#else
#include <fidoconf.h>
#endif
#include <common.h>

#include <version.h>
#include <toss.h>
#include <post.h>
#include <global.h>
#include <version.h>
#include <areafix.h>

/* Warning : the code is totaly untested */

void post(int c, unsigned int *n, char *params[])
{
   char *area = NULL;
   FILE *text = NULL;
   s_area *echo = NULL;
   FILE *f = NULL;

   s_message msg;

   CHAR *textBuffer = NULL;

   int quit;
   int export=0;
   char buffer[256];

   time_t t = time (NULL);
   struct tm *tm;

   memset(&msg, 0, sizeof(s_message));

   for (quit = 0;*n < c && !quit; (*n)++) {
      if (*params[*n] == '-') {
         switch(params[*n][1]) {
            case 'a':    // address
               switch(params[*n][2]) {
                  case 't':
                     string2addr(params[++(*n)], &(msg.destAddr));
                     break;
                  case 'f':
                     string2addr(params[++(*n)], &(msg.origAddr));
                     break;
                  default:
                     quit = 1;
                     break;
               }; break;
            case 'n':    // name
               switch(params[*n][2]) {
                  case 't':
                     msg.toUserName = (char *) malloc(strlen(params[++(*n)]) + 1);
                     strcpy(msg.toUserName, params[*n]);
                     break;
                  case 'f':
                     msg.fromUserName = (char *) malloc(strlen(params[++(*n)]) + 1);
                     strcpy(msg.fromUserName, params[*n]);
                     break;
                  default:
                     quit = 1;
                     break;
               }; break;
            case 'f':    // flags
               msg.attributes = atoi(params[++(*n)]);
               break;
            case 'e':    // echo name
               area = params[++(*n)];
               echo = getArea(config, area);
               break;
            case 's':    // subject
               msg.subjectLine = (char *) malloc(strlen(params[++(*n)]) + 1);
               strcpy(msg.subjectLine, params[*n]);
               break;
            case 'x':    // export message
               export=1; (*n)++;
               break;
	    default:
               quit = 1;
               break;
         };
      } else {
         if ((text = fopen(params[*n], "rt")) != NULL) {
            /* reserve 512kb + 1 (or 32kb+1) text Buffer */
            textBuffer = (CHAR *) malloc(TEXTBUFFERSIZE+1); 
            for (msg.textLength = 0; msg.textLength < (long) TEXTBUFFERSIZE; msg.textLength++) {
               if ((textBuffer[msg.textLength] = getc(text)) == 0)
                  break;
               if (feof(text)) {
                  textBuffer[++msg.textLength] = 0;
                  break;
               }; /* endif */
               if ('\n' == textBuffer[msg.textLength])
                  textBuffer[msg.textLength] = '\r';
            }; /* endfor */
            textBuffer[msg.textLength-1] = 0;
            fclose(text);
         };
      };  
   };
   // msg.attributes |= MSGLOCAL; // Always set bit for LOCAL
   // won't be set in the msgbase, because the mail is processed if it were received
   (*n)--; tm = gmtime(&t);
   strftime(msg.datetime, 21, "%d %b %y  %T", tm);
   if ((msg.destAddr.zone != 0) && (textBuffer != NULL)) { 
      // Dumbchecks
      if (msg.origAddr.zone == 0) // maybe origaddr isn't specified ?
         msg.origAddr = config->addr[0];
      if (msg.fromUserName == NULL)
          msg.fromUserName = strdup(config->sysop);
      if (msg.toUserName == NULL)
          msg.toUserName = strdup("All");
      if (msg.subjectLine == NULL)
          msg.subjectLine = strdup("");
      msg.netMail = area == NULL;

      /* reserve mem for the real text */
      /* !!! warning - I suppose that 512 bytes will be enough
      for kludges and tearline + origin */
      msg.text = (CHAR *) malloc(msg.textLength + 1 + 512);
      createKludges(msg.text, area, &msg.origAddr, &msg.destAddr);

      strcat(msg.text, textBuffer);
      
      free(textBuffer);

      sprintf(msg.text + strlen(msg.text), "--- %s\r * Origin: %s (%s)",
              versionStr, config->name, aka2str(msg.origAddr));

      msg.textLength = strlen(msg.text);
      
      writeLogEntry (hpt_log, '1', "Start posting...");

      if (!export)
        putMsgInArea(echo, &msg, 1, msg.attributes);
      else {
        if (msg.netMail)
          processNMMsg(&msg, NULL);
        else
          processEMMsg(&msg, msg.origAddr, 1);
      }
      
      /* Don't work very well... some code in putMsgInArea rewrite msg.destAddr */
      sprintf (buffer,"Posting msg. from %s -> %s in area: %s",
                      aka2str(msg.origAddr), aka2str(msg.destAddr), area);
      writeLogEntry (hpt_log, '2', buffer);
      
      if ((config->echotosslog) && (!export)) {
        f=fopen(config->echotosslog, "a");
      
        if (f==NULL)
          writeLogEntry (hpt_log, '9', "Could not open or create EchoTossLogFile.");
        else {
          fprintf(f,"%s\n",area);
          fclose(f);
        }
      }
      
   };
   freeMsgBuffers(&msg);
}
