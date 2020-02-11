#ifndef __GTP_RULE_H__
#define __GTP_RULE_H__

#include "utlt_debug.h"

#include "libgtp5gnl/gtp5gnl.h"

Status GtpPdrAdd(const char *ifname, struct gtp5g_pdr *pdr);
Status GtpPdrMod(const char *ifname, struct gtp5g_pdr *pdr);
Status GtpPdrDel(const char *ifname, uint32_t id);

Status GtpFarAdd(const char *ifname, struct gtp5g_far *far);
Status GtpFarMod(const char *ifname, struct gtp5g_far *far);
Status GtpFarDel(const char *ifname, uint32_t id);

#endif /* __GTP_RULE_H__ */