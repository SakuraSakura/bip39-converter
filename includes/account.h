#ifndef ACCOUNT_H_
#define ACCOUNT_H_

#include <vector>
#include <string>

using byte = unsigned char;

namespace MySeedConv {

  enum class NetworkFlag {
    LIVENET,
    TESTNET,
    ERROR
  };

  class Account {
   public:
    Account() = default;
    ~Account() = default;

    void recover_from_seed(std::vector<byte> seed);
    void recover_from_hex(std::string seed_hex_encoded);
    void recover_from_phrases(std::vector<std::string> phrases);
    void recover_from_base58(std::string seed_base58_encoded);
    void generate_seed(NetworkFlag network_flag);

    bool is_valid();
    NetworkFlag get_network_flag();
    std::string get_network_flag_str();
    std::vector<std::vector<byte>> get_keys(int key_count, int key_size);
    std::vector<std::string> get_keys_str(int key_count, int key_size);
    std::vector<byte> get_seed();
    std::string get_seed_str();
    std::vector<std::string> get_phrase();
    std::string get_base58_encoded();
    

   private:
    std::vector<byte> seed_;
    NetworkFlag network_flag_;

    void check_network_flag();
  };

}

#endif
