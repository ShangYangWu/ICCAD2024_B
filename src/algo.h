#ifndef ALGO_H
#define ALGO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include "module.h"
using namespace std;

class Algo 
{
public:
    Algo(fstream& input_file)
    {
        parse(input_file);
    };
    ~Algo() 
    {
        clear();
    };

/* parser */
    void parse(fstream& input_file);
    void write(fstream& output_file);
    void print(); // parser checker

/* set functions */
    void addInstance(Instance* instance) { _instanceArray.emplace_back(instance); _instanceName2Id[instance->getInstanceName()] = _instanceArray.size()-1; }
    void addMultibitIns(Instance* instance) { _multibitInsArray.emplace_back(instance); } // 0826 updated
    void addFF(FlipFlop* ff) { _ffArray.emplace_back(ff); _ffName2Id[ff->getLibCellName()] = _ffArray.size()-1; }
    void addGate(Gate* gate) { _gateArray.emplace_back(gate); _gateName2Id[gate->getLibCellName()] = _gateArray.size()-1; }
    void addNet(Net* net) { _netArray.emplace_back(net); _netName2Id[net->getNetName()] = _netArray.size()-1; }
    void delNet(Net* net) { 
        _netName2Id.erase(net->getNetName());                             
        auto target = find(_netArray.begin(), _netArray.end(), net);
        if (target != _netArray.end()) {
            _netArray.erase(target);
        }
    }
    void addClkNet(Net* net) { _clkNetArray.emplace_back(net); }
    void addIONet(Net* net) { _IONetArray.emplace_back(net); }
    void addMappingHistory(pair<string, string> record) { _mappingHistory.emplace_back(record); }
    void lowestCostFF();

/* initialization functions */
    void legalInit();           // legalize the initial network
    void buildFFGraph();
    void InitNeighbors(Instance* starter, Instance* passby, pair<Pin*, Pin*> &pin_pair, vector<Instance*> &visited_gate);
    void InitSite();
    void InitBinUsage();
    void initfeasible();

/* operation function */
    double CalculateCost(FlipFlop* FF); // for sorting FF lib
    void MergeInstance(set<Instance*> flipflops, Instance* target, FlipFlop* FF, Site* site);
    void Banking();
    void selectBanking(set<Instance*> FF);
    map<string, set<Instance*>>  Window(int &placementRowIdx, int &siteIdy, int length , int width);
    void locationDecision(set<Instance*> flipflops, FlipFlop* FF);
    double costBeforeMerge(set<Instance*> flipflops, FlipFlop* FF, Site* site, double cost);
    double calAvailableSiteCost(set<Instance*> flipflops, FlipFlop* FF, Site* site, double cost, double oriCost);
    vector<Point> findOverlapFeasible(set<Instance*> flipflops);
    void updateSlack(set<Instance*> ins, FlipFlop* FF, Site* curSite, Instance* mergeIns);
    void updatenextSlack(Instance* ins, Edge* edge, int OutPinindex,Instance* mergeIns);
    void updateFeasible(set<Instance*> target);
    int  addBinUsage(int x1, int x2, int y1, int y2);
    void subBinUsage(int x1, int x2, int y1, int y2);
    int  nextBinx(int);
    int  upBiny(int);

/* export function */
    void objectiveFunctionBefore();
    void printBinUsage();
    void mapMutiFF();
    void writeInit(fstream& output_file);

private:
    double                      _alpha;
    double                      _beta;
    double                      _gamma;
    double                      _lambda;
    Die                         _die;                   // die information
    vector<PlacementRow*>       _placementRowArray;     // placement row array of the design
    vector<Bin*>                _binArray;              // bin array of the design
    vector<vector<double>>      _binUsagerate;          // bin usage rate
    vector<PrimaryInput*>       _primaryInputs;         // primary inputs
    vector<PrimaryOutput*>      _primaryOutputs;        // primary outputs
    vector<Instance*>           _onePinNetInstance;     // Instances connect with net with only one pin 
    double                      _displaymentDelay;      // displayment delay

/*Cell Library*/
    size_t                          _instanceNum;           // number of instances
    size_t                          _flipFlopNum;           // number of flip-flops
    size_t                          _gateNum;               // number of gates
    vector<Instance*>               _instanceArray;         // instance array of the design
    vector<Instance*>               _multibitInsArray;      // initial muti-bit flip-flop instance array of the design // 0826 updated
    vector<FlipFlop*>               _ffArray;               // flip-flop array of the design
    vector<Gate*>                   _gateArray;             // gate array of the design
    unordered_map<string, size_t>   _instanceName2Id;       // mapping from instance name to id
    unordered_map<string, size_t>   _ffName2Id;             // mapping from flip-flop name to id
    unordered_map<string, size_t>   _gateName2Id;           // mapping from gate name to id
    map<int, FlipFlop*>             _lowestCostFFMap;       // mapping from bit to lowest cost FF
    map<int,int>                    _previousSize;          // previous size of the FF

/*Net List*/
    size_t                          _netNum;                // number of nets
    vector<Net*>                    _netArray;              // net array of the design
    vector<Net*>                    _clkNetArray;           // clock net array of the design
    vector<Net*>                    _IONetArray;            // I/O net array of the design
    unordered_map<string, size_t>   _netName2Id;            // mapping from net name to id
    unordered_map<string, unordered_set<Instance*>> _sameclk; // flip flop instances with same clock

/*Bin*/
    int                      _binWidth;              // width of the bin
    int                      _binHeight;             // height of the bin
    double                      _binMaxUtli;            // maximum utilization of the bin

/*Site*/
    double                      _siteWidth;             // width of the site
    double                      _siteHeight;            // height of the site
    size_t                      _siteNum;               // number of sites of a row

/*Norm factor*/
    double                      _avgWL;                 // average wire length
    double                      _areaNorm;              // area norm factor
    double                      _pwrNorm;               // power norm factor
    double                      _qpinNorm;              // qpin delay norm factor

/* Record */
    bool _rtn;                                          // for slack updatings
    vector<pair<string, string>> _mappingHistory;       // mapping record

/*clean up algo*/
    void clear();

/* helper function */
    double _totalCost;                                 // total cost of the design
    set<Instance*> _visited;
};
 
class Candidate{
public:
    Candidate(double value, bool type, Instance* ff) : value(value), type(type), ff(ff) { }
    ~Candidate() { }
    double value;
    bool type;
    Instance* ff;
};

#endif // ALGO_H