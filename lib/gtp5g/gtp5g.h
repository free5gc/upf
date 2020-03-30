/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _GTP_H_
#define _GTP_H_

/* General GTP protocol related definitions. */

#define GTP1U_PORT	2152

#define GTP_TPDU	255

struct gtp1_header {	/* According to 3GPP TS 29.060. */
	__u8	flags;
	__u8	type;
	__be16	length;
	__be32	tid;
} __attribute__ ((packed));

#define GTP1_F_NPDU	0x01
#define GTP1_F_SEQ	0x02
#define GTP1_F_EXTHDR	0x04
#define GTP1_F_MASK	0x07

/* Maybe add this part to if_link.h */
enum ifla_gtp5g_role {
    GTP5G_ROLE_UPF = 0,
    GTP5G_ROLE_RAN,
};

enum {
    IFLA_GTP5G_UNSPEC,

    IFLA_GTP5G_FD1,
    IFLA_GTP5G_PDR_HASHSIZE,
    IFLA_GTP5G_ROLE,

    __IFLA_GTP5G_MAX,
};
#define IFLA_GTP5G_MAX (__IFLA_GTP5G_MAX - 1)
/* end of part */

enum gtp5g_cmd {
    GTP5G_CMD_UNSPEC = 0,

    GTP5G_CMD_ADD_PDR,
    GTP5G_CMD_ADD_FAR,

    GTP5G_CMD_DEL_PDR,
    GTP5G_CMD_DEL_FAR,

    GTP5G_CMD_GET_PDR,
    GTP5G_CMD_GET_FAR,

    __GTP5G_CMD_MAX,
};
#define GTP5G_CMD_MAX (__GTP5G_CMD_MAX - 1)

/* This const value need to bigger than the Layer 1 attr size,
 * like GTP5G_PDR_ATTR_MAX and GTP5G_FAR_ATTR_MAX
 */
#define GTP5G_ATTR_MAX 0x10

enum gtp5g_device_attrs {
    GTP5G_LINK = 1,
    GTP5G_NET_NS_FD,
};

enum gtp5g_pdr_attrs {
    /* gtp5g_device_attrs in this part */

    GTP5G_PDR_ID = 3,
    GTP5G_PDR_PRECEDENCE,
    GTP5G_PDR_PDI,
    GTP5G_OUTER_HEADER_REMOVAL,
    GTP5G_PDR_FAR_ID,

    /* Not in 3GPP spec, just used for routing */
    GTP5G_PDR_ROLE_ADDR_IPV4,

    /* Not in 3GPP spec, just used for buffering */
    GTP5G_PDR_UNIX_SOCKET_PATH,

    __GTP5G_PDR_ATTR_MAX,
};
#define GTP5G_PDR_ATTR_MAX (__GTP5G_PDR_ATTR_MAX - 1)

/* Nest in GTP5G_PDR_PDI */
enum gtp5g_pdi_attrs {
    GTP5G_PDI_UE_ADDR_IPV4 = 1,
    GTP5G_PDI_F_TEID,
    GTP5G_PDI_SDF_FILTER,

    __GTP5G_PDI_ATTR_MAX,
};
#define GTP5G_PDI_ATTR_MAX (__GTP5G_PDI_ATTR_MAX - 1)

/* Nest in GTP5G_PDI_F_TEID */
enum gtp5g_f_teid_attrs {
    GTP5G_F_TEID_I_TEID = 1,
    GTP5G_F_TEID_GTPU_ADDR_IPV4,

    __GTP5G_F_TEID_ATTR_MAX,
};
#define GTP5G_F_TEID_ATTR_MAX (__GTP5G_F_TEID_ATTR_MAX - 1)

/* Nest in GTP5G_PDI_SDF_FILTER */
enum gtp5g_sdf_filter_attrs {
    GTP5G_SDF_FILTER_FLOW_DESCRIPTION = 1,
    GTP5G_SDF_FILTER_TOS_TRAFFIC_CLASS,
    GTP5G_SDF_FILTER_SECURITY_PARAMETER_INDEX,
    GTP5G_SDF_FILTER_FLOW_LABEL,
    GTP5G_SDF_FILTER_SDF_FILTER_ID,

