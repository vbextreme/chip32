#ifndef __CHASH_H__
#define __CHASH_H__

#include <sos_type.h>
__extern_begin

typedef void(*rbhashfree_f)(uint32_t hash, const char* name, void* a);
typedef uint32_t(*rbhash_f)(const void* name, size_t len);
typedef int(*rbhashcmp_f)(void* a, size_t lenA, uint32_t hash, void* data, void* b, size_t lenB);

typedef struct rbhashElement{
	void* data;        /**< user data*/
	uint32_t hash;     /**< hash */
	uint32_t len;      /**< len of key*/
	uint32_t distance; /**< distance from hash*/
	char key[];        /**< flexible key*/
}rbhashElement_s;

typedef struct rbhash{
	rbhashElement_s* table; /**< hash table*/
	size_t size;            /**< total bucket of table*/
	size_t elementSize;     /**< sizeof rbhashElement*/
	size_t count;           /**< bucket used*/
	size_t min;             /**< percentage of free bucket*/
	size_t maxdistance;     /**< max distance from hash*/
	size_t keySize;         /**< key size*/
	rbhashfree_f del;       /**< function free data*/
	rbhash_f hashing;       /**< function calcolate hash*/
}rbhash_s;

typedef struct rbhashSearch rbhashSearch_s;

/*************/
/* hashalg.c */
/*************/

void hash_seed(uint32_t val);
uint32_t hash_one_at_a_time(const char *key, size_t len);
uint32_t hash_fasthash(const char* data, size_t len);
uint32_t hash_kr(const char*s, size_t len);
uint32_t hash_sedgewicks(const char* str, size_t len);
uint32_t hash_sobel(const char* str, size_t len);
uint32_t hash_weinberger(const char* str, size_t len);
uint32_t hash_elf(const char* str, size_t len);
uint32_t hash_sdbm(const char* str, size_t len);
uint32_t hash_bernstein(const char* str, size_t len);
uint32_t hash_knuth(const char* str, size_t len);
uint32_t hash_partow(const char* str, size_t len);
uint32_t hash_int(const unsigned* key, __unused size_t len);
uint64_t hash64_splitmix(uint64_t x);

/*rbhash.c*/

/** create rbhash
 * @param size number of starting element of table
 * @param min percentage min need to be available, if 0 the hash table is not automatic reallocated
 * @param keysize the max size of key
 * @param hashing hash function
 * @param del the function to delete each element
 * @return 0 successfull -1 error, err is pushed and errno is setted
 */
rbhash_s* rbhash_new(size_t size, size_t min, size_t keysize, rbhash_f hashing, rbhashfree_f del);

/** delete all hash table, call delete function to user data
 * @param rbh hash table
 */
void rbhash_free(rbhash_s* rbh);

/** cleanup */
void rbhash_free_auto(rbhash_s** rbh);

/** cleanup */
#define __rbhash_free __cleanup(rbhash_free_auto)

/** add new element to hash table
 * @param rbh hashtable
 * @param hash the hash value of key
 * @param key the key
 * @param len len of key
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
int rbhash_add_hash(rbhash_s* rbh, uint32_t hash, const void* key, size_t len, void* data);

/** add new element to hash table, call rbhash_add_hash calcolated hash with rbhash->hashing
 * @param rbh hashtable
 * @param key the key
 * @param len len of key
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
int rbhash_add(rbhash_s* rbh, const void* key, size_t len, void* data);

/** add new element to hash table only if key not exists, call rbhash_find and use rbhash->hashing
 * @param rbh hashtable
 * @param key the key
 * @param len len of key
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
int rbhash_add_unique(rbhash_s* rbh, const void* key, size_t len, void* data);

/** find rbhashElement
 * @param rbh hashtable
 * @param hash hash of key
 * @param key key to find
 * @param len len of key
 * @return rbhashElement or NULL for error
 */
rbhashElement_s* rbhash_find_hash_raw(rbhash_s* rbh, uint32_t hash, const void* key, size_t len);

/** find rbhashElement
 * @param rbh hashtable
 * @param hash hash of key
 * @param key key to find
 * @param len len of key
 * @return user data associated to key or NULL for error
 */
void* rbhash_find_hash(rbhash_s* rbh, uint32_t hash, const void* key, size_t len);

/** find rbhashElement, use rbhash_find_hash called with rbhash->hashing
 * @param rbh hashtable
 * @param key key to find
 * @param len len of key
 * @return user data associated to key or NULL for error
 */
void* rbhash_find(rbhash_s* rbh, const void* key, size_t len);

/** start new search on hash table, with search can get duplicate key in hash
 * @param rbh hashtable
 * @param key key to find
 * @param len size of key
 * @return search or null for error
 */
rbhashSearch_s* rbhash_search_new(rbhash_s* rbh, const void* key, size_t len);

/** search on hash table
 * @param rbh hashtable
 * @param search search object
 * @return data or null for error
 */
void* rbhash_search_continue(rbhash_s* rbh, rbhashSearch_s* search);

/** delete search when end of use
 * @param search object to free
 */
void rbhash_search_delete(rbhashSearch_s* search);

	/** remove element from hash table, automatic call delete function to user data
 * @param rbh hashtable
 * @param hash hash of key
 * @param key key to find
 * @param len len of key
 * @return 0 successfull -1 error
 */
int rbhash_remove_hash(rbhash_s* rbh, uint32_t hash, const void* key, size_t len);

/** remove element from hash table, automatic call delete function to user data, call rbhash_remove_hash with rbhash->hashing
 * @param ht hashtable
 * @param key key to find
 * @param len len of key
 * @return 0 successfull -1 error
 */
int rbhash_remove(rbhash_s* ht, const void* key, size_t len);

/** total memory usage
 * @param rbh
 * @return memory usage
 */
size_t rbhash_mem_total(rbhash_s* rbh);

/** count bucket
 * @param rbh
 * @return bucket count
 */
size_t rbhash_bucket_used(rbhash_s* rbh);

/** count number of collision, all hash with not have .distance == 0
 * @param rbh
 * @return total collision
 */
size_t rbhash_collision(rbhash_s* rbh);

/** max distance of slot
 * @param rbh
 * @return distance
 */
size_t rbhash_distance_max(rbhash_s* rbh);

const char* rbhash_slot_key(rbhash_s* rbh, size_t slot);
void* rbhash_slot_data(rbhash_s* rbh, size_t slot);
uint32_t rbhash_hash(rbhash_s* rbh, const void* key, size_t len);


size_t rbhash_dnswap(void);


__extern_end
#endif
