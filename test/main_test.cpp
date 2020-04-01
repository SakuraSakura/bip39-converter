#include <iostream>
#include <vector>
#include <string>

#include "account.h"

using namespace std;

int main() {
  MySeedConv::Account acct;
  acct.recover_from_hex("442f54cd072a9638be4a0344e1a6e5f01");
  //acct.recover_from_base58("9J877LVjhr3Xxd2nGzRVRVNUZpSKJF4TH");
  //acct.recover_from_phrases(vector<string> {
    //"during", "kingdom", "crew", "atom", "practice", "brisk", "weird",
    //"document", "eager", "artwork", "ride", "then"
      //});
  //acct.generate_seed(MySeedConv::NetworkFlag::LIVENET);
  auto phrases = acct.get_phrase();
  cout << "phrase: " << phrases[0];
  for (auto it = phrases.begin() + 1; it != phrases.end(); ++it)
    cout << " " << *it;
  cout << endl;
  cout << "base58: " << acct.get_base58_encoded() << endl;
  cout << "hex: " << acct.get_seed_str() << endl;
  cout << "network: " << acct.get_network_flag_str() << endl;
  cout << "key10: " << acct.get_keys_str(10, 32).back() << endl;

  /*
		phrase:  "during kingdom crew atom practice brisk weird document eager artwork ride then",
		base58:  "9J877LVjhr3Xxd2nGzRVRVNUZpSKJF4TH",
		hex:     "442f54cd072a9638be4a0344e1a6e5f01",
		network: "TESTNET",
		key10:   "be4a4a0730e19371edef04d38f9ca8187c376633f906ad602b7716e6d19b8374",
  */
  return 0;
}
