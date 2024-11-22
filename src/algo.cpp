#include <limits>
#include <algorithm>
#include <stack>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include "algo.h"

using namespace std;

// divide Die into windows for divide and conquer
map<string, set<Instance*>> Algo::Window(int &placementRowIdy, int &siteIdx, int placementRow_y , int site_x){
    set<Instance*> flipflops;
    map<string, set<Instance*>> clk2FF;
    for(int i = placementRowIdy;i<placementRowIdy+placementRow_y&&i<_placementRowArray.size();i++){
        for(int j = siteIdx ; j < siteIdx+site_x&&j<_placementRowArray[0]->getSiteArray().size();j++){
            if(_placementRowArray[i]->getSiteArray()[j]->getOcc()!=NULL && _placementRowArray[i]->getSiteArray()[j]->getOcc()->getInstType()==0){
                if(_placementRowArray[i]->getSiteArray()[j]->getOcc()&&_placementRowArray[i]->getSiteArray()[j]->getOcc()->getlock()==0){
                    flipflops.insert(_placementRowArray[i]->getSiteArray()[j]->getOcc());
                    _placementRowArray[i]->getSiteArray()[j]->getOcc()->setlock(true);
                }
            }

        }
    }

    for(const auto& ff: flipflops){
        clk2FF[ff->getClk()].insert(ff);
    }

    return clk2FF;
}

// send set<FF*>{select_candidates} to selectBanking algo
void Algo::Banking(){
    lowestCostFF();
    int placementRowIdy = 0, siteIdx = 0;
    //int total_WindowsX = _placementRowArray[0]->getSiteArray().size()/100+1, total_WindowsY = _placementRowArray.size()/100+1;
    int total_WindowsX = 1, total_WindowsY = 1;
    // if(_instanceArray.size()>300000){
        total_WindowsX = _placementRowArray[0]->getSiteArray().size()/100+1; total_WindowsY = _placementRowArray.size()/100+1;
    // }
    //cout<<"total_WindowsX: "<<total_WindowsX<<" total_WindowsY: "<<total_WindowsY<<endl;
    //cout<<_ffArray.size()<<endl;
    int placementRow_y = _placementRowArray.size()/total_WindowsY, site_x = _placementRowArray[0]->getSiteArray().size()/total_WindowsX;
    //cout<<"placementRow_y: "<<placementRow_y<<" site_x: "<<site_x<<endl;
    //step1: set instance's clk
    //cout<<"clkNetArray size: "<<_clkNetArray.size()<<endl;
    for(const auto& clk_net : _clkNetArray){
        unordered_set<Instance*> candidates;
        for(const auto& pin : clk_net->getPinList() ){
            if(pin->getInstName()!=""){
                Instance* ins = _instanceArray[_instanceName2Id[pin->getInstName()]];
                ins->setClk(clk_net->getNetName());
                if(ins->getInstType() == 0 && ins->getlock()==0){ // 0: flip-flop
                    candidates.insert(ins);
                }
            }
        }
        _sameclk[clk_net->getNetName()] = candidates;
    }
    /*cout<<"placementRowIdy: "<<placementRowIdy<<" siteIdx: "<<siteIdx<<endl;
    cout<<"placementRow_y: "<<placementRow_y<<" siteId_x: "<<site_x<<endl;*/
    placementRowIdy = placementRowIdy*placementRow_y;
    siteIdx = siteIdx*site_x;
    for(int i=0;i<=total_WindowsY;i++){
        placementRowIdy = i * placementRow_y;
        for(int j=0;j<=total_WindowsX;j++){
            map<string, set<Instance*>> clk2FF = Window(placementRowIdy,siteIdx,placementRow_y,site_x);
            for(auto clk:clk2FF){
                selectBanking(clk.second);
            }    
            siteIdx = j*site_x;
            //cout<<"placementRowIdy: "<<placementRowIdy<<" siteIdx: "<<siteIdx<<endl;
        } 
    }
    //Window(placementRowIdx,siteIdy,placementRow_x,site_y);
}

