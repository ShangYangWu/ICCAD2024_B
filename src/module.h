#ifndef MODULE_H
#define MODULE_H
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;
class Site;
class Pin
{
public:
    Pin(string& name, double x, double y):
        _pinName(name), _instName(""), _libPinName(""), _x(x), _y(y) { } 
    ~Pin() {}
    
    /*get functions*/
    string getPinName() { return _pinName; }
    double getX() { return _x; }
    double getY() { return _y; }
    string getInstName() { return _instName; }
    string getLibPinName() { return _libPinName; }
    string getNetName() { return _netName; }
    size_t getPinType() { return _pinType; }
    double getOriginalQWL() { return _originalQwl; }
    double getOriginalDWL() { return _originalDwl; }
    double getOriginalQpinDelay() { return _originalqpindelay; }

    /*set functions*/
    void setX(double x) { _x = x; }
    void setY(double y) { _y = y; }
    void setPinName(string& name){ _pinName = name; }
    void setInstName(string& instName) { _instName = instName; }
    void setLibPinName(string& libPinName) { _libPinName = libPinName; }
    void setNetName(string& netName) { _netName = netName; }
    void setPinType(size_t type) { _pinType = type; }
    void setOriginalDWL(double wl) { _originalDwl = wl; }
    void setOriginalQWL(double wl) { _originalQwl = wl; }
    void setOriginalQpinDelay(double delay) { _originalqpindelay = delay; }

private:
    string              _pinName;               // pin name
    string              _instName;              // instance name of the pin
    string              _libPinName;            // library pin name of the pin
    string              _netName;               // net name
    double              _x;                     // x coordinate of the pin
    double              _y;                     // y coordinate of the pin
    size_t              _pinType;               // type of the pin (0: input, 1: output, 2: clock)
    double              _originalDwl;           // original D wirelength
    double              _originalQwl;           // original Q wirelength
    double              _originalqpindelay;     // original Q pin delay
};



class FlipFlop
{
public:
    FlipFlop(string& libCellName, double width, double height, size_t pinCount, size_t bits):
        _libCellName(libCellName), _width(width), _height(height), _pinNum(pinCount), _bitNum(bits) {}
    ~FlipFlop() {}

    /*get functions*/
    pair<double, double> getOutline() {return {_width, _height};}
    size_t getBits() { return _bitNum; }
    string getLibCellName() { return _libCellName; }
    vector<Pin*> getPinList() { return _pinList; }
    vector<Pin*> getDPinList() { return _dpinlist; }
    vector<Pin*> getQPinList() { return _qpinlist; }
    vector<Pin*> getClkPinList() { return _clkpinlist; }
    size_t getPinId(string pinName) { return _pinName2Id[pinName]; }
    size_t getPinNum() { return _pinNum; }
    double getQpinDelay() { return _QpinDelay; }
    double getGatePower() { return _gatePower; }
    double getCP() { return _costPerformance; }
    double getWidth() { return _width; }
    double getHeight() { return _height; }

    /*set functions*/
    void addPin(Pin* pin) { _pinList.emplace_back(pin); _pinName2Id[pin->getPinName()] = _pinList.size()-1; }
    void addDpin(Pin* pin) { _dpinlist.emplace_back(pin); }
    void addQpin(Pin* pin) { _qpinlist.emplace_back(pin); }
    void addClkpin(Pin* pin)  { _clkpinlist.emplace_back(pin); }
    void setQpinDelay(double delay) { _QpinDelay = delay; }
    void setGatePower(double power) { _gatePower = power; }
    void setCP(double cp) { _costPerformance = cp; }
    void sortPins() {
        sort(_pinList.begin(), _pinList.end(), [](Pin* a, Pin* b) { 
            if(a->getPinName() == "CLK") return false;
            if(b->getPinName() == "CLK") return true;
            return a->getPinName() < b->getPinName(); 
            });
    }

private:
    string                          _libCellName;           // library cell name of the flip-flop
    double                          _width;                 // width of the flip-flop
    double                          _height;                // height of the flip-flop
    size_t                          _bitNum;                // number of bits of the flip-flop
    size_t                          _pinNum;                // number of pins of the flip-flop
    vector<Pin*>                    _pinList;               // pins of the flip-flop
    vector<Pin*>                    _dpinlist;
    vector<Pin*>                    _qpinlist;
    vector<Pin*>                    _clkpinlist;
    unordered_map<string, size_t>   _pinName2Id;            // mapping from pin name to id
    double                          _QpinDelay;             // delay of the Q pin
    double                          _gatePower;             // power consumption of the flip-flop
    double                          _costPerformance;       // consider area and power
};

