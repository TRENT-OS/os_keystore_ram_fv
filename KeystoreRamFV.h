/*
 * Copyright (C) 2021, HENSOLDT Cyber GmbH
 */

#pragma once

#define KeystoreRamFV_KEY_NAME_SIZE 17
#define KeystoreRamFV_KEY_DATA_SIZE 2048

#define KeystoreRamFV_MAX_APP_ID 255

#define KeystoreRamFV_ERR_NONE                 0
#define KeystoreRamFV_ERR_GENERIC             -1
#define KeystoreRamFV_ERR_INVALID_PARAMETER   -2
#define KeystoreRamFV_ERR_OUT_OF_SPACE        -3
#define KeystoreRamFV_ERR_DUPLICATED          -4
#define KeystoreRamFV_ERR_NOT_FOUND           -5
#define KeystoreRamFV_ERR_READ_ONLY           -6


typedef struct KeystoreRamFV_KeyRecord {
    unsigned int readOnly;
    char name[KeystoreRamFV_KEY_NAME_SIZE];
    char data[KeystoreRamFV_KEY_DATA_SIZE];
} KeystoreRamFV_KeyRecord_t;


typedef struct KeystoreRamFV_ElementAdmin {
    unsigned int isFree;
    unsigned int appId;
} KeystoreRamFV_ElementAdmin_t;


typedef struct KeystoreRamFV_ElementRecord {
    KeystoreRamFV_ElementAdmin_t admin;
    KeystoreRamFV_KeyRecord_t key;
} KeystoreRamFV_ElementRecord_t;


typedef struct KeystoreRamFV {
    unsigned long freeSlots;
    unsigned long maxElements;
    KeystoreRamFV_ElementRecord_t *elementStore;
} KeystoreRamFV_t;

typedef struct KeystoreRamFV_Result {
    signed error;
    unsigned long index;
} KeystoreRamFV_Result_t;

void
KeystoreRamFV_init(
    KeystoreRamFV_t *keyStore,
    unsigned long maxElements,
    KeystoreRamFV_ElementRecord_t *elementStore);

unsigned int
KeystoreRamFV_initWithReadOnlyKeys(
    KeystoreRamFV_t *keyStore,
    unsigned int const *appIds,
    KeystoreRamFV_KeyRecord_t const *keys,
    unsigned long nrKeys,
    unsigned long maxElements,
    KeystoreRamFV_ElementRecord_t *elementStore);

void
KeystoreRamFV_wipe(
    KeystoreRamFV_t *keyStore);

KeystoreRamFV_Result_t
KeystoreRamFV_add(
    KeystoreRamFV_t *keyStore,
    unsigned int appId,
    KeystoreRamFV_KeyRecord_t const *key);

KeystoreRamFV_Result_t
KeystoreRamFV_get(KeystoreRamFV_t const *keyStore,
    unsigned int appId,
    const char name [KeystoreRamFV_KEY_NAME_SIZE],
    KeystoreRamFV_KeyRecord_t *key);

unsigned int
KeystoreRamFV_getByIndex(
    KeystoreRamFV_t const *keyStore,
    unsigned int appId,
    unsigned long index,
    KeystoreRamFV_KeyRecord_t *key);

unsigned int
KeystoreRamFV_delete(
    KeystoreRamFV_t *keyStore,
    unsigned int appId,
    const char name [KeystoreRamFV_KEY_NAME_SIZE]);
