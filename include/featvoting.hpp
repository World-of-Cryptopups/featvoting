#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>

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
  ACTION uvote(name voter, name author, uint64_t pollid);

  // admin actions
  ACTION xcpoll(string title, string description, int64_t startdate, int64_t enddate);
  ACTION xpolladdfeat(name author, uint64_t pollid);
  ACTION xpollremfeat(name author, uint64_t pollid);
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

  TABLE polls_s
  {
    uint64_t id;
    string title;
    string description;
    vector<Feat> feats;
    int64_t startdate;
    int64_t enddate;

    uint64_t primary_key() const { return id; };
  };

  TABLE cvotes_s
  {
    uint64_t voteid;
    name user;
    name feat_author;
    uint64_t pollid;

    uint64_t primary_key() const { return voteid; };
    uint64_t by_user() const { return user.value; };
    uint64_t by_pollid() const { return pollid; };
  };

  TABLE config_s
  {
    uint64_t pollscounter = 100000;
    uint64_t votescounter = 100000;
  };

  // types
  typedef multi_index<"users"_n, users_s> users_t;
  typedef multi_index<"subfeats"_n, subfeats_s> subfeats_t;
  typedef multi_index<"apprfeats"_n, apprfeats_s> apprfeats_t;
  typedef multi_index<"currentvotes"_n, cvotes_s, indexed_by<"user"_n, const_mem_fun<cvotes_s, uint64_t, &cvotes_s::by_user>>, indexed_by<"pollid"_n, const_mem_fun<cvotes_s, uint64_t, &cvotes_s::by_pollid>>> cvotes_t;
  typedef multi_index<"polls"_n, polls_s> polls_t;
  typedef singleton<"config"_n, config_s> config_t;

  // tables
  config_t config = config_t(get_self(), get_self().value);
  users_t users = users_t(get_self(), get_self().value);
  subfeats_t subfeats = subfeats_t(get_self(), get_self().value);
  apprfeats_t apprfeats = apprfeats_t(get_self(), get_self().value);
  cvotes_t cvotes = cvotes_t(get_self(), get_self().value);
  polls_t polls = polls_t(get_self(), get_self().value);

  // checks if user has voted already to pollid
  bool has_voted(name user, uint64_t pollid)
  {
    auto idx = cvotes.get_index<"user"_n>();

    for (auto itr = idx.begin(); itr != idx.end(); itr++)
    {
      if (itr->user == user && itr->pollid == pollid)
      {
        return true;
      }
    }

    return false;
  }

  // checks if poll exists in
  bool author_exists(vector<Feat> feats, name author)
  {
    for (Feat i : feats)
    {
      if (i.author == author)
      {
        return true;
      }
    }

    return false;
  }

  // get current ts
  int64_t now()
  {
    return current_time_point().time_since_epoch().count();
    // return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
  }
};
