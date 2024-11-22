#include "algo.h"

using namespace std;

// init Site is_occupied or not
void Algo::InitSite(){
    size_t x1 = _placementRowArray[0]->getX();
    size_t y1 = _placementRowArray[0]->getY();
    size_t x2 = _placementRowArray[_placementRowArray.size()-1]->getX()+(_placementRowArray[_placementRowArray.size()-1]->getsiteW()*_placementRowArray[_placementRowArray.size()-1]->getSiteArray().size());
    size_t y2 = _placementRowArray[_placementRowArray.size()-1]->getY();

    for(const auto& ins: _instanceArray){
        //check on site
        if(ins->getInstType() == 0){
            if(ins->getX()<x1 || ins->getX()>x2+_siteWidth || ins->getY()<y1 || ins->getY()>y2+_siteHeight){
                ins->setInitOnSite(0);
            }else{
                ins->setInitOnSite(1);
            }
        }

        pair<double, double> outline;
        if(ins->getInstType()==1){
            outline = _gateArray[_gateName2Id[ins->getLibCellName()]]->getOutline();
        }else{
            outline = _ffArray[_ffName2Id[ins->getLibCellName()]]->getOutline();
        }
        const double ins_x1 = ins->getX(), ins_x2 = ins->getX()+outline.first,
                        ins_y1 = ins->getY(), ins_y2 = ins->getY()+outline.second;

        int rowCount = 0;
        for(const auto& placementrow:_placementRowArray){
            if(placementrow->getY() < ins_y1 || placementrow->getY() >= ins_y2){
                continue;
            }
            int x_start = (ins_x1-placementrow->getX())/placementrow->getsiteW(),  x_end = ceil((ins_x2-placementrow->getX())/placementrow->getsiteW()),
                y_start = (ins_y1-placementrow->getY())/placementrow->getsiteH();

            //check on site
            rowCount++;
            if(ins->getInstType()==0 && rowCount==1){
                if(placementrow->getSiteArray()[x_start]->getX()==ins->getX() && placementrow->getSiteArray()[x_start]->getY()==ins->getY()){
                    ins->setInitOnSite(1);
                }else{
                    ins->setInitOnSite(0);
                }
            }

            for(int x=x_start; x<x_end; ++x){
                if(x >= 0 && x < placementrow->getSiteArray().size()){
                    Site* const site = placementrow->getSiteArray()[x];
                    site->setOcc(ins);
                    ins->addSite(site);
                }            
            }                              
        }
    }
}

// place all the instances in site to legalize the initial network
void Algo::legalInit() {
    for(size_t idx=0 ; idx< _instanceArray.size(); ++idx)
    {
        if(_instanceArray[idx]->getInstType()==0 && _instanceArray[idx]->getInitOnSite() == 0)
        {
            // search for legal site 
            // closest abs(deltaY)
            double delta_y = 0;
            PlacementRow* closestRow = _placementRowArray[0];
            for(size_t i=0; i<_placementRowArray.size(); ++i)
            {
                if(i==0){
                    delta_y = abs(_placementRowArray[i]->getY() - _instanceArray[idx]->getY());
                }else{
                    if(abs(_placementRowArray[i]->getY() - _instanceArray[idx]->getY()) < delta_y){
                        delta_y = abs(_placementRowArray[i]->getY() - _instanceArray[idx]->getY());
                        closestRow = _placementRowArray[i];
                    }
                }
            }

            // abs(deltaX)
            size_t index = abs(_instanceArray[idx]->getX()-closestRow->getX())/closestRow->getsiteW();
            size_t site_usage = _ffArray[_ffName2Id[_instanceArray[idx]->getInstanceName()]]->getWidth()/closestRow->getsiteW();
            if(_instanceArray[idx]->getX()<closestRow->getX())
            {
                index = 0;
            }
            if(_instanceArray[idx]->getX()>closestRow->getX()+closestRow->getsiteW()*closestRow->getSiteArray().size())
            {
                index = closestRow->getSiteArray().size()-site_usage-1;
            }
            
            for(size_t i=0; i+index < closestRow->getSiteArray().size() || i-index ==0 ; ++i)
            {
                // check if the site is occupied => continue
                // else => set the instance to the site
                bool r_flag = false, l_flag = false; // false: not occupied, true: occupied
                for(size_t j = 0 ; j < site_usage ; ++j)
                {
                    // cout << "check right" << endl;
                    if(index+i+j>closestRow->getSiteArray().size()-1 && closestRow->getSiteArray()[index+i+j]->getOcc() != NULL)
                    {
                        r_flag = true;
                    }
                    // cout << "check left" << endl;
                    if(index+i-j<0 && closestRow->getSiteArray()[index+i-j]->getOcc() != NULL)
                    {
                        l_flag = true;
                    }
                }
                if(r_flag == 0)
                {
                    // set on the right site
                    _instanceArray[idx]->setX(closestRow->getSiteArray()[index+i]->getX());
                    _instanceArray[idx]->setY(closestRow->getSiteArray()[index+i]->getY());
                    for(size_t k=0; k<site_usage; ++k){
                        _instanceArray[idx]->addSite(closestRow->getSiteArray()[index+i+k]);
                        closestRow->getSiteArray()[index+i+k]->setOcc(_instanceArray[idx]);
                    }
                    break;
                }
                if(l_flag == 0)
                {
                    // set on the left site
                    _instanceArray[idx]->setX(closestRow->getSiteArray()[index-i]->getX());
                    _instanceArray[idx]->setY(closestRow->getSiteArray()[index-i]->getY());
                    for(size_t k=0; k<site_usage; ++k){
                        _instanceArray[idx]->addSite(closestRow->getSiteArray()[index-i+k]);
                        closestRow->getSiteArray()[index-i+k]->setOcc(_instanceArray[idx]);
                    }
                    break;
                }
                i+=site_usage;
            }
        }
    }
}

