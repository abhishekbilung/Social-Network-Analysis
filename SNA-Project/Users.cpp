#include "Users.h"

//constructor definition
User::User(int id, const string& uname, const string& pwd, char g, const string& ctry)
        : user_id(id), username(uname), password(pwd), gender(g), country(ctry) {}


//Methode definiton
bool User::follows(int target_user_id) const {
        for (auto& following : followings) {
            if (following.first == target_user_id) {
                return true;
            }
        }
        return false;
    }