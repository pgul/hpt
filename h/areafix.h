/* $Id$ */

#ifndef _AREAFIX_H
#define _AREAFIX_H

#include <fcommon.h>

#define NOTHING 0
#define LIST    1
#define HELP    2
#define ADD     3
#define DEL     4
#define AVAIL   5
#define QUERY   6
#define UNLINK  7
#define PAUSE   8
#define RESUME  9
#define INFO    10
#define RESCAN  11
#define REMOVE  12
#define ADD_RSC 13
#define PACKER  14
#define RSB     15
#define DONE    100
#define STAT    101
#define AFERROR   255

char *print_ch(int len, char ch);
int processAreaFix(s_message *msg, s_pktHeader *pktHeader, unsigned force_pwd);
void afix(hs_addr addr, char *cmd);
void autoPassive(void);
int relink (char *straddr);
char *rescan(s_link *link, char *cmd);
char *errorRQ(char *line);
int isPatternLine(char *s);
void makeMsgToSysop(char *areaName, hs_addr fromAddr, hs_addr *uplinkAddr);

/*  forwardRequest()
    Forward request to areatag. Request initiated by dwlink.
    lastRlink is pointer to last requested link in uplinks array, uses for
    repeat calls of forwardRequest() (uplinks array is sorted by
    forward-priority of link). lastRlink may be NULL
    Return values:
    0 - request is forwarded
    1 - echo already requested (link is queued to echo-subcribing) or action isn't required
    2 - request is not forwarded (deny area-link, not found, ...)
*/
int forwardRequest(char *areatag, s_link *dwlink, s_link **lastRlink);

int forwardRequestToLink (char *areatag, s_link *uplink, s_link *dwlink, int act);
void sendAreafixMessages();
char *do_delete(s_link *link, s_area *area);

/* test area-link pair to mandatory
   return 1 if area is mandatory for link, 0 otherwize
 */
int mandatoryCheck(s_area area, s_link *link);

#endif
