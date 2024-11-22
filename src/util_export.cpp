#include "algo.h"

using namespace std;

// calculate the origin cost of circuit
void Algo::objectiveFunctionBefore(){
    _totalCost = 0;
    double totalPower = 0, totalArea = 0, totalTNS = 0;
    int vioBin = 0;
    int i = 0;
    for(const auto& ins : _instanceArray){
        if(ins->getInstType() == 0){
            FlipFlop* FF = _ffArray[_ffName2Id[ins->getLibCellName()]];
            totalPower += FF->getGatePower();
            totalArea += FF->getWidth() * FF->getHeight();
            for(const auto& s : ins->getTimingSlack()){
                if(s.second < 0)
                    totalTNS -= s.second;
            }
        }
    }
    for(const auto& bin : _binArray){
        if(bin->getUtil() > _binMaxUtli)
            vioBin++;
    }
    _totalCost = _alpha * totalTNS + _beta * totalPower + _gamma * totalArea + _lambda * vioBin;
    cout << "Total Cost: " << _totalCost << " = " << _alpha << " * " << totalTNS << " + " << _beta << " * " << totalPower << " + " << _gamma << " * " << totalArea << " + " << _lambda << " * " << vioBin << endl;
}

// we exclude initial mutibit FF from FF list, therefore we need to insert back mutibit FF before output is exported
void Algo::mapMutiFF(){ 
    for(const auto& ins: _multibitInsArray){
        // map to _historymap
        string new_ins_name = "C" + to_string(_instanceArray.size()+1);
        string ffname = ins->getLibCellName();
        Instance* target = new Instance(new_ins_name, ffname, ins->getX(), ins->getY());
        for(const auto& pin: ins->getPinList()){
            pair<string, string> record;
            record.first =  ins->getInstanceName() + "/" + pin->getPinName();
            record.second = target->getInstanceName() + "/" + pin->getPinName();
            addMappingHistory(record);      
        }
        addInstance(target);
    }
}

// ensuring at least legalized output could be performed
void Algo::writeInit(fstream& outFile) 
{
    if(!outFile.is_open()){
        cerr << "Error: cannnot open file" << endl;
    }else{
        outFile.clear();
        
        size_t ffcount = 0;
        for(const auto& ins: _instanceArray){
            if(ins->getInstType()==0){
                ffcount++;
            }
        }
        outFile << "CellInst " << ffcount << endl;
        // cout << "size " << _instanceArray.size() << " insNum " << _instanceNum << endl;
        size_t i = _instanceArray.size()+1;
        for(size_t idx=0; idx< _instanceArray.size(); ++idx){
            if(_instanceArray[idx]->getInstType() ==0){
                outFile << "Inst " 
                    << "C" + to_string(i) << " "
                    << _instanceArray[idx]->getLibCellName() << " " 
                    << _instanceArray[idx]->getX() << " "
                    << _instanceArray[idx]->getY() << endl;
                i++;
            }
        }
        i = _instanceArray.size()+1;
        for(size_t idx=0 ; idx< _instanceArray.size(); ++idx)
        {
            if(_instanceArray[idx]->getInstType() ==0){
                for(auto& pin : _instanceArray[idx]->getPinList())
                {
                    outFile << _instanceArray[idx]->getInstanceName() << "/" << pin->getPinName() << " map " << "C" + to_string(i) << "/" << pin->getPinName() << endl;
                }
                i++;
            }
        }
        outFile.close();
    }
}

// write output to outputfile
void Algo::write(fstream& outFile){
    if(!outFile.is_open()){
        cerr << "Error: cannnot open file" << endl;
    }else{
        outFile.clear();
        outFile << "CellInst " << _instanceArray.size()-_instanceNum << endl;
        for(size_t idx=_instanceNum; idx< _instanceArray.size(); ++idx){
            outFile << "Inst " 
                 << _instanceArray[idx]->getInstanceName() << " "
                 << _instanceArray[idx]->getLibCellName() << " " 
                 << _instanceArray[idx]->getX() << " "
                 << _instanceArray[idx]->getY() << endl;
        }
        for(const auto& map : _mappingHistory){
            outFile << map.first << " map " << map.second << endl;            
        }

        outFile.close();
    }
    return; 
}