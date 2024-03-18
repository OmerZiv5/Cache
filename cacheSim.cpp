#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include "helper.cpp"
#include <vector>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

int main(int argc, char **argv) {

    if (argc < 19) {
        cerr << "Not enough arguments" << endl;
        return 0;
    }

    // Get input arguments

    // File
    // Assuming it is the first argument
    char* fileString = argv[1];
    ifstream file(fileString); //input file stream
    string line;
    if (!file || !file.good()) {
        // File doesn't exist or some other error
        cerr << "File not found" << endl;
        return 0;
    }

    unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
            L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

    for (int i = 2; i < 19; i += 2) {
        string s(argv[i]);
        if (s == "--mem-cyc") {
            MemCyc = atoi(argv[i + 1]);
        } else if (s == "--bsize") {
            BSize = atoi(argv[i + 1]);
        } else if (s == "--l1-size") {
            L1Size = atoi(argv[i + 1]);
        } else if (s == "--l2-size") {
            L2Size = atoi(argv[i + 1]);
        } else if (s == "--l1-cyc") {
            L1Cyc = atoi(argv[i + 1]);
        } else if (s == "--l2-cyc") {
            L2Cyc = atoi(argv[i + 1]);
        } else if (s == "--l1-assoc") {
            L1Assoc = atoi(argv[i + 1]);
        } else if (s == "--l2-assoc") {
            L2Assoc = atoi(argv[i + 1]);
        } else if (s == "--wr-alloc") {
            WrAlloc = atoi(argv[i + 1]);
        } else {
            cerr << "Error in arguments" << endl;
            return 0;
        }
    }

    init_level(L1Assoc, L1Size, BSize, 1);
    init_level(L2Assoc, L2Size, BSize, 2);
    double command_counter = 0;

    while (getline(file, line)) {

        stringstream ss(line);
        string address;
        char operation = 0; // read (R) or write (W)
        if (!(ss >> operation >> address)) {
            // Operation appears in an Invalid format
            cout << "Command Format error" << endl;
            return 0;
        }

        // DEBUG - remove this line
        cout << "operation: " << operation;

        string cutAddress = address.substr(2); // Removing the "0x" part of the address

        // DEBUG - remove this line
        cout << ", address (hex)" << cutAddress;

        unsigned long int num = 0;
        num = strtoul(cutAddress.c_str(), NULL, 16);

        if(operation == 'r'){
            read(num, BSize, L1Cyc, L2Cyc, MemCyc);
        }
        else{
            write(num, BSize, L1Cyc, L2Cyc, MemCyc, WrAlloc);
        }

        // DEBUG - remove this line
        cout << " (dec) " << num << endl;
        command_counter++;


        // DEBUG - Remove this shit
        cout << "-------------------------------------" << endl;
        cout << "L1 cache" << endl;
        for(int i = 0; i < level1.num_rows; i++){
            for(int j = 0; j < level1.ways; j++){
                cout << "block: " << L1[j][i].min_addr << "-" << L1[j][i].max_addr << "  |  ";
            }
            cout << endl;
        }
        cout << "L2 cache" << endl;
        for(int i = 0; i < level2.num_rows; i++){
            for(int j = 0; j < level2.ways; j++){
                cout << "block: " << L2[j][i].min_addr << "-" << L2[j][i].max_addr << "  |  ";
            }
            cout << endl;
        }
    }

    // Cleaning memory
    std::vector<std::vector<int>>::iterator ita = LRU1.begin();
    while(ita != LRU1.end()){
        LRU1.erase(ita);
    }
    std::vector<std::vector<int>>::iterator itb = LRU2.begin();
    while(itb != LRU2.end()){
        LRU2.erase(itb);
    }
    std::vector<std::vector<Block>>::iterator itc = L1.begin();
    while(itc != L1.end()){
        L1.erase(itc);
    }
    std::vector<std::vector<Block>>::iterator itd = L2.begin();
    while(itd != L2.end()){
        L2.erase(itd);
    }

    double L1MissRate;
    double L2MissRate;
    double avgAccTime;

    L1MissRate = level1.misses / command_counter;
    L2MissRate = level2.misses / attempts_L2;
    avgAccTime = total_time / command_counter;

    printf("L1miss=%.03f ", L1MissRate);
    printf("L2miss=%.03f ", L2MissRate);
    printf("AccTimeAvg=%.03f\n", avgAccTime);

    return 0;
}
