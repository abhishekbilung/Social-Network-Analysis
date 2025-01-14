#include "Global.h"    
#include <iostream>

//GLobal Variables

vector<User> users;
int next_user_id = 1;
unordered_map<string, vector<Post>> userPosts;
unordered_set<string> onlineUsers; 
unordered_map<string, vector<string>> adjList;
User* current_user = nullptr; 
User* search_user;

//Function to load user data
void loadUserData() {
    ifstream inFile(USER_FILE);
    if (!inFile) {
        cerr << "Error opening user file for reading!" << endl;
        return;
    }

    string line;
    while (getline(inFile, line)) {
       
        size_t commaPos1 = line.find(',');
        size_t commaPos2 = line.find(',', commaPos1 + 1);
        size_t commaPos3 = line.find(',', commaPos2 + 1);
        size_t commaPos4 = line.find(',', commaPos3 + 1);

     
        if (commaPos1 != string::npos && commaPos2 != string::npos &&
            commaPos3 != string::npos && commaPos4 != string::npos) {

            try {
                // Parse ID
                int id = stoi(line.substr(0, commaPos1));
                

                // Parse username
                string uname = line.substr(commaPos1 + 1, commaPos2 - commaPos1 - 1);
                

                // Parse password
                string pwd = line.substr(commaPos2 + 1, commaPos3 - commaPos2 - 1);
                

                
                char gender = line[commaPos3 + 1];
                

                
                string country = line.substr(commaPos4 + 1);
               

                
                User user(id, uname, pwd, gender, country);

                
                users.push_back(user);
                next_user_id = max(next_user_id, id + 1); // Update next_user_id
            } catch (const std::invalid_argument& e) {
                cerr << "Error parsing data in line: " << line << endl;
                cerr << "Invalid argument for stoi, skipping this line." << endl;
                continue; // Skip this line and move to the next one
            } catch (const std::out_of_range& e) {
                cerr << "Value out of range in line: " << line << endl;
                cerr << "Skipping this line." << endl;
                continue;
            }
        }
    }

    inFile.close();
    // cout << "User data loaded successfully!" << endl;
}

//FUnction to save user data
void saveUserData() {
    ofstream outFile(USER_FILE);
    if (!outFile) {
        cerr << "Error opening user file for writing!" << endl;
        return;
    }

    for (const auto& user : users) {
        outFile << user.user_id << ","
                << user.username << ","
                << user.password << ","
                << user.gender << ","
                << user.country << endl;
    }

    outFile.close();
    cout << "User data saved successfully!" << endl;
}

//Function to load post data from file
void loadPostData(User* user) {
    if (user == nullptr) {
        cerr << "Null pointer passed to loadPostData" << endl;
        return;
    }
    ifstream inFile("Post/" + user->username + ".txt");
    if (!inFile.is_open()) {
        cerr << "Error opening post file for user: " << user->username << endl;
        return;
    }

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue; 
        

        stringstream ss(line);
        string post_id_str, content, likes_str, likedby_str, comment_count_str, comments_str;

        
        if (getline(ss, post_id_str, '|') && getline(ss, content, '|') &&
            getline(ss, likes_str, '|') && getline(ss, likedby_str, '|') &&
            getline(ss, comment_count_str, '|') && getline(ss, comments_str)) {
            
       
            auto trim = [](string& str) {
                const auto start = str.find_first_not_of('|');
                if (start == string::npos) return;
                const auto end = str.find_last_not_of('|');
                str = str.substr(start, end - start + 1);
            };

            trim(post_id_str);
            trim(content);
            trim(likes_str);
            trim(likedby_str);
            trim(comment_count_str);
            trim(comments_str);

            Post post;
            try {
                post.thread_id = stoi(post_id_str);
                post.content = content;
                post.likes = stoi(likes_str);
                post.commentCount = stoi(comment_count_str);
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid argument for stoi: " << e.what() << endl;
                cerr << "Failed to parse post ID: " << post_id_str << ", likes: " << likes_str << ", comment count: " << comment_count_str << endl;
                continue; 
            } catch (const std::out_of_range& e) {
                cerr << "Out of range error for stoi: " << e.what() << endl;
                cerr << "Failed to parse post ID: " << post_id_str << ", likes: " << likes_str << ", comment count: " << comment_count_str << endl;
                continue;
            }

            
            bool duplicate = false;
            for (const auto& existingPost : user->posts) {
                if (existingPost.thread_id == post.thread_id) {
                    duplicate = true;
                    break;
                }
            }
            
            if (duplicate) {
                continue; 
            }

           
            if (!likedby_str.empty()) {
                stringstream likedby_ss(likedby_str);
                string likedby_entry;
                while (getline(likedby_ss, likedby_entry, ';')) {
                    likedby_entry = likedby_entry.substr(likedby_entry.find_first_of('(') + 1, likedby_entry.find_last_of(')') - likedby_entry.find_first_of('(') - 1);
                    stringstream pair(likedby_entry);
                    string user_id_str, username;
                    try {
                        if (getline(pair, user_id_str, ',') && getline(pair, username)) {
                            if (all_of(user_id_str.begin(), user_id_str.end(), ::isdigit)) {
                                post.likedBy.push_back({stoi(user_id_str), username});
                            } else {
                                cerr << "Invalid user ID in likedBy: " << user_id_str << endl;
                            }
                        }
                    } catch (const std::invalid_argument& e) {
                        cerr << "Invalid argument for stoi in likedBy: " << e.what() << endl;
                        cerr << "Failed to parse user ID: " << user_id_str << endl;
                    }
                }
            }

            
            if (!comments_str.empty()) {
                stringstream comments_ss(comments_str);
                string comment_entry;
                while (getline(comments_ss, comment_entry, ';')) {
                    comment_entry = comment_entry.substr(comment_entry.find_first_of('(') + 1, comment_entry.find_last_of(')') - comment_entry.find_first_of('(') - 1);
                    stringstream comment_data(comment_entry);
                    string user_id_str, username, comment_content;
                    try {
                        if (getline(comment_data, user_id_str, ',') && getline(comment_data, username, ',') &&
                            getline(comment_data, comment_content)) {
                            if (all_of(user_id_str.begin(), user_id_str.end(), ::isdigit)) {
                                post.comments.push_back({stoi(user_id_str), username, comment_content});
                            } else {
                                cerr << "Invalid user ID in comments: " << user_id_str << endl;
                            }
                        }
                    } catch (const std::invalid_argument& e) {
                        cerr << "Invalid argument for stoi in comments: " << e.what() << endl;
                        cerr << "Failed to parse user ID: " << user_id_str << endl;
                    }
                }
            }

            
            user->posts.push_back(post);
            userPosts[user->username].push_back(post);
        } else {
            cerr << "Failed to parse line: " << line << endl;
        }
    }

    inFile.close();
}

//Function to save post data into files
void savePostData(User* user) {
    if (user == nullptr) {
        cerr << "Null pointer passed to savePostData" << endl;
        return;
    }

    
    ofstream outFile("Post/" + user->username + ".txt");
    if (!outFile) {
        cerr << "Error opening file for user: " << user->username << endl;
        return;
    }

    
    set<int> posted_ids;

    
    for (const auto& post : user->posts) {
        if (posted_ids.find(post.thread_id) != posted_ids.end()) {
            cerr << "Duplicate post detected with ID: " << post.thread_id << endl;
            continue; 
        }

        
        posted_ids.insert(post.thread_id);

        outFile << post.thread_id << "|" << post.content<< "|" << post.likes << "|";

        
        if (!post.likedBy.empty()) {
            for (size_t i = 0; i < post.likedBy.size(); ++i) {
                outFile << "(" << post.likedBy[i].first << "," << post.likedBy[i].second << ")";
                if (i != post.likedBy.size() - 1) {
                    outFile << ";"; 
                }
            }
        } else {
            outFile << "()";
        }
        outFile << "|" << post.commentCount << "|";

        
        if (!post.comments.empty()) {
            for (size_t i = 0; i < post.comments.size(); ++i) {
                outFile << "(" << post.comments[i].user_id << "," << post.comments[i].username << "," << post.comments[i].comment_content << ")";
                if (i != post.comments.size() - 1) {
                    outFile << ";";  
                }
            }
        } else {
            outFile << "()";
        }
        outFile << endl;  
    }

    outFile.close();
}

