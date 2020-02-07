#include "utlt_hash.h"

#include <string.h>
#include <stdint.h>

#define InitialMax 15 /* tunable == 2^n - 1 */

void *AllocArray(Hash *ht, int max) {
    uint32_t size = sizeof(*ht->array) * (max + 1);
    void *ptr = UTLT_Calloc(1, size);
    memset(ptr, 0, size);
    return ptr;
}

#define Free(__buf) UTLT_Free(__buf)

Hash *HashMake() {
    Hash *ht;
    utime_t now = TimeNow();

    ht = UTLT_Malloc(sizeof(Hash));

    ht->entry = NULL;
    ht->count = 0;
    ht->max = InitialMax;
    ht->seed = (unsigned int)((now >> 32) ^ now ^ 
                              (uintptr_t)ht ^ (uintptr_t)&now) - 1;
    ht->array = AllocArray(ht, ht->max);
    ht->hashFunc = NULL;

    return ht;
}

Hash *HashMakeCustom(HashFunc hashFunc) {
    Hash *ht = HashMake();
    ht->hashFunc = hashFunc;
    return ht;
}

void HashDestroy(Hash *ht) {
    HashEntry *he = NULL, *next_he = NULL;

    UTLT_Assert(ht, return, "Null param");
    UTLT_Assert(ht->array, return, "Null param");

    HashClear(ht);

    he = ht->entry;
    while(he) {
        next_he = he->next;

        Free(he);
        he = next_he;
    }

    Free(ht->array);
    Free(ht);
}

// Hash iteration functions.
HashIndex *HashNext(HashIndex *hi) {
    hi->this = hi->next;
    while (!hi->this) {
        if (hi->index > hi->ht->max)
            return NULL;

        hi->this = hi->ht->array[hi->index++];
    }
    hi->next = hi->this->next;
    return hi;
}

HashIndex *HashFirst(Hash *ht) {
    HashIndex *hi;

    hi = &ht->iterator;

    hi->ht = ht;
    hi->index = 0;
    hi->this = NULL;
    hi->next = NULL;
    return HashNext(hi);
}

void HashThis(HashIndex *hi, const void **key, int *klen, void **val) {
    if (key)  *key  = hi->this->key;
    if (klen) *klen = hi->this->klen;
    if (val)  *val  = (void *)hi->this->val;
}

const void *HashThisKey(HashIndex *hi) {
    const void *key;

    HashThis(hi, &key, NULL, NULL);
    return key;
}

int HashThisKeyLen(HashIndex *hi) {
    int klen;

    HashThis(hi, NULL, &klen, NULL);
    return klen;
}

void *HashThisVal(HashIndex *hi) {
    void *val;

    HashThis(hi, NULL, NULL, &val);
    return val;
}

// Expanding a hash table
static void ExpandArray(Hash *ht) {
    HashIndex *hi;
    HashEntry **new_array;
    unsigned int new_max;

    new_max = ht->max * 2 + 1;
    new_array = AllocArray(ht, new_max);
    for (hi = HashFirst(ht); hi; hi = HashNext(hi)) {
        unsigned int i = hi->this->hash & new_max;
        hi->this->next = new_array[i];
        new_array[i] = hi->this;
    }
    Free(ht->array);
    ht->array = new_array;
    ht->max = new_max;
}

static unsigned int HashfuncDefault(const char *char_key, int *klen, unsigned int hash) {
    const unsigned char *key = (const unsigned char *)char_key;
    const unsigned char *p;
    int i;
    
    if (*klen == HASH_KEY_STRING) {
        for (p = key; *p; p++) {
            hash = hash * 33 + *p;
        }
        *klen = p - key;
    }
    else {
        for (p = key, i = *klen; i; i--, p++) {
            hash = hash * 33 + *p;
        }
    }

    return hash;
}

unsigned int CoreHashfuncDefault(const char *char_key, int *klen) {
    return HashfuncDefault(char_key, klen, 0);
}

/*
 * This is where we keep the details of the hash function and control
 * the maximum collision rate.
 *
 * If val is non-NULL it creates and initializes a new hash entry if
 * there isn't already one there; it returns an updatable pointer so
 * that hash entries can be removed.
 */

static HashEntry **FindEntry(Hash *ht, const void *key, int klen, const void *val) {
    HashEntry **hep, *he;
    unsigned int hash;

    if (ht->hashFunc)
        hash = ht->hashFunc(key, &klen);
    else
        hash = HashfuncDefault(key, &klen, ht->seed);

    /* scan linked list */
    for (hep = &ht->array[hash & ht->max], he = *hep;
         he; hep = &he->next, he = *hep) {
        if (he->hash == hash
            && he->klen == klen
            && memcmp(he->key, key, klen) == 0)
            break;
    }
    if (he || !val)
        return hep;

    /* add a new entry for non-NULL values */
    if ((he = ht->entry) != NULL)
        ht->entry = he->next;
    else
        he = UTLT_Malloc(sizeof(*he));
    
    he->next = NULL;
    he->hash = hash;
    he->key  = key;
    he->klen = klen;
    he->val  = val;
    *hep = he;
    ht->count++;
    return hep;
}

void *HashGet(Hash *ht, const void *key, int klen) {
    HashEntry *he;
    he = *FindEntry(ht, key, klen, NULL);
    if (he)
        return (void *)he->val;
    else
        return NULL;
}

void HashSet(Hash *ht, const void *key, int klen, const void *val) {
    HashEntry **hep;
    hep = FindEntry(ht, key, klen, val);
    if (*hep) {
        if (!val) {
            /* delete entry */
            HashEntry *old = *hep;
            *hep = (*hep)->next;
            old->next = ht->entry;
            ht->entry = old;
            --ht->count;
        }
        else {
            /* replace entry */
            (*hep)->val = val;
            /* check that the collision rate isn't too high */
            if (ht->count > ht->max) {
                ExpandArray(ht);
            }
        }
    }
    /* else key not present and val==NULL */
}

void *HashGetOrSet(Hash *ht, const void *key, int klen, const void *val) {
    HashEntry **hep;
    hep = FindEntry(ht, key, klen, val);
    if (*hep) {
        val = (*hep)->val;
        /* check that the collision rate isn't too high */
        if (ht->count > ht->max) {
            ExpandArray(ht);
        }
        return (void *)val;
    }
    /* else key not present and val==NULL */
    return NULL;
}

unsigned int HashCount(Hash *ht) {
    return ht->count;
}

void HashClear(Hash *ht) {
    HashIndex *hi;
    for (hi = HashFirst(ht); hi; hi = HashNext(hi))
        HashSet(ht, hi->this->key, hi->this->klen, NULL);
}

/* This is basically the following...
 * for every element in hash table {
 *    comp elemeny.key, element.value
 * }
 *
 * Like with table_do, the comp callback is called for each and every
 * element of the hash table.
 */
int HashDo(HashDoCallbackFunc *comp, void *rec, const Hash *ht) {
    HashIndex  hix;
    HashIndex *hi;
    int rv, dorv  = 1;

    hix.ht    = (Hash *)ht;
    hix.index = 0;
    hix.this  = NULL;
    hix.next  = NULL;

    if ((hi = HashNext(&hix))) {
        /* Scan the entire table */
        do {
            rv = (*comp)(rec, hi->this->key, hi->this->klen, hi->this->val);
        } while (rv && (hi = HashNext(hi)));

        if (rv == 0) {
            dorv = 0;
        }
    }
    return dorv;
}
