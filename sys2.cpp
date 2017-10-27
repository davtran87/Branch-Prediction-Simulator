
/*
 * File:   sys1.cpp
 * Author: David Tran
 * Purpose: The purpose of this program is to use a 2 bit branch predictor and
 * read a trace file of forward and backwards conditional branches
 * with type 1 and type 2. The program will
 * ignore type 2 branches. The program will display number of type 1 branches,
 * each F/B branch, if it is taken or not, mispredictions and the rate of mispredictions.
 * Created on March 20, 2017, 10:38 PM
 * Version: 1.0
 * Compilation method: dtran7@unixlab ~]$ g++ sys2.cpp -o sys2
 * Execution method: [dtran7@unixlab ~]$ ./sys2 ~whsu/csc656/Traces/S17/P1/li1.trace 256 64 -v
 *                   [dtran7@unixlab ~]$ ./sys2 ~whsu/csc656/Traces/S17/P1/li2.trace 256 64 -v
 *                   [dtran7@unixlab ~]$ ./sys2 ~whsu/csc656/Traces/S17/P1/li1.trace 1024 256 -v
 *                   [dtran7@unixlab ~]$ ./sys2 ~whsu/csc656/Traces/S17/P1/li2.trace 1024 256 -v
 * The -v is verbose mode and it is optional, with -v used as an argument, it will print out
 * verbose information.




 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>
#include <bitset>
#include <math.h>
#include <string.h>
#include <iomanip>

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
    int pcAdd, type, tarAdd, branchStatus, tag, predIndex, btbIndex;

};
/*
 * Class used for BTB info
 * valid holds either 1 or 0, 1 being valid.
 * tag holds tag that is matched with branch tag.
 */
class BTB {
public:
    int valid;
    int tag;
};

/* Function Declaration */
int predUpdate(int n, int taken);


