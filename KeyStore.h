

#define KEY_NAME_SIZE 17
#define KEY_DATA_SIZE 2048

#define MAX_APP_ID 255

#define ERR_NONE                 0
#define ERR_GENERIC             -1
#define ERR_INVALID_PARAMETER   -2
#define ERR_OUT_OF_SPACE        -3
#define ERR_DUPLICATED          -4
#define ERR_NOT_FOUND           -5
#define ERR_READ_ONLY           -6


typedef struct int_key_record {
    unsigned int read_only;
    char name[KEY_NAME_SIZE];
    char data[KEY_DATA_SIZE];
} key_record_t;


typedef struct int_element_admin {
    unsigned int is_free;
    unsigned int app_id;
} element_admin_t;


typedef struct int_element_record {
    element_admin_t admin;
    key_record_t key;
} element_record_t;


typedef struct int_key_store {
    unsigned long free_slots;
    unsigned long max_elements;
    element_record_t *element_store;
} key_store_t;

typedef struct int_key_store_result {
    signed error;
    unsigned long index;
} key_store_result_t;

void key_store_init(
    key_store_t *key_store,
    unsigned long max_elements,
    element_record_t *element_store);
unsigned int key_store_init_with_read_only_keys(
    key_store_t *key_store,
    unsigned int const *app_ids,
    key_record_t const *keys,
    unsigned long nr_keys,
    unsigned long max_elements,
    element_record_t *element_store);
void key_store_wipe(key_store_t *key_store);
key_store_result_t key_store_add(key_store_t *key_store, unsigned int app_id, key_record_t const *key);
key_store_result_t key_store_get(key_store_t const *key_store, unsigned int app_id, const char name [KEY_NAME_SIZE], key_record_t *key);
unsigned int key_store_get_by_index(key_store_t const *key_store, unsigned int app_id, unsigned long index, key_record_t *key);
unsigned int key_store_delete(key_store_t *key_store, unsigned int app_id, const char name [KEY_NAME_SIZE]);
