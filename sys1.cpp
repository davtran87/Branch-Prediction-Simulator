
/* 
 * File:   sys1.cpp
 * Author: David Tran
 * Purpose: The purpose of this program is to read a trace file of forward
 * and backwards conditional branches with type 1 and type 2. The program will
 * ignore type 2 branches. The program will display number of type 1 branches, 
 * each F/B branch, if it is taken or not, mispredictions and the rate of mispredictions. 
 * Created on March 20, 2017, 10:38 PM
 * Version: 1.0
 * Compilation method: [dtran7@unixlab ~]$ g++ sys1.cpp -o sys1
 * Execution method: [dtran7@unixlab ~]$ ./sys1 ~whsu/csc656/Traces/S17/P1/li1.trace
 *                   [dtran7@unixlab ~]$ ./sys1 ~whsu/csc656/Traces/S17/P1/li1.trace
 *                   [dtran7@unixlab ~]$ ./sys1 ~whsu/csc656/Traces/S17/P1/li1.trace [-v]
 *                   [dtran7@unixlab ~]$ ./sys1 ~whsu/csc656/Traces/S17/P1/li2.trace [-v]
 * Above is all execution methods for sys1 with verbose on and off.
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

/*
 * Class used for Branch Info
 * pcAdd is the starting address
 * type is the type either 1 or 2 throw out type 2
 * tarAdd target address used to compare with pcAdd
 * branchStatus either 1 or 0. 1 for taken and 0 for not taken.
 */
class BranchInfo {
public:
    int pcAdd, type, tarAdd, branchStatus;

};


int main(int argc, char** argv) 
{
    int numForward = 0, numBackward = 0, totalNumBranches = 0, takenForward = 0, takenBackward = 0, misPredict = 0, count=0; // initialization of variables
        
    
    bool verboseMode = false;
    BranchInfo object; //create Branch info object
    std::vector<BranchInfo> v; // create a vector to store BranchInfo Object

    ifstream myFile; // an input file stream object
    myFile.open(argv[1]); //open file from arg[1]
    
    if(argc < 2)
    {
        cout << "Too little arguments Ex. ./a.exe test.txt";
        exit(EXIT_FAILURE);
    }
    
    if(argc > 2)
        verboseMode = true;

    
    if (myFile.fail()) //check if open file failed and prints error if so
        cout << "error opening file" << endl;
    
    while (!myFile.eof())  // while loop used to read file and store BranchInfo objects in a vector
    {
        myFile >> std::hex >> object.pcAdd >> object.type >> object.tarAdd >> object.branchStatus;
        v.push_back(object);
    }
    
    for (unsigned int i = 0; i < v.size(); i++) // loop to iterate through each object in vector
    {

        if (v[i].type != 1) // if type 2 skip to next iteration
            continue;
        if(verboseMode) // checks if verbose mode is on and prints verbose information.
        {
            cout << std::hex << v[i].pcAdd << " ";
            cout << v[i].type << " ";
            cout << std::hex << v[i].tarAdd << " ";
            cout << v[i].branchStatus << " " << endl;
        }
        totalNumBranches++; // increment branches that are not type 2
        if ((v[i].pcAdd - v[i].tarAdd) < 0) // checks if branch is forward or backwards
            numForward++;
        else
            numBackward++;

        if ((v[i].pcAdd - v[i].tarAdd) < 0 && v[i].branchStatus == 1)//if it is forward and taken (incorrect prediction)
        { 
            takenForward++, misPredict++; 
            continue;
        }

        if (v[i].pcAdd - v[i].tarAdd > 0 && v[i].branchStatus == 1) // if backwards and taken (correct prediction)
        {
            takenBackward++;
            continue;
        }
        if ((v[i].pcAdd - v[i].tarAdd) > 0 && v[i].branchStatus == 0) // if it is backwards and not taken (incorrect prediction)
             misPredict++;
        

    }
    /*  displays needed information. */
    cout << std::dec << "Number of branches = " << totalNumBranches << endl; // ie only type 1 branches not type 2
    cout << std::dec <<  "Number of forward branches = " << numForward << endl;
    cout << std::dec <<  "Number of forward taken branches = " << takenForward << endl;
    cout << std::dec <<  "Number of backward branches = " << numBackward << endl;
    cout << std::dec <<  "Number of backward taken branches = " << takenBackward << endl;
    cout << std::dec <<  "Number of mispredictions = " << misPredict << " " << (double)misPredict/totalNumBranches << endl;
    myFile.close(); // closes the file
    return 0;
}

