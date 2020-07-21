
#include "KeyStore.h"

#include "stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

static key_store_t key_store;


static
int name_compare(key_name_t const *a, key_name_t const *b)
{
    if (a == NULL)
    {
        return -1;
    }

    if (b == NULL)
    {
        return -1;
    }

    for (unsigned int k = 0; k < KEY_NAME_SIZE; k++)
    {
        if (a->name[k] != b->name[k])
        {
            return -1;
        }
    }

    return 0;
}


static
void reset_element_key(unsigned int index)
{
    key_store.element_store[index].key.size = 0;

    for (unsigned int k = 0; k < KEY_NAME_SIZE; k++)
    {
        key_store.element_store[index].key.name.name[k] = '\0';
    }

    for (unsigned int k = 0; k < KEY_DATA_SIZE; k++)
    {
        key_store.element_store[index].key.data.data[k] = '\0';
    }
}


static
void delete_element(unsigned int index)
{
    if (!key_store.element_store[index].admin.is_free)
    {
        key_store.free_slots += 1;

        key_store.element_store[index].admin.is_free = 1;
        key_store.element_store[index].admin.app_id = 0;

        reset_element_key(index);
    }
}

static
unsigned int find_element(key_name_t const *name)
{
    if (name == NULL)
    {
        return NR_ELEMENTS;
    }

    for (unsigned int k = 0; k < NR_ELEMENTS; k++)
    {
        if (!key_store.element_store[k].admin.is_free)
        {
            if (0 == name_compare(name, &key_store.element_store[k].key.name))
            {
                return k;
            }
        }
    }

    return NR_ELEMENTS;
}

static
unsigned int find_free_element(void)
{
    for (unsigned int k = 0; k < NR_ELEMENTS; k++)
    {
        if (key_store.element_store[k].admin.is_free)
        {
            return k;
        }
    }

    return NR_ELEMENTS;
}


void key_store_init(void)
{
    key_store.free_slots = NR_ELEMENTS;
    for (unsigned int k = 0; k < NR_ELEMENTS; k++)
    {
        key_store.element_store[k].admin.index = k;
        key_store.element_store[k].admin.is_free = 1;
        key_store.element_store[k].admin.app_id = 0;
        reset_element_key(k);
    }
}


void key_store_wipe(void)
{
    for (unsigned int k = 0; k < NR_ELEMENTS; k++)
    {
        delete_element(k);
    }
}


int key_store_add(unsigned int app_id, key_record_t const *key, unsigned int *index)
{
    if (key == NULL)
    {
        return -1;
    }

    if (index == NULL)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    if (0 == key_store.free_slots)
    {
        return -1;
    }

    if (NR_ELEMENTS > find_element(&key->name))
    {
        return -1;
    }

    unsigned int element_index = find_free_element();

    if (NR_ELEMENTS == element_index)
    {
        return -1;
    }

    *index = element_index;

    key_store.free_slots -= 1;

    key_store.element_store[element_index].admin.app_id = app_id;
    key_store.element_store[element_index].admin.is_free = 0;

    key_store.element_store[element_index].key = *key;

    return 0;
}


int key_store_get(unsigned int app_id, key_name_t const *name, key_record_t *key, unsigned int *index)
{
    if (name == NULL)
    {
        return -1;
    }

    if (key == NULL)
    {
        return -1;
    }

    if (index == NULL)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    unsigned int element_index = find_element(name);

    if (NR_ELEMENTS == element_index)
    {
        return -1;
    }

    if (app_id != key_store.element_store[element_index].admin.app_id)
    {
        return -1;
    }

    *index = element_index;

    *key = key_store.element_store[element_index].key;

    return 0;
}

int key_store_get_by_index(unsigned int app_id, unsigned int index, key_record_t *key)
{
    if (key == NULL)
    {
        return -1;
    }

    if (index >= NR_ELEMENTS)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    if (key_store.element_store[index].admin.is_free)
    {
        return -1;
    }

    if (app_id != key_store.element_store[index].admin.app_id)
    {
        return -1;
    }

    *key = key_store.element_store[index].key;

    return 0;
}

int key_store_delete(unsigned int app_id, key_name_t const *name)
{
    if (name == NULL)
    {
        return -1;
    }

    if (app_id > MAX_APP_ID)
    {
        return -1;
    }

    unsigned int element_index = find_element(name);

    if (NR_ELEMENTS == element_index)
    {
        return -1;
    }

    if (app_id != key_store.element_store[element_index].admin.app_id)
    {
        return -1;
    }

    delete_element(element_index);    

    return 0;
}

#ifdef __cplusplus
}
#endif