//Function to load followers from files
void loadFollowers(User* loaduser) {
    if (loaduser == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

    loaduser->followersCount = 0;
    ifstream followers_file("followers/" + loaduser->username + ".txt");  
    if (followers_file.is_open()) {
        string line;
        while (getline(followers_file, line)) {
            stringstream ss(line);
            string follower_id_str, follower_username;

            
            if (getline(ss, follower_id_str, ' ') && getline(ss, follower_username, ' ')) {
                int follower_id = stoi(follower_id_str);

                
                for (auto& user : users) {
                    if (user.user_id == follower_id) {
                        loaduser->followers.push_back(make_pair(user.user_id,user.username));
                        loaduser->followersCount++;
                        break;
                    }
                }
            }
        }
        followers_file.close();
    } else {
        cout<<"";
        // cout << "Unable to open file to load followers for user: " << loaduser->username << endl;
    }
}

//Function to load followings from files
void loadFollowings(User* loaduser) {
    if (loaduser == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

    loaduser->followingsCount = 0;
    ifstream followings_file("followings/" + loaduser->username + ".txt");  
    if (followings_file.is_open()) {
        string line;
        while (getline(followings_file, line)) {
            stringstream ss(line);
            string following_id_str, following_username;

            
            if (getline(ss, following_id_str, ' ') && getline(ss, following_username, ' ')) {
                
                int following_id = stoi(following_id_str);

                
                for (auto& user : users) {
                    if (user.user_id == following_id) {
                        loaduser->followings.push_back(make_pair(user.user_id,user.username));
                        loaduser->followingsCount++;
                        break;
                    }
                }
            }
        }
        followings_file.close();
    } else {
        cout<<"";
        // cout << "Unable to open file to load followings for user: " << loaduser->username << endl;
    }
}

//Function to remove follower from files
void removeFollower() {
    if (search_user == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

    string filename = "followers/" + search_user->username + ".txt";  
    ifstream followers_file(filename);
    ofstream temp_file("followers/temp.txt"); 

    if (!followers_file.is_open() || !temp_file.is_open()) {
        cout << "Unable to open file to modify followers." << endl;
        return;
    }

    string line;
    bool removed = false;
    bool first_line = true;

    while (getline(followers_file, line)) {
        stringstream ss(line);
        string user_id_str, username;
        getline(ss, user_id_str, ' ');
        getline(ss, username);

        if (user_id_str != to_string(current_user->user_id)) {
            if (!first_line) {
                temp_file << endl;
            }
            temp_file << user_id_str << " " << username;  
            first_line = false;
        } else {
            removed = true;
        }
    }

    followers_file.close();
    temp_file.close();

    if (removed) {
        remove(filename.c_str());
        rename("followers/temp.txt", filename.c_str());  
        cout << "Follower removed successfully." << endl;
    } else {
        cout << "Follower not found." << endl;
        remove("followers/temp.txt");
    }
}

//Function to remove followings from files
void removeFollowing() {
    if (current_user == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

    string filename = "followings/" + current_user->username + ".txt";  
    ifstream followings_file(filename);
    ofstream temp_file("followings/temp.txt");  

    if (!followings_file.is_open() || !temp_file.is_open()) {
        cout << "Unable to open file to modify followings." << endl;
        return;
    }

    string line;
    bool removed = false;
    bool first_line = true;

    while (getline(followings_file, line)) {
        stringstream ss(line);
        string user_id_str, username;
        getline(ss, user_id_str, ' ');  
        getline(ss, username);

        if (user_id_str != to_string(search_user->user_id)) {
            if (!first_line) {
                temp_file << endl;
            }
            temp_file << user_id_str << " " << username;  
            first_line = false;
        } else {
            removed = true;
        }
    }

    followings_file.close();
    temp_file.close();

    if (removed) {
        remove(filename.c_str());
        rename("followings/temp.txt", filename.c_str());  
        cout << "Following removed successfully." << endl;
    } else {
        cout << "Following not found." << endl;
        remove("followings/temp.txt");
    }
}

//Function to add follower into files
void addFollower(User* new_follower) {
    if (search_user == nullptr || new_follower == nullptr) {
        cout << "Current user or follower is not set." << endl;
        return;
    }

    ifstream followers_file("followers/" + search_user->username + ".txt");
    string line;
    bool already_follower = false;

   
    while (getline(followers_file, line)) {
        stringstream ss(line);
        string follower_id_str;
        getline(ss, follower_id_str, ' '); 
        if ( follower_id_str == to_string(new_follower->user_id)) {
            already_follower = true;
            break;
        }
    }
    followers_file.close();

    if (already_follower) {
        cout << new_follower->username << " is already a follower." << endl;
        return;
    }

    ofstream followers_file_append("followers/" + search_user->username + ".txt", ios::app);  // Append mode
    if (followers_file_append.is_open()) {
        followers_file_append << new_follower->user_id << " " << new_follower->username << endl;  // Write user_id and username with a space
        followers_file_append.close();
        cout << "Successfully added " << new_follower->username << " to "<<search_user->username<<" followerss." << endl;
    } else {
        cout << "Unable to open file to save follower." << endl;
    }
}



// Function to add followings into files
void addFollowing(User* new_following) {
    if (current_user == nullptr || new_following == nullptr) {
        cout << "Current user or following is not set." << endl;
        return;
    }

    // Check existing followings
    ifstream followings_file("followings/" + current_user->username + ".txt");
    string line;
    bool already_following = false;

    while (getline(followings_file, line)) {
        stringstream ss(line);
        string following_id_str;
        getline(ss, following_id_str, ' ');  
        if (following_id_str == to_string(new_following->user_id)) {
            already_following = true;
            break;
        }
    }
    followings_file.close();

    if (already_following) {
        cout << new_following->username << " is already a following." << endl;
        return;
    }

    // Open the file in append mode
    ofstream followings_file_append("followings/" + current_user->username + ".txt", ios::app);
    if (followings_file_append.is_open()) {
        // Check if the file is empty by reopening it in read mode
        ifstream check_file("followings/" + current_user->username + ".txt");
        bool is_empty = check_file.peek() == std::ifstream::traits_type::eof(); // Check if the file is empty
        check_file.close();

        // Write a newline only if the file is not empty
        if (!is_empty) {
            followings_file_append << endl; // Add a newline only if the file is not empty
        }
        followings_file_append << new_following->user_id << " " << new_following->username; // Write user_id and username
        followings_file_append.close();
        cout << "Successfully added " << new_following->username << " to " << current_user->username << "'s followings." << endl;
    } else {
        cout << "Unable to open file to save following." << endl;
    }
}


//Function to display followers 
void displayFollowers(User* search) {
    if (search == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

    
    string filename = "followers/" + search->username + ".txt";
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening file for followers: " << filename << endl;
        return;
    }

    string line;
    bool hasFollowers = false;

    
    cout << "\n===============================" << endl;
    cout << "    Followers of " << search->username << ":" << endl;
    cout << "===============================" << endl;

    while (getline(inFile, line)) {
        if (line.empty()) continue; 

        stringstream ss(line);
        int user_id;
        string username;

        if (ss >> user_id >> username) {
            hasFollowers = true;
            cout << "• ID: " << user_id << "\t Username: " << username << endl;
        }
    }

    inFile.close();

    if (!hasFollowers) {
        cout << "No Followers!" << endl;
    }

    cout << "===============================" << endl;
}

//Function to display followings
void displayFollowings(User* search) {
    if (search == nullptr) {
        cout << "Current user is not set." << endl;
        return;
    }

   
    string filename = "followings/" + search->username + ".txt";
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening file for followings: " << filename << endl;
        return;
    }

    string line;
    bool hasFollowings = false;

    
    cout << "\n===============================" << endl;
    cout << "    Followings of " << search->username << ":" << endl;
    cout << "===============================" << endl;

    while (getline(inFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        int user_id;
        string username;

        if (ss >> user_id >> username) {
            hasFollowings = true;
            cout << "• ID: " << user_id << "\t Username: " << username << endl;
        }
    }

    inFile.close();

    if (!hasFollowings) {
        cout << "No Followings!" << endl;
    }

    cout << "===============================" << endl;
}

// Function to check if a username exists
bool userExists(const string& username) {
    for (const auto& user : users) {
        if (user.username == username) {
            return true;
        }
    }
    return false;
}



// Function to register a new user
void registerUser() {
    string username, password, country;
    char gender;

    cout << "   Enter username: ";
    cin >> username;

    // Check if the username already exists
    if (userExists(username)) {
        cout << "Username already exists!" << endl;
        return;
    }

    cout << "   Enter password: ";
    cin >> password;

    // Input validation for gender
    cout << "   Enter gender (M/F/O): ";
    cin >> gender;
    while (gender != 'M' && gender != 'F' && gender != 'O') {
        cout << "Invalid input. Please enter 'M' for male, 'F' for female, or 'O' for other: ";
        cin >> gender;
    }

    cout << "   Enter country: ";
    cin.ignore(); 
    getline(cin, country);

    // Create a new user and add it to the users list
    users.emplace_back(next_user_id++, username, password, gender, country);

   
    saveUserData();

    cout << "User registered successfully!" << endl;
}


// Function to log in a user
bool loginUser() {
    string username, password;
    cout << "   Enter username: ";
    cin >> username;
    cout << "   Enter password: ";
    cin >> password;

    for (auto& user : users) {
        if (user.username == username && user.password == password) {
            onlineUsers.insert(username);
            current_user = &user;
            cout<<endl;
            cout << "Logged in successfully!" << endl;
            return true;
        }
    }
    cout << "Invalid username or password!" << endl;
    return false;
}

// Function to log out a user
void logoutUser() {
    if (current_user) {
        onlineUsers.erase(current_user->username);
        current_user = nullptr;
        cout << "User logged out successfully!" << endl;
    } else {
        cout << "No user is logged in!" << endl;
    }
}

// Function to delete a user
void deleteUser() {
    string username;
    cout << "Enter username to delete: ";
    cin >> username;

    auto it = remove_if(users.begin(), users.end(), [&username](const User& user) {
        return user.username == username;
    });

    if (it == users.end()) {
        cout << "User not found!" << endl;
        return;
    }

    users.erase(it, users.end());
    userPosts.erase(username);
    saveUserData();
    savePostData(current_user);
    cout << "User deleted successfully!" << endl;
}

//function to show login user details
void viewDetails()
{   
    loadPostData(current_user);
    if (current_user) {
        cout << endl<<"********** 🌐 Threads **********" <<endl;
        cout << "\nUser@Profile - \n" <<endl;
        cout << "Username 👤 : " << current_user->username << endl <<endl;
        cout << "Followers 👥 : " << current_user->followersCount<<" ";
        cout << "Followings 🫂 : " << current_user->followingsCount<<" ";
        
        cout << endl <<endl ;
        cout << "#️⃣ threads -"<<endl<<endl;
        for (const auto& post : current_user->posts) {
            cout << "     thread ☆ : " << post.thread_id << endl;
            cout << "                Content 📑 : " << post.content << endl;
            cout << "                Likes ❤️  : " << post.likes << " [";
        for (size_t i = 0; i < post.likedBy.size(); ++i) 
        {
            cout << "User: " << post.likedBy[i].second;
            if (i != post.likedBy.size() - 1) 
            {
                cout << ", ";
            }
        }
        cout << "]" << endl;

    
        cout << "                Comments 💬 : " << post.commentCount << " [";
        for (size_t i = 0; i < post.comments.size(); ++i) {
            cout << "User: " << post.comments[i].username << ":" << post.comments[i].comment_content; 
            if (i != post.comments.size() - 1) {
            cout << ", "; 
            }
        }
            cout << "]" << endl;
        }
    }
    else
    {
    cout << "No user is logged in!" << endl;
    }
}

//funtion to show search user details
void SearchUserDetails(User* search)
{   
   
    loadPostData(search);
    if (current_user) {
        cout << endl << "\nSearch@Profile - \n" <<endl;
        cout << "Username 👤 : " << search->username << endl <<endl;
        cout << "Followers 👥 : " << search->followersCount<<" ";
        cout << "Followings 🫂 : " << search->followingsCount<<" ";
        
        cout << endl <<endl ;
        cout << "#️⃣ threads -"<<endl<<endl;
    
        for (const auto& post : search->posts) {
            cout << "     thread ☆ : " << post.thread_id << endl;
            cout << "                Content 📑 : " << post.content << endl;
            cout << "                Likes ❤️  : " << post.likes << " [";
        for (size_t i = 0; i < post.likedBy.size(); ++i) 
        {
            cout << "User: " << post.likedBy[i].second;
            if (i != post.likedBy.size() - 1) 
            {
                cout << ", ";
            }
        }
        cout << "]" << endl;

        cout << "                Comments 💬 : " << post.commentCount << " [";
        for (size_t i = 0; i < post.comments.size(); ++i) {
            cout << "User: " << post.comments[i].username << ":" << post.comments[i].comment_content;
            if (i != post.comments.size() - 1) {
            cout << ", ";  
        }
           
        }
           cout << "]" << endl;
        }
    }
    else
    {
    cout << "No user is logged in!" << endl;
    }
 }

void displayAllUsers()
{
    cout << "\nThreads Users 👤 \n";
    
    cout << "===========================" << endl;
    int i=1;
    for (const auto& user : users) {
        cout << i << ". " << user.username << endl;
        i++;
    }
    cout<<endl<<"===========================";
}

void displayMainMenu() {
    cout << "\nOperations to Perform:\n";
    cout << "1. Social Network Analysis" << endl;
    cout << "2. Threads" << endl;
   
}

// Function to display the first menu
void displayFirstMenu() {

    cout << endl<<"********** 🌐 Welcom to Threads! **********" <<endl;
    cout << "\nOperations to Performs:\n"<<endl;
    cout << "   1. Register ➡️" << endl;
    cout << "   2. Login ✅" << endl;
    cout << "   3. Delete Account ❌" << endl;
    cout << "   4. Exit 🚩" << endl;
    cout << "   Enter your choice: ";
}

// Function to display the second menu
void displaySecondMenu() {
        cout << "\nOperations to Perform:\n";
        cout << "1. Refresh" << endl;
        cout << "2. Add Post" << endl;
        cout << "3. Delete Post" <<endl;
        cout << "4. See Followers" << endl;
        cout << "5. See Followings" << endl;
        cout << "6. Show Users" << endl;
        cout << "7. Search User" << endl;
        cout << "8. Logout"<< endl;
        cout << "9. Exit" << endl;
        cout << "Enter your choice: ";       
}

// Function to display the third menu
void displayThirdMenu() {
    cout << "\nOperations to Perform:\n";
    cout << "1. Follow" << endl;
    cout << "2. Unfollow" << endl;
    cout << "3. See Followers" << endl;
    cout << "4. See Followings" << endl;
    cout << "5. Like Post" << endl;
    cout << "6. Unlike Post" <<endl;
    cout << "7. Comment on Post" << endl;
    cout << "8. Back" << endl;
    cout << "9. Exit" << endl;
    cout << "Enter your choice: ";
}

// Function to like a post
void likePost() {
    
    int post_id;
    cout << "Enter Post ID to like: ";
    cin >> post_id;

    auto it = find_if(search_user->posts.begin(), search_user->posts.end(), [post_id](const Post& post) {
        return post.thread_id == post_id;
    });

    if (it == search_user->posts.end()) {
        cout << "Post not found!" << endl;
        return;
    }

  Post* post = &(*it);



    auto liked_it = std::find_if(post->likedBy.begin(), post->likedBy.end(),
        [&](const std::pair<int, std::string>& pair) {
            return pair.second == current_user->username;  
        });

    // If the current user has already liked the post, display a message
    if (liked_it != post->likedBy.end()) {
        cout << "You have already liked this post!" << endl;
        return;
    }
   
    post->likedBy.push_back(make_pair(current_user->user_id, current_user->username));
    post->likes++;  
    cout << "Liked post ID " << post_id << "!" << endl;

    savePostData(search_user);

    
}


//Function to update the post data in the file
void updatePostFile(User* search_user, const Post& post, int current_user_id) {
    string filename = "Post/" + search_user->username + ".txt";
    string tempFilename = "Post/temp.txt";
    
    ifstream infile(filename);
    ofstream tempfile(tempFilename);  

    if (!infile.is_open() || !tempfile.is_open()) {
        cout << "Error: Unable to open the file!" << endl;
        return;
    }

    string line;
    bool found = false;

    // Read original file line by line
    while (getline(infile, line)) {
        size_t firstPipe = line.find('|');  
        if (firstPipe == string::npos) {
            tempfile << line << endl;  
            continue;  
        }

        int line_post_id = stoi(line.substr(0, firstPipe)); 
        if (line_post_id == post.thread_id) {
            // We found the correct post to update
            size_t secondPipe = line.find('|', firstPipe + 1);
            size_t thirdPipe = line.find('|', secondPipe + 1);
            if (thirdPipe == string::npos) {
                cout << "Error: Invalid post format!" << endl;
                tempfile << line << endl;  
                continue;  

            // Extract likedBy section (after third '|')
            string likedBySection = line.substr(thirdPipe + 1);
            stringstream searchPattern;
            searchPattern << "(" << current_user_id << ",";  

            size_t userLikePos = likedBySection.find(searchPattern.str());  
            if (userLikePos != string::npos) {
                // Found the user in the likedBy section
                size_t entryEndPos = likedBySection.find(')', userLikePos);
                if (entryEndPos != string::npos) {
                    // Remove the entry "(user_id,username)"
                    likedBySection.erase(userLikePos, (entryEndPos - userLikePos) + 1);

                    // Handle leftover semicolons
                    if (userLikePos < likedBySection.size() && likedBySection[userLikePos] == ';') {
                        likedBySection.erase(userLikePos, 1); 
                    } else if (userLikePos > 0 && likedBySection[userLikePos - 1] == ';') {
                        likedBySection.erase(userLikePos - 1, 1); 
                    }

                 
                    string updatedPostLine = line.substr(0, thirdPipe + 1) + likedBySection;
                    tempfile << updatedPostLine << endl; 
                    found = true;  
                }
            } else {
                
                tempfile << line << endl;
            }
        } else {
            
            tempfile << line << endl;
        }
    }

    infile.close();  
    tempfile.close(); 

    if (found) {
        
        remove(filename.c_str());
        rename(tempFilename.c_str(), filename.c_str());
    } else {
        cout << "Error: Post not found or user did not like the post!" << endl;
        remove(tempFilename.c_str());  // Cleanup the temp file if no post was updated
    }
}

}

void unlikePost(User* search_user) {
    int post_id;
    cout << "Enter Post ID to unlike: ";
    cin >> post_id;

    auto it = find_if(search_user->posts.begin(), search_user->posts.end(), [post_id](const Post& post) {
        return post.thread_id == post_id;
    });

    if (it == search_user->posts.end()) {
        cout << "Post not found!" << endl;
        return;
    }

    Post* post = &(*it);

    
    if (current_user->username == search_user->username) {
        cout << "You cannot unlike your own post!" << endl;
        return;
    }

   
    auto liked_it = std::find_if(post->likedBy.begin(), post->likedBy.end(),
        [&](const std::pair<int, std::string>& pair) {
            return pair.second == current_user->username;  
        });

    if (liked_it != post->likedBy.end()) {
        // Remove user from likedBy list and decrement the like count
        post->likedBy.erase(liked_it);
        post->likes--;
        cout << "Unliked post ID " << post_id << "!" << endl;
        
        // Update the post data in the file
        updatePostFile(search_user, *post, current_user->user_id);
    } else {
        cout << "You haven't liked this post yet!" << endl;
        return;
    }

}



// Function to comment on a post
void commentOnPost() {
    int post_id;
    string comment;
    cout << "Enter Post ID to comment on: ";
    cin >> post_id;
    cin.ignore();
    cout << "  Enter your comment: ";
    getline(cin, comment);

    auto it = find_if(search_user->posts.begin(), search_user->posts.end(), [post_id](const Post& post) {
        return post.thread_id == post_id;
    });

    if (it == search_user->posts.end()) {
        cout << "Post not found!" << endl;
        return;
    }

    Post* post = &(*it);
    post->commentCount++;
    post->comments.push_back({current_user->user_id,current_user->username,comment});
    cout << "Comment added to post ID " << post_id << "!" << endl;
    savePostData(search_user);
}

// Function to add a new post
void addPost() {
    if (!current_user) {
        cout << "No user is logged in!" << endl;
        return;
    }

    string content;
    cout << "Enter post content: ";
    cin.ignore();
    getline(cin, content);

    Post new_post;
    new_post.thread_id = current_user->posts.empty() ? 1 : current_user->posts.back().thread_id + 1;
    new_post.content = content;
    new_post.likes = 0;
    new_post.commentCount = 0;

    current_user->posts.push_back(new_post);
    userPosts[current_user->username].push_back(new_post);
    savePostData(current_user);
    loadPostData(current_user);

    cout << "Post added successfully!" << endl;
}

//Funtion to delete post
void deletePost() {
    if (!current_user) {
        cout << "No user is logged in!" << endl;
        return;
    }

    if (current_user->posts.empty()) {
        cout << "You have no posts to delete!" << endl;
        return;
    }

    int postid;
    cout << "Enter the Post ID to delete: ";
    cin >> postid;

    if (postid < 1 || postid > current_user->posts.size()) {
        cout << "Invalid post number!" << endl;
        return;
    }

    postid--;

    Post deletedPost = current_user->posts[postid];

    current_user->posts.erase(current_user->posts.begin() + postid);

    auto& userPostList = userPosts[current_user->username];
    for (auto it = userPostList.begin(); it != userPostList.end(); ++it) {
        if (it->thread_id == deletedPost.thread_id) {
            userPostList.erase(it);
            break;
        }
    }

    for (size_t i = postid; i < current_user->posts.size(); ++i) {
        current_user->posts[i].thread_id--;
    }

    for (size_t i = 0; i < userPostList.size(); ++i) {
        if (userPostList[i].thread_id > deletedPost.thread_id) {
            userPostList[i].thread_id--;
        }
    }

    savePostData(current_user);
    loadPostData(current_user);

    cout << "Post deleted and IDs adjusted successfully!" << endl;
}

// Function to view a specific user and their options
void viewUser() {
    string username;
    cout << "Enter the username to view: ";
    cin >> username;

    auto it = find_if(users.begin(), users.end(), [&username](const User& user) {
        return user.username == username;
    });

    if (it == users.end()) {
        cout << "User not found!" << endl;
        return;
    }
    
   
    search_user = &(*it); 
     
    cout<<endl;
    
    int choice;
    while (true) {
        loadFollowers(current_user);
        loadFollowers(search_user);
        loadFollowings(current_user);
        loadFollowings(search_user);
        viewDetails();
        SearchUserDetails(search_user);
        displayThirdMenu();
        cin >> choice;
        switch (choice) {
            case 1:
                addFollower(current_user);
                addFollowing(search_user);
                break;
            case 2:
                removeFollower();
                removeFollowing();
                break;
            case 3:
                displayFollowers(search_user);
                break;
            case 4:
                displayFollowings(search_user);
                break;
            case 5:
                likePost();
                break;
            case 6:
                unlikePost(search_user);
                break;
            case 7:
                commentOnPost();
                break;
            case 8:
                return; // Back to Second Menu
            case 9:
                exit(0); // Exit
            default:
                cout << "Invalid choice, please try again." << endl;
                break;
        }
    }
}

//Function to read USERs data
void readUserData(const string& filepath, unordered_map<string, string>& users_map) {
    ifstream infile(filepath);
    if (!infile) {

        return;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string user_id_str, username;
        ss >> user_id_str >> username;

        if (!user_id_str.empty() && !username.empty()) {
            users_map[user_id_str] = username;  
        }
    }
}

// Function to interpret Degree Centrality
string interpretDegreeCentrality(double value) {
    if (value > 0.6) {
        return "Node is highly connected and central in the network.";
    } else if (value > 0.3) {
        return "Node has a moderate level of connectivity.";
    } else {
        return "Node has few connections and is likely peripheral.";
    }
}

// Function to interpret Closeness Centrality
string interpretClosenessCentrality(double value) {
    if (value > 0.6) {
        return "Node can quickly reach other nodes, indicating high closeness centrality.";
    } else if (value > 0.3) {
        return "Node has a moderate ability to reach others.";
    } else {
        return "Node has limited reachability within the network.";
    }
}

// Function to interpret Betweenness Centrality
string interpretBetweennessCentrality(double value) {
    if (value > 0.5) {
        return "Node serves as a key bridge between clusters in the network.";
    } else if (value > 0.1) {
        return "Node has moderate bridging importance.";
    } else {
        return "Node has low influence in connecting different parts of the network.";
    }
}

// Function to interpret Eigenvector Centrality
string interpretEigenvectorCentrality(double value) {
    if (value > 0.5) {
        return "Node is well-connected to other highly influential nodes.";
    } else if (value > 0.2) {
        return "Node is connected to other moderately influential nodes.";
    } else {
        return "Node is not well-connected to influential nodes.";
    }
}

// Function to interpret Local Clustering Coefficient
string interpretLocalClusteringCoefficient(double value) {
    if (value > 0.7) {
        return "Node's neighbors are highly interconnected.";
    } else if (value > 0.3) {
        return "Node's neighbors have a moderate level of interconnection.";
    } else {
        return "Node's neighbors are not well interconnected.";
    }
}

// Function to interpret Global Clustering Coefficient
string interpretGlobalClusteringCoefficient(double value) {
    if (value > 0.5) {
        return "Network is highly clustered, with many tightly knit communities.";
    } else if (value > 0.2) {
        return "Network has a moderate level of clustering.";
    } else {
        return "Network has low clustering.";
    }
}

// Function to interpret Graph Density
string interpretGraphDensity(double value) {
    if (value > 0.5) {
        return "Network is dense, with many connections between nodes.";
    } else if (value > 0.1) {
        return "Network is moderately dense.";
    } else {
        return "Network is sparse, with few connections between nodes.";
    }
}


void calculateDegreeCentrality() {
    if (current_user == nullptr) {
        cerr << "Current user is not set." << endl;
        return;
    }

    auto currentUserIt = adjList.find(current_user->username);
    int current_user_degree = (currentUserIt != adjList.end()) ? adjList.at(current_user->username).size() : 0;

    int max_degree = 0;

    for (const auto& user_pair : adjList) {
        int user_degree = user_pair.second.size();
        max_degree = max(max_degree, user_degree);
    }

    
    double result = (max_degree > 0) ? (static_cast<double>(current_user_degree) / max_degree) : 0.0;

   
    cout << endl << "Current user degree: " << current_user_degree << endl;
    cout << "Max degree among all users: " << max_degree << endl;
    cout << "Degree centrality : " << result;
    cout<<" -> "<<interpretDegreeCentrality(result);
}


vector<string> colors = {"lightblue", "pink", "lightgray", "yellow", "green", "purple", "orange", "red", "cyan", "magenta"};


unordered_map<string, string> colorMap;


string getColor(const string& attributeValue) {
    if (colorMap.find(attributeValue) == colorMap.end()) {
        
        if (colorMap.size() >= colors.size()) {
            
            char color[8];
            snprintf(color, sizeof(color), "#%06X", rand() % 0xFFFFFF);
            colorMap[attributeValue] = color;
        } else {
            
            colorMap[attributeValue] = colors[colorMap.size()];
        }
    }
    return colorMap[attributeValue];
}

// Helper function to add an edge between users in the adjacency list
void addToAdjList(unordered_map<string, vector<string>>& adjList, const string& user1, const string& user2) {
    adjList[user1].push_back(user2);
    adjList[user2].push_back(user1);
}

// Function to generate adjacency list based on the chosen attribute (gender or country)
void generateAdjacencyList( unordered_map<string, vector<string>>& adjList, const string& attribute) {
    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            if (attribute == "gender" && users[i].gender == users[j].gender) {
                addToAdjList(adjList, users[i].username, users[j].username);
            }
            else if (attribute == "country" && users[i].country == users[j].country) {
                addToAdjList(adjList, users[i].username, users[j].username);
            }
        }
    }
}

// Function to generate a DOT file for Graphviz from the adjacency list
void generateDotFile(const unordered_map<string, vector<string>>& adjList, const string& attribute, const string& filename) {
    ofstream dotFile(filename);
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    dotFile << "graph G {\n";
    dotFile << "    node [style=filled, shape=circle];\n";

    unordered_set<string> legendValues; 

  
    for (const auto& user : users) {
        string attributeValue = (attribute == "gender") ? string(1, user.gender) : user.country;
        string color = getColor(attributeValue);
        dotFile << "    \"" << user.username << "\" [fillcolor=\"" << color << "\"];\n";
        legendValues.insert(attributeValue); 
    }

    set<string> drawnEdges;

    for (const auto& entry : adjList) {
        const string& user = entry.first;
        for (const auto& neighbor : entry.second) {
            string edge = (user < neighbor) ? (user + "-" + neighbor) : (neighbor + "-" + user);
            if (drawnEdges.find(edge) == drawnEdges.end()) {
                dotFile << "    \"" << user << "\" -- \"" << neighbor << "\" [color=blue];\n";
                drawnEdges.insert(edge);
            }
        }
    }

    // Create a subgraph to hold the legend in the top-right corner
    dotFile << "    subgraph cluster_legend {\n";
    dotFile << "        label=\"Legend\";\n";
    dotFile << "        labelloc=\"t\";\n";
    dotFile << "        fontsize=12;\n";
    dotFile << "        style=dashed;\n";
    dotFile << "        color=gray;\n";
    dotFile << "        node [shape=plaintext, fontsize=10];\n";

    // Add only relevant attribute values to the legend
    for (const auto& value : legendValues) {
        if (colorMap.find(value) != colorMap.end()) { // Ensure the value has a color mapping
            dotFile << "        \"" << value << "_legend\" [label=\"" << value << "\", fillcolor=\"" << colorMap[value] << "\", style=filled, width=0.15, height=0.15];\n";
        }
    }

    dotFile << "    }\n";

    dotFile << "}\n";
    dotFile.close();
}

// Function to render the DOT file as an image

void renderDotFile(const string& dotFilename, const string& outputFilename) {
    
    string command = "dot -Tpng " + dotFilename + " -o " + outputFilename;
    system(command.c_str());

   
    string openCommand = "code " + outputFilename;
    system(openCommand.c_str());
    cout << "Graph image generated and opened in VS Code: " << outputFilename << endl;
}


// Function to delete files when "close" is entered
void deleteFiles(const string& dotFilename, const string& outputFilename) {
    if (remove(dotFilename.c_str()) == 0) {
        cout << dotFilename << " deleted successfully.\n";
    } else {
        cerr << "Error deleting " << dotFilename << ".\n";
    }

    if (remove(outputFilename.c_str()) == 0) {
        cout << outputFilename << " deleted successfully.\n";
    } else {
        cerr << "Error deleting " << outputFilename << ".\n";
    }
}


// //Funtion to create Adjacency list
unordered_map<string, vector<string>> createAdjacencyList() {
    unordered_map<string, unordered_set<string>> adjList;

    for (const User& user : users) {
        
        adjList[user.username] = unordered_set<string>(); 

        string followersFilename = "followers/" + user.username + ".txt";
        ifstream followersFile(followersFilename);

        if (!followersFile) {
            // cerr << "Error opening followers file: " << followersFilename << endl;
        } else {
            string line;
            while (getline(followersFile, line)) {
                stringstream ss(line);
                string follower_user_id_str, follower_username;
                ss >> follower_user_id_str >> follower_username;

                adjList[user.username].insert(follower_username);
            }
            followersFile.close();
        }

        string followingsFilename = "followings/" + user.username + ".txt";
        ifstream followingsFile(followingsFilename);

        if (!followingsFile) {
            // cerr << "Error opening followings file: " << followingsFilename << endl;
        } else {
            string line;
            while (getline(followingsFile, line)) {
                stringstream ss(line);
                string following_user_id_str, following_username;
                ss >> following_user_id_str >> following_username;
                adjList[user.username].insert(following_username);
            }
            followingsFile.close();
        }
    }

   
    unordered_map<string, vector<string>> result;
    for (const auto& [user, followingSet] : adjList) {
        result[user] = vector<string>(followingSet.begin(), followingSet.end());
    }

    return result;
}


void generateDotFile(const unordered_map<string, vector<string>>& adj, const string& filename) {
    ofstream dotFile(filename);
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    dotFile << "graph G {\n"; 
    vector<string> colors = {
        "lightblue", "lightgreen", "lightyellow", 
        "lightpink", "lightgray", "lightcyan" 
    };

    int colorIndex = 0; 
    unordered_set<string> drawnEdges; 

    for (const auto& pair : adj) {
        const string& node = pair.first;

     
        dotFile << "    \"" << node << "\" [fillcolor=\"" << colors[colorIndex % colors.size()] 
                << "\", style=\"filled\"];\n";
        colorIndex++;

        for (const string& neighbor : pair.second) {
            
            string edge = (node < neighbor) ? (node + "-" + neighbor) : (neighbor + "-" + node);
            if (drawnEdges.find(edge) == drawnEdges.end()) {
                
                dotFile << "    \"" << node << "\" -- \"" << neighbor << "\" [color=\"blue\"];\n"; 
                drawnEdges.insert(edge); 
            }
        }
    }

    dotFile << "}\n"; 
    dotFile.close(); 
}


// Function to generate the DOT file for a specific subgraph
void generateSubgraph(const unordered_map<string, vector<string>>& adj, const string& node, const string& filename) {
    ofstream dotFile(filename);
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    dotFile << "graph G {\n"; 
    unordered_map<string, bool> visited; 
    vector<string> neighbors; 

    if (adj.find(node) != adj.end()) {
        neighbors = adj.at(node); 
        visited[node] = true; 
        dotFile << "    " << node << " [fillcolor=\"white\", style=\"filled\"];\n";
    } else {
        cerr << "Node not found in the graph." << endl;
        return;
    }

    vector<string> colors = {
        "lightblue", "lightgreen", "lightyellow", 
        "lightpink", "lightgray", "lightcyan"
    };

    int colorIndex = 0; 
    unordered_set<string> drawnEdges; 

    for (const auto& neighbor : neighbors) {
        dotFile << "    " << neighbor << " [fillcolor=\"" << colors[colorIndex % colors.size()] 
                << "\", style=\"filled\"];\n"; 
    
        string edge = (node < neighbor) ? (node + "-" + neighbor) : (neighbor + "-" + node);
        if (drawnEdges.find(edge) == drawnEdges.end()) {
            dotFile << "    " << node << " -- " << neighbor << " [color=\"black\"];\n"; 
            drawnEdges.insert(edge); 
        }
        visited[neighbor] = true; 
        colorIndex++; 
    }

    for (const auto& neighbor : neighbors) {
        if (adj.find(neighbor) != adj.end()) {
            for (const auto& neighborOfNeighbor : adj.at(neighbor)) {
                if (find(neighbors.begin(), neighbors.end(), neighborOfNeighbor) != neighbors.end()) {
                    string edge = (neighbor < neighborOfNeighbor) ? (neighbor + "-" + neighborOfNeighbor) : (neighborOfNeighbor + "-" + neighbor);
                    if (drawnEdges.find(edge) == drawnEdges.end()) {
                        dotFile << "    " << neighbor << " -- " << neighborOfNeighbor 
                                << " [color=\"black\"];\n"; 
                        drawnEdges.insert(edge); 
                    }
                }
            }
        }
    }

    dotFile << "}\n"; 
    dotFile.close(); 
}


void generateAndOpenGraph(const string& dotFilename) {
    string pngFilename = current_user->username+".png"; 
    string command = "dot -Tpng " + dotFilename + " -o " + pngFilename; 
    system(command.c_str()); 

    string openCommand = "code " + pngFilename; 
    system(openCommand.c_str()); 
}

// Function to wait for user input before deleting files
void waitForInputAndCleanUp(const string& dotFilename, const string& pngFilename) {
    string userInput;
    cout << "Type 'close' to exit:\n";
    
    while (true) {
        cout << ":"; 
        getline(cin, userInput);
        if (userInput == "close") {
            break; 
        }
    }

    // Delete the files
    remove(dotFilename.c_str());
    remove(pngFilename.c_str());
    cout << "Files deleted: " << dotFilename << " and " << pngFilename << endl;
}

// Function to calculate Closeness Centrality
void calculateClosenessCentrality() {
    unordered_map<string, int> distances;
    unordered_set<string> visited;
    queue<pair<string, int>> q;

    
    q.push({current_user->username, 0});
    visited.insert(current_user->username);

    while (!q.empty()) {
        auto [current, dist] = q.front();
        q.pop();
        distances[current] = dist;

        if (adjList.find(current) != adjList.end()) {
            for (const string& neighbor : adjList.at(current)) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push({neighbor, dist + 1});
                }
            }
        }
    }

    double sumDistances = 0;
    for (const auto& pair : distances) {
        sumDistances += pair.second;
    }

    int totalNodes = adjList.size();

    if (sumDistances == 0 || distances.size() <= 1) {
        cout << " 0.0 (No reachable nodes or only one node)";
        return;
    }

    double closenessCentrality = (distances.size() - 1) / sumDistances;
    cout << closenessCentrality ;
    cout<<" -> "<<interpretClosenessCentrality(closenessCentrality);
}



