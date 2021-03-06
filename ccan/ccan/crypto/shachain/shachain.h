/* MIT (BSD) license - see LICENSE file for details */
#ifndef CCAN_CRYPTO_SHACHAIN_H
#define CCAN_CRYPTO_SHACHAIN_H
#include "config.h"
#include <ccan/crypto/sha256/sha256.h>
#include <stdbool.h>
#include <stdint.h>

/* Useful for testing. */
#ifndef shachain_index_t
#define shachain_index_t uint64_t
#endif

/**
 * shachain_from_seed - Generate an unpredictable SHA from a seed value.
 * @seed: (secret) seed value to use
 * @index: index of value to generate.
 * @hash: value generated
 *
 * There will be no way to derive the result from that generated for
 * any *lesser* index.
 *
 * Example:
 * #include <time.h>
 *
 * static void next_hash(struct sha256 *hash)
 * {
 *	static uint64_t index = 0;
 *	static struct sha256 seed;
 *
 *	// First time, initialize seed.
 *	if (index == 0) {
 *		// DO NOT DO THIS!  Very predictable!
 *		time_t now = time(NULL);
 *		memcpy(&seed, &now, sizeof(now));
 *	}
 *
 *	shachain_from_seed(&seed, index++, hash);
 * }
 */
void shachain_from_seed(const struct sha256 *seed, shachain_index_t index,
			struct sha256 *hash);

/**
 * shachain - structure for recording/deriving incrementing chain members
 * @max_index: maximum index value successfully shachain_add_hash()ed.
 * @num_valid: number of known[] array valid.  If non-zero, @max_index valid.
 * @known: known values to allow us to derive those <= @max_index.
 *
 * This is sufficient storage to derive any shachain hash value previously
 * added.
 */
struct shachain {
	shachain_index_t max_index;
	unsigned int num_valid;
	struct {
		shachain_index_t index;
		struct sha256 hash;
	} known[sizeof(shachain_index_t) * 8];
};

/**
 * shachain_init - initialize an shachain
 * @chain: the chain to initialize
 *
 * Alternately, ensure that it's all zero.
 */
void shachain_init(struct shachain *chain);

/**
 * shachain_add_hash - record the hash for the next index.
 * @chain: the chain to add to
 * @index: the index of the hash
 * @hash: the hash value.
 *
 * You can only add index 0 (for a freshly initialized chain), or one more
 * than the previously successfully added value.
 *
 * This can fail (return false without altering @chain) if the hash
 * for this index isn't consistent with previous hashes (ie. wasn't
 * generated from the same seed), though it can't always detect that.
 * If the hash is inconsistent yet undetected, the next addition will
 * fail.
 *
 * Example:
 * static void next_hash(const struct sha256 *hash)
 * {
 *	static uint64_t index = 0;
 *	static struct shachain chain;
 *
 *	if (!shachain_add_hash(&chain, index++, hash))
 *		errx(1, "Corrupted hash value?");
 * }
 */
bool shachain_add_hash(struct shachain *chain,
		       shachain_index_t index, const struct sha256 *hash);

/**
 * shachain_get_hash - get the hash for a given index.
 * @chain: the chain query
 * @index: the index of the hash to get
 * @hash: the hash value.
 *
 * This will return true and set @hash to that given in the successful
 * shachain_get_hash() call for that index.  If there was no
 * successful shachain_get_hash() for that index, it will return
 * false.
 *
 * Example:
 * #include <ccan/structeq/structeq.h>
 *
 * static void next_hash(const struct sha256 *hash)
 * {
 *	static uint64_t index = 0;
 *	static struct shachain chain;
 *
 *	if (!shachain_add_hash(&chain, index++, hash))
 *		errx(1, "Corrupted hash value?");
 *	else {
 *		struct sha256 check;
 *		assert(shachain_get_hash(&chain, index-1, &check));
 *		assert(structeq(&check, hash));
 *	}
 * }
 */
bool shachain_get_hash(const struct shachain *chain,
		       shachain_index_t index, struct sha256 *hash);
#endif /* CCAN_CRYPTO_SHACHAIN_H */
