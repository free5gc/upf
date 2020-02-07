#include <stdio.h>
#include <yaml.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_yaml.h"

#define MAX_NUM_OF_HOSTNAME 16

yaml_document_t *document = NULL;

void LoadYamlFile() {
    FILE *file;
    yaml_parser_t parser;

    file = fopen("./testutlt_assets/testconfig.yaml", "rb");
    UTLT_Assert(file, return, "fail to open yaml file");

    UTLT_Assert(yaml_parser_initialize(&parser), return, "fail to initialize parser");
    yaml_parser_set_input_file(&parser, file);

    document = UTLT_Calloc(1, sizeof(yaml_document_t));
    
    if (!yaml_parser_load(&parser, document)) {
        UTLT_Free(document);
        yaml_parser_delete(&parser);
        UTLT_Assert(!fclose(file), return, "fail to close yaml file");
    }
}

void DeleteYamlDocument() {
    yaml_document_delete(document);
    UTLT_Free(document);
}

int SetProtocolIter(YamlIter *protoList, YamlIter *protoIter) {
    if (YamlIterType(protoList) == YAML_SCALAR_NODE) {
        return 1;
    } else if (YamlIterType(protoList) == YAML_SEQUENCE_NODE) {
        if (!YamlIterNext(protoList))
            return 1;
        YamlIterChild(protoList, protoIter);
    } else if (YamlIterType(protoList) == YAML_MAPPING_NODE) {
        memcpy(protoIter, protoList, sizeof(YamlIter));
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "Unknown node type");
        return 1;
    }

    return 0;
}

Status TestYaml_1() {
    LoadYamlFile();
    
    YamlIter rootIter;

    YamlIterInit(&rootIter, document);
    
    UTLT_Assert(YamlIterNext(&rootIter), return STATUS_ERROR, "The next node of rootIter is NULL");
    const char *rootKey = YamlIterGet(&rootIter, GET_KEY);
    UTLT_Assert(rootKey, return STATUS_ERROR, "The rootKey is NULL");
    UTLT_Assert(!strcmp(rootKey, "configuration"), return STATUS_ERROR, "rootKey error: need \"configuration\", not %s", rootKey);

    YamlIter upfIter;
    YamlIterChild(&rootIter, &upfIter);
    UTLT_Assert(YamlIterNext(&upfIter), return STATUS_ERROR, "The next node of upfIter is NULL");
    const char *upfKey = YamlIterGet(&upfIter, GET_KEY);
    UTLT_Assert(upfKey, return STATUS_ERROR, "The upfKey is NULL");
    UTLT_Assert(!strcmp(upfKey, "pfcp"), return STATUS_ERROR, "upfKey error: need \"pfcp\", not %s", upfKey);
    
    YamlIter pfcpList, pfcpIter;
    YamlIterChild(&upfIter, &pfcpList);
    UTLT_Assert(!SetProtocolIter(&pfcpList, &pfcpIter), return STATUS_ERROR, "SetProtocolIter error: need 0, not 1");
    
    UTLT_Assert(YamlIterNext(&pfcpIter), return STATUS_ERROR, "The next node of pfcpIter is NULL");
    const char *pfcpKey = YamlIterGet(&pfcpIter, GET_KEY);
    UTLT_Assert(pfcpKey, return STATUS_ERROR, "The pfcpKey is NULL");
    UTLT_Assert(!strcmp(pfcpKey, "addr"), return STATUS_ERROR, "pfcpKey error: need \"addr\", not %s", pfcpKey);
    
    YamlIter hostnameIter;
    YamlIterChild(&pfcpIter, &hostnameIter);
    UTLT_Assert(YamlIterType(&hostnameIter) != YAML_MAPPING_NODE, return STATUS_ERROR, "hostnameIter is type YAML_MAPPING_NODE");
    
    const char *hostname = YamlIterGet(&hostnameIter, GET_VALUE);
    UTLT_Assert(hostname, return STATUS_ERROR, "The hostname is NULL");
    UTLT_Assert(!strcmp(hostname, "192.188.2.2"), return STATUS_ERROR, "hostname error: need \"192.188.2.2\", not %s", hostname);
    
    DeleteYamlDocument();

    return STATUS_OK;
}

Status YamlTest(void *data) {
    Status status;
    
    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = TestYaml_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestYaml_1 fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