// Function to calculate Betweenness Centrality
void calculateBetweennessCentrality() {
    unordered_map<string, double> centrality;

    for (const auto& pair : adjList) {
        centrality[pair.first] = 0.0;
    }

    for (const auto& source : adjList) {
        string s = source.first;

        unordered_map<string, int> dist;
        unordered_map<string, int> numShortestPaths;
        unordered_map<string, vector<string>> predecessors;
        queue<string> q;
        stack<string> stack;

        dist[s] = 0;
        numShortestPaths[s] = 1;
        q.push(s);

        while (!q.empty()) {
            string v = q.front();
            q.pop();
            stack.push(v);

            if (adjList.find(v) != adjList.end()) {
                for (const string& neighbor : adjList[v]) {
                    if (dist.find(neighbor) == dist.end()) {
                        dist[neighbor] = dist[v] + 1;
                        q.push(neighbor);
                    }

                    if (dist.find(neighbor) != dist.end() && dist[neighbor] == dist[v] + 1) {
                        numShortestPaths[neighbor] += numShortestPaths[v];
                        predecessors[neighbor].push_back(v);
                    }
                }
            }
        }

        unordered_map<string, double> dependency;
        for (const auto& pair : numShortestPaths) {
            dependency[pair.first] = 0.0;
        }

        while (!stack.empty()) {
            string w = stack.top();
            stack.pop();

            for (const string& pred : predecessors[w]) {
                double fraction = static_cast<double>(numShortestPaths[pred]) / numShortestPaths[w];
                dependency[pred] += fraction * (1.0 + dependency[w]);
            }

            if (w != s) {
                centrality[w] += dependency[w];
            }
        }
    }

    cout << current_user->username << ": " << centrality[current_user->username];
    cout<<" -> "<<interpretBetweennessCentrality(centrality[current_user->username]);
}



