#include "algo.h"

using namespace std;

void Algo::updatenextSlack(Instance* ins, Edge* edge, int OutPinindex,Instance* mergeIns){
    Pin* pin1=edge->getStart().first;
    Pin *pin2=edge->getStart().second;
    Pin *pin3=edge->getEnd().first;
    Pin *pin4=edge->getEnd().second;

    double oriPinX = pin1->getX() + ins->getX();
    double oriPinY = pin1->getY() + ins->getY();
    double newPinX = _ffArray[_ffName2Id[mergeIns->getLibCellName()]]->getQPinList()[OutPinindex]->getX() + mergeIns->getX();//
    double newPinY = _ffArray[_ffName2Id[mergeIns->getLibCellName()]]->getQPinList()[OutPinindex]->getY() + mergeIns->getY();//
   
    double orignalWLq0 = abs(oriPinX - (pin2->getX()+_instanceArray[_instanceName2Id[pin2->getInstName()]]->getX()))+abs(oriPinY - (pin2->getY()+_instanceArray[_instanceName2Id[pin2->getInstName()]]->getY()));
    // double orignalWLd1 = abs(pin3->getX()+_instanceArray[_instanceName2Id[pin3->getInstName()]]->getX()-pin4->getX()-_instanceArray[_instanceName2Id[pin4->getInstName()]]->getX())+
    //                     abs(pin3->getY()+_instanceArray[_instanceName2Id[pin3->getInstName()]]->getY()-pin4->getY()-_instanceArray[_instanceName2Id[pin4->getInstName()]]->getY());
    double newWLq0 = abs(newPinX - (pin2->getX()+_instanceArray[_instanceName2Id[pin2->getInstName()]]->getX()))+abs(newPinY - (pin2->getY()+_instanceArray[_instanceName2Id[pin2->getInstName()]]->getY()));

    double mapvalue_orginal = _ffArray[_ffName2Id[ins->getLibCellName()]]->getQpinDelay()+_displaymentDelay *(orignalWLq0);
    // cout<<"orignalWLq0: "<<_displaymentDelay *(orignalWLq0)<<endl;
    // cout<<"qpinDelay: "<<_ffArray[_ffName2Id[ins->getLibCellName()]]->getQpinDelay()<<endl;
    double mapvalue_new = _displaymentDelay *newWLq0+_ffArray[_ffName2Id[mergeIns->getLibCellName()]]->getQpinDelay();
    
    Instance* nextIns = _instanceArray[_instanceName2Id[pin4->getInstName()]];
    map<double, set<Pin*>>  priorityPin=nextIns->getPriorityPin();
    double maxvalue = priorityPin.rbegin()->first;
    // for(const auto& p : priorityPin){
    //     cout<<"priorityPin: "<<p.first<<endl;
    //     for(const auto& pin : p.second){
    //         cout<<"pin name: "<<pin->getInstName()<<endl;
    //     }
    // }
    nextIns->deletePriorityPin(mapvalue_orginal,pin1);
    // cout<<"delete insname: "<<pin1->getInstName()<<" prioritypin:"<<mapvalue_orginal<<endl;
    nextIns->addPriorityPin(mapvalue_new,pin1);
    // cout<<"add insname: "<<pin1->getInstName()<<" prioritypin:"<<mapvalue_new<<endl;
    // cout<<"maxvalue: "<<maxvalue<<endl;
    // for(const auto& p : priorityPin){
    //     cout<<"priorityPin: "<<p.first<<endl;
    //     for(const auto& pin : p.second){
    //         cout<<"pin name: "<<pin->getPinName()<<endl;
    //     }
    // }
    if(nextIns->getPriorityPin().rbegin()->first!=maxvalue){
        // cout<<"ori slack: "<<nextIns->getTimingSlack()[pin4]<<endl;
        double newslack = nextIns->getTimingSlack()[pin4]+(maxvalue-nextIns->getPriorityPin().rbegin()->first);
        // cout<<"newslack: "<<newslack<<endl;
        nextIns->setTimingSlack(pin4,newslack);
    }
}

void Algo::updateSlack(set<Instance*> ins, FlipFlop* FF, Site* curSite, Instance* mergeIns){
    int i = 0;
    //cout<<"mergeIns: "<<mergeIns->getInstanceName()<<endl;
    // if(mergeIns->getInstanceName()=="C109119"){
    //     cout<<"C109119"<<endl;
    // }
    /*if(mergeIns->getInstanceName()=="C119092"){
        cout<<curSite->getX()<<" "<<curSite->getY()<<endl;
    }*/
    for(const auto& ff : ins){

        for(const auto& p : ff->getPinList()){
            // if(mergeIns->getInstanceName()=="C109119"){
            //     cout<<ff->getInstanceName()<<" "<<p->getPinName()[0]<<endl;
            //     cout<<ff->getTimingSlack()["D"]<<endl;
            // }
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
            /*if(ff->getInstanceName()=="C100567"){
                cout << "!!!!!!!" << endl;
                cout << "oriPinX: " << oriPinX << " oriPinY: " << oriPinY << endl;
                cout << "prePinX: " << prePinX << " prePinY: " << prePinY << endl;
                cout << "oriDWL: " << oriDWL << endl;
                cout << "!!!!!!!" << endl;
            }*/
            double mergePinX = curSite->getX() + FF->getPinList()[i]->getX(); // TODO: 改成intputPin
            double mergePinY = curSite->getY() + FF->getPinList()[i]->getY(); // TODO: 改成intputPin

            double mergeDWL = abs(mergePinX - prePinX) + abs(mergePinY - prePinY);
            Pin* setpin=ff->getPin("D");
            double mergeSlack = ff->getTimingSlack()[setpin] + _displaymentDelay * (oriDWL - mergeDWL);
            // if(mergeIns->getInstanceName()=="C119092"){
            //     cout<<ff->getInstanceName()<<endl;
            //     cout<<"oriDWL:"<<oriDWL<<" mergeDWL:"<<mergeDWL<<endl;
            // }
            // if(mergeSlack < 0){//&&mergeIns->getInstanceName()=="C119092"){
            //     cout << mergeIns->getInstanceName()<<" Bits:"<<FF->getBits() << " " << p->getPinName() << " " << mergeSlack << endl;
            //     cout <<"orislack: "<<ff->getTimingSlack()["D"]<<endl;
            //     cout << "mergeSlack: " << mergeSlack << endl;
            // }
            // cout << "oriWL: " << oriDWL << " mergeWL: " << mergeDWL << endl;
            
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            Pin* mergePin = mergeIns->getPin(p->getPinName());
            mergeIns->setTimingSlack(mergePin, mergeSlack);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
        i++;
    }
}