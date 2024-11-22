# include "algo.h"

using namespace std;

void Algo::parse(fstream& input_file)
{
    string str;
    while(input_file >> str)
    {
        if(str == "Alpha")
        {
            string alpha = "";
            input_file >> alpha; _alpha = stod(alpha);
        }else if(str == "Beta")
        {
            string beta = "";
            input_file >> beta; _beta = stod(beta);
        }else if(str == "Gamma")
        {
            string gamma = "";
            input_file >> gamma; _gamma = stod(gamma);
        }else if(str == "Lambda")
        {
            string lambda = "";
            input_file >> lambda; _lambda = stod(lambda);
        }
        else if(str == "DieSize")
        {
            string x1 = "", y1 = "", x2 = "", y2 = "";
            input_file >> x1 >> y1 >> x2 >> y2;
            _die._x1 = stod(x1); _die._y1 = stod(y1);
            _die._x2 = stod(x2); _die._y2 = stod(y2);
        }
        else if(str == "NumInput")
        {
            string numInput = "";
            input_file >> numInput; _die._inputNum = stoi(numInput);
            for(size_t i = 0 ; i < _die._inputNum ; i++)
            {
                string name = "", x = "", y = "",input_syntax="";
                input_file >> input_syntax >> name >> x >> y;
                Pin* pin = new Pin(name, stod(x), stod(y));
                _die._inputPinArray.emplace_back(pin);
                _die._inputPinName2Id[name] = i;
                PrimaryInput* pi = new PrimaryInput(name, name, stod(x), stod(y));
                pi->setPin(pin);
                pi->setInstType(2); //primary input
                _primaryInputs.emplace_back(pi);
            }
        }else if(str == "NumOutput")
        {
            string numOutput = "";
            input_file >> numOutput; _die._outputNum = stoi(numOutput);
            for(size_t i = 0 ; i < _die._outputNum ; i++)
            {
                string name = "", x = "", y = "",output_syntax="";
                input_file >> output_syntax >> name >> x >> y;
                Pin* pin = new Pin(name, stod(x), stod(y));
                pin->setPinType(1); //output
                _die._outputPinArray.emplace_back(pin);
                _die._outputPinName2Id[name] = i;
                PrimaryOutput* po = new PrimaryOutput(name, name, stod(x), stod(y));
                po->setPin(pin);
                po->setInstType(3);
                _primaryOutputs.emplace_back(po);
            }
        }
        else if(str == "FlipFlop")
        {
            string bits = "", ffname = "", width = "", height = "", pinCount = "";
            input_file >> bits >> ffname >> width >> height >> pinCount;
            if(_ffArray.size() == 0) _areaNorm = stod(width)*stod(height); // the first ff in library should be norm factor
            FlipFlop* ff = new FlipFlop(ffname, stod(width), stod(height), stoi(pinCount), stoi(bits));
            for(size_t i = 0 ; i < stoi(pinCount) ; i++)
            {
                string pinName = "", x = "", y = "", p_syntax="";
                input_file >> p_syntax >> pinName >> x >> y;
                Pin* pin = new Pin(pinName, stod(x), stod(y));
                pin->setLibPinName(ffname);
                if(pinName[0] == 'C' || pinName[0] == 'c') pin->setPinType(2); //CLK
                else if(pinName[0] == 'Q' || pinName[0] == 'q') pin->setPinType(1); //Q
                else if(pinName[0] == 'D'|| pinName[0] == 'd') pin->setPinType(0); //D
                ff->addPin(pin);
                if(pinName[0] == 'D'){
                    ff->addDpin(pin);
                }else{
                    if(pinName[0] == 'Q'){
                        ff->addQpin(pin);
                    }else{
                        ff->addClkpin(pin);
                    }
                }
            }
            addFF(ff);
        }
        else if(str == "Gate")
        {
            string gname = "", width = "", height = "", pinCount = "";
            input_file >> gname >> width >> height >> pinCount;
            Gate* gate = new Gate(gname, stod(width), stod(height), stoi(pinCount));
            for(size_t i = 0 ; i < stoi(pinCount) ; i++)
            {
                string pinName = "", x = "", y = "", p_syntax="";
                input_file >> p_syntax >> pinName >> x >> y;
                Pin* pin = new Pin(pinName, stod(x), stod(y));
                pin->setLibPinName(gname);
                if(pinName[0] == 'I' || pinName[0] == 'i') pin->setPinType(0); //input
                else if(pinName[0] == 'O' || pinName[0] == 'o') pin->setPinType(1); //output
                gate->addPin(pin);
                if(pinName[0]=='i' || pinName[0]=='I'){
                    gate->addInputpin(pin);
                }else{
                    gate->addOutputpin(pin);
                }
            }
            addGate(gate);
        }
        else if(str == "NumInstances")
        {
            string numInstances = "";
            input_file >> numInstances; _instanceNum = stoi(numInstances);
            for(size_t i = 0 ; i < _instanceNum ; i++)
            {
                string iname = "", libCellName="", x = "", y = "", inst_syntax = "";
                input_file >> inst_syntax >> iname >> libCellName >> x >> y;
                Instance* instance = new Instance(iname, libCellName, stod(x), stod(y));
                if(_ffName2Id.find(libCellName) != _ffName2Id.end()){
                    instance->setInstType(0); //is Flip-flop
                    if(_ffArray[_ffName2Id[libCellName]]->getBits()!=1){
                        instance->setlock(1); // lock mutibit FF
                        addMultibitIns(instance);
                    }
                }
                else if(_gateName2Id.find(libCellName) != _gateName2Id.end()){ 
                    instance->setInstType(1); //is Gate
                }
                addInstance(instance);
            }
        }
        else if(str == "NumNets")
        {
            string numNets = "";
            input_file >> numNets; _netNum = stoi(numNets);
            size_t inval = 0;
            for(size_t i = 0 ; i < _netNum ; i++)
            {
                string nname = "", pinCount = "", n_syntax="";
                input_file >> n_syntax >> nname >> pinCount;
                Net* net = new Net(nname, stoi(pinCount));
                vector<Instance*> tempInst;
                bool invalid_net = false; 
                for(size_t j = 0 ; j < stoi(pinCount) ; j++)
                {
                    string pinName = "", p_syntax="", temp="";
                    input_file >> p_syntax >> temp;
                    if(temp.find("/") != string::npos) //not Die I/O
                    {
                        istringstream iss(temp); string instName="", libPinName="";
                        getline(iss, instName, '/'); getline(iss, libPinName, '/');
                        Instance* const instance = _instanceArray[_instanceName2Id[instName]];
                        if(instance->getInstType()==0) //instance is Flip-flop
                        {
                            FlipFlop* ff = _ffArray[_ffName2Id[instance->getLibCellName()]];
                            string ffName = ff->getLibCellName(); size_t pinId = ff->getPinId(libPinName);
                            Pin* pin = _ffArray[_ffName2Id[ffName]]->getPinList()[pinId]; // get specific pin
                            string pname = pin->getPinName();
                            string plibCellName = pin->getLibPinName();
                            if(pname[0] == 'Q'){
                                if(instance->getPin(pname) == nullptr){ // need to build new pin for the instance
                                    Pin* newPin = new Pin(pname, pin->getX(), pin->getY());
                                    newPin->setInstName(instName); newPin->setLibPinName(plibCellName); newPin->setNetName(nname);  newPin->setPinType(pin->getPinType());
                                    instance->addPin(newPin); //add pin to instance
                                    instance->addOutPin(newPin);
                                    net->addPin(newPin);
                                    tempInst.emplace_back(_instanceArray[_instanceName2Id[instName]]); //add net to instance                             
                                }else{ // instance already has the pin
                                    delNet(net);
                                    inval++;
                                    Pin* newPin = instance->getPin(pname);
                                    net = _netArray[_netName2Id[newPin->getNetName()]];
                                    invalid_net = true;
                                }
                            }else{
                                Pin* newPin = new Pin(pname, pin->getX(), pin->getY());
                                newPin->setInstName(instName); newPin->setLibPinName(plibCellName); newPin->setNetName(nname);  newPin->setPinType(pin->getPinType());
                                instance->addPin(newPin); //add pin to instance
                                if(pname[0] == 'D'){
                                    instance->addInPin(newPin);
                                    instance->setTimingSlack(newPin, 0); // init timing slack of gate to 0
                                    if(invalid_net == true){
                                        net->addPinNum();
                                    }
                                }else{
                                    instance->addClkPin(newPin);
                                }
                                net->addPin(newPin);
                                tempInst.emplace_back(_instanceArray[_instanceName2Id[instName]]); //add net to instance
                            }
                            if(stoi(pinCount)==1) { _onePinNetInstance.emplace_back(_instanceArray[_instanceName2Id[instName]]); }
                        }
                        else if(instance->getInstType()==1) //instance is Gate
                        {
                            Gate* gate = _gateArray[_gateName2Id[instance->getLibCellName()]];
                            string gateName = gate->getLibCellName(); size_t pinId = gate->getPinId(libPinName);
                            Pin* pin = _gateArray[_gateName2Id[gateName]]->getPinList()[pinId]; // get specific
                            string pname = pin->getPinName();
                            string plibCellName = pin->getLibPinName();

                            Pin* newPin = new Pin(pname, pin->getX(), pin->getY()); // recover to pin offset
                            newPin->setInstName(instName); newPin->setLibPinName(plibCellName); newPin->setNetName(nname); newPin->setPinType(pin->getPinType());
                            instance->addPin(newPin); //add pin to instance
                            if(pname[0] == 'I' || pname[0] == 'i'){
                                instance->addInPin(newPin);
                                if(invalid_net == true){
                                    net->addPinNum();
                                }
                            }else{
                                if(pname[0] == 'O' || pname[0] == 'o'){
                                    instance->addOutPin(newPin);
                                }else{
                                    instance->addClkPin(newPin);
                                }
                            }
                            net->addPin(newPin);
                            tempInst.emplace_back(_instanceArray[_instanceName2Id[instName]]); //add net to instance
                        }
                    }
                    else //is Die I/O
                    {
                        if(_die._inputPinName2Id.find(temp) != _die._inputPinName2Id.end()) //is Input
                        {
                            Pin* pin = _die._inputPinArray[_die._inputPinName2Id[temp]]; pin->setNetName(nname);
                            net->addPin(pin);
                        }    
                        else if(_die._outputPinName2Id.find(temp) != _die._outputPinName2Id.end()) //is Output
                        {
                            Pin* pin = _die._outputPinArray[_die._outputPinName2Id[temp]]; pin->setNetName(nname);
                            net->addPin(pin);
                        }    
                    }
                }

                if(invalid_net == false){
                    addNet(net);
                }

                bool clk = false;
                for(auto& p : net->getPinList()){
                    if(p->getPinType()==2){
                        addClkNet(net);
                        net->setType(0);
                        clk = true;
                        break;
                    }    
                }
                if(!clk){
                    addIONet(net);
                    net->setType(1);
                }

                for(size_t k = 0 ; k < tempInst.size() ; k++)
                {
                    tempInst[k]->addNet(net);
                }
            }

        }
        else if(str == "BinWidth")
        {
            string binWidth = "";
            input_file >> binWidth; _binWidth = stod(binWidth);
        }
        else if(str == "BinHeight")
        {
            string binHeight = "";
            input_file >> binHeight; _binHeight = stod(binHeight);
        }
        else if(str == "BinMaxUtil")
        {
            string binMaxUtil = "";
            input_file >> binMaxUtil; _binMaxUtli = stod(binMaxUtil);
        }
        else if(str == "PlacementRows")
        {
            string x = "", y = "", width = "", height = "", numOfSites = "";
            input_file >> x >> y >> width >> height >> numOfSites;
            PlacementRow* row = new PlacementRow(stod(x), stod(y), stoi(numOfSites), stod(width), stod(height));
            _placementRowArray.emplace_back(row); _siteNum = stoi(numOfSites); _siteWidth = stod(width); _siteHeight = stod(height);
        }
        else if(str == "DisplacementDelay")
        {
            string delay = "";
            input_file >> delay; _displaymentDelay = stod(delay);
        }
        else if(str == "QpinDelay") //only Flip-flop
        {
            string libCellName = "", delay = "";
            input_file >> libCellName >> delay;
            FlipFlop* ff = _ffArray[_ffName2Id[libCellName]];
            if(_ffName2Id[libCellName] == 0) _qpinNorm = stod(delay);  // the first ff in library should be norm factor
            ff->setQpinDelay(stod(delay));
        }
        else if(str == "TimingSlack") //only Flip-flop (in Instance)
        {
            string instCellName = "", pinName="", slack = "";
            input_file >> instCellName >> pinName >> slack;
            Instance* instance = _instanceArray[_instanceName2Id[instCellName]];
            Pin* setpin = instance->getPin(pinName);
            instance->setTimingSlack(setpin, stod(slack));
        }
        else if(str == "GatePower") //only Flip-flop
        {
            string libCellName = "", power = "";
            input_file >> libCellName >> power;
            FlipFlop* ff = _ffArray[_ffName2Id[libCellName]];
            if(_ffName2Id[libCellName] == 0) _pwrNorm = stod(power);  // the first ff in library should be norm factor
            ff->setGatePower(stod(power));
        }
    }
    _flipFlopNum = _ffArray.size(); _gateNum = _gateArray.size();
}

