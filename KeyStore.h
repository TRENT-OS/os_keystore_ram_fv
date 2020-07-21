

#define KEY_NAME_SIZE 32
#define KEY_DATA_SIZE 1024

#define NR_ELEMENTS 16
#define MAX_APP_ID 255


typedef struct int_key_name {
    char name[KEY_NAME_SIZE];
} key_name_t;


typedef struct int_key_data {
    char data[KEY_DATA_SIZE];
} key_data_t;


typedef struct int_key_record {
    key_name_t name;
    key_data_t data;
    unsigned int size;	
} key_record_t;        


typedef struct int_element_admin {
    unsigned int index;
    unsigned int is_free;
    unsigned int app_id;
} element_admin_t;


typedef struct int_element_record {
    element_admin_t admin;
    key_record_t key;	
} element_record_t;        


typedef struct int_key_store {
    unsigned free_slots;
    element_record_t element_store[NR_ELEMENTS];
} key_store_t;


void key_store_init(void);
void key_store_wipe(void);
int key_store_add(unsigned int app_id, key_record_t const *key, unsigned int *index);
int key_store_get(unsigned int app_id, key_name_t const *name, key_record_t *key, unsigned int *index);
int key_store_get_by_index(unsigned int app_id, unsigned int index, key_record_t *key);
int key_store_delete(unsigned int app_id, key_name_t const *name);

