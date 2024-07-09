#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Define SHA-256 constants
constexpr unsigned int SHA256_DIGEST_LENGTH = 32; // SHA-256 produces a 256-bit (32-byte) hash

// Right rotate (circular right shift) function
inline uint32_t ROTR(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

// Right shift function
inline uint32_t SHR(uint32_t x, uint32_t n) {
    return x >> n;
}

// SHA-256 functions
inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t Sigma0(uint32_t x) {
    return ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22);
}

inline uint32_t Sigma1(uint32_t x) {
    return ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25);
}

inline uint32_t sigma0(uint32_t x) {
    return ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3);
}

inline uint32_t sigma1(uint32_t x) {
    return ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10);
}

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

// SHA-256 main processing function
void sha256(const std::string& input, uint32_t hash[8]) {
    std::istringstream is(input);
    std::stringstream ss;
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t temp1, temp2;

    // Initialize hash values
    a = H[0]; b = H[1]; c = H[2]; d = H[3];
    e = H[4]; f = H[5]; g = H[6]; h = H[7];

    uint64_t length = 0;
    char block[64];

    while (is.read(block, 64)) {
        length += 64;
        for (int i = 0; i < 16; ++i) {
            W[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
                  (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                  (static_cast<uint32_t>(block[i * 4 + 2]) <<  8) |
                  (static_cast<uint32_t>(block[i * 4 + 3]) <<  0);
        }

        for (int i = 16; i < 64; ++i) {
            W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
        }

        for (int i = 0; i < 64; ++i) {
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
        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    // Compute final hash as a string
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << std::setw(8) << std::setfill('0') << H[i];
    }

    // Copy hash to output
    std::string hashStr = ss.str();

    // Output the hash value
    std::cout << "SHA-256 hash of the file is: " << hashStr << std::endl;
}

int main() {
    std::string filename = "your_file.txt";
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    // Calculate the SHA-256 hash of the file
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string fileContent = oss.str();

    uint32_t hash[8];
    sha256(fileContent, hash);

    file.close();

    return 0;
}