// main_algo_1, decide "weather banking or not(util_cost.cpp)" and "where to place the FFs(util_location.cpp)"
void Algo::selectBanking(set<Instance*> FF){
    while(FF.size()){
        vector<Candidate> candidates_X;
        vector<Instance*> deleteFF;

        for(const auto& ff: FF){
            FeasibleSquare* fs = ff->getFeasible();
            vector<Point> squareOutline=fs->getSquareOutline();

            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            // if(squareOutline[0].x==squareOutline[1].x&&squareOutline[1].x==squareOutline[2].x&&squareOutline[2].x==squareOutline[3].x){
            //     set<Instance*> selectedFF;
            //     selectedFF.insert(ff);
            //     deleteFF.push_back(ff);
            //     locationDecision(selectedFF, _lowestCostFFMap[selectedFF.size()]);
            //     //cout<<"FF is a point"<<endl;
            //     continue;
            // }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            // cout<<"-----------------"<<"\n";
            // for(int i=0;i<squareOutline.size();i++){
            //     cout<<"x: "<<squareOutline[i].x<<" y: "<<squareOutline[i].y<<endl;
            // }
            // cout<<"-----------------"<<"\n";
            double min_x,max_x;
            //tmp feaible region
            double centerX = ff->getX(), centerY = ff->getY();
            unordered_map<Pin*, double> firstslack = ff->getTimingSlack();
            double slack = firstslack.begin()->second;
            min_x = min(min(min(squareOutline[0].x,squareOutline[1].x),squareOutline[2].x),squareOutline[3].x);
            max_x = max(max(max(squareOutline[0].x,squareOutline[1].x),squareOutline[2].x),squareOutline[3].x);
            double enlarge=0.15;
            double enlarge_num=0;

            if(_lowestCostFFMap.find(2)!=_lowestCostFFMap.end()){
               enlarge_num=CalculateCost(_lowestCostFFMap[1])-CalculateCost(_lowestCostFFMap[2])/2;
            }
            if(enlarge*(max_x-min_x)>enlarge_num){
                enlarge_num=enlarge*(max_x-min_x);
            }

            Candidate candidatex1(min_x-enlarge_num, true, ff);
            Candidate candidatex2(max_x+enlarge_num, false, ff);
            ff->setFeasibleRegionwidth(max_x - min_x);
            candidates_X.push_back(candidatex1);
            candidates_X.push_back(candidatex2);
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // for(const auto& ff: deleteFF){
        //     FF.erase(ff);
        // }
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        if(candidates_X.size()==0){
            //cout<<"candidates_X size: "<<candidates_X.size()<<endl;
            return ;
        }
        sort(candidates_X.begin(), candidates_X.end(), [](Candidate a, Candidate b){ return a.value < b.value; });
        
        //////////////////////////////////////////////////
        int selectD=1;
        // cout<<"------------------------"<<endl;
        // cout<<"All candidates: "<<endl;
        // for(int i=0;i<candidates_X.size();i++){
        //     cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
        // }

        // find  selectD
        // cout<<"candidates_X size: "<<candidates_X.size()<<endl;
        for(int i=1;i<candidates_X.size();i++){
            if(candidates_X[i-1].type==1&&candidates_X[i].type==0){
                selectD = i;
                break;
            }
            // cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
        }
        set<Instance*> selectedXFF;//can be merged
        //cout << "selectD: " << selectD << endl;

        for(int i=0;i<selectD;i++){
            if(candidates_X[i].type==1){
                selectedXFF.insert(candidates_X[i].ff);
            }
            //cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
        }

        //cout<<"selectedYFF size: "<<selectedXFF.size()<<endl;
        vector<Candidate> candidates_Y;
        for (const auto& ff : selectedXFF) {
            double min_y,max_y;
            FeasibleSquare* fs = ff->getFeasible();
            vector<Point> squareOutline=fs->getSquareOutline();
            min_y = min(squareOutline[2].y,squareOutline[0].y);
            max_y = max(squareOutline[2].y,squareOutline[0].y);
            double enlarge=0.15;
            double enlarge_num=0;
            if(_lowestCostFFMap.find(2)!=_lowestCostFFMap.end())
               enlarge_num=CalculateCost(_lowestCostFFMap[1])-CalculateCost(_lowestCostFFMap[2])/2;
            if(enlarge*(max_y-min_y)>enlarge_num){
                enlarge_num=enlarge*(max_y-min_y);
            }
            // cout<<"enlarge_num: "<<enlarge_num<<endl;
            Candidate candidatex3(min_y-enlarge_num, true, ff);
            Candidate candidatex4(max_y+enlarge_num, false, ff);;

            ff->setFeasibleRegionheight(max_y - min_y);
            candidates_Y.push_back(candidatex3);
            candidates_Y.push_back(candidatex4);
        }
        sort(candidates_Y.begin(), candidates_Y.end(), [](Candidate a, Candidate b){ return a.value < b.value; });
        // for(int i=0;i<candidates_Y.size();i++){
        //     cout<<"value: "<<candidates_Y[i].value<<" type: "<<candidates_Y[i].type<<" FF name: "<<candidates_Y[i].ff->getInstanceName()<<endl;
        // }
        int selectDy=1;
        for(int i=1;i<candidates_Y.size();i++){
            if(candidates_Y[i-1].type==1&&candidates_Y[i].type==0){
                selectDy = i;
                break;
            }
        }
        set<Instance*> selectedFF;//can be merged
        vector<Instance*> selectedFFs;
        //cout << "selectDy: " << selectDy << endl;
        for(int i=0;i<selectDy;i++){
            if(candidates_Y[i].type==1&&selectedFF.count(candidates_Y[i].ff)==0){
                selectedFF.insert(candidates_Y[i].ff);
                selectedFFs.push_back(candidates_Y[i].ff);
            }
            //cout<<"value: "<<candidates_Y[i].value<<" type: "<<candidates_Y[i].type<<" FF name: "<<candidates_Y[i].ff->getInstanceName()<<endl;
        }
        //cout<<"selectedFF size: "<<selectedFF.size()<<endl;
        //cout<<"selectedFFs size: "<<selectedFFs.size()<<endl;
        sort(selectedFFs.begin(), selectedFFs.end(), [](Instance* a, Instance* b){ return a->getFeasibleRegionwidth()*a->getFeasibleRegionheight() > b->getFeasibleRegionwidth()*b->getFeasibleRegionheight(); });
        /*for(int i=0;i<selectedFFs.size();i++){
            cout<<"selectedFFs: "<<selectedFFs[i]->getInstanceName()<<endl;
            cout<<"area: "<<selectedFFs[i]->getFeasibleRegionwidth()*selectedFFs[i]->getFeasibleRegionheight()<<endl;
        }
        cout<<endl;*/
        // map<int,bool> FFlibary;
        // for(const auto& ff : _ffArray){
        //     int byte=ff->getBits();
        //     FFlibary[byte] = true;
        // }
        int eraseCount = 0;

        while(_lowestCostFFMap.find(selectedFF.size()) == _lowestCostFFMap.end()){ // && selectedFF.size() != 8
            //selectedFF.erase(selectedFF.begin());
            selectedFF.erase(selectedFFs[eraseCount]);
            //cout<<"erase FF: "<<selectedFFs[eraseCount]->getInstanceName()<<endl;
            eraseCount++;
            //cout<<"1"<<endl;
        }
        if(selectedFF.size() > 1){
            //cout<<"-----------------合併了---------------------"<<selectedFF.size()<<endl;
        }

        locationDecision(selectedFF, _lowestCostFFMap[selectedFF.size()]);
        for (const auto& ff : selectedFF) {
            int count = 0; // 用于跟踪每个 ff 删除的元素数量
	    // cout << ff->getInstanceName() ;
            FF.erase(ff);
        }
    }

//     vector<Candidate> candidates_X;

//     for(const auto& ff: FF){
// // if(ff->getInstanceName()=="C43456"){
// //     cout << ff->getInstanceName() << " selected" << endl;
// // }
//         FeasibleSquare* fs = ff->getFeasible();
//         vector<Point> squareOutline=fs->getSquareOutline();
//         // if(squareOutline[0].x==squareOutline[1].x&&squareOutline[1].x==squareOutline[2].x&&squareOutline[2].x==squareOutline[3].x){
//         //     set<Instance*> selectedFF;
//         //     selectedFF.insert(ff);
//         //     locationDecision(selectedFF, _lowestCostFFMap[selectedFF.size()]);
//         //     continue;
//         // }
//         /*cout<<"-----------------"<<"\n";
//         for(int i=0;i<squareOutline.size();i++){
//             cout<<"x: "<<squareOutline[i].x<<" y: "<<squareOutline[i].y<<endl;
//         }
//         cout<<"-----------------"<<"\n";*/
//         double min_x,max_x;
//         //tmp feaible region
//         double centerX = ff->getX(), centerY = ff->getY();
//         //cout<<centerX<<" "<<centerY<<endl;
//         unordered_map<Pin*, double> firstslack = ff->getTimingSlack();
//         double slack = firstslack.begin()->second;
//         min_x = min(min(min(squareOutline[0].x,squareOutline[1].x),squareOutline[2].x),squareOutline[3].x);
//         Candidate candidatex1(min_x, true, ff);
//         max_x = max(max(max(squareOutline[0].x,squareOutline[1].x),squareOutline[2].x),squareOutline[3].x);
//         Candidate candidatex2(max_x, false, ff);
//         ff->setFeasibleRegionwidth(max_x - min_x);
//         candidates_X.push_back(candidatex1);
//         candidates_X.push_back(candidatex2);
//         //
//         /*if(ff->getClk()=="104174"){
//             for(const auto& pin: ff->getPinList()){
//                 //cout<<"pin name: "<<pin->getPinName()<<endl;
//             }
//         }*/
//     }
//     sort(candidates_X.begin(), candidates_X.end(), [](Candidate a, Candidate b){ return a.value < b.value; });

//     while(candidates_X.size()){
//         int selectD=1;
//         /*cout<<"------------------------"<<endl;
//         cout<<"All candidates: "<<endl;
//         for(int i=0;i<candidates_X.size();i++){
//             cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
//         }*/

//         //find  selectD
//         for(int i=1;i<candidates_X.size();i++){
//             if(candidates_X[i-1].type==1&&candidates_X[i].type==0){
//                 selectD = i;
//                 break;
//             }
//             //cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
//         }
//         set<Instance*> selectedXFF;//can be merged
//         //cout << "selectD: " << selectD << endl;

//         for(int i=0;i<selectD;i++){
//             if(candidates_X[i].type==1){
//                 selectedXFF.insert(candidates_X[i].ff);
//             }
//             //cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
//         }

//         //cout<<"selectedYFF size: "<<selectedXFF.size()<<endl;
//         vector<Candidate> candidates_Y;
//         for (const auto& ff : selectedXFF) {
//             double min_y,max_y;
//             FeasibleSquare* fs = ff->getFeasible();
//             vector<Point> squareOutline=fs->getSquareOutline();
//             min_y = min(squareOutline[2].y,squareOutline[0].y);
//             Candidate candidatex3(min_y, true, ff);
//             max_y = max(squareOutline[2].y,squareOutline[0].y);
//             Candidate candidatex4(max_y, false, ff);
//             ff->setFeasibleRegionheight(max_y - min_y);
//             candidates_Y.push_back(candidatex3);
//             candidates_Y.push_back(candidatex4);
//         }
//         sort(candidates_Y.begin(), candidates_Y.end(), [](Candidate a, Candidate b){ return a.value < b.value; });
//         /*for(int i=0;i<candidates_Y.size();i++){
//             cout<<"value: "<<candidates_Y[i].value<<" type: "<<candidates_Y[i].type<<" FF name: "<<candidates_Y[i].ff->getInstanceName()<<endl;
//         }*/
//         int selectDy=1;
//         for(int i=1;i<candidates_Y.size();i++){
//             if(candidates_Y[i-1].type==1&&candidates_Y[i].type==0){
//                 selectDy = i;
//                 break;
//             }
//         }
//         set<Instance*> selectedFF;//can be merged
//         vector<Instance*> selectedFFs;
//         //cout << "selectDy: " << selectDy << endl;
//         for(int i=0;i<selectDy;i++){
//             if(candidates_Y[i].type==1&&selectedFF.count(candidates_Y[i].ff)==0){
//                 selectedFF.insert(candidates_Y[i].ff);
//                 selectedFFs.push_back(candidates_Y[i].ff);
//             }
//             //cout<<"value: "<<candidates_Y[i].value<<" type: "<<candidates_Y[i].type<<" FF name: "<<candidates_Y[i].ff->getInstanceName()<<endl;
//         }
//         //cout<<"selectedFF size: "<<selectedFF.size()<<endl;
//         //cout<<"selectedFFs size: "<<selectedFFs.size()<<endl;
//         sort(selectedFFs.begin(), selectedFFs.end(), [](Instance* a, Instance* b){ return a->getFeasibleRegionwidth()*a->getFeasibleRegionheight() > b->getFeasibleRegionwidth()*b->getFeasibleRegionheight(); });
//         /*for(int i=0;i<selectedFFs.size();i++){
//             cout<<"selectedFFs: "<<selectedFFs[i]->getInstanceName()<<endl;
//             cout<<"area: "<<selectedFFs[i]->getFeasibleRegionwidth()*selectedFFs[i]->getFeasibleRegionheight()<<endl;
//         }
//         cout<<endl;*/
//         // map<int,bool> FFlibary;
//         // for(const auto& ff : _ffArray){
//         //     int byte=ff->getBits();
//         //     FFlibary[byte] = true;
//         // }
//         int eraseCount = 0;

//         while(_lowestCostFFMap.find(selectedFF.size()) == _lowestCostFFMap.end()){ // && selectedFF.size() != 8
//             //selectedFF.erase(selectedFF.begin());
//             selectedFF.erase(selectedFFs[eraseCount]);
//             //cout<<"erase FF: "<<selectedFFs[eraseCount]->getInstanceName()<<endl;
//             eraseCount++;
//             //cout<<"1"<<endl;
//         }
//         if(selectedFF.size() > 1){

//             //cout<<"-----------------合併了---------------------"<<selectedFF.size()<<endl;
//         }

//         locationDecision(selectedFF, _lowestCostFFMap[selectedFF.size()]);
//         // cout << "location end" << endl;
//         //delete selected FF
//         // 删除与 selectedFF 中的每个 ff 匹配的前两个元素

//         for (const auto& ff : selectedFF) {
//             int count = 0; // 用于跟踪每个 ff 删除的元素数量
// 	    // cout << ff->getInstanceName() ;
//             for (int i = candidates_X.size() - 1; i >= 0; --i) { // 从后向前遍历
//                 if (candidates_X[i].ff == ff) {
//                     candidates_X.erase(candidates_X.begin() + i);
//                     count++;
//                     if (count == 2) { // 如果已删除两个匹配元素，则跳出循环
//                         break;
//                     }
//                 }
//             }
//         }
//  	// cout << endl;
//         /*/cout<<"End candidates: "<<endl;
//         for(int i=0;i<candidates_X.size();i++){
//             cout<<"value: "<<candidates_X[i].value<<" type: "<<candidates_X[i].type<<" FF name: "<<candidates_X[i].ff->getInstanceName()<<endl;
//         }*/
//     }
}

// update "instance info" and "MappingHistory" after merging to export
void Algo::MergeInstance(set<Instance*> flipflops, Instance* target, FlipFlop* FF, Site* site){
    // mapping logic
    // first: D->D0, Q->Q0 // second: D->D1, Q->Q1
    vector<Pin*> Dpins;
    size_t Dpin_count = 0;
    vector<Pin*> Qpins;
    size_t Qpin_count = 0;
    string clk_pin_name;
    updateSlack(flipflops, FF, site, target);
    
    for(const auto& ff: flipflops){
        pair<string, string> record;
        for(const auto& pin : ff->getInPinList()){
            record.first =  ff->getInstanceName() + "/" + pin->getPinName();
            const int pinID_in_net = _netName2Id[pin->getNetName()];
            Net* net = _netArray[_netName2Id[pin->getNetName()]];

            string FFpin_name = FF->getDPinList()[Dpin_count]->getPinName();
            pin->setX(FF->getDPinList()[Dpin_count]->getX());
            pin->setY(FF->getDPinList()[Dpin_count]->getY());
            net->resetMapping(pin, FFpin_name, pinID_in_net);
            pin->setPinName(FFpin_name);
            Dpins.emplace_back(pin);
            record.second = target->getInstanceName() + "/" + FFpin_name;
            Dpin_count++;       
        }
        addMappingHistory(record);

        for(const auto& pin : ff->getOutPinList()){
            record.first =  ff->getInstanceName() + "/" + pin->getPinName();
            const int pinID_in_net = _netName2Id[pin->getNetName()];
            Net* net = _netArray[_netName2Id[pin->getNetName()]];

            string FFpin_name = FF->getQPinList()[Qpin_count]->getPinName();
            pin->setX(FF->getQPinList()[Qpin_count]->getX());
            pin->setY(FF->getQPinList()[Qpin_count]->getY());
            net->resetMapping(pin, FFpin_name, pinID_in_net);
            pin->setPinName(FFpin_name);
            Dpins.emplace_back(pin);
            record.second = target->getInstanceName() + "/" + FFpin_name;
            Qpin_count++;  
        }
        addMappingHistory(record);

        for(const auto& pin : ff->getClkPinList()){
            record.first =  ff->getInstanceName() + "/" + pin->getPinName();
            clk_pin_name = pin->getPinName();
            record.second = target->getInstanceName() + "/" + pin->getPinName();
        }
        addMappingHistory(record);
    }

    _instanceArray.emplace_back(target);

    for(const auto& pin : Dpins){
        target->addPin(pin);
    }
    for(const auto& pin : Qpins){
        target->addPin(pin);
    }
    Pin* new_clkpin = new Pin(clk_pin_name, target->getX()+FF->getPinList().back()->getX(), target->getY()+FF->getPinList().back()->getY());
    new_clkpin->setX(target->getX()+FF->getPinList().back()->getX());
    new_clkpin->setY(target->getY()+FF->getPinList().back()->getY());
    target->addPin(new_clkpin);
    target->setInstType(0);
}

// deconstructor
void Algo::clear(){
    for(size_t i = 0 ; i < _instanceArray.size(); ++i){
        delete _instanceArray[i];
    }
    for(size_t i = 0 ; i < _ffArray.size(); ++i){
        delete _ffArray[i];
    }
    for(size_t i = 0 ; i < _gateArray.size(); ++i){
        delete _gateArray[i];
    }
    for(size_t i = 0 ; i < _netArray.size(); ++i){
        delete _netArray[i];
    }
    for(size_t i = 0 ; i < _clkNetArray.size(); ++i){
        delete _clkNetArray[i];
    }
    for(size_t i = 0 ; i < _IONetArray.size(); ++i){
        delete _IONetArray[i];
    }
    for(size_t i = 0 ; i < _binArray.size(); ++i){
        delete _binArray[i];
    }
    for(size_t i = 0 ; i < _placementRowArray.size(); ++i){
        delete _placementRowArray[i];
    }
    for(size_t i = 0 ; i < _primaryInputs.size(); ++i){
        delete _primaryInputs[i];
    }
    _mappingHistory.clear();
}