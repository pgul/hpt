#define PERLFILE	"filter.pl"
#define PERLFILT	"filter"
#define PERLPKT		"process_pkt"
#define PERLPKTDONE	"pkt_done"
#define PERLAFTERUNP	"after_unpack"
#define PERLBEFOREPACK	"before_pack"
#define PERLEXIT	"hpt_exit"
#define PERLROUTE	"route"
#define PERLSCAN	"scan"

int perlscanmsg(char *area, s_message *msg);
s_route *perlroute(s_message *msg);
int perlfilter(s_message *msg, s_addr pktOrigAddr, int secure);
int perlpkt(const char *fname, int secure);
void perlpktdone(const char *fname, int rc);
void perlafterunp(void);
void perlbeforepack(void);
