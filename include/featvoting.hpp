#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

using namespace std;
using namespace eosio;

// feat struct
struct Feat
{
  name author;
  string title;
};

CONTRACT featvoting : public contract
{
public:
  using contract::contract;

  // init
  ACTION init();

  // user actions
  ACTION ureguser(name user);
  ACTION usubmitfeat(name user, string title);
  ACTION uvote(name voter, name author);

  // admin actions
  ACTION xcpoll(string title, string description, uint64_t startdate, uint64_t enddate);
  ACTION xpolladdfeat(name author);
  ACTION xpollremfeat(name author);
  ACTION xapprfeat(name author);
  ACTION xdapprfeat(name author);
  ACTION xeraseafeats();
  ACTION xerasesfeats();

private:
  TABLE users_s
  {
    name key;

    uint64_t primary_key() const { return key.value; };
  };

  TABLE subfeats_s
  {
    name author;
    string title;

    uint64_t primary_key() const { return author.value; };
  };

  TABLE apprfeats_s
  {
    name author;
    string title;

    uint64_t primary_key() const { return author.value; };
  };

  TABLE events_s
  {
    uint64_t id;
    string title;
    string description;
    vector<Feat> feats;
    uint64_t startdate;
    uint64_t enddate;

    uint64_t primary_key() const { return id; };
  };

  TABLE cvotes_s
  {
    name user;
    name feat_author;

    uint64_t primary_key() const { return user.value; };
  };

  TABLE config_s
  {
    uint64_t eventscounter;
  };

  // types
  typedef multi_index<"users"_n, users_s> users_t;
  typedef multi_index<"subfeats"_n, subfeats_s> subfeats_t;
  typedef multi_index<"apprfeats"_n, apprfeats_s> apprfeats_t;
  typedef multi_index<"currentvotes"_n, cvotes_s> cvotes_t;
  typedef singleton<"config"_n, config_s> config_t;

  // tables
  config_t config = config_t(get_self(), get_self().value);
  users_t users = users_t(get_self(), get_self().value);
  subfeats_t subfeats = subfeats_t(get_self(), get_self().value);
  apprfeats_t apprfeats = apprfeats_t(get_self(), get_self().value);
  cvotes_t cvotes = cvotes_t(get_self(), get_self().value);
};
