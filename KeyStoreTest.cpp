/*
 *  Copyright (C) 2019, Hensoldt Cyber GmbH
 */

#include <gtest/gtest.h>
#include <limits.h>
#include <stdio.h>

extern "C"
{
#include "KeyStore.h"
}


class Test_KeyStore : public testing::Test
{
    protected:
};


static
void create_key_name(unsigned int app_id, unsigned int some_value, char name[KEY_NAME_SIZE])
{
    static char line[32];

    for (unsigned int k = 0; k < KEY_NAME_SIZE; k++)
    {
        name[k] = '\0';
    }

    sprintf(name, "%08x:%08x", app_id, some_value);
}


static
key_record_t init_key_record(unsigned int app_id, unsigned int index)
{
    key_record_t key_record;

    create_key_name(app_id, index, key_record.name);
    for (unsigned int k = 0; k < KEY_DATA_SIZE; ++k)
    {
        key_record.data[k] = (char)((app_id * index + k) % 256);
    }

    return key_record;
}


static
int compare_key_records(key_record_t a, key_record_t b)
{
    for (unsigned int k = 0; k < KEY_DATA_SIZE; ++k)
    {
        if (a.data[k] != b.data[k])
        {
            return -1;
        }
    }

    for (unsigned int k = 0; k < KEY_NAME_SIZE; ++k)
    {
        if (a.name[k] != b.name[k])
        {
            return -1;
        }
    }

    return 0;
}


// Expectation: a Key Store after Init() is empty - as observed by key_store_get_by_index.
TEST(Test_KeyStore, empty_after_init)
{
    key_store_init();

    for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
    {
        for (unsigned int j = 0; j < NR_ELEMENTS; ++j)
        {
            key_record_t key;
            int result = key_store_get_by_index(k, j, &key);
            ASSERT_TRUE(result != 0);
        }
    }
}


// Expectation: calling Wipe() on an empty Key Store leaves it still empty.
TEST(Test_KeyStore, still_empty_after_init_plus_wipe)
{
    key_store_init();

    key_store_wipe();

    for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
    {
        for (unsigned int j = 0; j < NR_ELEMENTS; ++j)
        {
            key_record_t key;
            int result = key_store_get_by_index(k, j, &key);
            ASSERT_TRUE(result != 0);
        }
    }
}


// Expectation: adding a key to an empty Key Store succeeds.
TEST(Test_KeyStore, add_key_to_empty_key_store_succeeds)
{
    key_store_init();

    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t result = key_store_add(app_id, &key);
    ASSERT_TRUE(result.error == 0);
}


// Expectation: an existing key can be found successfully and correctly by its index.
TEST(Test_KeyStore, find_key_by_index_works)
{
    key_store_init();

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    int result;
    key_record_t found_key;
    result = key_store_get_by_index(app_id, add_result.index, &found_key);
    ASSERT_TRUE(result == 0);

    result = compare_key_records(key, found_key);
    ASSERT_TRUE(result == 0);
}


// Expectation: an existing key can be found successfully and correctly by its name.
TEST(Test_KeyStore, find_key_by_name_works)
{
    key_store_init();

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;
    key_store_result_t get_result = key_store_get(app_id, key.name, &found_key);
    ASSERT_TRUE(get_result.error == 0);

    int result = compare_key_records(key, found_key);
    ASSERT_TRUE(result == 0);
}


// Expectation: an existing key cannot be found successfully by its index if the app_id does not match.
TEST(Test_KeyStore, find_key_by_index_with_wrong_id_fails)
{
    key_store_init();

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;
    int result = key_store_get_by_index(app_id + 1, add_result.index, &found_key);
    ASSERT_TRUE(result != 0);
}


// Expectation: an existing key cannot be found successfully by its name if the app_id does not match.
TEST(Test_KeyStore, find_key_by_name_with_wrong_id_fails)
{
    key_store_init();

    unsigned int app_id = 1;
    unsigned int some_value = 4711;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;
    key_store_result_t get_result = key_store_get(app_id + 1, key.name, &found_key);
    ASSERT_TRUE(get_result.error != 0);
}


