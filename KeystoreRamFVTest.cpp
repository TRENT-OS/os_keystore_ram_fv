/*
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include <gtest/gtest.h>
#include <limits.h>
#include <stdio.h>

#include <vector>

extern "C"
{
#include "KeystoreRamFV.h"
}


class Test_KeystoreRamFV : public testing::Test
{
    protected:
};


class KeyStore
{
    public:
    enum {NR_ELEMENTS = 16};

    KeyStore(unsigned int size = NR_ELEMENTS) : keystore_elements(size) {}
    unsigned int size() const { return keystore_elements.size(); }
    KeystoreRamFV_ElementRecord_t *get_element_buf() { return &keystore_elements[0]; }
    KeystoreRamFV_t *operator & () {return &key_store;}

    private:
    std::vector<KeystoreRamFV_ElementRecord_t> keystore_elements;
    KeystoreRamFV_t key_store;
};

static
void create_key_name(unsigned int app_id, unsigned int some_value, char name[KeystoreRamFV_KEY_NAME_SIZE])
{
    static char line[32];

    for (unsigned int k = 0; k < KeystoreRamFV_KEY_NAME_SIZE; k++)
    {
        name[k] = '\0';
    }

    snprintf(name, KeystoreRamFV_KEY_NAME_SIZE, "%04x:%04x", app_id, some_value);
}


static
KeystoreRamFV_KeyRecord_t init_key_record(unsigned int app_id, unsigned int index)
{
    KeystoreRamFV_KeyRecord_t key_record;

    create_key_name(app_id, index, key_record.name);
    for (unsigned int k = 0; k < KeystoreRamFV_KEY_DATA_SIZE; ++k)
    {
        key_record.data[k] = (char)((app_id * index + k) % 256);
    }

    return key_record;
}


static
int compare_key_records(KeystoreRamFV_KeyRecord_t a, KeystoreRamFV_KeyRecord_t b)
{
    for (unsigned int k = 0; k < KeystoreRamFV_KEY_DATA_SIZE; ++k)
    {
        if (a.data[k] != b.data[k])
        {
            return -1;
        }
    }

    for (unsigned int k = 0; k < KeystoreRamFV_KEY_NAME_SIZE; ++k)
    {
        if (a.name[k] != b.name[k])
        {
            return -1;
        }
    }

    return 0;
}


// Expectation: a Key Store after Init() is empty - as observed by key_store_get_by_index.
TEST(Test_KeystoreRamFV, empty_after_init)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
    {
        for (unsigned int j = 0; j < key_store.size(); ++j)
        {
            KeystoreRamFV_KeyRecord_t key;
            int result = KeystoreRamFV_getByIndex(&key_store, k, j, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, result);
        }
    }
}


// Expectation: calling Wipe() on an empty Key Store leaves it still empty.
TEST(Test_KeystoreRamFV, still_empty_after_init_plus_wipe)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_wipe(&key_store);

    for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
    {
        for (unsigned int j = 0; j < key_store.size(); ++j)
        {
            KeystoreRamFV_KeyRecord_t key;
            int result = KeystoreRamFV_getByIndex(&key_store, k, j, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, result);
        }
    }
}


// Expectation: adding a key to an empty Key Store succeeds.
TEST(Test_KeystoreRamFV, add_key_to_empty_key_store_succeeds)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
}


// Expectation: an existing key can be found successfully and correctly by its index.
TEST(Test_KeystoreRamFV, find_key_by_index_works)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    int result;
    KeystoreRamFV_KeyRecord_t found_key;
    result = KeystoreRamFV_getByIndex(&key_store, app_id, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

    result = compare_key_records(key, found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
}


// Expectation: an existing key can be found successfully and correctly by its name.
TEST(Test_KeystoreRamFV, find_key_by_name_works)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;
    KeystoreRamFV_Result_t get_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, get_result.error);

    int result = compare_key_records(key, found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
}


// Expectation: an existing key cannot be found successfully by its index if the app_id does not match.
TEST(Test_KeystoreRamFV, find_key_by_index_with_wrong_id_fails)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;
    int result = KeystoreRamFV_getByIndex(&key_store, app_id + 1, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);
}


// Expectation: an existing key cannot be found successfully by its name if the app_id does not match.
TEST(Test_KeystoreRamFV, find_key_by_name_with_wrong_id_fails)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;
    KeystoreRamFV_Result_t get_result = KeystoreRamFV_get(&key_store, app_id + 1, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, get_result.error);
}


// Expectation: adding a key does really only create one entry in the Key Store.
// Test method: for an empty Key Store to which one key has been added: get_by_index with any
// combination of (app_id, index) succeeds precisely only once. This holds for all app_ids.
TEST(Test_KeystoreRamFV, add_really_creates_only_one_key)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l <= KeystoreRamFV_MAX_APP_ID; ++l)
    {
        KeystoreRamFV_wipe(&key_store);

        KeystoreRamFV_KeyRecord_t key = init_key_record(l, 0);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, l, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

        unsigned int found = 0;
        for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
        {
            for (unsigned int j = 0; j < key_store.size(); ++j)
            {
                KeystoreRamFV_KeyRecord_t key;
                if (0 == KeystoreRamFV_getByIndex(&key_store, k, j, &key))
                {
                    ++found;
                }
            }
        }

        ASSERT_TRUE(found == 1);
    }
}


// Expectation: a Key Store can be filled up to its capacity.
// Test method: adding keys with identical app_id to an empty Key Store succeeds exactly capacity times.
TEST(Test_KeystoreRamFV, key_store_can_be_filled)
{
    unsigned int app_id = 1;
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
    }

    for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(k, 0);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, k, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result.error);
    }
}


// Expectation: any "slot" in the Key Store can be occupied by a key with arbitrary app_id.
// Test method: adding keys with varying app_ids to an empty Key Store succeeds exactly capacity times.
TEST(Test_KeystoreRamFV, key_slot_is_not_restricted_to_app_id)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        unsigned int app_id = l % KeystoreRamFV_MAX_APP_ID;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
    }

    for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(k, 0);
        unsigned int index;
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, k, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result.error);
    }
}


// Expectation: keys added for an app_id have to have unique names.
TEST(Test_KeystoreRamFV, key_cannot_be_added_twice_for_one_app_id)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_Result_t result;
    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

    result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_DUPLICATED, result.error);
}

// Expectation: keys added for different app_ids do not have to have unique names.
TEST(Test_KeystoreRamFV, key_can_be_added_twice_for_different_app_ids)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_Result_t result;
    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

    result = KeystoreRamFV_add(&key_store, app_id + 1, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
}

// Expectation: when having a Key Store filled with keys of identical app_ids: find by index succeeds and is correct.
TEST(Test_KeystoreRamFV, find_by_index_works_with_key_store_filled_with_identical_app_ids)
{
    KeyStore key_store;

    unsigned int app_id = 1;
    unsigned int key_index[key_store.size()];


    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
        key_index[l] = result.index;
    }

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        int result;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);

        KeystoreRamFV_KeyRecord_t found_key;
        result = KeystoreRamFV_getByIndex(&key_store, app_id, key_index[l], &found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

        result = compare_key_records(key, found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
    }
}


// Expectation: when having a Key Store filled with keys of identical app_ids: find by name succeeds and is correct.
TEST(Test_KeystoreRamFV, find_by_name_works_with_key_store_filled_with_identical_app_ids)
{
    unsigned int app_id = 1;

    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        unsigned int index;
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
    }

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);

        KeystoreRamFV_KeyRecord_t found_key;
        KeystoreRamFV_Result_t add_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

        int result = compare_key_records(key, found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
    }
}

// Expectation: when having a Key Store filled with keys of different app_ids: find by index succeeds and is correct.
TEST(Test_KeystoreRamFV, find_by_index_works_with_key_store_filled_with_different_app_ids)
{
    KeyStore key_store;

    unsigned int key_index[key_store.size()];

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        unsigned int app_id = l % KeystoreRamFV_MAX_APP_ID;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        unsigned int index;
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
        key_index[l] = result.index;
    }

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        int result;
        unsigned int app_id = l % KeystoreRamFV_MAX_APP_ID;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);

        KeystoreRamFV_KeyRecord_t found_key;
        result = KeystoreRamFV_getByIndex(&key_store, app_id, key_index[l], &found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

        result = compare_key_records(key, found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
    }
}


// Expectation: when having a Key Store filled with keys of different app_ids: find by name succeeds and is correct.
TEST(Test_KeystoreRamFV, find_by_name_works_with_key_store_filled_with_different_app_ids)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        unsigned int app_id = l % KeystoreRamFV_MAX_APP_ID;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
    }

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        unsigned int app_id = l % KeystoreRamFV_MAX_APP_ID;
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);

        KeystoreRamFV_KeyRecord_t found_key;
        KeystoreRamFV_Result_t add_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

        int result = compare_key_records(key, found_key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
    }
}


// Expectation: a deleted key cannot be found any more by name or by index.
TEST(Test_KeystoreRamFV, deleted_key_cannot_be_found_any_more)
{
    unsigned int app_id = 1;

    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;

    int result;
    result = KeystoreRamFV_getByIndex(&key_store, app_id, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

    KeystoreRamFV_Result_t get_result;
    get_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, get_result.error);

    result = KeystoreRamFV_delete(&key_store, app_id, key.name);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

    result = KeystoreRamFV_getByIndex(&key_store, app_id, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, result);

    get_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, result);
}

// Expectation: an existing key can be deleted successfully independent of its app_id.
// Test method: deleting a key of a full Key Store succeeds and allows a new key to be added.
// This is independent of the app_id of the deleted and added key.
TEST(Test_KeystoreRamFV, key_deletion_works_independent_of_app_id)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int app_id = 0; app_id <= KeystoreRamFV_MAX_APP_ID; ++app_id)
    {
        KeystoreRamFV_wipe(&key_store);

        unsigned int key_to_be_deleted_index = app_id % key_store.size();
        KeystoreRamFV_KeyRecord_t key_to_be_deleted;

        for (unsigned int l = 0; l < key_store.size(); ++l)
        {
            KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
            KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        int result = KeystoreRamFV_delete(&key_store, app_id, key_to_be_deleted.name);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, key_store.size() + 1);
        KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);
    }
}


// Expectation: deleting a key with correct name but wrong app_id fails.
TEST(Test_KeystoreRamFV, deletion_fails_with_wrong_app_id)
{
    unsigned int app_id = 1;

    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    int result = KeystoreRamFV_delete(&key_store, app_id + 1, key.name);
    ASSERT_EQ(KeystoreRamFV_ERR_NOT_FOUND, result);
}


// Expectation: after deleting a key from a full Key Store it is possible to add a key successfully independent of its app_id.
TEST(Test_KeystoreRamFV, empty_slot_from_deletion_can_be_filled_with_any_app_id)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int app_id = 0; app_id <= KeystoreRamFV_MAX_APP_ID; ++app_id)
    {
        KeystoreRamFV_wipe(&key_store);

        unsigned int key_to_be_deleted_index = app_id % key_store.size();
        KeystoreRamFV_KeyRecord_t key_to_be_deleted;

        for (unsigned int l = 0; l < key_store.size(); ++l)
        {
            KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
            KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        unsigned int key_to_be_deleted_app_id = app_id;
        for (unsigned int k = 0; k <= KeystoreRamFV_MAX_APP_ID; ++k)
        {
            int result = KeystoreRamFV_delete(&key_store, key_to_be_deleted_app_id, key_to_be_deleted.name);
            ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

            KeystoreRamFV_KeyRecord_t key = init_key_record(k, key_store.size() + 1 + k);
            KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, k, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

            key_to_be_deleted_app_id = k;
            key_to_be_deleted = key;
        }
    }
}


// Expectation: if deleting a key from a full Key Store and adding a new one then the (resulting) index of both is identical.
TEST(Test_KeystoreRamFV, delete_add_on_full_key_store_involves_identical_index)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    for (unsigned int app_id = 0; app_id <= KeystoreRamFV_MAX_APP_ID; ++app_id)
    {
        KeystoreRamFV_wipe(&key_store);

        unsigned int key_to_be_deleted_index = app_id % key_store.size();
        KeystoreRamFV_KeyRecord_t key_to_be_deleted;

        for (unsigned int l = 0; l < key_store.size(); ++l)
        {
            KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
            KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
            ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        int result = KeystoreRamFV_delete(&key_store, app_id, key_to_be_deleted.name);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, key_store.size() + 1);
        KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

        ASSERT_TRUE(add_result.index == key_to_be_deleted_index);
    }
}


// Expectation: key_store_add handles illegal arguments properly.
TEST(Test_KeystoreRamFV, add_catches_illegal_arguments)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    KeystoreRamFV_Result_t result;
    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);

    result = KeystoreRamFV_add(&key_store, app_id + 1, NULL);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result.error);

    result = KeystoreRamFV_add(&key_store, KeystoreRamFV_MAX_APP_ID + 1, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result.error);
}


// Expectation: key_store_get handles illegal arguments properly.
TEST(Test_KeystoreRamFV, get_catches_illegal_arguments)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;
    KeystoreRamFV_Result_t get_result;

    get_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, get_result.error);

    get_result = KeystoreRamFV_get(&key_store, KeystoreRamFV_MAX_APP_ID + 1, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, get_result.error);

    get_result = KeystoreRamFV_get(&key_store, app_id, NULL, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, get_result.error);

    get_result = KeystoreRamFV_get(&key_store, app_id, key.name, NULL);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, get_result.error);
}


// Expectation: key_store_get_by_index handles illegal arguments properly.
TEST(Test_KeystoreRamFV, get_by_index_catches_illegal_arguments)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_Result_t get_result;
    KeystoreRamFV_KeyRecord_t found_key;
    int result;

    result = KeystoreRamFV_getByIndex(&key_store, app_id, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);

    result = KeystoreRamFV_getByIndex(&key_store, KeystoreRamFV_MAX_APP_ID + 1, add_result.index, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);

    result = KeystoreRamFV_getByIndex(&key_store, app_id, key_store.size(), &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);

    result = KeystoreRamFV_getByIndex(&key_store, KeystoreRamFV_MAX_APP_ID + 1, add_result.index, NULL);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);
}


// Expectation: key_store_delete handles illegal arguments properly.
TEST(Test_KeystoreRamFV, delete_catches_illegal_arguments)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    KeystoreRamFV_KeyRecord_t found_key;

    KeystoreRamFV_Result_t get_result = KeystoreRamFV_get(&key_store, app_id, key.name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, get_result.error);

    int result;

    result = KeystoreRamFV_delete(&key_store, KeystoreRamFV_MAX_APP_ID + 1, key.name);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);

    result = KeystoreRamFV_delete(&key_store, app_id, NULL);
    ASSERT_EQ(KeystoreRamFV_ERR_INVALID_PARAMETER, result);

    result = KeystoreRamFV_delete(&key_store, app_id, key.name);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
}


// Expectation: an existing read only key can be found successfully and correctly by its name.
TEST(Test_KeystoreRamFV, find_read_only_key_by_name_works)
{
    unsigned int app_ids[1];
    KeystoreRamFV_KeyRecord_t keys[1];
    KeyStore key_store;

    app_ids[0] = 1;
    keys[0] = init_key_record(app_ids[0], 0);

    unsigned int init_result = KeystoreRamFV_initWithReadOnlyKeys(&key_store, app_ids, keys, 1, key_store.size(), key_store.get_element_buf());
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, init_result);

    KeystoreRamFV_KeyRecord_t found_key;
    KeystoreRamFV_Result_t get_result = KeystoreRamFV_get(&key_store, app_ids[0], keys[0].name, &found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, get_result.error);

    int result = compare_key_records(keys[0], found_key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, result);
}


// Expectation: an existing read only key cannot be deleted.
TEST(Test_KeystoreRamFV, delete_read_only_key_does_not_work)
{
    unsigned int app_ids[1];
    KeystoreRamFV_KeyRecord_t keys[1];
    KeyStore key_store;

    app_ids[0] = 1;
    keys[0] = init_key_record(app_ids[0], 0);

    unsigned int init_result = KeystoreRamFV_initWithReadOnlyKeys(&key_store, app_ids, keys, 1, key_store.size(), key_store.get_element_buf());
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, init_result);

    int result = KeystoreRamFV_delete(&key_store, app_ids[0], keys[0].name);
    ASSERT_EQ(KeystoreRamFV_ERR_READ_ONLY, result);
}


// Expectation: initializing a Key Store with read only keys of which some are
// duplicate keys fails but nevertheless initializes the Key Store in a way as
// an ordinary initialization (= without using read only keys) would have done.
TEST(Test_KeystoreRamFV, initialize_with_duplicate_read_only_keys_fails_properly)
{
    KeyStore key_store;

    KeystoreRamFV_init(&key_store, key_store.size(), key_store.get_element_buf());

    unsigned int app_id = 0;

    KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, 0);
    KeystoreRamFV_Result_t add_result = KeystoreRamFV_add(&key_store, app_id, &key);
    ASSERT_EQ(KeystoreRamFV_ERR_NONE, add_result.error);

    unsigned int app_ids[2];
    KeystoreRamFV_KeyRecord_t keys[2];

    app_ids[0] = app_ids[1] = 1;
    keys[0] = init_key_record(app_ids[0], 0);
    keys[1] = init_key_record(app_ids[1], 0);

    unsigned int result = KeystoreRamFV_initWithReadOnlyKeys(&key_store, app_ids, keys, 2, key_store.size(), key_store.get_element_buf());
    ASSERT_EQ(KeystoreRamFV_ERR_DUPLICATED, result);

    for (unsigned int l = 0; l < key_store.size(); ++l)
    {
        KeystoreRamFV_KeyRecord_t key = init_key_record(app_id, l);
        KeystoreRamFV_Result_t result = KeystoreRamFV_add(&key_store, app_id, &key);
        ASSERT_EQ(KeystoreRamFV_ERR_NONE, result.error);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    std::getchar(); // keep console window open until Return keystroke

    return 0;
}
