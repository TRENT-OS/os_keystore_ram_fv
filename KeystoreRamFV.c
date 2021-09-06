/*
 * Copyright (C) 2021, HENSOLDT Cyber GmbH
 */

#include "KeystoreRamFV.h"

#include "stdlib_fv.h"

#ifdef __cplusplus
extern "C"
{
#endif


static void
resetElementKey(KeystoreRamFV_t *key_store, unsigned long index)
{
    memset_fv(
        key_store->elementStore[index].key.name,
        0,
        KeystoreRamFV_KEY_NAME_SIZE);
    memset_fv(
        key_store->elementStore[index].key.data,
        0,
        KeystoreRamFV_KEY_DATA_SIZE);
}


static void
deleteElement(KeystoreRamFV_t *key_store, unsigned long index)
{
    if (!key_store->elementStore[index].admin.isFree)
    {
        key_store->freeSlots += 1;

        key_store->elementStore[index].admin.isFree = 1;
        key_store->elementStore[index].admin.appId = 0;

        resetElementKey(key_store, index);
    }
}


static unsigned long
findElement(
    KeystoreRamFV_t const *key_store,
    unsigned long max,
    const unsigned int appId,
    const char name [KeystoreRamFV_KEY_NAME_SIZE])
{
    if (name == NULL)
    {
        return max;
    }

    if (max > key_store->maxElements)
    {
        max = key_store->maxElements;
    }

    for (unsigned long k = 0; k < max; k++)
    {
        if (!key_store->elementStore[k].admin.isFree)
        {
            if (appId == key_store->elementStore[k].admin.appId &&
                0 == memcmp_fv(
                        name,
                        (void *) key_store->elementStore[k].key.name,
                        KeystoreRamFV_KEY_NAME_SIZE))
            {
                return k;
            }
        }
    }

    return max;
}

static unsigned long
findFreeElement(KeystoreRamFV_t const *key_store)
{
    for (unsigned long k = 0; k < key_store->maxElements; k++)
    {
        if (key_store->elementStore[k].admin.isFree)
        {
            return k;
        }
    }

    return key_store->maxElements;
}

void
KeystoreRamFV_init(
    KeystoreRamFV_t *key_store,
    unsigned long maxElements,
    KeystoreRamFV_ElementRecord_t *elementStore)
{
    key_store->maxElements = maxElements;
    key_store->elementStore = elementStore;
    key_store->freeSlots = key_store->maxElements;
    for (unsigned long k = 0; k < key_store->maxElements; k++)
    {
        key_store->elementStore[k].admin.isFree = 1;
        key_store->elementStore[k].admin.appId = 0;
        resetElementKey(key_store, k);
    }
}

unsigned int
KeystoreRamFV_initWithReadOnlyKeys(
    KeystoreRamFV_t *key_store,
    unsigned int const *appIds,
    KeystoreRamFV_KeyRecord_t const *keys,
    unsigned long nr_keys,
    unsigned long maxElements,
    KeystoreRamFV_ElementRecord_t *elementStore)
{
    unsigned int result = KeystoreRamFV_ERR_NONE;

    key_store->maxElements = maxElements;
    key_store->elementStore = elementStore;

    if (nr_keys > key_store->maxElements)
    {
        nr_keys = 0;
        result = -1;
    }

    for (unsigned long k = 0; k < nr_keys; k++)
    {
        // if there is a duplicate, return an empty keystore
        if (findElement(key_store, k, appIds[k], keys[k].name) < k)
        {
            nr_keys = 0;
            result = KeystoreRamFV_ERR_DUPLICATED;
            break;
        }

        key_store->elementStore[k].admin.isFree = 0;
        key_store->elementStore[k].admin.appId = appIds[k];
        key_store->elementStore[k].key.readOnly = 1;
        memcpy_fv(
            key_store->elementStore[k].key.name,
            keys[k].name,
            KeystoreRamFV_KEY_NAME_SIZE);
        memcpy_fv(
            key_store->elementStore[k].key.data,
            keys[k].data,
            KeystoreRamFV_KEY_DATA_SIZE);
    }

    for (unsigned long k = nr_keys; k < key_store->maxElements; k++)
    {
        key_store->elementStore[k].admin.isFree = 1;
        key_store->elementStore[k].admin.appId = 0;
        key_store->elementStore[k].key.readOnly = 0;
        resetElementKey(key_store, k);
    }

    key_store->freeSlots = key_store->maxElements - nr_keys;
    return result;
}


void
KeystoreRamFV_wipe(KeystoreRamFV_t *key_store)
{
    for (unsigned long k = 0; k < key_store->maxElements; k++)
    {
        if (!key_store->elementStore[k].admin.isFree &&
            !key_store->elementStore[k].key.readOnly)
        {
            deleteElement(key_store, k);
        }
    }
}


KeystoreRamFV_Result_t
KeystoreRamFV_add(
    KeystoreRamFV_t *key_store,
    unsigned int appId,
    KeystoreRamFV_KeyRecord_t const *key)
{
    KeystoreRamFV_Result_t result =
        {KeystoreRamFV_ERR_INVALID_PARAMETER, key_store->maxElements};

    if (key == NULL)
    {
        return result;
    }

    if (appId > KeystoreRamFV_MAX_APP_ID)
    {
        return result;
    }

    if (0 == key_store->freeSlots)
    {
        result.error = KeystoreRamFV_ERR_OUT_OF_SPACE;
        return result;
    }

    if (key_store->maxElements >
            findElement(key_store, key_store->maxElements, appId, key->name))
    {
        result.error = KeystoreRamFV_ERR_DUPLICATED;
        return result;
    }

    result.index = findFreeElement(key_store);

    if (key_store->maxElements == result.index)
    {
        result.error = KeystoreRamFV_ERR_OUT_OF_SPACE;
        return result;
    }

    key_store->freeSlots -= 1;

    key_store->elementStore[result.index].admin.isFree = 0;
    key_store->elementStore[result.index].admin.appId = appId;

    key_store->elementStore[result.index].key.readOnly = 0;
    memcpy_fv(
        key_store->elementStore[result.index].key.name,
        key->name,
        KeystoreRamFV_KEY_NAME_SIZE);
    memcpy_fv(
        key_store->elementStore[result.index].key.data,
        key->data,
        KeystoreRamFV_KEY_DATA_SIZE);

    result.error = KeystoreRamFV_ERR_NONE;
    return result;
}


KeystoreRamFV_Result_t
KeystoreRamFV_get(
    KeystoreRamFV_t const *key_store,
    unsigned int appId,
    const char name [KeystoreRamFV_KEY_NAME_SIZE],
    KeystoreRamFV_KeyRecord_t *key)
{
    KeystoreRamFV_Result_t result =
        {KeystoreRamFV_ERR_INVALID_PARAMETER, key_store->maxElements};

    if (name == NULL)
    {
        return result;
    }

    if (key == NULL)
    {
        return result;
    }

    if (appId > KeystoreRamFV_MAX_APP_ID)
    {
        return result;
    }

    result.index = findElement(key_store, key_store->maxElements, appId, name);

    if (key_store->maxElements == result.index)
    {
        result.error = KeystoreRamFV_ERR_NOT_FOUND;
        return result;
    }

    memcpy_fv(
        key->name,
        key_store->elementStore[result.index].key.name,
        KeystoreRamFV_KEY_NAME_SIZE);
    memcpy_fv(
        key->data,
        key_store->elementStore[result.index].key.data,
        KeystoreRamFV_KEY_DATA_SIZE);
    key->readOnly = key_store->elementStore[result.index].key.readOnly;

    result.error = KeystoreRamFV_ERR_NONE;
    return result;
}

unsigned int
KeystoreRamFV_getByIndex(
    KeystoreRamFV_t const *key_store,
    unsigned int appId,
    unsigned long index,
    KeystoreRamFV_KeyRecord_t *key)
{
    if (key == NULL)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    if (index >= key_store->maxElements)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    if (appId > KeystoreRamFV_MAX_APP_ID)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    if (key_store->elementStore[index].admin.isFree)
    {
        return KeystoreRamFV_ERR_NOT_FOUND;
    }

    if (appId != key_store->elementStore[index].admin.appId)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    memcpy_fv(
        key->name,
        key_store->elementStore[index].key.name,
        KeystoreRamFV_KEY_NAME_SIZE);
    memcpy_fv(
        key->data,
        key_store->elementStore[index].key.data,
        KeystoreRamFV_KEY_DATA_SIZE);
    key->readOnly = key_store->elementStore[index].key.readOnly;

    return KeystoreRamFV_ERR_NONE;
}

unsigned int
KeystoreRamFV_delete(
    KeystoreRamFV_t *key_store,
    unsigned int appId,
    const char name [KeystoreRamFV_KEY_NAME_SIZE])
{
    if (name == NULL)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    if (appId > KeystoreRamFV_MAX_APP_ID)
    {
        return KeystoreRamFV_ERR_INVALID_PARAMETER;
    }

    unsigned long element_index =
        findElement(key_store, key_store->maxElements, appId, name);

    if (key_store->maxElements == element_index)
    {
        return KeystoreRamFV_ERR_NOT_FOUND;
    }

    if (key_store->elementStore[element_index].key.readOnly)
    {
        return KeystoreRamFV_ERR_READ_ONLY;
    }

    deleteElement(key_store, element_index);

    return KeystoreRamFV_ERR_NONE;
}

#ifdef __cplusplus
}
#endif