// Expectation: adding a key does really only create one entry in the Key Store.
// Test method: for an empty Key Store to which one key has been added: get_by_index with any
// combination of (app_id, index) succeeds precisely only once. This holds for all app_ids.
TEST(Test_KeyStore, add_really_creates_only_one_key)
{
    key_store_init();

    for (unsigned int l = 0; l <= MAX_APP_ID; ++l)
    {
        key_store_wipe();

        key_record_t key = init_key_record(l, 0);
        key_store_result_t result = key_store_add(l, &key);
        ASSERT_TRUE(result.error == 0);

        unsigned int found = 0;
        for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
        {
            for (unsigned int j = 0; j < NR_ELEMENTS; ++j)
            {
                key_record_t key;
                if (0 == key_store_get_by_index(k, j, &key))
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
TEST(Test_KeyStore, key_store_can_be_filled)
{
    unsigned int app_id = 1;

    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        key_record_t key = init_key_record(app_id, l);
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
    }

    for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
    {
        key_record_t key = init_key_record(k, 0);
        key_store_result_t result = key_store_add(k, &key);
        ASSERT_TRUE(result.error != 0);
    }
}


// Expectation: any "slot" in the Key Store can be occupied by a key with arbitrary app_id.
// Test method: adding keys with varying app_ids to an empty Key Store succeeds exactly capacity times.
TEST(Test_KeyStore, key_slot_is_not_restricted_to_app_id)
{
    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        unsigned int app_id = l % MAX_APP_ID;
        key_record_t key = init_key_record(app_id, l);
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
    }

    for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
    {
        key_record_t key = init_key_record(k, 0);
        unsigned int index;
        key_store_result_t result = key_store_add(k, &key);
        ASSERT_TRUE(result.error != 0);
    }
}


// Expectation: keys added for an app_id have to have unique names.
TEST(Test_KeyStore, key_cannot_be_added_twice_for_one_app_id)
{
    key_store_init();

    key_store_result_t result;
    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    result = key_store_add(app_id, &key);
    ASSERT_TRUE(result.error == 0);

    result = key_store_add(app_id, &key);
    ASSERT_TRUE(result.error != 0);
}

// Expectation: keys added for different app_ids do not have to have unique names.
TEST(Test_KeyStore, key_can_be_added_twice_for_different_app_ids)
{
    key_store_init();

    key_store_result_t result;
    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    result = key_store_add(app_id, &key);
    ASSERT_TRUE(result.error == 0);

    result = key_store_add(app_id + 1, &key);
    ASSERT_TRUE(result.error == 0);
}

// Expectation: when having a Key Store filled with keys of identical app_ids: find by index succeeds and is correct.
TEST(Test_KeyStore, find_by_index_works_with_key_store_filled_with_identical_app_ids)
{
    unsigned int app_id = 1;
    unsigned int key_index[NR_ELEMENTS];

    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        key_record_t key = init_key_record(app_id, l);
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
        key_index[l] = result.index;
    }

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        int result;
        key_record_t key = init_key_record(app_id, l);

        key_record_t found_key;
        result = key_store_get_by_index(app_id, key_index[l], &found_key);
        ASSERT_TRUE(result == 0);

        result = compare_key_records(key, found_key);
        ASSERT_TRUE(result == 0);
    }
}


// Expectation: when having a Key Store filled with keys of identical app_ids: find by name succeeds and is correct.
TEST(Test_KeyStore, find_by_name_works_with_key_store_filled_with_identical_app_ids)
{
    unsigned int app_id = 1;

    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        key_record_t key = init_key_record(app_id, l);
        unsigned int index;
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
    }

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        key_record_t key = init_key_record(app_id, l);

        key_record_t found_key;
        key_store_result_t add_result = key_store_get(app_id, key.name, &found_key);
        ASSERT_TRUE(add_result.error == 0);

        int result = compare_key_records(key, found_key);
        ASSERT_TRUE(result == 0);
    }
}

// Expectation: when having a Key Store filled with keys of different app_ids: find by index succeeds and is correct.
TEST(Test_KeyStore, find_by_index_works_with_key_store_filled_with_different_app_ids)
{
    unsigned int key_index[NR_ELEMENTS];

    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        unsigned int app_id = l % MAX_APP_ID;
        key_record_t key = init_key_record(app_id, l);
        unsigned int index;
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
        key_index[l] = result.index;
    }

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        int result;
        unsigned int app_id = l % MAX_APP_ID;
        key_record_t key = init_key_record(app_id, l);

        key_record_t found_key;
        result = key_store_get_by_index(app_id, key_index[l], &found_key);
        ASSERT_TRUE(result == 0);

        result = compare_key_records(key, found_key);
        ASSERT_TRUE(result == 0);
    }
}


