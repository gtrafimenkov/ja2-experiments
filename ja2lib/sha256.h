#ifndef JA2_SHA256
#define JA2_SHA256

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t data[64];
  uint32_t datalen;
  uint64_t bitlen;
  uint32_t state[8];
} SHA256_CTX;

typedef uint8_t SHA256[32];
typedef char SHA256STR[65];

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final(SHA256_CTX *ctx, SHA256 hash);

// Print hash to string as hex.
// The string must be at least 65 characters long.
void sha256_to_string(const SHA256 hash, SHA256STR string);

#endif  // JA2_SHA256
