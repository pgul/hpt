/*:ts=8*/
/*****************************************************************************
 * HPT --- FTN NetMail/EchoMail Tosser
 *****************************************************************************
 * Copyright (C) 1997-1998
 *
 * Matthias Tichy
 *
 * Fido:     2:2433/1245 2:2433/1247 2:2432/601.29
 * Internet: mtt@tichy.de
 *
 * Grimmestr. 12         Buchholzer Weg 4
 * 33098 Paderborn       40472 Duesseldorf
 * Germany               Germany
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <msgapi.h>

#include <fidoconfig.h>
#include <common.h>

#include <fcommon.h>
#include <pkt.h>
#include <patmat.h>
#include <scan.h>
#include <log.h>
#include <global.h>
#include <version.h>

#include <recode.h>
#include <toss.h>

s_statScan statScan;

void cvtAddr(const NETADDR aka1, s_addr *aka2)
{
  aka2->zone = aka1.zone;
  aka2->net  = aka1.net;
  aka2->node = aka1.node;
  aka2->point = aka1.point;
}

void convertMsgHeader(XMSG xmsg, s_message *msg)
{
   // convert header
   msg->attributes  = xmsg.attr;

   msg->origAddr.zone  = xmsg.orig.zone;
   msg->origAddr.net   = xmsg.orig.net;
   msg->origAddr.node  = xmsg.orig.node;
   msg->origAddr.point = xmsg.orig.point;
   msg->origAddr.domain =  NULL;

   msg->destAddr.zone  = xmsg.dest.zone;
   msg->destAddr.net   = xmsg.dest.net;
   msg->destAddr.node  = xmsg.dest.node;
   msg->destAddr.point = xmsg.dest.point;
   msg->destAddr.domain = NULL;

   strcpy(msg->datetime, (char *) xmsg.__ftsc_date);
   msg->subjectLine = (char *) malloc(strlen((char *)xmsg.subj)+1);
   msg->toUserName  = (char *) malloc(strlen((char *)xmsg.to)+1);
   msg->fromUserName = (char *) malloc(strlen((char *)xmsg.from)+1);
   strcpy(msg->subjectLine, (char *) xmsg.subj);
   strcpy(msg->toUserName, (char *) xmsg.to);
   strcpy(msg->fromUserName, (char *) xmsg.from);

   // recoding subjectLine to TransportCharset
   if (config->outtab != NULL) recodeToTransportCharset(msg->subjectLine);
}

void convertMsgText(HMSG SQmsg, s_message *msg, s_addr ourAka)
{
   char    *kludgeLines, viaLine[100];
   UCHAR   *ctrlBuff;
   UINT32  ctrlLen;
   time_t  tm;
   struct tm *dt;

   // get kludge lines
   ctrlLen = MsgGetCtrlLen(SQmsg);
   ctrlBuff = (unsigned char *) malloc(ctrlLen+1);
   MsgReadMsg(SQmsg, NULL, 0, 0, NULL, ctrlLen, ctrlBuff);
   kludgeLines = (char *) CvtCtrlToKludge(ctrlBuff);
   free(ctrlBuff);

   // make text
   msg->textLength = MsgGetTextLen(SQmsg);

   time(&tm);
   dt = gmtime(&tm);
   sprintf(viaLine, "\001Via %u:%u/%u.%u @%04u%02u%02u.%02u%02u%02u.UTC %s",
           ourAka.zone, ourAka.net, ourAka.node, ourAka.point,
           dt->tm_year + 1900, dt->tm_mon + 1, dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec, versionStr);

   msg->text = (char *) calloc(1, msg->textLength+strlen(kludgeLines)+strlen(viaLine)+1);


   strcpy(msg->text, kludgeLines);
//   strcat(msg->text, "\001TID: ");
//   strcat(msg->text, versionStr);
//   strcat(msg->text, "\r");

   MsgReadMsg(SQmsg, NULL, 0, msg->textLength, (unsigned char *) msg->text+strlen(msg->text), 0, NULL);

   strcat(msg->text, viaLine);

   // recoding text to TransportCharSet
   if (config->outtab != NULL) recodeToTransportCharset(msg->text);

   free(kludgeLines);
}

void makePktHeader(s_message *msg, s_pktHeader *header)
{
   if (msg != NULL) {
      header->origAddr = msg->origAddr;
      header->destAddr = msg->destAddr;
   }
   header->minorProductRev = VER_MINOR;
   header->majorProductRev = VER_MAJOR;
   header->hiProductCode   = 0;
   header->loProductCode   = 0xfe;
   memset(&(header->pktPassword), 0, 9); // no password
   time(&(header->pktCreated));
   header->capabilityWord  = 1;
   header->prodData        = 0;
}

s_route *findRouteForNetmail(s_message msg)
{
   char buff[72], addrStr[24];
   UINT i;

   sprintf(addrStr, "%u:%u/%u.%u", msg.destAddr.zone, msg.destAddr.net, msg.destAddr.node, msg.destAddr.point);

   if ((msg.attributes & MSGFILE) == MSGFILE) {
      // if msg has file attached
      for (i=0; i < config->routeFileCount; i++) {
         if (patmat(addrStr, config->routeFile[i].pattern))
            return &(config->routeMail[i]);
      }
   } else {
      // if msg has no file attached
      for (i=0; i < config->routeMailCount; i++) {
         if (patmat(addrStr, config->routeMail[i].pattern))
            return &(config->routeMail[i]);
      }
   }

   for (i=0; i < config->routeCount; i++) {
      if(patmat(addrStr, config->route[i].pattern))
         return &(config->route[i]);
   }

   
   // if no aka is found return first link
   sprintf(buff, "No route for %s found. Using last route statement", addrStr);
   writeLogEntry(log, '8', buff);
   return &(config->route[config->routeCount-1]);
}

s_link *getLinkForRoute(s_route *route, s_message *msg) {
   static s_link tempLink;
   s_link *getLink;
    
   if (route->target == NULL) {
      memset(&tempLink, 0, sizeof(s_link));

      tempLink.hisAka = msg->destAddr;
      tempLink.ourAka = &(config->addr[0]);
       
      switch (route->routeVia) {
          
         case host:
            tempLink.hisAka.node  = 0;
            tempLink.hisAka.point = 0;
            break;

         case boss:
            tempLink.hisAka.point = 0;
            break;

         case noroute:
            break;

         case hub:
            tempLink.hisAka.node -= (tempLink.hisAka.node % 100);
            tempLink.hisAka.point = 0;
            break;
      }

      getLink = getLinkFromAddr(*config, tempLink.hisAka);

      if (getLink != NULL) return getLink;
      else return &tempLink;
      
   } else return route->target;
}

void processAttachs(s_link *link, s_message *msg)
{
   FILE *flo;
   char *running = msg->subjectLine;
   char *token;
   char *newSubjectLine = (char *) calloc(strlen(msg->subjectLine)+1, 1);
   
   flo = fopen(link->floFile, "a");

   token = strsep(&running, " \t");

   while (token != NULL) {
      if (flo != NULL) fprintf(flo, "%s\n", token);
      if (strrchr(token, PATH_DELIM)!= NULL)
         strcat(newSubjectLine, strrchr(token, PATH_DELIM)+1);
      else
         strcat(newSubjectLine, token);

      strcat(newSubjectLine, " ");

      token = strsep(&running, " \t");
   }
   
   if (flo!= NULL) {
      fclose(flo);
   } else writeLogEntry(log, '9', "Could not open FloFile");

   // replace subjectLine
   free(msg->subjectLine);
   msg->subjectLine = newSubjectLine;
}

int packMsg(HMSG SQmsg, XMSG xmsg)
{
   FILE        *pkt, *req;
   char        buff[90];
   e_prio      prio;
   s_message   msg;
   s_pktHeader header;
   s_route     *route;
   s_link      *link, *virtualLink;

   convertMsgHeader(xmsg, &msg);

   // prepare virtual link...
   virtualLink = malloc(sizeof(s_link));
   virtualLink->hisAka = msg.destAddr;
   virtualLink->name = (char *) malloc(strlen(msg.toUserName)+1);
   strcpy(virtualLink->name, msg.toUserName);

   // note: link->floFile used for create 12345678.?ut mail packets & ...

   if ((xmsg.attr & MSGFRQ) == MSGFRQ) {
	   // if msg has request flag then put the subjectline into request file.
	   if (createOutboundFileName(virtualLink, NORMAL, REQUEST) == 0) {
		   req = fopen(virtualLink->floFile, "a");
		   if (req!=NULL) {
			   fprintf(req, msg.subjectLine);
			   fprintf(req, "\n");
			   fclose(req);
		   } else writeLogEntry(log, '9', "Could not open Request File");

		   remove(virtualLink->bsyFile);
		   free(virtualLink->bsyFile);
		   // mark Mail as sent
		   xmsg.attr |= MSGSENT;
		   MsgWriteMsg(SQmsg, 0, &xmsg, NULL, 0, 0, 0, NULL);
		   free(virtualLink->floFile);
	   }
   } /* endif */

   if ((xmsg.attr & MSGFILE) == MSGFILE) {
	   // file attach
	   prio = NORMAL;
	   if ((xmsg.attr & MSGCRASH) == MSGCRASH) prio = CRASH;
	   if ((xmsg.attr & MSGHOLD)  == MSGHOLD)  prio = HOLD;
	   // routing of files is not possible for this time
//	   if (prio != NORMAL) {
//		   createOutboundFileName(virtualLink, prio, FLOFILE);
//	   } else {
//		   route = findRouteForNetmail(msg);
//		   link = getLinkForRoute(route, &msg);
//		   prio = cvtFlavour2Prio(route->flavour);
//		   createOutboundFileName(virtualLink, prio, FLOFILE);
//	   } /* endif */
           if (createOutboundFileName(virtualLink, prio, FLOFILE) == 0) {
              processAttachs(virtualLink, &msg);
              remove(virtualLink->bsyFile);
              free(virtualLink->bsyFile);
              // mark Mail as sent
              xmsg.attr |= MSGSENT;
              MsgWriteMsg(SQmsg, 0, &xmsg, NULL, 0, 0, 0, NULL);
              free(virtualLink->floFile);
	   }
   } /* endif */
   
   if ((xmsg.attr & MSGCRASH) == MSGCRASH) {
	   // crash-msg -> make CUT
	   if (createOutboundFileName(virtualLink, CRASH, PKT) == 0) {
		   convertMsgText(SQmsg, &msg, msg.origAddr);
		   makePktHeader(&msg, &header);
		   pkt = openPktForAppending(virtualLink->floFile, &header);
		   writeMsgToPkt(pkt, msg);
		   closeCreatedPkt(pkt);
		   sprintf(buff, "Crash-Msg packed: %u:%u/%u.%u -> %u:%u/%u.%u", msg.origAddr.zone, msg.origAddr.net, msg.origAddr.node, msg.origAddr.point, msg.destAddr.zone, msg.destAddr.net, msg.destAddr.node, msg.destAddr.point);
		   writeLogEntry(log, '7', buff);

		   remove(virtualLink->bsyFile);
		   free(virtualLink->bsyFile);
		   // mark Mail as sent
		   xmsg.attr |= MSGSENT;
		   MsgWriteMsg(SQmsg, 0, &xmsg, NULL, 0, 0, 0, NULL);
		   free(virtualLink->floFile);
	   }
   } else
   
   if ((xmsg.attr & MSGHOLD) == MSGHOLD) {
	   // hold-msg -> make HUT
	   if (createOutboundFileName(virtualLink, HOLD, PKT) == 0) {
		   convertMsgText(SQmsg, &msg, msg.origAddr);
		   makePktHeader(&msg, &header);
		   pkt = openPktForAppending(virtualLink->floFile, &header);
		   writeMsgToPkt(pkt, msg);
		   closeCreatedPkt(pkt);
		   sprintf(buff, "Hold-Msg packed: %u:%u/%u.%u -> %u:%u/%u.%u", msg.origAddr.zone, msg.origAddr.net, msg.origAddr.node, msg.origAddr.point, msg.destAddr.zone, msg.destAddr.net, msg.destAddr.node, msg.destAddr.point);
		   writeLogEntry(log, '7', buff);
		   
		   remove(virtualLink->bsyFile);
		   free(virtualLink->bsyFile);
		   // mark Mail as sent
		   xmsg.attr |= MSGSENT;
		   MsgWriteMsg(SQmsg, 0, &xmsg, NULL, 0, 0, 0, NULL);
		   free(virtualLink->floFile);
	   }
   } else {
      
	   // no crash, no hold flag -> route netmail
	   route = findRouteForNetmail(msg);
	   link = getLinkForRoute(route, &msg);
	   
	   prio = cvtFlavour2Prio(route->flavour);
	   if (createOutboundFileName(link, prio, PKT) == 0) {
		   convertMsgText(SQmsg, &msg, *(link->ourAka));
		   makePktHeader(NULL, &header);
		   header.destAddr = link->hisAka;
		   header.origAddr = *(link->ourAka);
		   if (link->pktPwd != NULL) strcpy(&(header.pktPassword[0]), link->pktPwd);
		   pkt = openPktForAppending(link->floFile, &header);
		   writeMsgToPkt(pkt, msg);
		   closeCreatedPkt(pkt);
		   sprintf(buff, "Msg from %u:%u/%u.%u -> %u:%u/%u.%u via %u:%u/%u.%u", msg.origAddr.zone, msg.origAddr.net, msg.origAddr.node, msg.origAddr.point, msg.destAddr.zone, msg.destAddr.net, msg.destAddr.node, msg.destAddr.point, link->hisAka.zone, link->hisAka.net, link->hisAka.node, link->hisAka.point);
		   writeLogEntry(log, '7', buff);
		   remove(link->bsyFile);
		   free(link->bsyFile);
		   // mark Mail as sent
		   xmsg.attr |= MSGSENT;
		   MsgWriteMsg(SQmsg, 0, &xmsg, NULL, 0, 0, 0, NULL);
		   free(link->floFile);
	   }
   }

   freeMsgBuffers(&msg);
   free(virtualLink->name);
   free(virtualLink);

   if ((xmsg.attr & MSGSENT) == MSGSENT) {
	   return 0;
   } else {
	   return 1;
   }
}

