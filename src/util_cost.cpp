#include "algo.h"

using namespace std;

// calculate FF library cost/performance
double Algo::CalculateCost(FlipFlop* FF){
    return _alpha * FF->getQpinDelay() +_beta * FF->getGatePower() + _gamma * FF->getHeight() * FF->getWidth();
}

// list cost/performance FF in order
void Algo::lowestCostFF(){
    map<FlipFlop*, double> costMap;
    for(const auto& ff : _ffArray){
        double cost = CalculateCost(ff);
        costMap[ff] = cost;
    }

    for(const auto& ffCost : costMap){
        if(_lowestCostFFMap.count(ffCost.first->getBits()) == 0){ // bit值不存在map
            _lowestCostFFMap[ffCost.first->getBits()] = ffCost.first;
        }
        else{
            if(costMap[ffCost.first] < costMap[_lowestCostFFMap[ffCost.first->getBits()]]){
                _lowestCostFFMap[ffCost.first->getBits()] = ffCost.first;
            }
        }
    }
    int prevSize = 0;
    for(const auto& lowestCostFFMap : _lowestCostFFMap){
        _previousSize[lowestCostFFMap.first] = prevSize;
        prevSize = lowestCostFFMap.first;
    }

    // print
    // for(const auto& ff : _lowestCostFFMap){
    //     cout << "FF bits: " << ff.first << " FF name: " << ff.second->getLibCellName() << endl;
    // }
}

// calculate the cost before merging
double Algo::costBeforeMerge (set<Instance*> flipflops, FlipFlop* FF, Site* curSite, double cost){
// cost before merge
    double oriTNS = 0;
    double oriArea = 0;
    double oriPower = 0;
    ///////////////////////////////////////////////////////////////或許可以移出來
    set<Instance*> nextIns;
    for(const auto& ff : flipflops){
        for(const auto& s : ff->getTimingSlack()){
            if(s.second < 0)
                oriTNS -= s.second;
        }
        oriPower += _ffArray[_ffName2Id[ff->getLibCellName()]]->getGatePower();
        oriArea += _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth() * _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
        for(const auto& nb : ff->getNextFF()){
            nextIns.insert(nb);
        }
    }
    for(const auto& nb : nextIns){
        for(const auto& s : nb->getTimingSlack()){
            if(s.second < 0)
                oriTNS -= s.second;
        }
    }
    ///////////////////////////////////////////////////////////////
    double oriCost = _alpha * oriTNS + _beta * oriPower + _gamma * oriArea + _lambda * cost; // TODO: binUtil
    return oriCost;
}

