/*
 * This source file is part of ARK
 * For the latest info, see https://github.com/ArkNX
 *
 * Copyright (c) 2013-2020 ArkNX authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include "base/AFPlatform.hpp"

#define MX (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z))
#define DELTA 0x9e3779b9

#define FIXED_KEY                                                                                                      \
    size_t i;                                                                                                          \
    uint8_t fixed_key[16];                                                                                             \
    memcpy(fixed_key, key, 16);                                                                                        \
    for (i = 0; (i < 16) && (fixed_key[i] != 0); ++i)                                                                  \
        ;                                                                                                              \
    for (++i; i < 16; ++i)                                                                                             \
        fixed_key[i] = 0;

namespace ark {

class AFXXTea
{
public:
    /**
     * Function: xxtea_encrypt
     * @data:    Data to be encrypted
     * @key:     Symmetric key
     * @encrypt_value: The value after encrypt
     * Returns:  Encrypted success or failure
     */
    static bool xxtea_encrypt(std::string const& data, std::string const& key, std::string& encrypt_value)
    {
        size_t out_len = 0;
        void* ret = xxtea_encrypt(data.c_str(), data.length(), key.c_str(), &out_len);
        if (ret == NULL)
        {
            return false;
        }

        encrypt_value.append((char*)ret, out_len);
        free(ret);
        return true;
    }

    /**
     * Function: xxtea_decrypt
     * @data:    Data to be decrypted
     * @key:     Symmetric key
     * @decrypt_value: The value after decrypt
     * Returns:  Decrypted success or failure
     */
    static bool xxtea_decrypt(std::string const& data, std::string const& key, std::string& decrypt_value)
    {
        size_t out_len = 0;
        void* ret = xxtea_decrypt(data.c_str(), data.length(), key.c_str(), &out_len);
        if (ret == NULL)
        {
            return false;
        }

        decrypt_value.append((char*)ret, out_len);
        free(ret);
        return true;
    }

    /**
     * Function: xxtea_encrypt
     * @data:    Data to be encrypted
     * @len:     Length of the data to be encrypted
     * @key:     Symmetric key
     * @out_len: Pointer to output length variable
     * Returns:  Encrypted data or %NULL on failure
     *
     * Caller is responsible for freeing the returned buffer.
     */
    static void* xxtea_encrypt(const void* data, size_t len, const void* key, size_t* out_len)
    {
        FIXED_KEY
        return xxtea_ubyte_encrypt((const uint8_t*)data, len, fixed_key, out_len);
    }

    /**
     * Function: xxtea_decrypt
     * @data:    Data to be decrypted
     * @len:     Length of the data to be decrypted
     * @key:     Symmetric key
     * @out_len: Pointer to output length variable
     * Returns:  Decrypted data or %NULL on failure
     *
     * Caller is responsible for freeing the returned buffer.
     */
    static void* xxtea_decrypt(const void* data, size_t len, const void* key, size_t* out_len)
    {
        FIXED_KEY
        return xxtea_ubyte_decrypt((const uint8_t*)data, len, fixed_key, out_len);
    }

protected:
    static uint32_t* xxtea_to_uint_array(const uint8_t* data, size_t len, int inc_len, size_t* out_len)
    {
        uint32_t* out;
        size_t n;

        n = (((len & 3) == 0) ? (len >> 2) : ((len >> 2) + 1));

        if (inc_len)
        {
            out = (uint32_t*)calloc(n + 1, sizeof(uint32_t));
            if (!out)
                return NULL;
            out[n] = (uint32_t)len;
            *out_len = n + 1;
        }
        else
        {
            out = (uint32_t*)calloc(n, sizeof(uint32_t));
            if (!out)
                return NULL;
            *out_len = n;
        }
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        memcpy(out, data, len);
#else
        for (size_t i = 0; i < len; ++i)
        {
            out[i >> 2] |= (uint32_t)data[i] << ((i & 3) << 3);
        }
#endif

        return out;
    }

    static uint8_t* xxtea_to_ubyte_array(const uint32_t* data, size_t len, int inc_len, size_t* out_len)
    {
        uint8_t* out;

        size_t m, n;

        n = len << 2;

        if (inc_len)
        {
            m = data[len - 1];
            n -= 4;
            if ((m < n - 3) || (m > n))
                return NULL;
            n = m;
        }

        out = (uint8_t*)malloc(n + 1);

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        memcpy(out, data, n);
#else
        for (size_t i = 0; i < n; ++i)
        {
            out[i] = (uint8_t)(data[i >> 2] >> ((i & 3) << 3));
        }
#endif

        out[n] = '\0';
        *out_len = n;

        return out;
    }

    static uint32_t* xxtea_uint_encrypt(uint32_t* data, size_t len, uint32_t* key)
    {
        uint32_t n = (uint32_t)len - 1;
        uint32_t z = data[n], y, p, q = 6 + 52 / (n + 1), sum = 0, e;

        if (n < 1)
            return data;

        while (0 < q--)
        {
            sum += DELTA;
            e = sum >> 2 & 3;

            for (p = 0; p < n; p++)
            {
                y = data[p + 1];
                z = data[p] += MX;
            }

            y = data[0];
            z = data[n] += MX;
        }

        return data;
    }

    static uint32_t* xxtea_uint_decrypt(uint32_t* data, size_t len, uint32_t* key)
    {
        uint32_t n = (uint32_t)len - 1;
        uint32_t z, y = data[0], p, q = 6 + 52 / (n + 1), sum = q * DELTA, e;

        if (n < 1)
            return data;

        while (sum != 0)
        {
            e = sum >> 2 & 3;

            for (p = n; p > 0; p--)
            {
                z = data[p - 1];
                y = data[p] -= MX;
            }

            z = data[n];
            y = data[0] -= MX;
            sum -= DELTA;
        }

        return data;
    }

    static uint8_t* xxtea_ubyte_encrypt(const uint8_t* data, size_t len, const uint8_t* key, size_t* out_len)
    {
        uint8_t* out;
        uint32_t *data_array, *key_array;
        size_t data_len, key_len;

        if (!len)
            return NULL;

        data_array = xxtea_to_uint_array(data, len, 1, &data_len);
        if (!data_array)
            return NULL;

        key_array = xxtea_to_uint_array(key, 16, 0, &key_len);
        if (!key_array)
        {
            free(data_array);
            return NULL;
        }

        out = xxtea_to_ubyte_array(xxtea_uint_encrypt(data_array, data_len, key_array), data_len, 0, out_len);

        free(data_array);
        free(key_array);

        return out;
    }

    static uint8_t* xxtea_ubyte_decrypt(const uint8_t* data, size_t len, const uint8_t* key, size_t* out_len)
    {
        uint8_t* out;
        uint32_t *data_array, *key_array;
        size_t data_len, key_len;

        if (!len)
            return NULL;

        data_array = xxtea_to_uint_array(data, len, 0, &data_len);
        if (!data_array)
            return NULL;

        key_array = xxtea_to_uint_array(key, 16, 0, &key_len);
        if (!key_array)
        {
            free(data_array);
            return NULL;
        }

        out = xxtea_to_ubyte_array(xxtea_uint_decrypt(data_array, data_len, key_array), data_len, 1, out_len);

        free(data_array);
        free(key_array);

        return out;
    }
};

} // namespace ark
