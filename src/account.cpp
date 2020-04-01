#include "account.h"

#include <cstdio>
#include <stdexcept>
#include <cassert>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "constant.h"
#include "bip39dict.h"
#include "util.h"

namespace MySeedConv {

  int bip39_masks[] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023};

  bool Account::is_valid() {
    return (seed_.size() == Constant::seed_length)
      && ( (seed_[Constant::seed_length] & 0x0f) == 0);
  }

  NetworkFlag Account::get_network_flag() { return network_flag_; }
  std::string Account::get_network_flag_str() {
    if (network_flag_ == NetworkFlag::LIVENET)
      return "LIVENET";
    else if (network_flag_ == NetworkFlag::TESTNET)
      return "TESTNET";
    else
      return "ERROR";
  }

  std::vector<byte> Account::get_seed() { return seed_; }

  void Account::recover_from_seed(std::vector<byte> seed) {
    if ( seed.size() != Constant::seed_length || (seed.back() & 0x0f) )
      throw std::runtime_error("seed length mismatch");
    seed_ = seed;
    check_network_flag();
  }

  void Account::recover_from_hex(std::string seed_hex_encoded) {
    seed_hex_encoded += "0";
    recover_from_seed(Util::hex_decode(seed_hex_encoded));
  }

  void Account::recover_from_phrases(std::vector<std::string> phrases) {
    if (phrases.size() != Constant::bip39_phrase_length)
      throw std::runtime_error("length mismatch for BIP39 phrases");
    seed_.clear();
    int remainder = 0;
    size_t bits = 0;

    for (size_t i = 0; i < phrases.size(); ++i) {
      int idx;
      auto it = std::lower_bound(bip39dict.begin(), bip39dict.end(), phrases[i]);
      if (it != bip39dict.end() && *it == phrases[i])
        idx = it - bip39dict.begin();
      else
        throw std::runtime_error("invalid word in BIP39 phrases");

      remainder = (remainder << 11) + idx;
      for (bits += 11; bits >= 8; bits -= 8) {
        int b = 0xff & (remainder >> (bits - 8));
        seed_.push_back(b);
        //printf(
          //"word[%d]: word: %s remainder: 0x%08x a: 0x%04x bits: %d seed: %s\n",
          //i, phrases[i].c_str(), remainder, b, bits, Util::hex_encode(seed_).c_str());
      }
      remainder &= bip39_masks[bits];
    }

    if (seed_.size() != Constant::seed_length - 1 || bits != 4)
      throw std::runtime_error("error when recover from phrases");
    seed_.push_back(remainder << 4);
    check_network_flag();
  }

  void Account::recover_from_base58(std::string seed_base58_encoded) {
    auto seed = Util::base58_decode(seed_base58_encoded);
    auto key_length = seed.size() - Constant::seed_header.size() 
      - Constant::seed_checksum_length;
    if ( key_length != Constant::seed_length || 
         (seed[Constant::seed_header.size() + Constant::seed_length - 1] & 0x0f) )
      throw std::runtime_error("invalid seed length");

    if (! std::equal(Constant::seed_header.begin(), Constant::seed_header.end(),
                     seed.begin()) )
      throw std::runtime_error("invalid seed header");

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_sha3_256();
    byte checksum[32];
    auto checksum_start = seed.size() - Constant::seed_checksum_length;

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, seed.data(), checksum_start);
    EVP_DigestFinal_ex(mdctx, checksum, NULL);
    EVP_MD_CTX_free(mdctx);

    if (! std::equal(seed.begin() + checksum_start, seed.end(),
                     checksum) )
      throw std::runtime_error("seed checksum mismatch");
    
    seed_.assign(seed.begin() + Constant::seed_header.size(),
                 seed.end() - Constant::seed_checksum_length);
    check_network_flag();
  }

  void Account::generate_seed(NetworkFlag network_flag) {
    byte seed[Constant::seed_length];

    if (!RAND_bytes(seed, Constant::seed_length - 1))
      throw std::runtime_error("unable to generate random seed");

    seed[Constant::seed_length - 1] = seed[Constant::seed_length - 2] & 0xf0;
    byte mode = (seed[0] & 0x80) | (seed[1] & 0x40) |
      (seed[2] & 0x20) | (seed[3] & 0x10);
    if (network_flag == NetworkFlag::TESTNET)
      mode = mode ^ 0xf0;
    else if (network_flag == NetworkFlag::ERROR)
      throw std::invalid_argument("network flag should be LIVENET or TESTNET");
    seed[Constant::seed_length - 2] = 
      mode | (seed[Constant::seed_length - 2] & 0x0f);

    seed_ = std::vector<byte>(seed, seed + Constant::seed_length);
    network_flag_ = network_flag;
  }

  std::vector<std::vector<byte>> Account::get_keys(int key_count, int key_size) {
    assert(key_count > 0);
    assert(key_size > 0);

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_shake256();
    byte keys[key_count][key_size];

    EVP_DigestInit_ex(mdctx, md, NULL);
    for (auto i = 0; i < 4; ++i)
      EVP_DigestUpdate(mdctx, seed_.data(), seed_.size());
    EVP_DigestFinalXOF(mdctx, keys[0], key_size * key_count);
    EVP_MD_CTX_free(mdctx);

    std::vector<std::vector<byte>> keys_v;
    for (int i = 0; i < key_count; ++i)
      keys_v.push_back(std::vector<byte>(keys[0] + key_size * i,
                                         keys[0] + key_size * (i + 1)));
    return keys_v;
  }

  std::vector<std::string> Account::get_keys_str(int key_count, int key_size) {
    auto keys = get_keys(key_count, key_size);
    std::vector<std::string> keys_str(keys.size());
    for (size_t i = 0; i < keys.size(); ++i)
      keys_str[i] = Util::hex_encode(keys[i]);
    return keys_str;
  }

  std::string Account::get_seed_str() {
    std::string seed_str = Util::hex_encode(seed_);
    assert(seed_str.size() == Constant::seed_length * 2);
    assert(seed_str.back() == '0');
    seed_str.pop_back();
    return seed_str;
  }

  std::vector<std::string> Account::get_phrase() {
    std::vector<std::string> phrases;
    int accumulator = 0, n = 0;
    size_t bits = 0;

    for (size_t i = 0; i < seed_.size(); ++i) {
      accumulator = (accumulator << 8) + (int) seed_[i];
      bits += 8;
      if (bits >= 11) {
        bits -= 11;
        n += 1;
        int idx = accumulator >> bits;
        accumulator &= bip39_masks[bits];
        std::string phrase = bip39dict[idx];
				//printf("word[%d]: bits: %d  index: %d  word: %s\n",
               //n, bits, idx, phrase);
        phrases.push_back(phrase);
      }
    }

    return phrases;
  }

  std::string Account::get_base58_encoded() {
    std::vector<byte> seed_packed;
    seed_packed.reserve(Constant::seed_header.size() +
                        Constant::seed_length +
                        Constant::seed_checksum_length);

    seed_packed.insert(seed_packed.end(), Constant::seed_header.begin(),
                       Constant::seed_header.end());
    seed_packed.insert(seed_packed.end(), seed_.begin(), seed_.end());
    
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_sha3_256();
    byte checksum[32];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, seed_packed.data(), seed_packed.size());
    EVP_DigestFinal_ex(mdctx, checksum, NULL);
    EVP_MD_CTX_free(mdctx);

    seed_packed.insert(seed_packed.end(), checksum,
                       checksum + Constant::seed_checksum_length);
    return Util::base58_encode(seed_packed);
  }

  void Account::check_network_flag() {
    byte mode = (seed_[0] & 0x80) | (seed_[1] & 0x40) |
      (seed_[2] & 0x20) | (seed_[3] & 0x10);
    if (mode == (seed_[Constant::seed_length - 2] & 0xf0) )
      network_flag_ = NetworkFlag::LIVENET;
    else if (mode == ((seed_[Constant::seed_length - 2] & 0xf0) ^ 0xf0) )
      network_flag_ = NetworkFlag::TESTNET;
    else
      network_flag_ = NetworkFlag::ERROR;
  }

}