//Function to calculate Eigenvector Centrality
void calculateEigenvectorCentrality() {
    unordered_map<string, double> centrality;
    vector<string> nodes;
    
    for (const auto& pair : adjList) {
        nodes.push_back(pair.first);
        centrality[pair.first] = 1.0;
    }
    
    const double tolerance = 1e-6;
    const int maxIterations = 100;
    int iteration = 0;
    
    while (iteration < maxIterations) {
        unordered_map<string, double> newCentrality;
        
        for (const string& node : nodes) {
            double sum = 0.0;
            for (const string& neighbor : adjList.at(node)) {
                sum += centrality[neighbor];
            }
            newCentrality[node] = sum;
        }
        
        double maxChange = 0.0;
        for (const string& node : nodes) {
            maxChange = max(maxChange, abs(newCentrality[node] - centrality[node]));
            centrality[node] = newCentrality[node];
        }
        
        if (maxChange < tolerance) break;
        iteration++;
    }
    
    double norm = 0.0;
    for (const auto& pair : centrality) {
        norm += pair.second * pair.second;
    }
    norm = sqrt(norm);
    
    cout<<centrality[current_user->username] / norm;
    cout<<" -> "<<interpretEigenvectorCentrality(centrality[current_user->username] / norm);
}


// Function to calculate Graph Density
void calculateGraphDensity() {
    int numEdges = 0;
    int numNodes = adjList.size(); 

    if (numNodes == 0) {
        cout << "Graph Density: 0.0" << endl;
        return; 
    }

  
    for (const auto& pair : adjList) {
        numEdges += pair.second.size(); 
    }

    if (numNodes <= 1) {
        cout << "Graph Density: 0.0" << endl;
        return;
    }


    double density = static_cast<double>(numEdges) / (numNodes * (numNodes - 1));
    cout << "Graph Density: " << density ;
    cout<<" -> "<<interpretGraphDensity(density);
}


