//#include "rosePublicConfig.h"
#include "Combinatorics.h"

#ifdef ROSE_HAVE_GCRYPT_H
#include <gcrypt.h>
#endif

#include <stdint.h>
#include <string>
#include <vector>

namespace Combinatorics {

    bool
    flip_coin() {
        static LinearCongruentialGenerator rng;
        return 0 == (rng() & 1);
    }

    std::vector <uint8_t>
    sha1_digest(const std::vector <uint8_t> &data) {
        return sha1_digest(&data[0], data.size());
    }

    std::vector <uint8_t>
    sha1_digest(const std::string &data) {
        return sha1_digest((const uint8_t *) &data[0], data.size());
    }

    std::vector <uint8_t>
    sha1_digest(const uint8_t *data, size_t size) {
#ifndef ROSE_HAVE_GCRYPT_H
        return std::vector<uint8_t>();
#else
        gcry_md_hd_t md; // message digest
        gcry_error_t error __attribute__((unused)) = gcry_md_open(&md, GCRY_MD_SHA1, 0);
        assert(GPG_ERR_NO_ERROR==error);
        gcry_md_write(md, data, size);
        assert(gcry_md_get_algo_dlen(GCRY_MD_SHA1)==20);
        gcry_md_final(md);
        uint8_t *d = gcry_md_read(md, GCRY_MD_SHA1);
        assert(d!=NULL);
        std::vector<uint8_t> retval(d, d+20);
        gcry_md_close(md);
        return retval;
#endif
    }

    uint64_t
    fnv1a64_digest(const std::vector <uint8_t> &data) {
        return fnv1a64_digest(&data[0], data.size());
    }

    uint64_t
    fnv1a64_digest(const std::string &data) {
        return fnv1a64_digest((const uint8_t *) &data[0], data.size());
    }

    uint64_t
    fnv1a64_digest(const uint8_t *data, size_t size) {
        uint64_t hash = 0xcbf29ce484222325ull;
        for (size_t i = 0; i < size; ++i)
            hash = (hash ^ data[i]) * 0x100000001b3ull;
        return hash;
    }

    std::string digest_to_string(const uint8_t *data, size_t size) {
        std::string str;
        for (size_t i = 0; i < size; ++i) {
            str += "0123456789abcdef"[(data[i] >> 4) & 0xf];
            str += "0123456789abcdef"[data[i] & 0xf];
        }
        return str;
    }

    std::string
    digest_to_string(const std::vector <uint8_t> &data) {
        return digest_to_string(&data[0], data.size());
    }

    std::string
    digest_to_string(const std::string &data) {
        return digest_to_string((const uint8_t *) &data[0], data.size());
    }

} // namespace
