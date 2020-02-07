#ifndef __UPDK_RULE_H__
#define __UPDK_RULE_H__

#include "updk/rule_pdr.h"
#include "updk/rule_far.h"
#include "updk/rule_qer.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/include"
 */

/**
 * Gtpv1TunnelCreatePDR - UPF receive CreatePDR in PFCP and it will call this function
 * 
 * @pdrPdr: UPDK_PDR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelCreatePDR(UPDK_PDR *pdr);

/**
 * Gtpv1TunnelUpdatePDR - UPF receive UpdatePDR in PFCP and it will call this function
 * 
 * @pdr: UPDK_PDR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelUpdatePDR(UPDK_PDR *pdr);

/**
 * Gtpv1TunnelRemovePDR - UPF receive RemovePDR in PFCP and it will call this function
 * 
 * @pdr: UPDK_PDR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelRemovePDR(UPDK_PDR *pdr);

/**
 * Gtpv1TunnelCreateFAR - UPF receive CreateFAR in PFCP and it will call this function
 * 
 * @far: UPDK_FAR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelCreateFAR(UPDK_FAR *far);

/**
 * Gtpv1TunnelUpdateFAR - UPF receive UpdateFAR in PFCP and it will call this function
 * 
 * @far: UPDK_FAR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelUpdateFAR(UPDK_FAR *far);

/**
 * Gtpv1TunnelRemoveFAR - UPF receive RemoveFAR in PFCP and it will call this function
 * 
 * @far: UPDK_FAR pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelRemoveFAR(UPDK_FAR *far);

/**
 * Gtpv1TunnelCreateQER - UPF receive CreateQER in PFCP and it will call this function
 * 
 * @qer: UPDK_QER pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelCreateQER(UPDK_QER *qer);

/**
 * Gtpv1TunnelUpdateQER - UPF receive UpdateQER in PFCP and it will call this function
 * 
 * @qer: UPDK_QER pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelUpdateQER(UPDK_QER *qer);

/**
 * Gtpv1TunnelRemoveQER - UPF receive RemoveQER in PFCP and it will call this function
 * 
 * @qer: UPDK_QER pointer in updk
 * @return: 0 or -1 if one of part is failed
 */
int Gtpv1TunnelRemoveQER(UPDK_QER *qer);

/* TODO: Our UPF do not handle these yet.
int Gtpv1TunnelCreateBAR(CreateBAR *createBar);
// int Gtpv1TunnelUpdateBAR(UpdateBAR *updateBar); // TODO: struct name shall be alias
int Gtpv1TunnelRemoveBAR(RemoveBAR *removeBar);

int Gtpv1TunnelCreateURR(CreateURR *createUrr);
int Gtpv1TunnelUpdateURR(UpdateURR *updateUrr);
int Gtpv1TunnelRemoveURR(RemoveURR *removeUrr);
*/
#endif /* __UPDK_RULE_H__ */