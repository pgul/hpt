/*
  AreaFix by Max Levenkov
  2:5000/117@fidonet
  ml@nsk.uland.com
*/

#include <fcommon.h>
#include <global.h>
#include <pkt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <toss.h>


int subscribeCheck(s_area area, s_message *msg) {
	int i,rc;
	
	for (i = 0; i<area.downlinkCount;i++) {
		rc = msg->origAddr.zone - area.downlinks[i]->hisAka.zone;
		rc += msg->origAddr.net - area.downlinks[i]->hisAka.net;
		rc += msg->origAddr.node - area.downlinks[i]->hisAka.node;
		rc += msg->origAddr.point - area.downlinks[i]->hisAka.point;
		if (rc==0) return 0;
	}
	
	return 1;
}

int subscribeAreaCheck(s_area area, s_message *msg, char *areaname) {
	int rc;

	if (stricmp(area.areaName,areaname)==0) {
		rc=subscribeCheck(area, msg);

		// 0 - already subscribed
		// 1 - need subscribe
		return rc;
	}

	// this is another area
	return 2;
}

int securityCheck(s_message *msg, s_addr *pktOrigAddr) {
	
	int rc;

	rc = pktOrigAddr->zone - msg->origAddr.zone;
	rc += pktOrigAddr->net - msg->origAddr.net;
	rc += pktOrigAddr->node - msg->origAddr.node;
	rc += pktOrigAddr->point - msg->origAddr.point;

	return rc;
}

int for_me_Check (s_message *msg, s_link *link) {

	int rc;
	s_addr *ourAka;

	ourAka=link->ourAka;

	rc = msg->destAddr.zone - ourAka->zone;
	rc += msg->destAddr.net - ourAka->net;
	rc += msg->destAddr.node - ourAka->node;
	rc += msg->destAddr.point - ourAka->point;

	return rc;
}

int addAka(FILE *f, s_link *link) {
	char straka[20], *cfg;
	long areapos,endpos,cfglen;
	
	if (link->hisAka.point!=0) {
		sprintf(straka," %u:%u/%u.%u",link->hisAka.zone,link->hisAka.net,link->hisAka.node,link->hisAka.point);
	}
	else sprintf(straka," %u:%u/%u",link->hisAka.zone,link->hisAka.net,link->hisAka.node);

	//current position
	fseek(f,-1,SEEK_CUR);
	areapos=ftell(f);
	
	// end of file
	fseek(f,0l,SEEK_END);
	endpos=ftell(f);
	cfglen=endpos-areapos;
	
	// storing end of file...
	cfg=(char *) calloc(cfglen,sizeof(char*));
	fseek(f,-cfglen,SEEK_END);
	fread(cfg,sizeof(char*),cfglen,f);
	
	// write config
	fseek(f,-cfglen,SEEK_END);
	fputs(straka,f);
	fputs(cfg,f);
	
	free(cfg);
	return 0;
}

int delAka(FILE *f, s_link *link) {
	int al,i=1;
	char straka[20], *cfg, c;
	long areapos,endpos,cfglen;
	
	if (link->hisAka.point!=0) {
		sprintf(straka,"%u:%u/%u.%u",link->hisAka.zone,link->hisAka.net,link->hisAka.node,link->hisAka.point);
	} else
		sprintf(straka,"%u:%u/%u",link->hisAka.zone,link->hisAka.net,link->hisAka.node);
	
	al=strlen(straka);

	// search for the aka string
	while (i!=0) {
		for (i=al; i>0; i--) {
			fseek(f,-2,SEEK_CUR);
			c=fgetc(f);
			if (straka[i-1]!=c) break;
		}
	}
	
	//current position
	areapos=ftell(f);

	// end of file
	fseek(f,0l,SEEK_END);
	endpos=ftell(f);
	cfglen=endpos-areapos-al;
	
	// storing end of file...
	cfg=(char *) calloc(cfglen,sizeof(char*));
	fseek(f,-cfglen-1,SEEK_END);
	fread(cfg,sizeof(char*),cfglen+1,f);
	
	// write config
	fseek(f,-cfglen-al-2,SEEK_END);
	fputs(cfg,f);

	truncate(getConfigFileName(),endpos-al-1);
	
	free(cfg);
	return 0;
}

void removelink (s_link *link, s_area *area) {
	int i, rc;
	s_link *links;

	for (i=0; i < area->downlinkCount; i++) {
		links = area->downlinks[i];
		rc = link->hisAka.zone - links->hisAka.zone;
		rc += link->hisAka.net - links->hisAka.net;
		rc += link->hisAka.node - links->hisAka.node;
		rc += link->hisAka.point - links->hisAka.point;
		if (rc==0) break;
	}
	
	area->downlinks[i] = area->downlinks[area->downlinkCount-1];
	area->downlinkCount--;
}