void scanNMArea(void)
{
   HAREA           netmail;
   HMSG            msg;
   unsigned long   highMsg, i, j;
   XMSG            xmsg;
   s_addr          dest, orig;
   int             for_us, from_us;

   netmail = MsgOpenArea((unsigned char *) config->netMailArea.fileName, MSGAREA_NORMAL, config->netMailArea.msgbType);
   if (netmail != NULL) {

      highMsg = MsgGetHighMsg(netmail);
      writeLogEntry(log, '1', "Scanning NetmailArea");

      // scan all Messages and test if they are already sent.
      for (i=1; i<= highMsg; i++) {
         msg = MsgOpenMsg(netmail, MOPEN_RW, i);

         // msg does not exist
         if (msg == NULL) continue;
         statScan.msgs++;

         MsgReadMsg(msg, &xmsg, 0, 0, NULL, 0, NULL);
         cvtAddr(xmsg.dest, &dest);
         for_us = 0;
         for (j=0; j < config->addrCount; j++)
            if (addrComp(dest, config->addr[j])==0) {for_us = 1; break;}
                
         // if not sent and not for us -> pack it
         if (((xmsg.attr & MSGSENT) != MSGSENT) && (for_us==0)) {
			 if (packMsg(msg, xmsg) == 0) statScan.exported++;
         }

         MsgCloseMsg(msg);

         cvtAddr(xmsg.orig, &orig);
         from_us = 0;
         for (j=0; j < config->addrCount; j++)
            if (addrComp(orig, config->addr[j])==0) {from_us = 1; break;}
         
         if ((!for_us) && (!from_us)) MsgKillMsg(netmail, i);

         // kill/sent flag
         if ((xmsg.attr & MSGKILL) == MSGKILL) MsgKillMsg(netmail, i);
      } /* endfor */

      MsgCloseArea(netmail);
   } else {
      writeLogEntry(log, '9', "Could not open NetmailArea");
   } /* endif */
}

