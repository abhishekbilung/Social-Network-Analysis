#ifndef GLOBAL_H
#define GLOBAL_H

#include "Header.h"
#include "Post.h"
#include "Users.h"


// Global variables

extern vector<User> users;
extern int next_user_id;
extern unordered_map<string, vector<Post>> userPosts;
extern unordered_set<string> onlineUsers; 
extern unordered_map<string, vector<string>> adjList;
extern User* current_user; 
extern User* search_user;

// Functions related to threads data creation
void loadUserData();  
void saveUserData();
void savePostData(User* user);
void loadFollowers(User* loaduser);
void loadFollowings(User* loaduser);
void removeFollower();
void removeFollowing();
void addFollower(User* new_follower);
void addFollowing(User* new_following);
void displayFollowers(User* search);
void displayFollowings(User* search);
bool userExists(const string& username);
void registerUser();
bool loginUser();
void logoutUser();
void deleteUser();
void viewDetails();
void SearchUserDetails(User* search);
void displayAllUsers();
void displayMainMenu();
void displayFirstMenu();
void displaySecondMenu();
void displayThirdMenu();
void likePost();
void updatePostFile(User* search_user, const Post& post, int current_user_id);
void unlikePost(User* search_user);
void commentOnPost();
void addPost();
void deletePost();
void viewUser();
void readUserData(const string& filepath, unordered_map<string, string>& users_map);

//Functions related to Social Network Analysis

string interpretDegreeCentrality(double value);
string interpretClosenessCentrality(double value);
string interpretBetweennessCentrality(double value);
string interpretEigenvectorCentrality(double value);
string interpretLocalClusteringCoefficient(double value);
string interpretGlobalClusteringCoefficient(double value);
string interpretGraphDensity(double value);
void calculateDegreeCentrality();
void addToAdjList(unordered_map<string, vector<string>>& adjList, const string& user1, const string& user2);
void generateAdjacencyList( unordered_map<string, vector<string>>& adjList, const string& attribute);
void generateDotFile(const unordered_map<string, vector<string>>& adjList, const string& attribute, const string& filename);
void renderDotFile(const string& dotFilename, const string& outputFilename);
void deleteFiles(const string& dotFilename, const string& outputFilename);
unordered_map<string, vector<string>> createAdjacencyList();
void generateDotFile(const unordered_map<string, vector<string>>& adj, const string& filename);
void generateSubgraph(const unordered_map<string, vector<string>>& adj, const string& node, const string& filename);
void generateAndOpenGraph(const string& dotFilename);
void waitForInputAndCleanUp(const string& dotFilename, const string& pngFilename);
void calculateClosenessCentrality();
void calculateBetweennessCentrality();
void calculateEigenvectorCentrality();
void calculateGraphDensity();
void calculateClusteringCoefficient();
int countTriangles();
int countTrianglesForNode(const string& node);
void calculateGlobalClusteringCoefficients();
void displayAdjacencyList();
void generateCommunityGraph(const vector<vector<User*>>& communities);
void deleteCommunityGraphFiles();
vector<vector<User*>> detectCommunities(const unordered_map<string, vector<string>>& adjList);
double computeCommunityDensity(const vector<User*>& community);
double computeModularity(const vector<User*>& community);
double computePermanence(const vector<User*>& community);
void computeCommunityMetrics(const vector<vector<User*>>& communities);
void displayAdmin();
User* findUserByUsername(const string& username);
double calculateDensity(const unordered_map<string, vector<string>>& adjList);
string generateUniqueColor(int index);
unordered_map<string, vector<string>> convertToUndirected(const unordered_map<string, vector<string>>& directedAdjList);
void generateGraph(const unordered_map<string, vector<string>>& adjList);

#endif
