#include "Header.h"    // Include the common header
#include "Post.h"      // For Post-related functionality
#include "Users.h"     // For User-related functionality
#include "Global.h"    // For global variables and functions
#include <iostream>


int main(int argc, char* argv[]) {
    loadUserData(); 
    int mainChoice, adminChoice, userChoice;
    
    while (true) {
        displayMainMenu();
        cout << "Enter Choice: ";
        cin >> mainChoice;
        
        switch (mainChoice) {
            case 1: // Admin section
                while (true) {
                    displayAdmin();
                    cout << "Enter Admin Choice: ";
                    cin >> adminChoice;
                    
                    switch (adminChoice) {
                        case 1: {
                            string attribute;
                            cout << "Enter the attribute name (gender or country): ";
                            cin >> attribute;
                            unordered_map<string, vector<string>> adjList;
                            generateAdjacencyList(adjList, attribute);
                            
                            string dotFilename = "graph.dot";
                            string outputFilename = "graph.png";
                            generateDotFile(adjList, attribute, dotFilename);
                            renderDotFile(dotFilename, outputFilename);
                            
                            string closeCommand;
                            cout << "Type 'close' to delete the generated files and exit: ";
                            cin >> closeCommand;
                            if (closeCommand == "close") {
                                deleteFiles(dotFilename, outputFilename);
                                break; // Exit admin menu
                            }
                            break;
                        }
                        case 2: 
                        {
                            string usr;
                            cout<<"Enter the Username:";
                            cin>>usr;
                            current_user=findUserByUsername(usr);
                            adjList = createAdjacencyList();
                            displayAdjacencyList();
                            break;
                        }
                        case 3:
                        {
                        adjList = createAdjacencyList();
                        adjList = convertToUndirected(adjList);
                        vector<vector<User*>> communities = detectCommunities(adjList);
                        // generateCommunityGraph(communities);

                      
                        computeCommunityMetrics(communities);

                           
                            cout << "Press Enter to delete generated files and exit...";
                            cin.ignore();  
                            
                            
                            string input;
                            cout << "Enter 'close' to delete generated files and exit: ";
                            while (true) {
                                getline(cin, input);
                                if (input == "close") {
                                    
                                    deleteCommunityGraphFiles();
                                    break;
                                }
                                else {
                                    cout << "Invalid input. Please enter 'close' to delete files and exit: ";
                                }
                            }
                        
                        }
                        break;
                        case 4:
                        {
                           adjList = createAdjacencyList();
                           adjList = convertToUndirected(adjList);
                        double density = calculateDensity(adjList);
                           cout << "Graph Density: " << density << endl;
                            generateGraph(adjList);
                        }
                        break;
                        default:
                            cout << "Invalid choice in Admin Menu, please try again." << endl;
                            break;
                    }
                }
                break;

            case 2: 
                displayFirstMenu();
         
                cin >> userChoice;

                switch (userChoice) {
                    case 1:
                        registerUser();
                        break;
                    case 2:
                        if (loginUser()) {
                            while (true) {
                                loadFollowers(current_user);
                                loadFollowings(current_user);
                                viewDetails();
                                displaySecondMenu();
                                
                                int userAction;
                         
                                cin >> userAction;
                                
                                switch (userAction) {
                                    case 1:
                                        loadFollowers(current_user);
                                        loadFollowings(current_user);
                                        break;
                                    case 2:
                                        addPost();
                                        break;
                                    case 3:
                                        deletePost();
                                        break;
                                    case 4:
                                        displayFollowers(current_user);
                                        break;
                                    case 5:
                                        displayFollowings(current_user);
                                        break;
                                    case 6:
                                        displayAllUsers();
                                        break;
                                    case 7:
                                        viewUser();
                                        break;
                                    case 8:
                                        logoutUser();
                                        break;
                                    case 9:
                                        exit(0);
                                    default:
                                        cout << "Invalid choice in User Action, please try again." << endl;
                                        break;
                                }
                                if (!current_user) break;
                            }
                        }
                        break;
                    case 3:
                        deleteUser();
                        break;
                    case 4:
                        exit(0);
                    default:
                        cout << "Invalid choice in User Menu, please try again.\n";
                        break;
                }
                break;

            default:
                cout << "Invalid choice in Main Menu, please try again.\n";
                break;
        }
    }

    return 0;
}