class Gate
{
public:
    Gate(string& libCellName, double width, double height, size_t pinCount):
        _libCellName(libCellName), _width(width), _height(height), _pinNum(pinCount) {}
    ~Gate() {}

    /*get functions*/
    pair<double, double> getOutline() { return {_width, _height}; }
    string getLibCellName() { return _libCellName; }
    vector<Pin*> getPinList() { return _pinList; }
    vector<Pin*> getInputPinList() { return _inputpinlist; }
    vector<Pin*> getOutputPinList() { return _outputpinlist; }
    size_t getPinId(string& pinName) { return _pinName2Id[pinName]; }
    size_t getPinNum() { return _pinNum; }
    double getWidth() { return _width; }
    double getHeight() { return _height; }

    /*set functions*/
    void addPin(Pin* pin) { _pinList.emplace_back(pin); _pinName2Id[pin->getPinName()] = _pinList.size()-1; }
    void addInputpin(Pin* pin) { _inputpinlist.emplace_back(pin); }
    void addOutputpin(Pin* pin) { _outputpinlist.emplace_back(pin); }

private:
    string                           _libCellName;           // library cell name of the gate
    double                           _width;                 // width of the gate
    double                           _height;                // height of the gate
    size_t                           _pinNum;                // number of pins of the gate
    vector<Pin*>                     _pinList;               // pins of the gate
    vector<Pin*>                     _inputpinlist;
    vector<Pin*>                     _outputpinlist;
    unordered_map<string, size_t>    _pinName2Id;  // mapping from pin name to id
};

struct Point {
    double x, y;
};

struct Line {
    Point start, end;
};

class FeasibleSquare
{
public:
    FeasibleSquare(Point p1, Point p2, Point p3, Point p4):
        _p1(p1),_p2(p2), _p3(p3), _p4(p4) { }
    ~FeasibleSquare() { }

    /*get functions*/
    vector<Point> getDimondOutline() { return {_p1, _p2, _p3, _p4}; } // bottom, right, top, left
    vector<Point> getSquareOutline() { return {_r1, _r2, _r3, _r4}; }

    /*set functions*/
    void setOutline(Point p1, Point p2, Point p3, Point p4) { 
        _p1=p1; _p2=p2; _p3=p3; _p4=p4;
        _r1 = {_p1.x * cos(rad) - _p1.y * sin(rad), _p1.x * sin(rad) + _p1.y * cos(rad)};
        _r2 = {_p2.x * cos(rad) - _p2.y * sin(rad), _p2.x * sin(rad) + _p2.y * cos(rad)};
        _r3 = {_p3.x * cos(rad) - _p3.y * sin(rad), _p3.x * sin(rad) + _p3.y * cos(rad)};
        _r4 = {_p4.x * cos(rad) - _p4.y * sin(rad), _p4.x * sin(rad) + _p4.y * cos(rad)};
    }
    void setSquareOutline() {
        _r1 = {_p1.x * cos(rad) - _p1.y * sin(rad), _p1.x * sin(rad) + _p1.y * cos(rad)};
        _r2 = {_p2.x * cos(rad) - _p2.y * sin(rad), _p2.x * sin(rad) + _p2.y * cos(rad)};
        _r3 = {_p3.x * cos(rad) - _p3.y * sin(rad), _p3.x * sin(rad) + _p3.y * cos(rad)};
        _r4 = {_p4.x * cos(rad) - _p4.y * sin(rad), _p4.x * sin(rad) + _p4.y * cos(rad)};        
    }
    
private:
    Point _p1, _p2, _p3, _p4;
    Point _r1, _r2, _r3, _r4;
    double rad = (-45) * M_PI / 180.0;
};

class Net //include pin
{
public:
    Net(string& name, size_t pinNum):
        _name(name), _pinNum(pinNum) {}
    ~Net() {}