// Function to Calculate Local Clustering Coefficient
void calculateClusteringCoefficient() {
    auto it = adjList.find(current_user->username);
    if (it == adjList.end()) {
        cout << "0.0" << endl;  
        return;
    }

    const vector<string>& neighbors = it->second;
    int neighborCount = neighbors.size();

    if (neighborCount < 2) {
        cout << "0.0" << endl;
        return;
    }

    int count = 0;

    for (size_t i = 0; i < neighborCount; ++i) {
        for (size_t j = i + 1; j < neighborCount; ++j) {
            // Ensure both neighbors exist in the adjacency list
            if (adjList.find(neighbors[i]) != adjList.end() && 
                adjList.find(neighbors[j]) != adjList.end() &&
                find(adjList.at(neighbors[i]).begin(), adjList.at(neighbors[i]).end(), neighbors[j]) != adjList.at(neighbors[i]).end()) {
                ++count;
            }
        }
    }

    int totalPossibleLinks = (neighborCount * (neighborCount - 1)) / 2;

    double clusteringCoefficient = (totalPossibleLinks > 0) ? static_cast<double>(count) / totalPossibleLinks : 0.0;
    cout << clusteringCoefficient;
    cout<<" -> "<<interpretLocalClusteringCoefficient(clusteringCoefficient);
}


