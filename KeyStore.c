
#include "KeyStore.h"

#include "fv_stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif


static
void reset_element_key(key_store_t *key_store, unsigned long index)
{
    memset(key_store->element_store[index].key.name, 0, KEY_NAME_SIZE);
    memset(key_store->element_store[index].key.data, 0, KEY_DATA_SIZE);
}


static
void delete_element(key_store_t *key_store, unsigned long index)
{
    if (!key_store->element_store[index].admin.is_free)
    {
        key_store->free_slots += 1;

        key_store->element_store[index].admin.is_free = 1;
        key_store->element_store[index].admin.app_id = 0;

        reset_element_key(key_store, index);
    }
}


static
unsigned long find_element(key_store_t const *key_store, unsigned long max, const unsigned int app_id, const char name [KEY_NAME_SIZE])
{
    if (name == NULL)
    {
        return max;
    }

    if (max > key_store->max_elements)
    {
        max = key_store->max_elements;
    }

    for (unsigned long k = 0; k < max; k++)
    {
        if (!key_store->element_store[k].admin.is_free)
        {
            if (app_id == key_store->element_store[k].admin.app_id &&
                0 == memcmp(name, (void *) key_store->element_store[k].key.name, KEY_NAME_SIZE))
            {
                return k;
            }
        }
    }

    return max;
}

static
unsigned long find_free_element(key_store_t const *key_store)
{
    for (unsigned long k = 0; k < key_store->max_elements; k++)
    {
        if (key_store->element_store[k].admin.is_free)
        {
            return k;
        }
    }

    return key_store->max_elements;
}

void key_store_init(
    key_store_t *key_store,
    unsigned long max_elements,
    element_record_t *element_store)
{
    key_store->max_elements = max_elements;
    key_store->element_store = element_store;
    key_store->free_slots = key_store->max_elements;
    for (unsigned long k = 0; k < key_store->max_elements; k++)
    {
        key_store->element_store[k].admin.is_free = 1;
        key_store->element_store[k].admin.app_id = 0;
        reset_element_key(key_store, k);
    }
}

unsigned int key_store_init_with_read_only_keys(
    key_store_t *key_store,
    unsigned int const *app_ids,
    key_record_t const *keys,
    unsigned long nr_keys,
    unsigned long max_elements,
    element_record_t *element_store)
{
    unsigned int result = 0;

    key_store->max_elements = max_elements;
    key_store->element_store = element_store;

    if (nr_keys > key_store->max_elements)
    {
        nr_keys = 0;
        result = -1;
    }

    for (unsigned long k = 0; k < nr_keys; k++)
    {
        // if there is a duplicate, return an empty keystore
        if (find_element(key_store, k, app_ids[k], keys[k].name) < k)
        {
            nr_keys = 0;
            result = -1;
            break;
        }

        key_store->element_store[k].admin.is_free = 0;
        key_store->element_store[k].admin.app_id = app_ids[k];
        key_store->element_store[k].key.read_only = 1;
        memcpy(key_store->element_store[k].key.name, keys[k].name, KEY_NAME_SIZE);
        memcpy(key_store->element_store[k].key.data, keys[k].data, KEY_DATA_SIZE);
    }

    for (unsigned long k = nr_keys; k < key_store->max_elements; k++)
    {
        key_store->element_store[k].admin.is_free = 1;
        key_store->element_store[k].admin.app_id = 0;
        key_store->element_store[k].key.read_only = 0;
        reset_element_key(key_store, k);
    }

    key_store->free_slots = key_store->max_elements - nr_keys;
    return result;
}


void key_store_wipe(key_store_t *key_store)
{
    for (unsigned long k = 0; k < key_store->max_elements; k++)
    {
        if (!key_store->element_store[k].admin.is_free &&
            !key_store->element_store[k].key.read_only)
        {
            delete_element(key_store, k);
        }
    }
}


key_store_result_t key_store_add(key_store_t *key_store, unsigned int app_id, key_record_t const *key)
{
    key_store_result_t result = {-1, key_store->max_elements};

    if (key == NULL)
    {
        return result;
    }

    if (app_id > MAX_APP_ID)
    {
        return result;
    }

    if (0 == key_store->free_slots)
    {
        return result;
    }

    if (key_store->max_elements > find_element(key_store, key_store->max_elements, app_id, key->name))
    {
        return result;
    }

    result.index = find_free_element(key_store);

    if (key_store->max_elements == result.index)
    {
        return result;
    }

    key_store->free_slots -= 1;

    key_store->element_store[result.index].admin.is_free = 0;
    key_store->element_store[result.index].admin.app_id = app_id;

    key_store->element_store[result.index].key.read_only = 0;
    memcpy(key_store->element_store[result.index].key.name, key->name, KEY_NAME_SIZE);
    memcpy(key_store->element_store[result.index].key.data, key->data, KEY_DATA_SIZE);

    result.error = 0;
    return result;
}


key_store_result_t key_store_get(key_store_t const *key_store, unsigned int app_id, const char name [KEY_NAME_SIZE], key_record_t *key)
{
    key_store_result_t result = {-1, key_store->max_elements};

    if (name == NULL)
    {
        return result;
    }

    if (key == NULL)
    {
        return result;
    }

    if (app_id > MAX_APP_ID)
    {
        return result;
    }

    result.index = find_element(key_store, key_store->max_elements, app_id, name);

    if (key_store->max_elements == result.index)
    {
        return result;
    }

    memcpy(key->name, key_store->element_store[result.index].key.name, KEY_NAME_SIZE);
    memcpy(key->data, key_store->element_store[result.index].key.data, KEY_DATA_SIZE);
    key->read_only = key_store->element_store[result.index].key.read_only;

    result.error = 0;
    return result;
}

unsigned int key_store_get_by_index(key_store_t const *key_store, unsigned int app_id, unsigned long index, key_record_t *key)
{
    if (key == NULL)
    {
        return -1;
    }

    if (index >= key_store->max_elements)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    if (key_store->element_store[index].admin.is_free)
    {
        return -1;
    }

    if (app_id != key_store->element_store[index].admin.app_id)
    {
        return -1;
    }

    memcpy(key->name, key_store->element_store[index].key.name, KEY_NAME_SIZE);
    memcpy(key->data, key_store->element_store[index].key.data, KEY_DATA_SIZE);
    key->read_only = key_store->element_store[index].key.read_only;

    return 0;
}

unsigned int key_store_delete(key_store_t *key_store, unsigned int app_id, const char name [KEY_NAME_SIZE])
{
    if (name == NULL)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    unsigned long element_index = find_element(key_store, key_store->max_elements, app_id, name);

    if (key_store->max_elements == element_index)
    {
        return -1;
    }

    if (key_store->element_store[element_index].key.read_only)
    {
        return -1;
    }

    delete_element(key_store, element_index);

    return 0;
}

#ifdef __cplusplus
}
#endif
