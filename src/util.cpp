#include "util.h"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "openssl/bn.h"

namespace MySeedConv { namespace Util {

  static const std::string base58_code_string
    ("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");

  std::string hex_encode(std::vector<byte> byte_array) {
    char *buf = new char[byte_array.size() * 2 + 1];
    for (size_t i = 0; i < byte_array.size(); ++i)
      sprintf(buf + i * 2, "%02X", byte_array[i]);
    std::string str(buf);
    delete[] buf;
    return str;
  }

  std::vector<byte> hex_decode(std::string str) {
    if (str.size() % 2)
      str.insert(str.begin(), '0');

    auto byte_array_size = str.size() / 2;
    std::vector<byte> byte_array(byte_array_size);

    for (size_t i = 0; i < byte_array_size; ++i)
      sscanf(str.data() + i * 2, "%2hhx", &byte_array[i]);
    return byte_array;
  }

  std::string base58_encode(std::vector<byte> byte_array) {
    BIGNUM *x = BN_bin2bn(byte_array.data(), byte_array.size(), NULL);
    BIGNUM *big_zero = BN_new();
    std::string encoded_str;
    encoded_str.reserve(byte_array.size() * 136 / 100);

    while (BN_cmp(x, big_zero) > 0) {
      auto mod = BN_div_word(x, 58);
      encoded_str.push_back(base58_code_string[mod]);
    }

    for (auto it = byte_array.begin(); (it != byte_array.end()) && (! *it); ++it)
      encoded_str.push_back(base58_code_string[0]);

    BN_free(x);
    BN_free(big_zero);
    std::reverse(encoded_str.begin(), encoded_str.end());
    return encoded_str;
  }

  std::vector<byte> base58_decode(std::string str) {
    BIGNUM *x = BN_new();
    auto it = str.begin();
    std::vector<byte> decoded;

    for (; *it == base58_code_string[0]; ++it)
      decoded.push_back(0);

    for (; it != str.end(); ++it) {
      auto pos = base58_code_string.find_first_of(*it);
      if (pos == std::string::npos)
        throw std::runtime_error("unable to decode base58 string");
      BN_mul_word(x, 58);
      BN_add_word(x, pos);
    }

    auto num_zeros = decoded.size();
    decoded.resize(num_zeros + BN_num_bytes(x));
    BN_bn2bin(x, decoded.data() + num_zeros);
    BN_free(x);
    return decoded;
  }

}}