char *list(s_message *msg, s_link *link) {

	int i,n,rc;
	char *report, addline[256];

	report=(char*) calloc(1,sizeof(char*));

	for (i=0,n=0; i< config->echoAreaCount; i++) {
		report=(char*) realloc(report, strlen(report)+
							   strlen(config->echoAreas[i].areaName)+3);
		rc=subscribeCheck(config->echoAreas[i],msg);
		if (!rc) {
			strcat(report,"*");
			n++;
		} else strcat(report," ");
		strcat(report,config->echoAreas[i].areaName);
		strcat(report,"\r");
	}
	
	sprintf(addline,"\r ---\r\r %i areas of %i\r\r",n,config->echoAreaCount);
	report=(char*) realloc(report, strlen(report)+strlen(addline)+1);
	strcat(report, addline);

	sprintf(addline,"areafix: list sent to %s",link->name);
	writeLogEntry(log, '8', addline);

	return report;
}


char *help(s_link *link) {
	FILE *f;
	int i=1;
	char *help, addline[256];
	long endpos;

	if (config->areafixhelp!=NULL) {
		if ((f=fopen(config->areafixhelp,"r")) == NULL)
			{
				fprintf(stderr,"areafix: cannot open help file \"%s\"\n",
						config->areafixhelp);
				return NULL;
			}
		
		fseek(f,0l,SEEK_END);
		endpos=ftell(f);
		
		help=(char*) calloc(endpos,sizeof(char*));

		fseek(f,0l,SEEK_SET);
		fread(help,1,endpos,f);
		
		for (i=0; i<endpos; i++) if (help[i]=='\n') help[i]='\r';

		fclose(f);

		sprintf(addline,"areafix: help sent to %s",link->name);
		writeLogEntry(log, '8', addline);

		return help;
	}

	return NULL;
}

int changeconfig(char *line, s_link *link, int i) {
	FILE *f;
	char *cfgline;
	
	if ((f=fopen(getConfigFileName(),"r+")) == NULL)
		{
			fprintf(stderr,"areafix: cannot open config file\n");
			return 1;
		}
	
	while ((cfgline = readLine(f))) {
		cfgline = trimLine(cfgline);
		if ((cfgline[0] != '#') && (cfgline[0] != 0)) {
			cfgline+=9;
			if (!strncasecmp(cfgline,line,strlen(line))) {
				cfgline-=9;
				free(cfgline);
				if (i) delAka(f,link); else addAka(f,link);
			}
		} else free(cfgline);
	}

	fclose(f);
	return 0;
}

void changeHeader(s_message *msg, s_link *link) {
	s_addr *ourAka;
	char *subject, *toname;
	
	ourAka=link->ourAka;
	
	msg->destAddr.zone =  link->hisAka.zone;
	msg->destAddr.net = link->hisAka.net;
	msg->destAddr.node = link->hisAka.node;
	msg->destAddr.point = link->hisAka.point;

	msg->origAddr.zone = ourAka->zone;
	msg->origAddr.net = ourAka->net;
	msg->origAddr.node = ourAka->node;
	msg->origAddr.point = ourAka->point;
	
	subject=(char *) calloc(15,sizeof(char*));
	strcpy(subject,"areafix report");
	free(msg->subjectLine);
	msg->subjectLine=subject;
	toname=msg->fromUserName;
	msg->fromUserName=msg->toUserName;
	msg->toUserName=toname;
}

int strip(char *text) {
	char *str;
	str = strchr( text, (int) '\r' );
	return( str!=NULL ? (int)(str-text)+1 : strlen(text) );
}

char *subscribe(s_link *link, s_message *msg, char *line) {
	int i, rc=2;
	char *report, addline[256], logmsg[256];
	s_area *area;

	if (line[0]=='+') line++;

	report=(char*) calloc(1,sizeof(char*));
	
	for (i = 0; i< config->echoAreaCount; i++) {
		rc=subscribeAreaCheck(config->echoAreas[i],msg,line);
		if ( rc==0 || rc ==1 ) break;
	}
	
	switch (rc) {
	case 0: sprintf(addline,"you are already linked to area %s\r",line);
		break;
	case 2:	sprintf(addline,"no area '%s' in my config\r",line);
		break;
	case 1:	changeconfig (line, link, 0);
		area = &(config->echoAreas[i]);
		area->downlinks = realloc(area->downlinks, sizeof(s_link*)*(area->downlinkCount+1));
		area->downlinks[area->downlinkCount] = link;
		area->downlinkCount++;
		sprintf(addline,"area %s subscribed\r",line);
		sprintf(logmsg,"areafix: %s subscribed to %s",link->name,line);
		writeLogEntry(log, '8', logmsg);
		break;
	}

	report=(char*) realloc(report, strlen(report)+strlen(addline)+1);
	strcpy(report, addline);
	return report;
}