// Expectation: when having a Key Store filled with keys of different app_ids: find by name succeeds and is correct.
TEST(Test_KeyStore, find_by_name_works_with_key_store_filled_with_different_app_ids)
{
    key_store_init();

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        unsigned int app_id = l % MAX_APP_ID;
        key_record_t key = init_key_record(app_id, l);
        key_store_result_t result = key_store_add(app_id, &key);
        ASSERT_TRUE(result.error == 0);
    }

    for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
    {
        unsigned int app_id = l % MAX_APP_ID;
        key_record_t key = init_key_record(app_id, l);

        key_record_t found_key;
        key_store_result_t add_result = key_store_get(app_id, key.name, &found_key);
        ASSERT_TRUE(add_result.error == 0);

        int result = compare_key_records(key, found_key);
        ASSERT_TRUE(result == 0);
    }
}


// Expectation: a deleted key cannot be found any more by name or by index.
TEST(Test_KeyStore, deleted_key_cannot_be_found_any_more)
{
    unsigned int app_id = 1;

    key_store_init();

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;

    int result;
    result = key_store_get_by_index(app_id, add_result.index, &found_key);
    ASSERT_TRUE(result == 0);

    key_store_result_t get_result;
    get_result = key_store_get(app_id, key.name, &found_key);
    ASSERT_TRUE(get_result.error == 0);

    result = key_store_delete(app_id, key.name);
    ASSERT_TRUE(result == 0);

    result = key_store_get_by_index(app_id, add_result.index, &found_key);
    ASSERT_TRUE(result != 0);

    get_result = key_store_get(app_id, key.name, &found_key);
    ASSERT_TRUE(get_result.error != 0);
}