void writeScanStatToLog(void) {
   char buff[100];
   
   writeLogEntry(log, '4', "Statistics");
   sprintf(buff, "    areas: % 4d   msgs: % 6d", statScan.areas, statScan.msgs);
   writeLogEntry(log, '4', buff);
   sprintf(buff, "    exported: % 4d", statScan.exported);
   writeLogEntry(log, '4', buff);
}

void pack(void) {

   if (config->outtab != NULL) getctab(&outtab, config->outtab);
   
   memset(&statScan, 0, sizeof(s_statScan));
   writeLogEntry(log, '4', "Start packing...");
   scanNMArea();
   statScan.areas++;
   writeScanStatToLog();
}

void scan(void)
{
   UINT i;
   FILE *f;
   char *line;
   s_area *area;

   // load recoding tables
   if (config->outtab != NULL) getctab(&outtab, config->outtab);

   // zero statScan
   memset(&statScan, 0, sizeof(s_statScan));
   writeLogEntry(log,'4', "Start scanning...");

   // open echotoss file
   f = fopen(config->echotosslog, "r");

   if (f == NULL) {
      // if echotoss file does not exist scan all areas
      writeLogEntry(log, '3', "EchoTossLogFile not found -> Scanning all echoAreas.");
      for (i = 0; i< config->echoAreaCount; i++) {
         if ((config->echoAreas[i].msgbType != MSGTYPE_PASSTHROUGH) && (config->echoAreas[i].downlinkCount > 0)) {
            scanEMArea(&(config->echoAreas[i]));
            statScan.areas++;
         }
      }
   } else {
      // else scan only those areas which are listed in the file
      writeLogEntry(log, '3', "EchoTossLogFile found -> Scanning only listed areas");

      while (!feof(f)) {
         line = readLine(f);

         if (line != NULL) {
            area = getArea(config, line);
            scanEMArea(area);
            free(line);
         }
      }

      fclose(f);
      remove(config->echotosslog);
   }

   
   arcmail();
   writeScanStatToLog();
}
