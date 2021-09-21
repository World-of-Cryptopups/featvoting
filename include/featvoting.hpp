#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

CONTRACT featvoting : public contract
{
public:
  using contract::contract;

  ACTION reguser(name user);
  ACTION submitfeat(name user, string title);
  ACTION erasefeats();

private:
  TABLE users {
    name key;

    uint64_t primary_key() const {return key.value;};
  };

  TABLE feats {
    name author;
    string title;

    uint64_t primary_key() const {return author.value;};
  };

  typedef multi_index<name("users"), users> users_table;
  typedef multi_index<name("feats"), feats> feats_table;
};