char *unsubscribe(s_link *link, s_message *msg, char *line) {
	int i, rc = 2;
	char *report, addline[256], logmsg[256];
	s_area *area;

	if (line[1]=='-') return NULL;
	line++;

	report=(char*) calloc(1,sizeof(char*));
	
	for (i = 0; i< config->echoAreaCount; i++) {
		rc=subscribeAreaCheck(config->echoAreas[i],msg,line);
		if ( rc==0 || rc ==1 ) break;
	}
	
	switch (rc) {
	case 1: sprintf(addline,"you are dont't subscribed to area %s\r",line);
		break;
	case 2: sprintf(addline,"no area '%s' in my config\r",line);
		break;
	case 0: changeconfig ( line, link, 1);
		area = &(config->echoAreas[i]);
		removelink(link, area);
		sprintf(addline,"area %s unsubscribed\r",line);
		sprintf(logmsg,"areafix: %s unsubscribed from %s",link->name,line);
		writeLogEntry(log, '8', logmsg);

		break;
	}

	report=(char*) realloc(report, strlen(report)+strlen(addline)+1);
	strcpy(report, addline);
	return report;
}

int tellcmd(char *line) {

	switch (line[0]) {
	case '%': 
		line++;
		if (stricmp(line,"list")==0) return 1;
		if (stricmp(line,"help")==0) return 2;
		break;
	case '\01': return 0;
		break;
	case '-'  : return 4;
		break;
	default: return 3;
		break;
        }
        return 0;
}

char *processcmd(s_link *link, s_message *msg, char *line, int cmd) {

	char *report;

	switch (cmd) {
	case 1: report = list (msg, link);
		break;
	case 2:	report = help (link);
		break;
	case 3: report = subscribe (link,msg,line);
		break;
	case 4: report = unsubscribe (link,msg,line);
		break;
	default: return NULL;
	}

	return report;
}


/* Note:
 * This is a simply msgid without any hash function...
 * Imho it is not necessary to create better msgid for this purpose.
 */

void createmsgid(char *msgid, s_link *link) {
	s_addr *ourAka;

	ourAka=link->ourAka;

	if (ourAka->point)
		sprintf(msgid,"\1MSGID: %u:%u/%u.%u %08lx\r",
				ourAka->zone,ourAka->net,ourAka->node,ourAka->point,time(NULL));
	else
		sprintf(msgid,"\1MSGID: %u:%u/%u %08lx\r",
				ourAka->zone,ourAka->net,ourAka->node,time(NULL));
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


int processAreaFix(s_message *msg, s_addr *pktOrigAddr)
{
	int i, rc, security=1, forme = 0;
	s_link *link = NULL;
	char *tmp, *textBuff, *report, *preport, logmsg[256];
	INT32 textlength;

	// 1st security check
	security=securityCheck(msg, pktOrigAddr);
		
	// find link
	for (i=0,rc=0; i< config->linkCount; i++) {
		rc = msg->origAddr.zone - config->links[i].hisAka.zone;
		rc += msg->origAddr.net - config->links[i].hisAka.net;
		rc += msg->origAddr.node - config->links[i].hisAka.node;
		rc += msg->origAddr.point - config->links[i].hisAka.point;
		if (rc == 0) {
			link=&(config->links[i]);
		}
	}

	//this is for me?
	if (link!=NULL)	forme=for_me_Check(msg, link);
	if (forme || link==NULL) {
		processNMMsg(msg);
		return 0;
	}

	// 2nd security ckeck. link,araefixing & password.
	if (!security) {
		if (link != NULL) {
			if (link->AreaFix==1) {
				if (link->areaFixPwd!=NULL) {
					if (stricmp(link->areaFixPwd,msg->subjectLine)==0) security=0;
				} else security=1;
			} else security=1;
		} else security=1;
	}
	
	tmp=(char*) calloc(128,sizeof(char*));
	createmsgid (tmp,link);
	report=(char*) calloc(strlen(tmp)+1,sizeof(char*));
	strcpy(report,tmp);
	
	if (!security) {
		
		textBuff=(char *) calloc(strlen(msg->text),sizeof(char*));
		strcpy(textBuff,msg->text);
		
		i=strip(textBuff);
		tmp=strtok(textBuff,"\n\r\t");
		textBuff+=i;
		
		while(tmp != NULL) {
			i = strip(textBuff);
			tmp = strtok(textBuff,"\n\r\t");
			textBuff += i;
			if (tmp != NULL) {
				preport = processcmd( link, msg, tmp, tellcmd(tmp) );
				if (preport!=NULL) {
					report=(char*) realloc(report,strlen(report)+strlen(preport)+1);
					strcat(report,preport);
					free(preport);
				}
			}
		}

		tmp=(char*) realloc(tmp,80*sizeof(char*));
		sprintf(tmp, " \r--- hpt areafix\r");
		report=(char*) realloc(report,strlen(report)+strlen(tmp)+1);
		strcat(report,tmp);

	} else {
		
		tmp=(char*) realloc(tmp,80*sizeof(char*));
		sprintf(tmp, " \r security violation!\r\r--- hpt areafix\r");
		report=(char*) realloc(report,strlen(report)+strlen(tmp)+1);
		strcat(report,tmp);
	}
	
	if (link!=NULL) {
		textlength=strlen(report);
		msg->textLength=(int)textlength;
		free(msg->text);
		msg->text=report;
		changeHeader(msg,link);
	}

	processNMMsg(msg);

	sprintf(logmsg,"areafix: sucessfully done for %s",link->name);
	writeLogEntry(log, '8', logmsg);
	
	free(tmp);
	return 0;
}
