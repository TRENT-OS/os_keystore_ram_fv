
# Running the unit tests of the Key Store
Assuming we are in a directory `keystore` containg the following files:

```
build
KeyStore.c
KeyStore.h
KeyStoreTest.cpp
```

## Prepare googletest
Assuming we are in the directory `keystore`:
```
cd ..
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
cmake ..
make
```

If successfull, googletest was compiled and its libraries
```
libgtest_main.a
libgmock.a
libgmock_main.a
libgtest.a
```
have been created in `../googletest/build/lib`.

## Run the unit tests
Assuming we are in the directory `keystore`:

```
./build
```


## Expected output

```
[==========] Running 24 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 24 tests from Test_KeyStore
[ RUN      ] Test_KeyStore.empty_after_init
[       OK ] Test_KeyStore.empty_after_init (0 ms)
[ RUN      ] Test_KeyStore.still_empty_after_init_plus_wipe
[       OK ] Test_KeyStore.still_empty_after_init_plus_wipe (1 ms)
[ RUN      ] Test_KeyStore.add_key_to_empty_key_store_succeeds
[       OK ] Test_KeyStore.add_key_to_empty_key_store_succeeds (0 ms)
[ RUN      ] Test_KeyStore.find_key_by_index_works
[       OK ] Test_KeyStore.find_key_by_index_works (0 ms)
[ RUN      ] Test_KeyStore.find_key_by_name_works
[       OK ] Test_KeyStore.find_key_by_name_works (0 ms)
[ RUN      ] Test_KeyStore.find_key_by_index_with_wrong_id_fails
[       OK ] Test_KeyStore.find_key_by_index_with_wrong_id_fails (0 ms)
[ RUN      ] Test_KeyStore.find_key_by_name_with_wrong_id_fails
[       OK ] Test_KeyStore.find_key_by_name_with_wrong_id_fails (0 ms)
[ RUN      ] Test_KeyStore.add_really_creates_only_one_key
[       OK ] Test_KeyStore.add_really_creates_only_one_key (14 ms)
[ RUN      ] Test_KeyStore.key_store_can_be_filled
[       OK ] Test_KeyStore.key_store_can_be_filled (2 ms)
[ RUN      ] Test_KeyStore.key_slot_is_not_restricted_to_app_id
[       OK ] Test_KeyStore.key_slot_is_not_restricted_to_app_id (1 ms)
[ RUN      ] Test_KeyStore.key_cannot_be_added_twice
[       OK ] Test_KeyStore.key_cannot_be_added_twice (0 ms)
[ RUN      ] Test_KeyStore.find_by_index_works_with_key_store_filled_with_identical_app_ids
[       OK ] Test_KeyStore.find_by_index_works_with_key_store_filled_with_identical_app_ids (0 ms)
[ RUN      ] Test_KeyStore.find_by_name_works_with_key_store_filled_with_identical_app_ids
[       OK ] Test_KeyStore.find_by_name_works_with_key_store_filled_with_identical_app_ids (0 ms)
[ RUN      ] Test_KeyStore.find_by_index_works_with_key_store_filled_with_different_app_ids
[       OK ] Test_KeyStore.find_by_index_works_with_key_store_filled_with_different_app_ids (0 ms)
[ RUN      ] Test_KeyStore.find_by_name_works_with_key_store_filled_with_different_app_ids
[       OK ] Test_KeyStore.find_by_name_works_with_key_store_filled_with_different_app_ids (1 ms)
[ RUN      ] Test_KeyStore.deleted_key_cannot_be_found_any_more
[       OK ] Test_KeyStore.deleted_key_cannot_be_found_any_more (0 ms)
[ RUN      ] Test_KeyStore.key_deletion_works_independent_of_app_id
[       OK ] Test_KeyStore.key_deletion_works_independent_of_app_id (32 ms)
[ RUN      ] Test_KeyStore.deletion_fails_with_wrong_app_id
[       OK ] Test_KeyStore.deletion_fails_with_wrong_app_id (0 ms)
[ RUN      ] Test_KeyStore.empty_slot_from_deletion_can_be_filled_with_any_app_id
[       OK ] Test_KeyStore.empty_slot_from_deletion_can_be_filled_with_any_app_id (534 ms)
[ RUN      ] Test_KeyStore.delete_add_on_full_key_store_involves_identical_index
[       OK ] Test_KeyStore.delete_add_on_full_key_store_involves_identical_index (33 ms)
[ RUN      ] Test_KeyStore.add_catches_illegal_arguments
[       OK ] Test_KeyStore.add_catches_illegal_arguments (0 ms)
[ RUN      ] Test_KeyStore.get_catches_illegal_arguments
[       OK ] Test_KeyStore.get_catches_illegal_arguments (0 ms)
[ RUN      ] Test_KeyStore.get_by_index_catches_illegal_arguments
[       OK ] Test_KeyStore.get_by_index_catches_illegal_arguments (0 ms)
[ RUN      ] Test_KeyStore.delete_catches_illegal_arguments
[       OK ] Test_KeyStore.delete_catches_illegal_arguments (0 ms)
[----------] 24 tests from Test_KeyStore (618 ms total)

[----------] Global test environment tear-down
[==========] 24 tests from 1 test suite ran. (618 ms total)
[  PASSED  ] 24 tests.
```