    /*get functions*/
    string getNetName() { return _name; }
    size_t getPinNum() { return _pinNum; }
    vector<Pin*>& getPinList() { return _pinList; }
    bool getType(){ return _type; }

    /*set functions*/
    void setType(const bool type){ _type=type; }
    void addPin(Pin* pin) { _pinList.emplace_back(pin); _pinName2Id[pin->getPinName()] = _pinList.size()-1; }
    void addPinNum() {_pinNum = _pinNum+1;}
    void resetMapping(Pin* pin, string name, size_t idx) { _pinName2Id.erase(pin->getPinName()); _pinName2Id[name] = idx; }

private:
    string                          _name;                  // net name
    size_t                          _pinNum;                // number of pins of the net
    vector<Pin*>                    _pinList;               // pins of the net
    unordered_map<string, size_t>   _pinName2Id;            // mapping from pin name to id
    bool                            _type;                  // 0 for clock signal net, 1 for I/O signal net
};

class Edge
{
public:
    Edge() {}
    ~Edge() {}

    /* get function */
    pair<Pin*, Pin*> getStart() { return _start; }
    pair<Pin*, Pin*> getEnd() { return _end; }

    /* set function */
    void setStart(Pin* a, Pin* b) { _start.first = a; _start.second = b; }
    void setEnd(Pin* a, Pin* b) { _end.first = a; _end.second = b; }

private:
    pair<Pin*, Pin*> _start;
    pair<Pin*, Pin*> _end;
};

class Instance
{
public:
    Instance(string& instName, string& libCellName, double x, double y):
        _instName(instName), _libCellName(libCellName), _x1(x), _y1(y), _lock(0) {}
    ~Instance() {}

    /*get functions*/
    int getInitOnSite() { return _initOnSite; }
    string getInstanceName() { return _instName; }
    string getLibCellName() { return _libCellName; }
    size_t getInstType() { return _instType; } // 0: flip-flop, 1: gate , 2: primary input
    double getX() { return _x1; } // min x coordinate of the instance
    double getY() { return _y1; } // min y coordinate of the instance  
    vector<Pin*> getPinList() { return _pinList; }
    vector<Pin*> getInPinList() { return _inPinList; }
    vector<Pin*> getOutPinList() { return _outPinList; }
    vector<Pin*> getClkPinList() { return _clkPinList; }
    vector<Net*> getNetList() { return _netList; }
    unordered_map<Pin*, double> getTimingSlack() { return _timingSlack; }
    unordered_set<Instance*> getNext(){ return _nextIns; }
    unordered_set<Instance*> getPrev(){ return _prevIns; }
    unordered_set<Instance*> getNextFF(){ return _nextFF; }
    unordered_set<Instance*> getPrevFF(){ return _prevFF; }
    Pin* getPin(string pinName) { 
        for(auto pin : _pinList) {
            if(pin->getPinName() == pinName) return pin;
        }
        return nullptr;
    }
    map<double, set<Pin*>> getPriorityPin() { return _priorityPin; }
    Edge* getNeighborFFMapping(Instance* ins) { return _neighborFFmapping[ins];}
    string getClk() { return _clk; }
    FeasibleSquare* getFeasible(string Dpin) { return _mapDpin2FeasibleSquare[Dpin]; }
    FeasibleSquare* getFeasible(){ return _feasibleSquare; }
    bool getlock() { return _lock; }
    vector<Site*> getSite() { return _siteList; }
    double getFeasibleRegionwidth() { return _feasibleRegionWidth; }
    double getFeasibleRegionheight() { return _feasibleRegionHeight; }