//Function to count triangles in the network
int countTriangles() {
    int triangleCount = 0;
    const auto& neighbors = adjList.at(current_user->username);
    size_t n = neighbors.size();

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            const string& neighbor1 = neighbors[i];
            const string& neighbor2 = neighbors[j];
            if (find(adjList.at(neighbor1).begin(), adjList.at(neighbor1).end(), neighbor2) != adjList.at(neighbor1).end()) {
                ++triangleCount;
            }
        }
    }

    return triangleCount;
}
int countTrianglesForNode(const string& node) {
    int triangleCount = 0;
    const auto& neighbors = adjList.at(node);

    for (size_t i = 0; i < neighbors.size(); ++i) {
        for (size_t j = i + 1; j < neighbors.size(); ++j) {
            // Check if both neighbors are connected
            if (adjList.find(neighbors[i]) != adjList.end() &&
                find(adjList.at(neighbors[i]).begin(), adjList.at(neighbors[i]).end(), neighbors[j]) != adjList.at(neighbors[i]).end()) {
                ++triangleCount;
            }
        }
    }
    return triangleCount;
}


// Function to Calculate Global Clustering Coefficient
void calculateGlobalClusteringCoefficient() {
    double totalTriangles = 0;
    double totalTriplets = 0;


    for (const auto& pair : adjList) {
        const string& node = pair.first;
        const auto& neighbors = pair.second; 
        size_t numNeighbors = neighbors.size();


        if (numNeighbors > 1) {
            totalTriplets += numNeighbors * (numNeighbors - 1) / 2; 
        }

        totalTriangles += countTrianglesForNode(node); 
    }

    if (totalTriplets == 0) {
        cout << "0.0" << endl; 
        return;
    }
    cout << (3 * totalTriangles) / totalTriplets;
    cout<<" -> "<<interpretGlobalClusteringCoefficient((3 * totalTriangles) / totalTriplets);
}


