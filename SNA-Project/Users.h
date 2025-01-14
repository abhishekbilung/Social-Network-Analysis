#ifndef USERS_H
#define USERS_H

#include "Header.h"
#include "Post.h"


// User class declaration
class User {
public:
    int user_id;
    string username;
    string password;
    char gender; // 'M', 'F', 'O'
    string country;
    int followersCount;
    int followingsCount;
    vector<Post> posts;
    vector<pair<int, string>> followers;   // ID, Name
    vector<pair<int, string>> followings;  // ID, Name

    // Constructor
    User(int id, const string& uname, const string& pwd, char g, const string& ctry);

    // Methods
    bool follows(int target_user_id) const;
};

#endif