    /*set functions*/
    void setInitOnSite(int onSite) { _initOnSite = onSite; }
    void setX(double x) { _x1 = x; } // min x coordinate of the instance
    void setY(double y) { _y1 = y; } // min y coordinate of the instance
    void setInstType(size_t type) { _instType = type; }
    void addPin(Pin* pin) { _pinList.emplace_back(pin); }
    void addInPin(Pin* pin) { _inPinList.emplace_back(pin); }
    void addOutPin(Pin* pin) { _outPinList.emplace_back(pin); }
    void addClkPin(Pin* pin) { _clkPinList.emplace_back(pin); }
    void setTimingSlack(Pin* setpin, double slack) { _timingSlack[setpin] = slack; }
    void addNext(Instance* next) { _nextIns.insert(next); }
    void addPrev(Instance* prev) { _prevIns.insert(prev); }
    void addNextFF(Instance* next) { _nextFF.insert(next); }
    void addPrevFF(Instance* prev) { _prevFF.insert(prev); }
    void addPriorityPin(double priority,Pin* pin) { _priorityPin[priority].insert(pin);}
    int deletePriorityPin(double priority, Pin* pin) {
        bool found = false; 
        auto it = _priorityPin.find(priority);
        if (it != _priorityPin.end()) {
            it->second.erase(pin);
            if (it->second.empty()) {
                _priorityPin.erase(it);
            }
            return 1;
        } else {
            for (auto it = _priorityPin.begin(); it != _priorityPin.end(); ++it) {
                if (found) break;
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ) {
                    if (*it2 == pin) {
                        found = true;
                        it2 = it->second.erase(it2); // erase returns the next iterator
                        if (it->second.empty()) {
                            _priorityPin.erase(it);
                        }
                        return 2;
                    } else {
                        ++it2;
                    }
                }
            }
        }
        return 0; // Return 0 if the pin was not found
    }
    void neighborFFMapping(Instance* ins, Edge* e) {_neighborFFmapping[ins] = e;}
    void removeNext(Instance* next) { _nextIns.erase(next); }
    void removePrev(Instance* prev) { _prevIns.erase(prev); }
    void removeNextFF(Instance* next) { _nextFF.erase(next); }
    void removePrevFF(Instance* prev) { _prevFF.erase(prev); }
    void setClk(string clk) { _clk = clk; }
    void addSite(Site* site) { _siteList.emplace_back(site); }
    void setMovable(const pair<double, double> move) { _movableCenter = move; }
    void setPriorityPin(map<double, set<Pin*>> priority) { _priorityPin = priority; }
    void setPinFeasible(string pin, FeasibleSquare* fs) { _mapDpin2FeasibleSquare[pin] = fs; }
    void initFeasibleSquare(FeasibleSquare* fs) { _feasibleSquare=fs; }
    void setFeasibleSquare(FeasibleSquare* fs) { _feasibleSquare->setOutline(fs->getDimondOutline()[0], fs->getDimondOutline()[1], fs->getDimondOutline()[2], fs->getDimondOutline()[3]); }
    void setFFfeasible(vector<vector<Point>>& rects);
    void setlock(bool lock) { _lock = lock; }
    void setFeasibleRegionwidth(double width) { _feasibleRegionWidth = width;}
    void setFeasibleRegionheight(double height) { _feasibleRegionHeight = height;}
    void addNet(Net* net) { _netList.emplace_back(net); }
    
    /*operation*/
    bool intersect(const Line &line1, const Line &line2, Point &intersection);
    double pointToSegmentDistance(const Point &p, const Point &v, const Point &w);
    bool isInside(const Point &p, const std::vector<Point> &polygon);
    void sortPins() {
        sort(_pinList.begin(), _pinList.end(), [](Pin* a, Pin* b) { 
            if(a->getPinName() == "CLK") return false;
            if(b->getPinName() == "CLK") return true;
            return a->getPinName() < b->getPinName(); 
            });
    }

private:
    string              _instName;              // instance name
    string              _libCellName;           // library cell name of the instance
    string              _clk;                   // clk info
    double              _x1;                    // min x coordinate of the instance
    double              _y1;                    // min y coordinate of the instance
    size_t              _instType;              // type of the instance (0: flip-flop, 1: gate, 2: pi, 3:po)
    vector<Pin*>                  _pinList;               // pin list
    vector<Pin*>                  _inPinList;
    vector<Pin*>                  _outPinList;
    vector<Pin*>                  _clkPinList;
    vector<Net*>                  _netList;               // net list
    unordered_map<Pin*, double> _timingSlack;           // timing slack of the flip-flop D pins <pinName, slack>
    unordered_set<Instance*>      _nextIns;               // next Instance
    unordered_set<Instance*>      _prevIns;               // previous Instance
    unordered_set<Instance*>      _nextFF;                // next FF
    unordered_set<Instance*>      _prevFF;                // previous FF
    unordered_map<Instance*, Edge*> _neighborFFmapping;
    map<double, set<Pin*> >        _priorityPin;           // priority queue for pin
    // unordered_set<Instance*>      _candidates;            // Flip Flop merging candidates
    pair<double, double>_movableCenter;         // the movable area center
    FeasibleSquare*      _feasibleSquare;       // the movable area
    unordered_map<string, FeasibleSquare*> _mapDpin2FeasibleSquare;
    bool                 _lock;                 // lock the instance 0: unlock, 1: lock
    vector<Site*>        _siteList;                 // site of the instance

    double               _feasibleRegionWidth;   // feasible region width
    double               _feasibleRegionHeight;  // feasible region height 
    int                 _initOnSite;            // 0: not on site, 1: on site
};

