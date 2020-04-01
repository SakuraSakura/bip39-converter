#ifndef CONSTANT_H_
#define CONSTANT_H_

#include <vector>

using byte = unsigned char;

namespace MySeedConv { namespace Constant {

  static const int seed_length = 17;
  static const int bip39_phrase_length = 12;
  static const std::vector<byte> seed_header = {0x5a, 0xfe, 0x02};
  static const int seed_checksum_length = 4;

}}

#endif