int main(int argc, char* argv[])
{
    /* Initialization of variables */
    int  numForward = 0, numBackward = 0, totalNumBranches = 0, takenForward = 0, takenBackward = 0,
         misPredictSys1 = 0, btbMiss=0, btbHit=0, correctPred=0, misPredict=0,
          numPredictTaken=0, pBufferSize=0, btbSize=0, temp=0, newState=0, currentState=0, count=0;
    bool verboseMode = false;

    pBufferSize = atoi(argv[2]); // prediction buffer size from argument to int
    btbSize = atoi(argv[3]); // btb buffer size from argument to int

    int predBufferState[pBufferSize]; // prediction buffer array

    BranchInfo branchInfo; //branch info object
    BTB btbInfo; // btb object.

    std::vector<BranchInfo> branchVec; // vector to hold Branch Info objects
    std::vector<BTB> btbVec; // vector to hold btb info objects.

    ifstream traceFile; // an input file stream for trace file

    if(argc < 4) // checks if enough arguments if not exit
    {
        cout << "Need more arguments Ex. ./a.exe test.txt 16 4 -v (-v is optional to print verbose information)";
        exit(EXIT_FAILURE);
    }
    else if (argc > 4) // checks to see if verbose mode is used.
        verboseMode = true;

    traceFile.open(argv[1]); // open trace file from argument
    if (traceFile.fail()) // check if open file fails
        cout << "error opening file" << endl;

    while (!traceFile.eof())  // while loop used to read file and store BranchInfo objects in a vector
    {
        traceFile >> std::hex >> branchInfo.pcAdd >> branchInfo.type >> branchInfo.tarAdd >> branchInfo.branchStatus;
        branchVec.push_back(branchInfo);
    }

    /* Initializing array and vectors */
    for(int i=0; i<pBufferSize; i++) // populating predict buffer with 1;
        predBufferState[i] = 1;
    for(int i=0; i<btbSize; i++) // initializing all BTB valid bits to 0 & store in vector tag is garbage;
    {
        btbInfo.valid = 0;
        btbVec.push_back(btbInfo);
    }

    for (unsigned int i = 0; i < branchVec.size(); i++) // loop to iterate through each object in vector
    {
        if (branchVec[i].type != 1) // if type 2 skip to next iteration
            continue;

        totalNumBranches++; // increment branches that are not type 2
        temp = branchVec[i].pcAdd >> 2; // shifting to throw away the last 2 bits of PC address for index
        branchVec[i].predIndex =  (temp)%pBufferSize; // calculate index for branch using mod to get last bits needed for index
        branchVec[i].btbIndex = (temp)%btbSize; // calculate btb index for branch using mod to get last bits needed
        branchVec[i].tag = branchVec[i].pcAdd >> ((int)log2(pBufferSize)); // get the tag by shifting logbase2 of prediction buffer size
        // cout << std::hex << "tag: " << branchVec[i].tag << endl;
        // cout << "index: " <<(branchVec[i].predIndex) << endl << "btbindex: " << branchVec[i].btbIndex << endl; //index for specific branch
        // cout << std::hex << "temp:" << temp << endl;
        // cout << std::hex << "tag: " << branchVec[i].tag << endl;
        // cout << "pred state " << predBufferState[branchVec[i].predIndex] << endl; //state in pred Buffer

        if(predBufferState[branchVec[i].predIndex] == 2 || predBufferState[branchVec[i].predIndex] == 3) // if branch is predicted taken
        {
            numPredictTaken++;

            if( btbVec[branchVec[i].btbIndex].valid == 1 && btbVec[branchVec[i].btbIndex].tag == branchVec[i].tag) // if btb bit is valid and btb tag matches branch tag else miss
                btbHit++;
            else
                btbMiss++;

        }

        // checks if branch is taken and if prediction is predicted correct.
        if((branchVec[i].branchStatus == 1 && (predBufferState[branchVec[i].predIndex] == 2 || predBufferState[branchVec[i].predIndex] == 3)) ||
                (branchVec[i].branchStatus == 0 && (predBufferState[branchVec[i].predIndex] == 0 || predBufferState[branchVec[i].predIndex] == 1)))
        {
            correctPred++;
        }
        else
            misPredict++;
        currentState = predBufferState[branchVec[i].predIndex];
        newState = predUpdate(predBufferState[branchVec[i].predIndex], branchVec[i].branchStatus); // call to update prediction buffer state.
        predBufferState[branchVec[i].predIndex] = newState; //updates current prediction state to new state.
        // cout << "newstate: " << newState << endl;

        if(branchVec[i].branchStatus == 1) // if the branch is predicted taken set current btb tag to equal branch tag and valid bit to 1.
        {
            btbVec[branchVec[i].btbIndex].tag = branchVec[i].tag;
            btbVec[branchVec[i].btbIndex].valid = 1;
        }
        /* Verbose mode output */

        if(verboseMode) //if verboseMode is true print verbose information
        {
            cout << std::dec << count << setw(3) << " ";
            count+=1;
            cout << std::hex << branchVec[i].predIndex  << setw(5)<< right << " ";
            cout << std::dec << currentState  << setw(3)<< right << " " << predBufferState[branchVec[i].predIndex] << setw(3) << " " ;
            cout << std::hex << branchVec[i].btbIndex << setw(3) << " " << branchVec[i].tag << setw(3) << " ";
            cout << std::dec << numPredictTaken << setw(3)<< " " << btbMiss << endl;

            //cout << std::hex << branchVec[i].tag << endl;
        }
        /* sys 1 stuff */
        if ((branchVec[i].pcAdd - branchVec[i].tarAdd) < 0) // checks if branch is forward or backwards
            numForward++;
        else
            numBackward++;

        if ((branchVec[i].pcAdd - branchVec[i].tarAdd) < 0 && branchVec[i].branchStatus == 1)//if it is forward and taken (incorrect prediction)
        {
            takenForward++, misPredictSys1++;
            continue;
        }

        if (branchVec[i].pcAdd - branchVec[i].tarAdd > 0 && branchVec[i].branchStatus == 1) // if backwards and taken (correct prediction)
        {
            takenBackward++;
            continue;
        }
        if ((branchVec[i].pcAdd - branchVec[i].tarAdd) > 0 && branchVec[i].branchStatus == 0) // if it is backwards and not taken (incorrect prediction)
             misPredictSys1++;
    }

    /* Prints all necessary information */
    cout << std::dec << "Number of branches = " << totalNumBranches << endl; // ie only type 1 branches not type 2
    cout << std::dec << "Number of forward branches = " << numForward << endl;
    cout << std::dec << "Number of forward taken branches = " << takenForward << endl;
    cout << std::dec << "Number of backward branches = " << numBackward << endl;
    cout << std::dec << "Number of backward taken branches = " << takenBackward << endl;
    //cout << std::dec << "Number of mispredictions for system 1 = " << misPredictSys1 << " " << (double)misPredictSys1/totalNumBranches << endl;
    cout << std::dec << "Number of mispredictions = " << misPredict << " " << (double)misPredict/totalNumBranches << endl;
    cout << std::dec << "Number of BTB misses = " << btbMiss << " " << (double)btbMiss/numPredictTaken << endl;
    // cout << numPredictTaken;
    //cout << verboseMode;
    traceFile.close(); // closes the file

    return 0;
}

/* This function updates the prediction state of the buffer
 * @return returns the updated state of the prediction.
 * @Param n is the current prediction before update
 *        taken is the actual behavior of the branch.
*/
int predUpdate(int n, int taken)
{
    if ( n == 0 && taken == 1) return n+1;
    if ( n == 1 && taken == 1) return n+1;
    if ( n == 2 && taken == 1) return n+1;
    if ( n == 3 && taken == 1) return n;
    if ( n == 0 && taken == 0) return n;
    if ( n == 1 && taken == 0) return n-1;
    if ( n == 2 && taken == 0) return n-1;
    if ( n == 3 && taken == 0) return n-1;
    return n;
}
