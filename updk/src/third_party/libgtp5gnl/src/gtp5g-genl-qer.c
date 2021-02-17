/* GTP specific Generic Netlink helper functions */

/* (C) 2014 by sysmocom - s.f.m.c. GmbH
 * (C) 2017 by Pablo Neira Ayuso <pablo@gnumonks.org>

 * Author: Yao-Wen Chang <yaowenowo@gmail.com>
 *			Muthuraman Elangovan <muthuramane.cs03g@g2.nctu.edu.tw>	
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include <libgtp5gnl/gtp5g.h>
#include <libgtp5gnl/gtp5gnl.h>

#include <net/if.h>
#include <linux/gtp5g.h>
#include <linux/if_link.h>

#include "internal.h"
#include "tools.h"

static void gtp5g_build_qer_payload(struct nlmsghdr *nlh, struct gtp5g_dev *dev, struct gtp5g_qer *qer)
{
	struct nlattr *mbr_nest;
	struct nlattr *gbr_nest;

    // Let kernel get dev easily
    if (dev->ifns >= 0)
        mnl_attr_put_u32(nlh, GTP5G_NET_NS_FD, dev->ifns);

	// Level 0 GTP5G
    mnl_attr_put_u32(nlh, GTP5G_LINK, dev->ifidx);

    // Level 1 QER
    mnl_attr_put_u32(nlh, GTP5G_QER_ID, qer->id);
    mnl_attr_put_u8(nlh, GTP5G_QER_GATE, qer->ul_dl_gate);
	
	//Level 2 MBR 
	mbr_nest = mnl_attr_nest_start(nlh, GTP5G_QER_MBR);
	mnl_attr_put_u32(nlh, GTP5G_QER_MBR_UL_HIGH32,  qer->mbr.ul_high);
	mnl_attr_put_u8(nlh, GTP5G_QER_MBR_UL_LOW8, qer->mbr.ul_low);
	mnl_attr_put_u32(nlh, GTP5G_QER_MBR_DL_HIGH32,  qer->mbr.dl_high);
	mnl_attr_put_u8(nlh, GTP5G_QER_MBR_DL_LOW8, qer->mbr.dl_low);
	mnl_attr_nest_end(nlh, mbr_nest);

	//Level 2 GBR 
	gbr_nest = mnl_attr_nest_start(nlh, GTP5G_QER_GBR);
	mnl_attr_put_u32(nlh, GTP5G_QER_GBR_UL_HIGH32,  qer->gbr.ul_high);
	mnl_attr_put_u8(nlh, GTP5G_QER_GBR_UL_LOW8, qer->gbr.ul_low);
	mnl_attr_put_u32(nlh, GTP5G_QER_GBR_DL_HIGH32,  qer->gbr.dl_high);
	mnl_attr_put_u8(nlh, GTP5G_QER_GBR_DL_LOW8, qer->gbr.dl_low);
	mnl_attr_nest_end(nlh, gbr_nest);

    mnl_attr_put_u32(nlh, GTP5G_QER_CORR_ID, qer->qer_corr_id);
    mnl_attr_put_u8(nlh, GTP5G_QER_RQI, qer->rqi);
    mnl_attr_put_u8(nlh, GTP5G_QER_QFI, qer->qfi);
    mnl_attr_put_u8(nlh, GTP5G_QER_PPI, qer->ppi);
    mnl_attr_put_u8(nlh, GTP5G_QER_RCSR, qer->rcsr);
}

int gtp5g_add_qer(int genl_id, struct mnl_socket *nl, struct gtp5g_dev *dev, struct gtp5g_qer *qer)
{
    struct nlmsghdr *nlh;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    uint32_t seq = time(NULL);

    if (!dev) {
        fprintf(stderr, "5G GTP device is NULL\n");
        return -1;
    }

    nlh = genl_nlmsg_build_hdr(buf, 
								genl_id, 
								NLM_F_EXCL | NLM_F_ACK, 
								++seq,
                               	GTP5G_CMD_ADD_QER);
	if (!nlh) {
        fprintf(stderr, "%s Netlink Msg header is NULL\n", __func__);
        return -1;
	}

    gtp5g_build_qer_payload(nlh, dev, qer);

    if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0) {
        perror("genl_socket_talk");
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(gtp5g_add_qer);

int gtp5g_mod_qer(int genl_id, struct mnl_socket *nl, struct gtp5g_dev *dev, struct gtp5g_qer *qer)
{
    struct nlmsghdr *nlh;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    uint32_t seq = time(NULL);

    if (!dev) {
        fprintf(stderr, "5G GTP device is NULL\n");
        return -1;
    }

    nlh = genl_nlmsg_build_hdr(buf, 
								genl_id, 
								NLM_F_REPLACE | NLM_F_ACK, 
								++seq,
                               	GTP5G_CMD_ADD_QER);

    gtp5g_build_qer_payload(nlh, dev, qer);

    if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0) {
        perror("genl_socket_talk");
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(gtp5g_mod_qer);


int gtp5g_del_qer(int genl_id, struct mnl_socket *nl, struct gtp5g_dev *dev, struct gtp5g_qer *qer)
{
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    uint32_t seq = time(NULL);

    if (!dev) {
        fprintf(stderr, "5G GTP device is NULL\n");
        return -1;
    }

    nlh = genl_nlmsg_build_hdr(buf, 
								genl_id, 
								NLM_F_ACK, 
								++seq,
                               	GTP5G_CMD_DEL_QER);

    gtp5g_build_qer_payload(nlh, dev, qer);

    if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0) {
        perror("genl_socket_talk");
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(gtp5g_del_qer);

static int genl_gtp5g_qer_validate_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

    if (mnl_attr_type_valid(attr, GTP5G_QER_ATTR_MAX) < 0)
        return MNL_CB_OK;

    switch (type) {
    case GTP5G_QER_ID:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_GATE:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_MBR:
    	if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_GBR:
    	if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_CORR_ID:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_RQI:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_QFI:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_PPI:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_RCSR:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_RELATED_TO_PDR:
		break;
    default:
		printf("%s: Unknown type type(%#x)\n", __func__, type);
        break;
    }

    tb[type] = attr;
    return MNL_CB_OK;

VALIDATE_FAIL:
    perror("QER mnl_attr_validate");
    return MNL_CB_ERROR;
}

static int genl_gtp5g_mbr_validate_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, GTP5G_QER_MBR_ATTR_MAX) < 0)
        return MNL_CB_OK;

 	switch (type) {
    case GTP5G_QER_MBR_UL_HIGH32:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_MBR_UL_LOW8:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_MBR_DL_HIGH32:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_MBR_DL_LOW8:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    default:
        break;
    }

    tb[type] = attr;
    return MNL_CB_OK;

VALIDATE_FAIL:
    perror("QER mnl_attr_validate");
    return MNL_CB_ERROR;
}

static int genl_gtp5g_gbr_validate_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, GTP5G_QER_GBR_ATTR_MAX) < 0)
        return MNL_CB_OK;

 	switch (type) {
    case GTP5G_QER_GBR_UL_HIGH32:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_GBR_UL_LOW8:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_GBR_DL_HIGH32:
    	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
        	goto VALIDATE_FAIL;
        break;
    case GTP5G_QER_GBR_DL_LOW8:
    	if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
        	goto VALIDATE_FAIL;
        break;
    default:
        break;
    }

    tb[type] = attr;
    return MNL_CB_OK;

VALIDATE_FAIL:
    perror("QER mnl_attr_validate");
    return MNL_CB_ERROR;
}

static int genl_gtp5g_qer_attr_list_cb(const struct nlmsghdr *nlh, void *data)
{
    struct nlattr *qer_tb[GTP5G_QER_ATTR_MAX + 1] = {};
    struct nlattr *mbr_tb[GTP5G_QER_MBR_ATTR_MAX + 1] = {};
    struct nlattr *gbr_tb[GTP5G_QER_GBR_ATTR_MAX + 1] = {};
    struct genlmsghdr *genl;

    mnl_attr_parse(nlh, 
					sizeof(*genl), 
					genl_gtp5g_qer_validate_cb, 
					qer_tb);

    if (qer_tb[GTP5G_QER_ID])
        printf("[QER ID: %u]\n", mnl_attr_get_u32(qer_tb[GTP5G_QER_ID]));

    if (qer_tb[GTP5G_QER_GATE])
        printf("\t Gate Status: %u\n", mnl_attr_get_u8(qer_tb[GTP5G_QER_GATE]));

    if (qer_tb[GTP5G_QER_MBR]) {
        mnl_attr_parse_nested(qer_tb[GTP5G_QER_MBR], 
				genl_gtp5g_mbr_validate_cb, 
				mbr_tb);
        printf("\t MBR Parameter Info\n");

		if (mbr_tb[GTP5G_QER_MBR_UL_HIGH32])
			printf("\t\t UL High: %u\n", mnl_attr_get_u32(mbr_tb[GTP5G_QER_MBR_UL_HIGH32]));

		if (mbr_tb[GTP5G_QER_MBR_UL_LOW8])
			printf("\t\t UL Low: %u\n", mnl_attr_get_u8(mbr_tb[GTP5G_QER_MBR_UL_LOW8]));

		if (mbr_tb[GTP5G_QER_MBR_DL_HIGH32])
			printf("\t\t DL High: %u\n", mnl_attr_get_u32(mbr_tb[GTP5G_QER_MBR_DL_HIGH32]));

		if (mbr_tb[GTP5G_QER_MBR_DL_LOW8])
			printf("\t\t DL Low: %u\n", mnl_attr_get_u8(mbr_tb[GTP5G_QER_MBR_DL_LOW8]));
	}

    if (qer_tb[GTP5G_QER_GBR]) {
        mnl_attr_parse_nested(qer_tb[GTP5G_QER_GBR], 
				genl_gtp5g_gbr_validate_cb, 
				gbr_tb);
        printf("\t GBR Parameter Info\n");

		if (gbr_tb[GTP5G_QER_GBR_UL_HIGH32])
			printf("\t\t UL High: %u\n", mnl_attr_get_u32(gbr_tb[GTP5G_QER_GBR_UL_HIGH32]));

		if (gbr_tb[GTP5G_QER_GBR_UL_LOW8])
			printf("\t\t UL Low: %u\n", mnl_attr_get_u8(gbr_tb[GTP5G_QER_GBR_UL_LOW8]));

		if (gbr_tb[GTP5G_QER_GBR_DL_HIGH32])
			printf("\t\t DL High: %u\n", mnl_attr_get_u32(gbr_tb[GTP5G_QER_GBR_DL_HIGH32]));

		if (gbr_tb[GTP5G_QER_GBR_DL_LOW8])
			printf("\t\t DL Low: %u\n", mnl_attr_get_u8(gbr_tb[GTP5G_QER_GBR_DL_LOW8]));
	}

    if (qer_tb[GTP5G_QER_CORR_ID])
        printf("\t Correlation ID: %u\n", mnl_attr_get_u32(qer_tb[GTP5G_QER_CORR_ID]));

    if (qer_tb[GTP5G_QER_RQI])
        printf("\t RQI: %u\n", mnl_attr_get_u8(qer_tb[GTP5G_QER_RQI]));

    if (qer_tb[GTP5G_QER_QFI])
        printf("\t QFI: %u\n", mnl_attr_get_u8(qer_tb[GTP5G_QER_QFI]));

    if (qer_tb[GTP5G_QER_PPI])
        printf("\t PPI: %u\n", mnl_attr_get_u8(qer_tb[GTP5G_QER_PPI]));

    if (qer_tb[GTP5G_QER_RCSR])
        printf("\t RCSR: %u\n", mnl_attr_get_u8(qer_tb[GTP5G_QER_RCSR]));

    if (qer_tb[GTP5G_QER_RELATED_TO_PDR]) {
        printf("\t Related PDR ID: ");
        u16_id_list_from_kernel_space_print(mnl_attr_get_payload(qer_tb[GTP5G_QER_RELATED_TO_PDR]),
            mnl_attr_get_payload_len(qer_tb[GTP5G_QER_RELATED_TO_PDR]) / (sizeof(uint16_t) / sizeof(char)));
        printf(" (Not a real IE)\n");
    }

    return MNL_CB_OK;
}

int gtp5g_list_qer(int genl_id, struct mnl_socket *nl)
{
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    uint32_t seq = time(NULL);

    nlh = genl_nlmsg_build_hdr(buf, 
								genl_id, 
								NLM_F_DUMP, 
								0,
                               	GTP5G_CMD_GET_QER);

    if (genl_socket_talk(nl, nlh, seq, genl_gtp5g_qer_attr_list_cb, NULL) < 0) {
        perror("genl_socket_talk");
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(gtp5g_list_qer);

void gtp5g_print_qer(struct gtp5g_qer *qer)
{
    if (!qer) {
        perror("QER is NULL");
        return;
    }

    printf("[QER No.%u Info]\n", qer->id);
    if (qer->related_pdr_num && qer->related_pdr_list) {
        printf("\t Related PDR ID: ");
        u16_id_list_from_kernel_space_print(qer->related_pdr_list, qer->related_pdr_num);
        printf(" (Not a real IE)\n");
    }
}
EXPORT_SYMBOL(gtp5g_print_qer);

static int genl_gtp5g_attr_cb(const struct nlmsghdr *nlh, void *data)
{
    struct nlattr *qer_tb[GTP5G_QER_ATTR_MAX + 1] = {};
    struct genlmsghdr *genl;
    struct gtp5g_qer *qer;

    mnl_attr_parse(nlh, sizeof(*genl), genl_gtp5g_qer_validate_cb, qer_tb);
    qer = *(struct gtp5g_qer **) data = gtp5g_qer_alloc();

	if (qer_tb[GTP5G_QER_ID])
        gtp5g_qer_set_id(qer, mnl_attr_get_u32(qer_tb[GTP5G_QER_ID]));

	if (qer_tb[GTP5G_QER_RELATED_TO_PDR]) {
        qer->related_pdr_num = mnl_attr_get_payload_len(qer_tb[GTP5G_QER_RELATED_TO_PDR]) / (sizeof(uint16_t) / sizeof(char));
        qer->related_pdr_list = calloc(1, mnl_attr_get_payload_len(qer_tb[GTP5G_QER_RELATED_TO_PDR]));
        memcpy(qer->related_pdr_list, 
				mnl_attr_get_payload(qer_tb[GTP5G_QER_RELATED_TO_PDR]), 
				mnl_attr_get_payload_len(qer_tb[GTP5G_QER_RELATED_TO_PDR]));
    }

    return MNL_CB_OK;
}

struct gtp5g_qer *gtp5g_qer_find_by_id(int genl_id, struct mnl_socket *nl, struct gtp5g_dev *dev, struct gtp5g_qer *qer)
{
    struct nlmsghdr *nlh;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct gtp5g_qer *rt_qer;
    uint32_t seq = time(NULL);

    if (!dev) {
        fprintf(stderr, "5G GTP device is NULL\n");
        return NULL;
    }

    nlh = genl_nlmsg_build_hdr(buf, 
								genl_id, 
								NLM_F_EXCL | NLM_F_ACK, 
								++seq,
                               	GTP5G_CMD_GET_QER);

    gtp5g_build_qer_payload(nlh, dev, qer);

    if (genl_socket_talk(nl, nlh, seq, genl_gtp5g_attr_cb, &rt_qer) < 0) {
        perror("genl_socket_talk");
        return NULL;
    }

    return rt_qer;
}
EXPORT_SYMBOL(gtp5g_qer_find_by_id);