// function for building FF graph using DP
void Algo::InitNeighbors(Instance* const starter, Instance* const passby, pair<Pin*, Pin*> &pin_pair, vector<Instance*> &visited_gate){
    if(passby->getInstanceName() != ""){ // Gate or FF
        if(passby->getInstType()==0){ // FF
            if(starter->getInstanceName() != passby->getInstanceName()){
                if(passby->getlock()==0){ // single-bit FF
                    Edge* e = new Edge();
                    e->setStart(pin_pair.first, pin_pair.second);
                    e->setEnd(pin_pair.first, pin_pair.second);
                    starter->addNextFF(passby);
                    passby->addPrevFF(starter);
                    starter->neighborFFMapping(passby, e);
                    passby->neighborFFMapping(starter, e);
                }
            }       
            return;
        }else{ // Gate
            if(passby->getNextFF().size()!=0){ // gate has been visited before, copy the terminal FF info to starter FF and visited gates
                for(const auto& terminal_FF: passby->getNextFF()){
                    Edge* e = new Edge();
                    e->setStart(pin_pair.first, pin_pair.second);
                    e->setEnd(passby->getNeighborFFMapping(terminal_FF)->getEnd().first, passby->getNeighborFFMapping(terminal_FF)->getEnd().second);
                    // copy the terminal FF info to starter FF
                    if(starter->getInstanceName()!=terminal_FF->getInstanceName()){
                        starter->addNextFF(terminal_FF);
                        terminal_FF->addPrevFF(starter);
                        starter->neighborFFMapping(terminal_FF, e);
                        terminal_FF->neighborFFMapping(starter, e);
                    }
                    // copy terminal FF info to visited gates
                    for(const auto& ins_gate: visited_gate){
                        ins_gate->addNextFF(terminal_FF);
                        ins_gate->neighborFFMapping(terminal_FF, e);
                    }
                }
            }else{ // gate hasnt been visited yet, need to record terminal FF info
                visited_gate.emplace_back(passby);
                for(const auto& out : passby->getOutPinList()){
                    Net* n = _netArray[_netName2Id[out->getNetName()]];
                    for(const auto& p : n->getPinList()){
                        if(p->getPinName() != out->getPinName()){
                            if(p->getInstName()!=""){ // Gate or FF
                                if(_instanceArray[_instanceName2Id[p->getInstName()]]->getInstType()==0){ // FF
                                    if(_instanceArray[_instanceName2Id[p->getInstName()]]->getlock()==0){ // single-bit FF
                                        Edge* e = new Edge();
                                        e->setStart(pin_pair.first, pin_pair.second);
                                        e->setEnd(out, p);
                                        if(starter->getInstanceName()!=p->getInstName()){
                                            starter->addNextFF(_instanceArray[_instanceName2Id[p->getInstName()]]);
                                            _instanceArray[_instanceName2Id[p->getInstName()]]->addPrevFF(starter);
                                            starter->neighborFFMapping(_instanceArray[_instanceName2Id[p->getInstName()]], e);
                                            _instanceArray[_instanceName2Id[p->getInstName()]]->neighborFFMapping(starter, e);
                                        }
                                        for(const auto& ins_gate: visited_gate){
                                            ins_gate->addNextFF(_instanceArray[_instanceName2Id[p->getInstName()]]);
                                            ins_gate->neighborFFMapping(_instanceArray[_instanceName2Id[p->getInstName()]], e);
                                        }
                                    }
                                }else{ // Gate
                                    InitNeighbors(starter, _instanceArray[_instanceName2Id[p->getInstName()]], pin_pair, visited_gate);
                                }
                            }
                        }
                    }
                }
                visited_gate.pop_back();
            }
        }
    }
    return;
}

// function for building FF graph using DP
void Algo::buildFFGraph(){
    for(const auto& ins : _instanceArray){
        if(ins->getInstType()==0){
            if(ins->getlock()==0){ // visit only single-bit FF
                for(const auto& out : ins->getOutPinList()){
                    Net* n = _netArray[_netName2Id[out->getNetName()]];
                    for(const auto& p : n->getPinList()){
                        if(p->getPinName() != out->getPinName()){
                            if(p->getInstName()!=""){
                                pair<Pin*, Pin*> pin_pair = {out, p};
                                vector<Instance*> visited_gate = {};
                                InitNeighbors(ins, _instanceArray[_instanceName2Id[p->getInstName()]], pin_pair, visited_gate);
                            }
                        }
                    }
                }
            }            
        }
    }
}
