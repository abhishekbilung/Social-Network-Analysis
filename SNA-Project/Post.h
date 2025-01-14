#ifndef POST_H
#define POST_H

#include "Header.h"

// Comment structure
struct Comment {
    int user_id;
    string username;
    string comment_content;
};

// Post structure
struct Post {
    int thread_id;
    string content;
    int likes;
    int commentCount;
    vector<pair<int, string>> likedBy; 
    vector<Comment> comments; 
};

#endif