// Expectation: an existing key can be deleted successfully independent of its app_id.
// Test method: deleting a key of a full Key Store succeeds and allows a new key to be added.
// This is independent of the app_id of the deleted and added key.
TEST(Test_KeyStore, key_deletion_works_independent_of_app_id)
{
    key_store_init();

    for (unsigned int app_id = 0; app_id <= MAX_APP_ID; ++app_id)
    {
        key_store_wipe();

        unsigned int key_to_be_deleted_index = app_id % NR_ELEMENTS;
        key_record_t key_to_be_deleted;

        for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
        {
            key_record_t key = init_key_record(app_id, l);
            key_store_result_t result = key_store_add(app_id, &key);
            ASSERT_TRUE(result.error == 0);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        int result = key_store_delete(app_id, key_to_be_deleted.name);
        ASSERT_TRUE(result == 0);

        key_record_t key = init_key_record(app_id, NR_ELEMENTS + 1);
        key_store_result_t add_result = key_store_add(app_id, &key);
        ASSERT_TRUE(add_result.error == 0);
    }
}


// Expectation: deleting a key with correct name but wrong app_id fails.
TEST(Test_KeyStore, deletion_fails_with_wrong_app_id)
{
    unsigned int app_id = 1;

    key_store_init();

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    int result = key_store_delete(app_id + 1, key.name);
    ASSERT_TRUE(result != 0);
}


// Expectation: after deleting a key from a full Key Store it is possible to add a key successfully independent of its app_id.
TEST(Test_KeyStore, empty_slot_from_deletion_can_be_filled_with_any_app_id)
{
    key_store_init();

    for (unsigned int app_id = 0; app_id <= MAX_APP_ID; ++app_id)
    {
        key_store_wipe();

        unsigned int key_to_be_deleted_index = app_id % NR_ELEMENTS;
        key_record_t key_to_be_deleted;

        for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
        {
            key_record_t key = init_key_record(app_id, l);
            key_store_result_t result = key_store_add(app_id, &key);
            ASSERT_TRUE(result.error == 0);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        unsigned int key_to_be_deleted_app_id = app_id;
        for (unsigned int k = 0; k <= MAX_APP_ID; ++k)
        {
            int result = key_store_delete(key_to_be_deleted_app_id, key_to_be_deleted.name);
            ASSERT_TRUE(result == 0);

            key_record_t key = init_key_record(k, NR_ELEMENTS + 1 + k);
            key_store_result_t add_result = key_store_add(k, &key);
            ASSERT_TRUE(add_result.error == 0);

            key_to_be_deleted_app_id = k;
            key_to_be_deleted = key;
        }
    }
}


// Expectation: if deleting a key from a full Key Store and adding a new one then the (resulting) index of both is identical.
TEST(Test_KeyStore, delete_add_on_full_key_store_involves_identical_index)
{
    key_store_init();

    for (unsigned int app_id = 0; app_id <= MAX_APP_ID; ++app_id)
    {
        key_store_wipe();

        unsigned int key_to_be_deleted_index = app_id % NR_ELEMENTS;
        key_record_t key_to_be_deleted;

        for (unsigned int l = 0; l < NR_ELEMENTS; ++l)
        {
            key_record_t key = init_key_record(app_id, l);
            key_store_result_t result = key_store_add(app_id, &key);
            ASSERT_TRUE(result.error == 0);

            if (l == key_to_be_deleted_index)
            {
                key_to_be_deleted = key;
            }
        }

        int result = key_store_delete(app_id, key_to_be_deleted.name);
        ASSERT_TRUE(result == 0);

        key_record_t key = init_key_record(app_id, NR_ELEMENTS + 1);
        key_store_result_t add_result = key_store_add(app_id, &key);
        ASSERT_TRUE(add_result.error == 0);

        ASSERT_TRUE(add_result.index == key_to_be_deleted_index);
    }
}


// Expectation: key_store_add handles illegal arguments properly.
TEST(Test_KeyStore, add_catches_illegal_arguments)
{
    key_store_init();

    key_store_result_t result;
    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    result = key_store_add(app_id, &key);
    ASSERT_TRUE(result.error == 0);

    result = key_store_add(app_id + 1, NULL);
    ASSERT_TRUE(result.error != 0);

    result = key_store_add(MAX_APP_ID + 1, &key);
    ASSERT_TRUE(result.error != 0);
}


// Expectation: key_store_get handles illegal arguments properly.
TEST(Test_KeyStore, get_catches_illegal_arguments)
{
    key_store_init();

    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;
    key_store_result_t get_result;

    get_result = key_store_get(app_id, key.name, &found_key);
    ASSERT_TRUE(get_result.error == 0);

    get_result = key_store_get(MAX_APP_ID + 1, key.name, &found_key);
    ASSERT_TRUE(get_result.error != 0);

    get_result = key_store_get(app_id, NULL, &found_key);
    ASSERT_TRUE(get_result.error != 0);

    get_result = key_store_get(app_id, key.name, NULL);
    ASSERT_TRUE(get_result.error != 0);
}


// Expectation: key_store_get_by_index handles illegal arguments properly.
TEST(Test_KeyStore, get_by_index_catches_illegal_arguments)
{
    key_store_init();

    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_store_result_t get_result;
    key_record_t found_key;
    int result;

    result = key_store_get_by_index(app_id, add_result.index, &found_key);
    ASSERT_TRUE(result == 0);

    result = key_store_get_by_index(MAX_APP_ID + 1, add_result.index, &found_key);
    ASSERT_TRUE(result != 0);

    result = key_store_get_by_index(app_id, NR_ELEMENTS, &found_key);
    ASSERT_TRUE(result != 0);

    result = key_store_get_by_index(MAX_APP_ID + 1, add_result.index, NULL);
    ASSERT_TRUE(result != 0);
}


// Expectation: key_store_delete handles illegal arguments properly.
TEST(Test_KeyStore, delete_catches_illegal_arguments)
{
    key_store_init();

    unsigned int app_id = 0;

    key_record_t key = init_key_record(app_id, 0);
    key_store_result_t add_result = key_store_add(app_id, &key);
    ASSERT_TRUE(add_result.error == 0);

    key_record_t found_key;

    key_store_result_t get_result = key_store_get(app_id, key.name, &found_key);
    ASSERT_TRUE(get_result.error == 0);

    int result;

    result = key_store_delete(MAX_APP_ID + 1, key.name);
    ASSERT_TRUE(result != 0);

    result = key_store_delete(app_id, NULL);
    ASSERT_TRUE(result != 0);

    result = key_store_delete(app_id, key.name);
    ASSERT_TRUE(result == 0);
}


// Expectation: an existing read only key can be found successfully and correctly by its name.
TEST(Test_KeyStore, find_read_only_key_by_name_works)
{
    unsigned int app_ids[1];
    key_record_t keys[1];

    app_ids[0] = 1;
    keys[0] = init_key_record(app_ids[0], 0);

    key_store_init_with_read_only_keys(app_ids, keys, 1);

    key_record_t found_key;
    key_store_result_t get_result = key_store_get(app_ids[0], keys[0].name, &found_key);
    ASSERT_TRUE(get_result.error == 0);

    int result = compare_key_records(keys[0], found_key);
    ASSERT_TRUE(result == 0);
}


// Expectation: an existing read only key cannot be deleted.
TEST(Test_KeyStore, delete_read_only_key_does_not_work)
{
    unsigned int app_ids[1];
    key_record_t keys[1];

    app_ids[0] = 1;
    keys[0] = init_key_record(app_ids[0], 0);

    int result = key_store_delete(app_ids[0], keys[0].name);
    ASSERT_TRUE(result != 0);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    std::getchar(); // keep console window open until Return keystroke

    return 0;
}