// calculate the placing cost of FF for each site in FF placeable region
double Algo::calAvailableSiteCost(set<Instance*> flipflops, FlipFlop* FF, Site* curSite, double cost, double oriCost){ // SHUYUN: add delta power and area
    double totalTNS = 0;
    int i = 0;
    for(const auto& ff : flipflops){
        // int x1 = ff->getX(), y1 = ff->getY();
        // int x2 = x1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth();
        // int y2 = y1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
        // subBinUsage(x1, x2, y1, y2);
        for(const auto& p : ff->getPinList()){
            /*if(ff->getInstanceName()=="C100566"||ff->getInstanceName()=="C100567"||ff->getInstanceName()=="C100587"||ff->getInstanceName()=="C100588"){
                 cout<<ff->getTimingSlack()[p->getPinName()]<<endl;
            }*/
            if(p->getPinType() != 0)
                continue;
            double oriPinX = p->getX() + ff->getX();
            double oriPinY = p->getY() + ff->getY();
            double prePinX = 0;
            double prePinY = 0;
            for(const auto& netPin : _netArray[_netName2Id[p->getNetName()]]->getPinList()){
                if(netPin->getPinType() == 1){
                    prePinX = netPin->getX() + _instanceArray[_instanceName2Id[netPin->getInstName()]]->getX();
                    prePinY = netPin->getY() + _instanceArray[_instanceName2Id[netPin->getInstName()]]->getY();
                    break;
                }
            }
            double oriDWL = abs(oriPinX - prePinX) + abs(oriPinY - prePinY);
            // if(ff->getInstanceName()=="C100567"){
            //     cout << "oriPinX: " << oriPinX << " oriPinY: " << oriPinY << endl;
            //     cout << "prePinX: " << prePinX << " prePinY: " << prePinY << endl;
            //     cout << "oriDWL: " << oriDWL << endl;
            // }
            double mergePinX = curSite->getX() + FF->getPinList()[i]->getX(); // TODO: 改成intputPin
            double mergePinY = curSite->getY() + FF->getPinList()[i]->getY(); // TODO: 改成intputPin
            
            double mergeDWL = abs(mergePinX - prePinX) + abs(mergePinY - prePinY);
            Pin* setpin=ff->getPin(p->getPinName());
            double mergeSlack = ff->getTimingSlack()[setpin] + _displaymentDelay * (oriDWL - mergeDWL);
            // if(ff->getInstanceName()=="C100566"||ff->getInstanceName()=="C100567"||ff->getInstanceName()=="C100587"||ff->getInstanceName()=="C100588"){
            //     if(mergeSlack < 0){
            //         cout<<curSite->getX()<<" "<<curSite->getY()<<endl;    
            //         cout<<ff->getInstanceName()<<endl;
            //         cout<<"oriDWL:"<<oriDWL<<" mergeDWL:"<<mergeDWL<<endl;
            //     }
            // }
            if(mergeSlack < 0)
                totalTNS -= mergeSlack;

            // SHUYUN: slack of next ins
            for(const auto& nb : ff->getNextFF()){
                // exclude multi-bit flip-flop
                if(_ffArray[_ffName2Id[nb->getLibCellName()]]->getBits() != 1)
                    continue;

                Edge* edge = ff->getNeighborFFMapping(nb);
                double oriQWL = abs(edge->getStart().first->getX() + ff->getX() - edge->getStart().second->getX() - _instanceArray[_instanceName2Id[edge->getStart().second->getInstName()]]->getX()) +
                                abs(edge->getStart().first->getY() + ff->getY() - edge->getStart().second->getY() - _instanceArray[_instanceName2Id[edge->getStart().second->getInstName()]]->getY());
                double newQWL = abs(FF->getQPinList()[i]->getX() + curSite->getX() - edge->getStart().second->getX() - _instanceArray[_instanceName2Id[edge->getStart().second->getInstName()]]->getX()) +
                                abs(FF->getQPinList()[i]->getY() + curSite->getY() - edge->getStart().second->getY() - _instanceArray[_instanceName2Id[edge->getStart().second->getInstName()]]->getY());               
                double oriSlack = nb->getTimingSlack()[edge->getEnd().second];
                double newSlack = nb->getTimingSlack()[edge->getEnd().second] + _ffArray[_ffName2Id[ff->getLibCellName()]]->getQpinDelay() - FF->getQpinDelay() + _displaymentDelay * (oriQWL - newQWL);
                newSlack = min(newSlack, oriSlack);
                if(newSlack < 0)
                    totalTNS -= newSlack;
                
            }
        }
        i++;
    }
    int x1 = curSite->getX(), y1 = curSite->getY();
    int x2 = x1 + FF->getWidth();
    int y2 = y1 + FF->getHeight();
    double mergerCost = _alpha * totalTNS + _beta * FF->getGatePower() + _gamma * FF->getWidth() * FF->getHeight() + _lambda * addBinUsage(x1,x2,y1,y2)  ; // TODO: binUtil
    // if(exceed > 0)
    //     cout<<mergerCost<<endl;
    subBinUsage(x1, x2, y1, y2);
    // for(const auto& ff : flipflops){
    //     int x1 = ff->getX(), y1 = ff->getY();
    //     int x2 = x1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth();
    //     int y2 = y1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
    //     addBinUsage(x1, x2, y1, y2);
    // }
    /*if(flipflops.find(_instanceArray[_instanceName2Id["C100566"]]) != flipflops.end()){
        cout<<"TNS:"<<totalTNS<<" Power:"<<FF->getGatePower()<<" Area:"<<FF->getWidth() * FF->getHeight()<<endl;
        cout<<"alpha:"<<_alpha<<" beta:"<<_beta<<" gamma:"<<_gamma<<endl;
        cout<<mergerCost<<endl;
    }*/
    // // cost before merge
    // double oriTNS = 0;
    // double oriArea = 0;
    // double oriPower = 0;
    // ///////////////////////////////////////////////////////////////或許可以移出來
    // set<Instance*> nextIns;
    // for(const auto& ff : flipflops){
    //     for(const auto& s : ff->getTimingSlack()){
    //         if(s.second < 0)
    //             oriTNS -= s.second;
    //     }
    //     oriPower += _ffArray[_ffName2Id[ff->getLibCellName()]]->getGatePower();
    //     oriArea += _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth() * _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
    //     for(const auto& nb : ff->getNextFF()){
    //         nextIns.insert(nb);
    //     }
    // }
    // for(const auto& nb : nextIns){
    //     for(const auto& s : nb->getTimingSlack()){
    //         if(s.second < 0)
    //             oriTNS -= s.second;
    //     }
    // }
    // ///////////////////////////////////////////////////////////////
    // double oriCost = _alpha * oriTNS + _beta * oriPower + _gamma * oriArea + _lambda * cost; // TODO: binUtil

    if(mergerCost < oriCost){
        // if(flipflops.find(_instanceArray[_instanceName2Id["C100566"]]) != flipflops.end()){
            //cout<<"merge"<<mergerCost<<endl;
        // }
        return mergerCost;
    }
    else{
        return -1;
    }
}
