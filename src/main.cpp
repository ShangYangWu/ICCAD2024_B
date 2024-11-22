#include <iostream>
#include <fstream>
#include <vector>
#include "algo.h"

using namespace std;

int main(int argc, char** argv)
{
    fstream input_file, output_file;

    if (argc == 3) {
        input_file.open(argv[1], ios::in);
        output_file.open(argv[2], ios::out);
        if (!input_file) {
            cerr << "Cannot open the input file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output_file) {
            cerr << "Cannot open the output file \"" << argv[3]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./main <input file> <output file> " << endl;
        exit(1);
    }

    /// parse input
    Algo* algo = new Algo(input_file);
    string filename = argv[1];

    /// stage1: prepare essential info for stage2 
    algo->InitSite();   // build site map for latter usage
    algo->legalInit();  // place init input to legal position
    algo->writeInit(output_file); // ensure legal output be performed
    algo->InitBinUsage();   // calculate bin usage for latter usage
    algo->buildFFGraph();   // build FF relationships
    algo->initfeasible();   // calculate initial feasible region
    
    // print estimated initial cost
    algo->objectiveFunctionBefore(); 

    /// stage2: select FF candiates, bank, place onto legal location
    /* algo explanation */
    // step1: Banking() 
    // => send candidates to algo->selectBanking()
    // step2: selectBanking() 
    // =>  1. merge FFs with better c/p FF in FF_library (algo->lowestCostFF())
    //     2. find location for FF placement (algo->locationDecision())
    //     3. calculate merging cost(algo->calAvailableSiteCost()) < origin cost (algo->costBeforeMerge())
    //     4. repeat 2.->3. until "3. passed" or "FFs aren't mergeable"
    //     5. algo->MergeInstance("FFs")
    algo->Banking();
    
    
    /// stage3: write output
    algo->mapMutiFF(); // we excluded initial mutibit FF from FF list; therefore, we need to insert back the FF before output is exported
    output_file.open(argv[2], ios::out);
    algo->write(output_file);

    return 0;
}