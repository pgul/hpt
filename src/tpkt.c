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
#include <pkt.h>
#include <typesize.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main()
{
   s_pktHeader  header;
   s_message    msg;
   FILE         *pkt;
   time_t       t;
   struct tm    *tm;

   header.origAddr.zone  = 2;
   header.origAddr.net   = 2432;
   header.origAddr.node  = 601;
   header.origAddr.point = 0;

   header.destAddr.zone  = 2;
   header.destAddr.net   = 2432;
   header.destAddr.node  = 601;
   header.destAddr.point = 29;

   header.hiProductCode  = 0;
   header.loProductCode  = 0xfe;
   header.majorProductRev = 0;
   header.minorProductRev = 24;

   //header.pktPassword[0] = 0;
   strcpy(header.pktPassword, "irkutsk");
   header.pktCreated = time(NULL);

   header.capabilityWord = 1;
   header.prodData = 0;

   pkt = createPkt("test.pkt", &header);
   if (pkt != NULL) {
      msg.origAddr.zone  = 2;
      msg.origAddr.net   = 2432;
      msg.origAddr.node  = 601;
      msg.origAddr.point = 0;

      msg.destAddr.zone  = 2;
      msg.destAddr.net   = 2432;
      msg.destAddr.node  = 601;
      msg.destAddr.point = 29;

      msg.attributes = 1;

      t = time (NULL);
      tm = gmtime(&t);
      strftime(msg.datetime, 21, "%d %b %y  %T", tm);

      msg.netMail = 1;
      msg.text = (char *) malloc(300);
      strcpy(msg.text, "\001TOPT 29\r+LinUxasd*");
      msg.toUserName = (char *) malloc(15);
      strcpy(msg.toUserName, "areafix");
      msg.fromUserName = (char *) malloc(10);
      strcpy(msg.fromUserName, "Hpt Test");
      msg.subjectLine = (char *) malloc(5);
      strcpy(msg.subjectLine, "irkutsk");
      msg.textLength = strlen(msg.text);

      writeMsgToPkt(pkt, msg);

      closeCreatedPkt(pkt);
   } else {
      printf("Could not create pkt");
   } /* endif */

   return 0;
}
