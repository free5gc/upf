#include <stdio.h>

#include "utlt_yaml.h"

void YamlIterSetElement(YamlIter *iter, yaml_node_type_t type) {
    UTLT_Assert(iter, return, "The yaml iter is NULL");
    
    if (type == YAML_SEQUENCE_NODE)
        iter->seqItem = iter->node->data.sequence.items.start - 1;
    else if (type == YAML_MAPPING_NODE)
        iter->mapPair = iter->node->data.mapping.pairs.start - 1;
}

void YamlIterInit(YamlIter *iter, yaml_document_t *doc) {
    UTLT_Assert(iter, return, "The yaml iter is NULL");
    UTLT_Assert(doc, return, "The initial document is NULL");
    
    memset(iter, 0, sizeof(YamlIter));

    iter->document = doc;
    iter->node = yaml_document_get_root_node(doc);
    UTLT_Assert(iter->node, return, "The node of iter is NULL");
    
    YamlIterSetElement(iter, iter->node->type);
}

int YamlIterNext(YamlIter *iter) {
    UTLT_Assert(iter, return STATUS_ERROR, "The yaml iter is NULL");
    UTLT_Assert(iter->document, return STATUS_ERROR, "The document of iter is NULL");
    UTLT_Assert(iter->node, return STATUS_ERROR, "The node of iter is NULL");
    
    switch(iter->node->type) {
        case YAML_SEQUENCE_NODE:
            if (iter->seqItem) {
                iter->seqItem++;
                if (iter->seqItem < iter->node->data.sequence.items.top)
                    return 1;
            }
            break;
        case YAML_MAPPING_NODE:
            if (iter->mapPair) {
                iter->mapPair++;
                if (iter->mapPair < iter->node->data.mapping.pairs.top)
                    return 1;
            }
            break;
        default:
            break;
    }

    return 0;
}

int YamlIterType(YamlIter *iter) {
    UTLT_Assert(iter, return STATUS_ERROR, "The yaml iter is NULL");
    UTLT_Assert(iter->node, return STATUS_ERROR, "The node of iter is NULL");
    
    return iter->node->type;
}

void YamlIterChild(YamlIter *parent, YamlIter *child) {
    UTLT_Assert(parent, return, "The parent yaml iter is NULL");
    UTLT_Assert(parent->document, return, "The document of parent iter is NULL");
    UTLT_Assert(parent->node, return, "The node of parent iter is NULL");
    UTLT_Assert(child, return, "The child yaml iter is NULL");
    
    memset(child, 0, sizeof(YamlIter));
    child->document = parent->document;

    switch(parent->node->type) {
        case YAML_SEQUENCE_NODE:
            UTLT_Assert(parent->seqItem, return, "The seqItem of parent iter is NULL");
            child->node = yaml_document_get_node(parent->document, *parent->seqItem);
            break;
        case YAML_MAPPING_NODE:
            UTLT_Assert(parent->mapPair, return, "The mapPair of parent iter is NULL");
            child->node = yaml_document_get_node(parent->document, parent->mapPair->value);
            break;
        default:
            UTLT_Assert(0, return, "Unknown node type");
    }
    UTLT_Assert(child->node, return, "The node of child iter is NULL");

    YamlIterSetElement(child, child->node->type);
}

const char *YamlIterGet(YamlIter *iter, int getType) {
    yaml_node_t *node = NULL;

    UTLT_Assert(iter, return NULL, "The yaml iter is NULL");
    UTLT_Assert(iter->document, return NULL, "The document of iter is NULL");
    UTLT_Assert(iter->node, return NULL, "The node of iter is NULL");
    
    switch(iter->node->type) {
        case YAML_SCALAR_NODE:
            return (const char *)iter->node->data.scalar.value;
        case YAML_SEQUENCE_NODE:
            UTLT_Assert(iter->seqItem, return NULL, "The seqItem of iter is NULL");
            node = yaml_document_get_node(iter->document, *iter->seqItem);
            break;
        case YAML_MAPPING_NODE:
            UTLT_Assert(iter->mapPair, return NULL, "The mapPair of iter is NULL");
            node = getType ? yaml_document_get_node(iter->document, iter->mapPair->value) : yaml_document_get_node(iter->document, iter->mapPair->key);
            break;
        default:
            UTLT_Assert(0, return NULL, "Unknown node type");
    }

    UTLT_Assert(node, return NULL, "The node from document is NULL");
    UTLT_Assert(node->type == YAML_SCALAR_NODE, return NULL, "The type of node is not YAML_SCALAR_NODE");
    
    return (const char *)node->data.scalar.value;
}