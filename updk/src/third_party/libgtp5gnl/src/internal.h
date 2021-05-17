#ifndef INTERNAL_H
#define INTERNAL_H 1

#include "config.h"
#ifdef HAVE_VISIBILITY_HIDDEN
#	define __visible	__attribute__((visibility("default")))
#	define EXPORT_SYMBOL(x)	typeof(x) (x) __visible
#else
#	define EXPORT_SYMBOL
#endif

#include <stdint.h>
#include <netinet/in.h>

struct gtp5g_dev {
    int ifns;
    uint32_t ifidx;
};

/* References:
 *	Spec:
 *		For IE Types, 3GPP TS 29.244 v16.4.0 (2020-06)
 * */
struct gtp5g_qer {
	uint32_t 	id; 					/* 8.2.75 QER_ID */
	uint8_t		ul_dl_gate;				/* 8.2.7 Gate Status */
	struct {
		uint32_t	ul_high;
		uint8_t		ul_low;
		uint32_t	dl_high;
		uint8_t		dl_low;
	} mbr;								/* 8.2.8 MBR */
	struct {
		uint32_t	ul_high;
		uint8_t		ul_low;
		uint32_t	dl_high;
		uint8_t		dl_low;
	} gbr;								/* 8.2.9 GBR */
	uint32_t		qer_corr_id;		/* 8.2.10 QER Correlation ID  */
	uint8_t			rqi;				/* 8.2.88 RQI */
	uint8_t			qfi;				/* 8.2.89 QFI */

	/* 8.2.115 Averaging Window (Optional) */

	uint8_t			ppi;				/* 8.2.116 Paging Policy Indicator */

	/* 8.2.139 Packet Rate Status */

	/* Rate Control Status Reporting */
	uint8_t			rcsr; 				/* 8.2.174 QER Control Indications */

    /* Not IEs in 3GPP Spec, for other purpose */
    int 			related_pdr_num;
    uint16_t 		*related_pdr_list;
};

struct gtp5g_outer_header_creation {
    uint16_t desp;					/* Outer Header Creation Description */
    uint32_t teid; 					/* TEID */
    struct in_addr peer_addr_ipv4; 	/* IPV4 Address */
    uint16_t port;
};

struct gtp5g_forwarding_policy {
#define MAX_LEN_OF_FORWARDING_POLICY_IDENTIFIER 0xff

    int len;
    char identifier[MAX_LEN_OF_FORWARDING_POLICY_IDENTIFIER + 1];
};

struct gtp5g_forwarding_parameter {
	//uint8_t dest_int;
	//char *network_instance;
    
    struct gtp5g_outer_header_creation *hdr_creation; /* Outer Header creation */
    struct gtp5g_forwarding_policy *fwd_policy;
};

struct gtp5g_far {
    uint32_t id;								/* FAR_ID */
    uint8_t apply_action; 						/* Apply Action */
    
    struct gtp5g_forwarding_parameter *fwd_param; /* Forwarding Parameters */

    /* Not IEs in 3GPP Spec, for other purpose */
    int related_pdr_num;
    uint16_t *related_pdr_list;
};

struct local_f_teid {
    uint32_t teid; // i_teid
    struct in_addr gtpu_addr_ipv4;
};

struct ip_filter_rule {
    uint8_t action;                   // permit only
    uint8_t direction;                  // in/out
    uint8_t proto;                      // number or "ip" which is not used for matching
    struct in_addr src, smask;          // ip addr or "any" -> 0.0.0.0
    struct in_addr dest, dmask;         // ip addr or "any" -> 0.0.0.0
    int sport_num;
    uint32_t *sport_list;               // one value, range or not existed -> [0, 0]
    int dport_num;
    uint32_t *dport_list;               // one value, range or not existed -> [0, 0]
};

struct sdf_filter {
    struct ip_filter_rule *rule;
    uint16_t *tos_traffic_class;
    uint32_t *security_param_idx;
    uint32_t *flow_label;               // exactly 3 Octets
    uint32_t *bi_id;
};

struct gtp5g_pdi {
	//uint8_t src_int;
	//char *network_instance;
    struct in_addr *ue_addr_ipv4;

	/* Local F-TEID */
    struct local_f_teid *f_teid;
    struct sdf_filter *sdf;
};

struct gtp5g_pdr {
    uint16_t id;
    uint32_t *precedence;
    struct gtp5g_pdi *pdi;

    uint8_t *outer_hdr_removal;

    uint32_t *far_id;

	uint32_t *qer_id;

    /* Not in 3GPP spec, just used for routing */
    struct in_addr *role_addr_ipv4;

    /* Not in 3GPP spec, just used for buffering */
    char *unix_sock_path;
};

#endif
