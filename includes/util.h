#ifndef UTIL_H_
#define UTIL_H_

#include <vector>
#include <string>

using byte = unsigned char;

namespace MySeedConv { namespace Util {

  std::string hex_encode(std::vector<byte> byte_array);
  std::vector<byte> hex_decode(std::string);
  std::string base58_encode(std::vector<byte> byte_array);
  std::vector<byte> base58_decode(std::string);

}}

#endif
