#include <featvoting.hpp>

// initialize db table
ACTION featvoting::init()
{
  require_auth(get_self());

  config.get_or_create(get_self(), config_s{});
}

// register user
ACTION featvoting::ureguser(name user)
{
  require_auth(user);

  users_t _users(get_self(), get_first_receiver().value);

  auto it = _users.find(user.value);

  if (it == _users.end())
  {
    // user is not in table
    _users.emplace(user, [&](auto &row)
                   { row.key = user; });
  }
}

// submit feature request
ACTION featvoting::usubmitfeat(name author, string title)
{
  require_auth(author);

  // check if user is registered or not
  check(users.find(author.value) != users.end(), "You are not registered!");

  // check if author has already submitted a feature request
  check(subfeats.find(author.value) == subfeats.end(), "You have already submitted a feature request!");

  // feature does not exist yet
  subfeats.emplace(get_self(), [&](auto &row)
                   {
                     row.author = author;
                     row.title = title;
                   });
}

// vote a feature request
ACTION featvoting::uvote(name user, name author)
{
  require_auth(user);

  auto _vfeats_it = apprfeats.find(author.value);

  check(_vfeats_it != apprfeats.end(), "Author's feature does not exist!");
  check(cvotes.find(user.value) == cvotes.end(), "You have already voted a feature request!");

  cvotes.emplace(get_self(), [&](auto &row)
                 {
                   row.user = user;
                   row.feat_author = _vfeats_it->author;
                 });
}

// approve a feature request
ACTION featvoting::xapprfeat(name author)
{
  require_auth(get_self());

  auto _feats_it = subfeats.find(author.value);

  check(_feats_it != subfeats.end(), "Author's feature does not exist!");
  check(apprfeats.find(author.value) == apprfeats.end(), "Author's feature has already been approved for voting.");

  // erase from submissions
  subfeats.erase(_feats_it);

  // add to approved feats for voting
  apprfeats.emplace(get_self(), [&](auto &row)
                    {
                      row.author = author;
                      row.title = _feats_it->title;
                    });
}

// disapprove a feature request
ACTION featvoting::xdapprfeat(name author)
{
  require_auth(get_self());

  auto _vfeats_it = apprfeats.find(author.value);

  check(_vfeats_it != apprfeats.end(), "Author's feature does not exist!");
  check(subfeats.find(author.value) == subfeats.end(), "Author's feature has already been approved for voting.");

  // erase from submissions
  apprfeats.erase(_vfeats_it);

  // add to approved feats for voting
  subfeats.emplace(get_self(), [&](auto &row)
                   {
                     row.author = author;
                     row.title = _vfeats_it->title;
                   });
}

// remove all approved feats
ACTION featvoting::xerasevfeats()
{
  require_auth(get_self());

  // Delete all records in apprfeats table
  auto itr = apprfeats.begin();
  while (itr != apprfeats.end())
  {
    itr = apprfeats.erase(itr);
  }
}

// remove all submittedfeats
ACTION featvoting::xerasefeats()
{
  require_auth(get_self());

  // Delete all records in subfeats table
  auto itr = subfeats.begin();
  while (itr != subfeats.end())
  {
    itr = subfeats.erase(itr);
  }
}

EOSIO_DISPATCH(featvoting, (init)(usubmitfeat)(uvote)(xapprfeat)(xdapprfeat)(xerasevfeats)(xerasefeats))
