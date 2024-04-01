#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>

using namespace std;

const int NUM_REPLICAS = 3; // Number of replicas for each file

struct File {
    string name;
    string data;
    time_t lastModified;

    File() : lastModified(time(nullptr)) {}

    File(const string& _name, const string& _data) : name(_name), data(_data), lastModified(time(nullptr)) {}
};

struct StorageNode {
    string id;
    unordered_map<string, File> files; // Files stored on this node
};

// DFS class
class DistributedFileSystem {
private:
    vector<StorageNode> storageNodes; // List of storage nodes
    mutex mtx; // Mutex for thread safety

public:
    // Initialize storage nodes
    DistributedFileSystem() {
        for (int i = 0; i < NUM_REPLICAS; ++i) {
            StorageNode node;
            node.id = "Node" + to_string(i + 1);
            storageNodes.push_back(node);
        }
    }

    // Write file to DFS
    void writeFile(const string& filename, const string& data) {
        mtx.lock(); // Lock mutex for thread safety

        // Replicate file to multiple nodes
        for (int i = 0; i < NUM_REPLICAS; ++i) {
            storageNodes[i].files[filename] = File(filename, data);
        }

        mtx.unlock(); // Unlock mutex
    }

    // Read file from DFS
    string readFile(const string& filename) {
        mtx.lock(); // Lock mutex for thread safety

        // Read file from the first available replica
        for (int i = 0; i < NUM_REPLICAS; ++i) {
            auto it = storageNodes[i].files.find(filename);
            if (it != storageNodes[i].files.end()) {
                mtx.unlock(); // Unlock mutex
                return it->second.data;
            }
        }

        mtx.unlock(); // Unlock mutex
        return "File not found";
    }

    // Delete file from DFS
    void deleteFile(const string& filename) {
        mtx.lock(); // Lock mutex for thread safety

        // Delete file from all replicas
        for (int i = 0; i < NUM_REPLICAS; ++i) {
            auto it = storageNodes[i].files.find(filename);
            if (it != storageNodes[i].files.end()) {
                storageNodes[i].files.erase(it);
            }
        }

        mtx.unlock(); // Unlock mutex
    }

    // List all files in DFS
    void listFiles() {
        mtx.lock(); // Lock mutex for thread safety

        cout << "Files in DFS:" << endl;
        for (int i = 0; i < NUM_REPLICAS; ++i) {
            cout << "Node " << storageNodes[i].id << ":" << endl;
            for (const auto& entry : storageNodes[i].files) {
                cout << entry.first << endl;
            }
        }

        mtx.unlock(); // Unlock mutex
    }
};

int main() {
    DistributedFileSystem dfs;

    // Write files to DFS
    dfs.writeFile("file1.txt", "Data for file1");
    dfs.writeFile("file2.txt", "Data for file2");
    dfs.writeFile("file3.txt", "Data for file3");

    // Read and print files from DFS
    cout << "Reading files from DFS:" << endl;
    cout << "file1.txt: " << dfs.readFile("file1.txt") << endl;
    cout << "file2.txt: " << dfs.readFile("file2.txt") << endl;
    cout << "file3.txt: " << dfs.readFile("file3.txt") << endl;

    // List all files in DFS
    dfs.listFiles();

    // Delete file from DFS
    dfs.deleteFile("file2.txt");

    // List all files in DFS after deletion
    cout << "After deleting file2.txt:" << endl;
    dfs.listFiles();

    return 0;
}