//Function to display Adjacency list and all calculation
void displayAdjacencyList() {
    
    cout<<endl<<"==========================================="<<endl;
    cout<<"Social Measures Stats for "<<current_user->username<<":";
    cout<<endl<<"==========================================="<<endl;
    cout<<endl<<"Adjacency List - "<<endl;
    for (const auto& entry : adjList) {
        cout << "  -"<<entry.first << " -> ";
        for (const string& follower : entry.second) {
            cout << follower << " ";
        }
        cout << endl;
    }
    
    calculateDegreeCentrality();
    cout<<"\n";
    cout<<"Closeness Centrality : "<<":";calculateClosenessCentrality();
    cout<<"\n";
    cout<<"Betweeness Centrality : "<<":";calculateBetweennessCentrality();
    cout<<"\n";
    cout<<"Eigenvector Centrality : "<<":";calculateEigenvectorCentrality();
    cout<<"\n";
    cout<<"Local Clustering Cofficient : "<<":";calculateClusteringCoefficient();
    cout<<"\n";
    cout<<"Global Clustering COfficient : "<<":";calculateGlobalClusteringCoefficient();
    cout<<"\n";
    calculateGraphDensity();
    cout<<endl<<endl<<"=========================================="<<endl;
    string subgraphNode = current_user->username + ""; 
    string subgraphFilename = current_user->username+".graph";
    generateSubgraph(adjList, subgraphNode, subgraphFilename);
    generateAndOpenGraph(subgraphFilename);
    waitForInputAndCleanUp(subgraphFilename, current_user->username+".png");
    
}

void generateCommunityGraph(const vector<vector<User*>>& communities) {
    std::ofstream outFile("all_communities_graph.dot");
    if (!outFile) {
        std::cerr << "Error opening file for community graph." << std::endl;
        return;
    }

    outFile << "graph all_communities {" << std::endl;

    // Set of unique colors
    std::string colors[] = {"lightblue", "lightgreen", "lightcoral", "lightsalmon", "lightyellow", "lightpink", "lightgray"};
    std::srand(std::time(0)); // Initialize random seed for colors

    // Loop over each community and generate subgraph
    for (size_t i = 0; i < communities.size(); ++i) {
        std::string communityColor = colors[std::rand() % 7];
        outFile << "    subgraph cluster_" << i << " {" << std::endl;
        outFile << "        label = \"Community " << i + 1 << "\";" << std::endl;
        outFile << "        style=filled; fillcolor=" << communityColor << ";" << std::endl;

        // Nodes and edges within the community
        std::unordered_set<std::string> userSet; // Track users in this community for faster lookup
        for (auto& user : communities[i]) {
            userSet.insert(user->username);
            outFile << "        \"" << user->username << "\" [label=\"" << user->username 
                    << "\" style=filled fillcolor=" << communityColor << "];" << std::endl;
        }

        // Add edges within the community
        for (auto& user : communities[i]) {
            for (auto& following : user->followings) {
                const std::string& followedUsername = following.second;

                if (userSet.count(followedUsername)) {
                    // Check if edge has already been added for mutual connections
                    if (user->username < followedUsername) {
                        outFile << "        \"" << user->username << "\" -- \"" << followedUsername << "\";" << std::endl;
                    }
                }
            }
        }

        outFile << "    }" << std::endl; // Close the subgraph for the community
    }

    outFile << "}" << std::endl;
    outFile.close();

    // Generate PNG using dot command
    system("dot -Tpng all_communities_graph.dot -o all_communities_graph.png");
    std::cout << "Graph for all communities generated successfully." << std::endl;

    // Open the generated PNG
    system("code all_communities_graph.png");
}

