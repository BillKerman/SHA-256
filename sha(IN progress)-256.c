#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Define SHA-256 constants
#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

// SHA-256 functions
#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x,n) ((x) >> (n))
#define Ch(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

// Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19)
const uint32_t H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311)
const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Function prototypes
void sha256_transform(uint32_t state[8], const unsigned char block[64]);
void sha256(const unsigned char *message, size_t len, unsigned char digest[SHA256_DIGEST_SIZE]);

// SHA-256 main processing function
void sha256_transform(uint32_t state[8], const unsigned char block[64]) {
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t temp1, temp2;

    // Initialize hash values
    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    // Prepare message schedule W
    for (int i = 0; i < 16; i++) {
        W[i] = (block[i * 4 + 0] << 24) | (block[i * 4 + 1] << 16) |
               (block[i * 4 + 2] <<  8) | (block[i * 4 + 3] <<  0);
    }
    for (int i = 16; i < 64; i++) {
        W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
    }

    // Main loop
    for (int i = 0; i < 64; i++) {
        temp1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
        temp2 = Sigma0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    // Update hash values
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

// SHA-256 initialization function
void sha256_init(uint32_t state[8], size_t *bitlen, size_t *datalen) {
    *bitlen = 0;
    *datalen = 0;
    state[0] = H[0]; state[1] = H[1]; state[2] = H[2]; state[3] = H[3];
    state[4] = H[4]; state[5] = H[5]; state[6] = H[6]; state[7] = H[7];
}

// SHA-256 update function: updates the hash calculation with input message
void sha256_update(uint32_t state[8], size_t *bitlen, size_t *datalen, const unsigned char data[], size_t len) {
    size_t i;

    for (i = 0; i < len; ++i) {
        state[(*datalen) >> 3] |= (uint32_t)data[i] << (24 - ((*datalen) & 0x07));
        (*datalen) += 8;

        if (*datalen == 512) {
            sha256_transform(state, (unsigned char*)state);
            *bitlen += 512;
            *datalen = 0;
        }
    }
}

// SHA-256 final function: completes the hash calculation and outputs the digest
void sha256_final(uint32_t state[8], size_t *bitlen, size_t *datalen, unsigned char digest[SHA256_DIGEST_SIZE]) {
    size_t i;

    i = *datalen;

    // Pad whatever data is left in the buffer
    if (*datalen < 448) {
        state[i >> 5] |= 0x80000000 >> (i & 0x1f);
        state[14] = *bitlen >> 32;
        state[15] = *bitlen & 0xffffffff;
    } else {
        state[i >> 5] |= 0x80000000 >> (i & 0x1f);
        sha256_transform(state, (unsigned char*)state);
        memset(state, 0, 56);
        state[14] = *bitlen >> 32;
        state[15] = *bitlen & 0xffffffff;
    }

    sha256_transform(state, (unsigned char*)state);

    // Output the hash digest
    for (i = 0; i < 8; i++) {
        digest[i * 4 + 0] = (state[i] >> 24) & 0xff;
        digest[i * 4 + 1] = (state[i] >> 16) & 0xff;
        digest[i * 4 + 2] = (state[i] >>  8) & 0xff;
        digest[i * 4 + 3] = (state[i] >>  0) & 0xff;
    }
}

// SHA-256 function for calculating hash from input message
void sha256(const unsigned char *message, size_t len, unsigned char digest[SHA256_DIGEST_SIZE]) {
    uint32_t state[8];
    size_t bitlen = 0;
    size_t datalen = 0;

    sha256_init(state, &bitlen, &datalen);
    sha256_update(state, &bitlen, &datalen, message, len);
    sha256_final(state, &bitlen, &datalen, digest);
}

int main() {
    FILE *fp;
    char filename[] = "example.txt";
    unsigned char digest[SHA256_DIGEST_SIZE];
    unsigned char buffer[SHA256_BLOCK_SIZE];
    size_t len;
    uint32_t read = 0;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 1;
    }

    uint32_t state[8];
    size_t bitlen = 0;
    size_t datalen = 0;

    sha256_init(state, &bitlen, &datalen);

    while ((read = fread(buffer, 1, SHA256_BLOCK_SIZE, fp))) {
        sha256_update(state, &bitlen, &datalen, buffer, read);
    }

    sha256_final(state, &bitlen, &datalen, digest);

    fclose(fp);

    // Print the SHA-256 digest
    printf("SHA-256 hash of %s is: ", filename);
    for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
        printf("%02x", digest[i]);
    }
    printf("\n");

    return 0;
}
