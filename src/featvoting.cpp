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
ACTION featvoting::uvote(name user, name author, uint64_t pollid)
{
  require_auth(user);

  auto p_itr = polls.find(pollid);

  // check if event does not exist
  check(p_itr != polls.end(), "Poll does not exist!");

  // check if author exists in poll
  check(author_exists(p_itr->feats, author), "Author does not exist in poll!");

  // check if user has voted to poll
  check(has_voted(user, pollid), "You have already voted!");

  // get voteid counter
  config_s current_config = config.get();
  uint64_t voteid = current_config.votescounter++;
  config.set(current_config, get_self());

  // add vote
  cvotes.emplace(user, [&](auto &row)
                 {
                   row.voteid = voteid;
                   row.user = user;
                   row.feat_author = author;
                   row.pollid = pollid;
                 });
}

// create a poll
ACTION featvoting::xcpoll(string title, string description, int64_t startdate, int64_t enddate)
{
  require_auth(get_self());

  // check dates
  check(now() < startdate, "Start Date should now be before now!");
  check(now() < enddate && startdate < enddate, "End Date should be after now and start date!");

  config_s current_config = config.get();
  uint64_t pollid = current_config.pollscounter++;
  config.set(current_config, get_self());

  // create the poll
  polls.emplace(get_self(), [&](auto &row)
                {
                  row.id = pollid;
                  row.title = title;
                  row.description = description;
                  row.feats = vector<Feat>{};
                  row.startdate = startdate;
                  row.enddate = enddate;
                });
}

// add feat to poll
ACTION featvoting::xpolladdfeat(name author, uint64_t pollid)
{
  require_auth(get_self());

  auto itr = polls.find(pollid);
  auto f = subfeats.find(author.value);

  // check if feat exists
  check(f != subfeats.end(), "Feature request of author does not exist!");

  // check if poll exists
  check(itr != polls.end(), "Poll does not exist!");

  // check if author exists in poll
  check(!author_exists(itr->feats, author), "Author already exists in poll!");

  vector<Feat> fx = itr->feats;
  fx.push_back(Feat{f->author, f->title});

  // remove author's feat from submitted ones
  subfeats.erase(f);

  // add feat to feats
  polls.modify(itr, get_self(), [&](auto &row)
               { row.feats = fx; });
}

// remove feat from poll
ACTION featvoting::xpollremfeat(name author, uint64_t pollid)
{
  require_auth(get_self());

  auto itr = polls.find(pollid);

  // check if poll exists
  check(itr != polls.end(), "Poll does not exist!");

  // check if author exists in poll
  check(author_exists(itr->feats, author), "Author does not exist in poll!");

  vector<Feat> fx = itr->feats;
  auto item = find_if(fx.begin(), fx.end(), [&](const Feat &e)
                      { return e.author == author; }); // get_feat(fx, author);

  // remove from poll
  fx.erase(item);
  polls.modify(itr, get_self(), [&](auto &row)
               { row.feats = fx; });

  // re-add to subfeats
  subfeats.emplace(get_self(), [&](auto &row)
                   {
                     row.author = item->author;
                     row.title = item->title;
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
ACTION featvoting::xeraseafeats()
{
  require_auth(get_self());

  // Delete all records in apprfeats table
  auto itr = apprfeats.begin();
  while (itr != apprfeats.end())
  {
    itr = apprfeats.erase(itr);
  }
}

// remove all submitted feats
ACTION featvoting::xerasesfeats()
{
  require_auth(get_self());

  // Delete all records in subfeats table
  auto itr = subfeats.begin();
  while (itr != subfeats.end())
  {
    itr = subfeats.erase(itr);
  }
}

EOSIO_DISPATCH(featvoting, (init)(usubmitfeat)(uvote)(xcpoll)(xpolladdfeat)(xpollremfeat)(xapprfeat)(xdapprfeat)(xeraseafeats)(xerasesfeats))
