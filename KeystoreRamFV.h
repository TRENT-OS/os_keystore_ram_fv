/*
 * Copyright (C) 2021, HENSOLDT Cyber GmbH
 */

#pragma once

#if !defined(KeystoreRamFV_KEY_NAME_SIZE)
#   define KeystoreRamFV_KEY_NAME_SIZE 16
#endif

/**
 * This defines the maximum size of struct OS_KeystoreRamFV_DataSubRecord, which consists of
 *  - a uint8_t array, containing the key data (size OS_KeystoreRamFV_MAX_KEY_SIZE bytes)
 *  - a uint32_t variable, containing the size of the used key data (size 4 bytes)
 *
 * In a total, this results in a value of 2084 bytes.
 */
#if !defined(KeystoreRamFV_KEY_DATA_SIZE)
#   define KeystoreRamFV_KEY_DATA_SIZE 2084
#endif

#define KeystoreRamFV_MAX_APP_ID 255

#define KeystoreRamFV_ERR_NONE                  ((unsigned int)  0)
#define KeystoreRamFV_ERR_GENERIC               ((unsigned int) -1)
#define KeystoreRamFV_ERR_INVALID_PARAMETER     ((unsigned int) -2)
#define KeystoreRamFV_ERR_OUT_OF_SPACE          ((unsigned int) -3)
#define KeystoreRamFV_ERR_DUPLICATED            ((unsigned int) -4)
#define KeystoreRamFV_ERR_NOT_FOUND             ((unsigned int) -5)
#define KeystoreRamFV_ERR_READ_ONLY             ((unsigned int) -6)


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
    unsigned int error;
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