void Algo::print()
{
/*Die Information*/
    cout << "<<Die Information>>" << endl;
    cout << "Alpha: " << _alpha << endl;
    cout << "Beta: " << _beta << endl;
    cout << "Gamma: " << _gamma << endl;
    cout << "Lambda: " << _lambda << endl;
    cout << "DieSize: " << _die._x1 << " " << _die._y1 << " " << _die._x2 << " " << _die._y2 << endl;
    cout << "NumInput: " << _die._inputNum << endl;
    for(size_t i = 0 ; i < _die._inputNum ; i++)
    {
        cout << "InputPin: " << _die._inputPinArray[i]->getPinName() << " " << _die._inputPinArray[i]->getX() << " " << _die._inputPinArray[i]->getY() << endl;
    }
    cout << "NumOutput: " << _die._outputNum << endl;
    for(size_t i = 0 ; i < _die._outputNum ; i++)
    {
        cout << "OutputPin: " << _die._outputPinArray[i]->getPinName() << " " << _die._outputPinArray[i]->getX() << " " << _die._outputPinArray[i]->getY() << endl;
    }
    cout << endl;
/*Cell Library Information*/
    cout << "<<Flip-flop>>" << endl;
    for(size_t i = 0 ; i < _flipFlopNum ; i++)
    {
        cout << "Flip-flop: " << _ffArray[i]->getLibCellName() << " "<< _ffArray[i]->getPinNum() << endl;
        for(size_t j = 0 ; j < _ffArray[i]->getPinNum() ; j++)
        {
            cout << "Pin: " << _ffArray[i]->getPinList()[j]->getPinName() << " " << _ffArray[i]->getPinList()[j]->getX() << " " << _ffArray[i]->getPinList()[j]->getY() << endl;
        }
        cout << "QPinDelay: " << _ffArray[i]->getQpinDelay() << ", GatePower: " << _ffArray[i]->getGatePower() << endl;
    }
    cout << "Gate" << endl;
    for(size_t i = 0 ; i < _gateNum ; i++)
    {
        cout << "Gate: " << _gateArray[i]->getLibCellName() << " "<< _gateArray[i]->getPinNum() << endl;
        for(size_t j = 0 ; j < _gateArray[i]->getPinNum() ; j++)
        {
            cout << "Pin: " << _gateArray[i]->getPinList()[j]->getPinName() << " " << _gateArray[i]->getPinList()[j]->getX() << " " << _gateArray[i]->getPinList()[j]->getY() << endl;
        }
    }
    cout << endl;
/*Instance Information*/
    cout << "<<Instance>>" << endl;
    for(size_t i = 0 ;i < _instanceNum ; i++)
    {
        cout << "Instance: " << _instanceArray[i]->getInstanceName() << " " << _instanceArray[i]->getLibCellName() << " " << _instanceArray[i]->getX() << " " << _instanceArray[i]->getY() << endl;
        cout << "InstType: " << _instanceArray[i]->getInstType() << endl;
        unordered_map<Pin*, double> timingSlack = _instanceArray[i]->getTimingSlack();
        for(unordered_map<Pin*, double>::iterator it = timingSlack.begin() ; it != timingSlack.end() ; it++)
        {
            cout << "TimingSlack: " << it->first<< " " << it->second << endl;
        }
    }
    cout << endl;
/*Net List Information*/
    cout << "<<Net List: I/O>>" << endl;
    for(size_t i = 0 ; i < _IONetArray.size() ; i++)
    {
        cout << "Net: " << _IONetArray[i]->getNetName() << " " << _IONetArray[i]->getPinNum() << endl;
        for(size_t j = 0 ; j < _IONetArray[i]->getPinNum() ; j++)
        {
            cout << "Pin: " << _IONetArray[i]->getPinList()[j]->getPinName() << " ";
            if(!_IONetArray[i]->getPinList()[j]->getInstName().empty())
            {
                 cout << "(" << _IONetArray[i]->getPinList()[j]->getInstName() << "/" << _IONetArray[i]->getPinList()[j]->getLibPinName() << ")";
            }
            cout << _IONetArray[i]->getPinList()[j]->getX() << " " << _IONetArray[i]->getPinList()[j]->getY() << endl;
        }
    }
    cout << "<<Net List: CLK>>" << endl;
    for(size_t i = 0 ; i < _clkNetArray.size() ; i++)
    {
        cout << "Net: " << _clkNetArray[i]->getNetName() << " " << _clkNetArray[i]->getPinNum() << endl;
        for(size_t j = 0 ; j < _clkNetArray[i]->getPinNum() ; j++)
        {
            cout << "Pin: " << _clkNetArray[i]->getPinList()[j]->getPinName() << " ";
            if(!_clkNetArray[i]->getPinList()[j]->getInstName().empty())
            {
                 cout << "(" << _clkNetArray[i]->getPinList()[j]->getInstName() << "/" << _clkNetArray[i]->getPinList()[j]->getLibPinName() << ")";
            }
            cout << _clkNetArray[i]->getPinList()[j]->getX() << " " << _clkNetArray[i]->getPinList()[j]->getY() << endl;
        }
    }
    cout << endl;
/*Bin Information*/
    cout << "<<Bin>>" << endl;
    cout << "BinWidth: " << _binWidth << " BinHeight: " << _binHeight << endl;
    cout << "BinMaxUtil: " << _binMaxUtli << endl;
    cout << endl;
/*Placement Row*/
    cout << "<<Placement Row>>" << endl;
    cout << "SiteWidth: " << _siteWidth << " SiteHeight: " << _siteHeight << " SiteNum: " << _siteNum << endl;
    for(size_t i = 0 ; i < _placementRowArray.size() ; i++)
    {
        cout << "PlacementRow: " << _placementRowArray[i]->getX() << " " << _placementRowArray[i]->getY() << endl;
        for(size_t j = 0 ; j < _placementRowArray[i]->getSiteArray().size() ; j++)
        {
            cout << "Site: " << _placementRowArray[i]->getSiteArray()[j]->getX() << " " << _placementRowArray[i]->getSiteArray()[j]->getY() << endl;
        }
    }
    cout << endl;
}