    __GTP5G_SDF_FILTER_ATTR_MAX,
};
#define GTP5G_SDF_FILTER_ATTR_MAX (__GTP5G_SDF_FILTER_ATTR_MAX - 1)

/* Nest in GTP5G_SDF_FILTER_FLOW_DESCRIPTION */
enum gtp5g_flow_description_attrs {
    GTP5G_FLOW_DESCRIPTION_ACTION = 1, // Only "permit"
    GTP5G_FLOW_DESCRIPTION_DIRECTION,
    GTP5G_FLOW_DESCRIPTION_PROTOCOL,
    GTP5G_FLOW_DESCRIPTION_SRC_IPV4,
    GTP5G_FLOW_DESCRIPTION_SRC_MASK,
    GTP5G_FLOW_DESCRIPTION_DEST_IPV4,
    GTP5G_FLOW_DESCRIPTION_DEST_MASK,
    GTP5G_FLOW_DESCRIPTION_SRC_PORT,
    GTP5G_FLOW_DESCRIPTION_DEST_PORT,

    __GTP5G_FLOW_DESCRIPTION_ATTR_MAX,
};
#define GTP5G_FLOW_DESCRIPTION_ATTR_MAX (__GTP5G_FLOW_DESCRIPTION_ATTR_MAX - 1)

enum gtp5g_far_attrs {
    /* gtp5g_device_attrs in this part */

    GTP5G_FAR_ID = 3,
    GTP5G_FAR_APPLY_ACTION,
    GTP5G_FAR_FORWARDING_PARAMETER,

    /* Not IEs in 3GPP Spec, for other purpose */
    GTP5G_FAR_RELATED_TO_PDR,

    __GTP5G_FAR_ATTR_MAX,
};
#define GTP5G_FAR_ATTR_MAX (__GTP5G_FAR_ATTR_MAX - 1)

#define FAR_ACTION_UPSPEC 0x00
#define FAR_ACTION_DROP   0x01
#define FAR_ACTION_FORW   0x02
#define FAR_ACTION_BUFF   0x04
#define FAR_ACTION_MASK   0x07
#define FAR_ACTION_NOCP   0x08
#define FAR_ACTION_DUPL   0x10

/* Nest in GTP5G_FAR_FORWARDING_PARAMETER */
enum gtp5g_forwarding_parameter_attrs {
    GTP5G_FORWARDING_PARAMETER_OUTER_HEADER_CREATION = 1,

    __GTP5G_FORWARDING_PARAMETER_ATTR_MAX,
};
#define GTP5G_FORWARDING_PARAMETER_ATTR_MAX (__GTP5G_FORWARDING_PARAMETER_ATTR_MAX - 1)

/* Nest in GTP5G_FORWARDING_PARAMETER_OUTER_HEADER_CREATION */
enum gtp5g_outer_header_creation_attrs {
    GTP5G_OUTER_HEADER_CREATION_DESCRIPTION = 1,
    GTP5G_OUTER_HEADER_CREATION_O_TEID,
    GTP5G_OUTER_HEADER_CREATION_PEER_ADDR_IPV4,
    GTP5G_OUTER_HEADER_CREATION_PORT,

    __GTP5G_OUTER_HEADER_CREATION_ATTR_MAX,
};
#define GTP5G_OUTER_HEADER_CREATION_ATTR_MAX (__GTP5G_OUTER_HEADER_CREATION_ATTR_MAX - 1)

enum {
    GTP5G_SDF_FILTER_ACTION_UNSPEC = 0,

    GTP5G_SDF_FILTER_PERMIT,

    __GTP5G_SDF_FILTER_ACTION_MAX,
};
#define GTP5G_SDF_FILTER_ACTION_MAX (__GTP5G_SDF_FILTER_ACTION_MAX - 1)

enum {
    GTP5G_SDF_FILTER_DIRECTION_UNSPEC = 0,

    GTP5G_SDF_FILTER_IN,
    GTP5G_SDF_FILTER_OUT,

    __GTP5G_SDF_FILTER_DIRECTION_MAX,
};
#define GTP5G_SDF_FILTER_DIRECTION_MAX (__GTP5G_SDF_FILTER_DIRECTION_MAX - 1)

#endif