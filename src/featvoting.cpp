#include <featvoting.hpp>

ACTION featvoting::reguser(name user) {
  require_auth(user);

  users_table _users(get_self(), get_first_receiver().value);

  auto it = _users.find(user.value);

  if (it == _users.end()){
    // user is not in table
    _users.emplace(user, [&](auto &row){
      row.key = user; 
    });
  }
}

ACTION featvoting::submitfeat(name author, string title) {
  require_auth(author);

  users_table _users(get_self(), get_first_receiver().value);
  feats_table _feats(get_self(), get_first_receiver().value);

  // check if user is registered or not
  check(_users.find(author.value) != _users.end(), "You are not registered!");

  // check if author has already submitted a feature request
  check(_feats.find(author.value) == _feats.end(), "You have already submitted a feature request!");

  // feature does not exist yet
  _feats.emplace(get_self(), [&](auto &row){
    row.author = author;
    row.title = title;
  });
}


ACTION featvoting::apprfeatvote(name author) {
  require_auth(get_self());  

  feats_table _feats(get_self(), get_first_receiver().value);
  votingfeats_table _vfeats(get_self(), get_first_receiver().value);

  auto _feats_it = _feats.find(author.value);

  check(_feats_it != _feats.end(), "Author's feature does not exist!");
  check(_vfeats.find(author.value) == _vfeats.end(), "Author's feature has already been approved for voting.");
  
  // erase from submissions
  _feats.erase(_feats_it);

  // add to approved feats for voting
  _vfeats.emplace(get_self(), [&](auto &row){
    row.author = author;
    row.title = _feats_it->title;
  });
}


ACTION featvoting::erasefeats() {
  require_auth(get_self());

  feats_table _feats(get_self(), get_self().value);

  // Delete all records in _feats table
  auto itr = _feats.begin();
  while (itr != _feats.end()){
    itr = _feats.erase(itr);
  }
}

EOSIO_DISPATCH(featvoting, (reguser)(submitfeat)(erasefeats)(apprfeatvote))