// Function to delete DOT and PNG files after use
void deleteCommunityGraphFiles() {
    std::string dotFile = "all_communities_graph.dot";
    std::string pngFile = "all_communities_graph.png";

    if (fs::exists(dotFile)) {
        fs::remove(dotFile);
        std::cout << "Deleted DOT file: " << dotFile << std::endl;
    }
    if (fs::exists(pngFile)) {
        fs::remove(pngFile);
        std::cout << "Deleted PNG file: " << pngFile << std::endl;
    }
}



// Function to detect communities based on mutual following using adjList
vector<vector<User*>> detectCommunities(const unordered_map<string, vector<string>>& adjList) {
    vector<vector<User*>> communities;
    unordered_map<string, bool> visited; 


    function<void(User*)> dfs = [&](User* user) {
        visited[user->username] = true;
        vector<User*> community;
        community.push_back(user);

    
        auto it = adjList.find(user->username);
        if (it != adjList.end()) {
            for (const string& followed_username : it->second) {
              
                auto followed_it = adjList.find(followed_username);
                if (followed_it != adjList.end() && 
                    find(followed_it->second.begin(), followed_it->second.end(), user->username) != followed_it->second.end()) {
               
                    User* followed_user = nullptr;
                    for (auto& u : users) {
                        if (u.username == followed_username) {
                            followed_user = &u;
                            break;
                        }
                    }
                    if (followed_user && !visited[followed_user->username]) {
                        community.push_back(followed_user);
                        visited[followed_user->username] = true;
                        dfs(followed_user); 
                    }
                }
            }
        }

        communities.push_back(community);
    };


    for (auto& user : users) {
        if (!visited[user.username]) {
            dfs(&user); 
        }
    }

    return communities;
}



double computeCommunityDensity(const vector<User*>& community) {
    int numEdges = 0;
    int numNodes = community.size();

    for (auto& user : community) {
        for (auto& following : user->followings) {
            if (find(community.begin(), community.end(), &users[following.first]) != community.end()) {
                numEdges++;
            }
        }
    }

  
    numEdges /= 2;


    int maxEdges = numNodes * (numNodes - 1) / 2;

    return (maxEdges == 0) ? 0.0 : (double)numEdges / maxEdges;
}
// Function to compute modularity of a community
double computeModularity(const vector<User*>& community) {
    int m = 0; // Total number of edges in the community
    unordered_set<int> communitySet;
    
    for (auto& user : community) {
        communitySet.insert(user->user_id);
        m += user->followingsCount;  // Adding user's outgoing edges
    }

    m /= 2;  // Because each edge is counted twice
    
    double modularity = 0.0;
    for (auto& user : community) {
        for (auto& following : user->followings) {
            int ki = user->followingsCount; 
            int kj = users[following.first].followingsCount;  
            
            bool sameCommunity = communitySet.find(following.first) != communitySet.end();
            modularity += (sameCommunity ? 1 : 0) - (ki * kj) / (2.0 * m);
        }
    }
    
    return modularity / (2.0 * m);
}

// Function to compute permanence of a community
double computePermanence(const vector<User*>& community) {
    double initialModularity = computeModularity(community);
    double permanence = 0.0;
    
    // Threshold for modularity change
    const double epsilon = 0.1;

    // Remove each user from the community one by one
    for (size_t i = 0; i < community.size(); ++i) {
        vector<User*> tempCommunity = community;
        tempCommunity.erase(tempCommunity.begin() + i);  // Remove user at index i

        double newModularity = computeModularity(tempCommunity);

       
        permanence += (abs(initialModularity - newModularity) < epsilon) ? 1 : -1;
    }
    
    return permanence;
}


void computeCommunityMetrics(const vector<vector<User*>>& communities) {
    for (size_t i = 0; i < communities.size(); ++i) {
        const vector<User*>& community = communities[i];
        cout << "\nCommunity " << i + 1 << " Metrics:" << endl;


        cout << "Users in Community " << i + 1 << ": ";
        for (auto& user : community) {
            cout << user->username << " ";
        }
        cout << endl;

        double modularity = computeModularity(community);
        double permanence = computePermanence(community);
        double density = computeCommunityDensity(community);
        
        cout << "Modularity: " << modularity << endl;
        cout << "Permanence: " << permanence << endl;
        cout << "Density: " << density << endl;
    }
}

void displayAdmin()
{  
   cout << endl;
   cout << "1.Link Prediction" <<endl;
   cout << "2.Social Measure" <<endl; 
   cout << "3.Community Detection" <<endl;
   cout << "4.Network Graph" <<endl;
}
User* findUserByUsername(const string& username) {
    for (auto& user : users) {
        if (user.username == username) {
            return &user;  
        }
    }
    return nullptr; 
} 

// Function to calculate the density of the graph
double calculateDensity(const unordered_map<string, vector<string>>& adjList) {
    int V = adjList.size();  
    int E = 0;               

    set<pair<string, string>> uniqueEdges;  
    for (const auto& entry : adjList) {
        const string& node = entry.first;
        for (const string& neighbor : entry.second) {
           
            if (uniqueEdges.find({neighbor, node}) == uniqueEdges.end()) {
                uniqueEdges.insert({node, neighbor});
            }
        }
    }
    E = uniqueEdges.size(); 

    if (V < 2) return 0.0;  
    return static_cast<double>(2 * E) / (V * (V - 1));
}

// Function to generate a unique color for each node
string generateUniqueColor(int index) {
    string colors[] = {
        "lightblue", "lightgreen", "lightcoral", "lightsalmon", "lightyellow", 
        "lightpink", "lightgray", "lavender", "peachpuff", "plum", "lightcyan", 
        "mistyrose", "beige", "lightsteelblue", "khaki"
    };
    return colors[index % 15];
}


unordered_map<string, vector<string>> convertToUndirected(const unordered_map<string, vector<string>>& directedAdjList) {
    unordered_map<string, vector<string>> undirectedAdjList;


    for (const auto& entry : directedAdjList) {
        const string& node = entry.first;
        undirectedAdjList[node]; 
        for (const string& neighbor : entry.second) {
            undirectedAdjList[node].push_back(neighbor);
            undirectedAdjList[neighbor].push_back(node);
        }
    }
    
    return undirectedAdjList;
}


// Function to generate the DOT file
void generateGraph(const unordered_map<string, vector<string>>& adjList) {
    ofstream outFile("graph.dot");
    if (!outFile) {
        cerr << "Error opening file to write graph." << endl;
        return;
    }

    outFile << "graph G {" << endl;
    outFile << "    node [shape=circle, style=filled, fontname=\"Helvetica\"];" << endl;
    outFile << "    edge [color=gray, fontname=\"Helvetica\"];" << endl;

    set<string> visitedNodes;
    int colorIndex = 0;

    for (const auto& entry : adjList) {
        const string& user = entry.first;
        if (visitedNodes.find(user) != visitedNodes.end()) continue;

        string color = generateUniqueColor(colorIndex++);
        visitedNodes.insert(user);

        outFile << "    \"" << user << "\" [label=\"" << user << "\" fillcolor=" << color << "];" << endl;

        for (const auto& neighbor : entry.second) {
            outFile << "    \"" << user << "\" -- \"" << neighbor << "\" [color=blue, style=dashed];" << endl;
            visitedNodes.insert(neighbor);
        }
    }

    outFile << "}" << endl;
    outFile.close();
    cout << "DOT file generated: graph.dot" << endl;


    system("dot -Tpng graph.dot -o graph.png");
    cout << "Graph image generated: graph.png" << endl;

   
    system("code graph.png");

    string userInput;
    cout << "Type 'close' to delete the image and DOT file: ";
    while (true) {
        getline(cin, userInput);
        if (userInput == "close") {
            break;
        }
    }

    if (remove("graph.png") == 0) {
        // cout << "Graph image deleted successfully." << endl;
    } else {
        cerr << "Error deleting the image." << endl;
    }

    if (remove("graph.dot") == 0) {
        // cout << "DOT file deleted successfully." << endl;
    } else {
        cerr << "Error deleting the DOT file." << endl;
    }
}