class PrimaryInput : public Instance
{
public:
    PrimaryInput(string& instName, string&libCellName, double x, double y): Instance(instName, libCellName, x, y) { }
    ~PrimaryInput() { }

    /*get functions*/
    Pin* getPin() { return _pin;}

    /*set functions*/
    void setPin(Pin* pin) { _pin = pin; }

private:
    Pin* _pin;
};

class PrimaryOutput : public Instance
{
public:
    PrimaryOutput(string& instName, string&libCellName, double x, double y): Instance(instName, libCellName, x, y) { }
    ~PrimaryOutput() { }

    /*get functions*/
    Pin* getPin() { return _pin;}

    /*set functions*/
    void setPin(Pin* pin) { _pin = pin; }

private:
    Pin* _pin;
};

class Bin
{
public:
    Bin(double x, double y):
        _x(x), _y(y), _util(0) {}
    ~Bin() {}

    /*get functions*/
    double getX() { return _x; }
    double getY() { return _y; }
    double getUtil() { return _util; }

    /*set functions*/
    void setUtil(const double util) {_util = util;}

private:
    double              _x;                     // x coordinate of the Bin
    double              _y;                     // y coordinate of the Bin
    double              _util;                  // utility of the Bin
};

class Site
{
public:
    Site(double x, double y, double width, double height):
        _x(x), _y(y), _occ(NULL), _width(width), _height(height) {}
    ~Site() {}

    /*get functions*/
    double getX() { return _x; }
    double getY() { return _y; }
    Instance* getOcc() { return _occ;}
    double getWidth() { return _width; }
    double getHeight() { return _height; }
    
    /*set functions*/
    void setOcc(Instance* ins) { _occ=ins;}

private:
    double              _x;                     // x coordinate of the site
    double              _y;                     // y coordinate of the site
    Instance*           _occ;                   // 0: not occupied, 1:gate, 2:FF
    double              _width;                 // width of the site
    double              _height;                // height of the site
};

class PlacementRow
{
public:
    PlacementRow(double x, double y, size_t siteNum, double siteWidth, double siteHeight):
        _x(x), _y(y), _siteW(siteWidth), _siteH(siteHeight)
    {
        for(size_t i = 0 ; i < siteNum ; i++)
        {
            Site* site = new Site(x + i*siteWidth, y, siteWidth, siteHeight);
            _siteArray.emplace_back(site);
        }
    }
    ~PlacementRow() {}

    /*get functions*/
    double getX() { return _x; }
    double getY() { return _y; }
    double getsiteW() { return _siteW; }
    double getsiteH() { return _siteH; }
    vector<Site*> getSiteArray() { return _siteArray; }

    /*set functions*/

private:
    double              _x;                     // x coordinate of the row start
    double              _y;                     // y coordinate of the row start
    double              _siteW;
    double              _siteH;
    vector<Site*>       _siteArray;             // site array of the row
};

struct Die
{
    double                              _x1;                    // min x coordinate of the die
    double                              _y1;                    // min y coordinate of the die
    double                              _x2;                    // max x coordinate of the die
    double                              _y2;                    // max y coordinate of the die
    size_t                              _inputNum;              // number of input pins of the die
    size_t                              _outputNum;             // number of output pins of the die
    vector<Pin*>                        _inputPinArray;         // input pin array of the die
    vector<Pin*>                        _outputPinArray;        // output pin array of the die
    unordered_map<string, size_t>       _inputPinName2Id;       // input pin name to id
    unordered_map<string, size_t>       _outputPinName2Id;      // output pin name to id
};


#endif // MODULE_H