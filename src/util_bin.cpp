#include "algo.h"

using namespace std;

// calcualte the initial bin usage
void Algo::InitBinUsage() { 
    int bin_row = (_die._x2 - _die._x1) / _binWidth+1;
    int bin_col = (_die._y2 - _die._y1) / _binHeight+1;
    
    vector<vector<double>> tmp(bin_col, vector<double>(bin_row, 0.0));
    _binUsagerate = tmp;
    for(const auto& ins: _instanceArray){
        pair<double, double> outline;
        if(ins->getInstType()==1){
            outline = _gateArray[_gateName2Id[ins->getLibCellName()]]->getOutline();
        }else{
            outline = _ffArray[_ffName2Id[ins->getLibCellName()]]->getOutline();
        }
        const double ins_x1 = ins->getX(), ins_x2 = ins->getX()+outline.first,
                        ins_y1 = ins->getY(), ins_y2 = ins->getY()+outline.second;
        addBinUsage(ins_x1, ins_x2, ins_y1, ins_y2);
    }
}

// update bin usage operations
int Algo::addBinUsage(int x1, int x2, int y1, int y2){
    int nextbinX = nextBinx(x1), upbinY = upBiny(y1);
    int BinX = x1/_binWidth, BinY = y1/_binHeight;
    int exceed = 0;
    if(x2<=nextbinX && y2<=upbinY){
        _binUsagerate[BinY][BinX] += (x2-x1)*(y2-y1);
    }else if(x2<=nextbinX && y2>upbinY){
        _binUsagerate[BinY][BinX] += (x2-x1)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX] += (x2-x1)*(y2-upbinY);
        if(_binUsagerate[BinY+1][BinX]/(_binWidth*_binHeight)>_binMaxUtli/100){
            exceed++;
        }
    }
    else if(x2>nextbinX && y2<=upbinY){
        _binUsagerate[BinY][BinX] += (nextbinX-x1)*(y2-y1);
        _binUsagerate[BinY][BinX+1] += (x2-nextbinX)*(y2-y1);
        if(_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
            exceed++;
        }
    }
    else if(x2>nextbinX && y2>upbinY){
        _binUsagerate[BinY][BinX] += (nextbinX-x1)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX] += (nextbinX-x1)*(y2-upbinY);
        _binUsagerate[BinY][BinX+1] += (x2-nextbinX)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX+1] += (x2-nextbinX)*(y2-upbinY);
        if(_binUsagerate[BinY+1][BinX]/(_binWidth*_binHeight)>_binMaxUtli/100){
            exceed++;
        }
        if(_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
            exceed++;
        }
        if(_binUsagerate[BinY+1][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
            exceed++;
        }
    }else{
        cout<<"error"<<endl;
    }
    if((_binUsagerate[BinY][BinX]/(_binWidth*_binHeight))>(_binMaxUtli/100)){
        exceed++;
    }
    if((x2>nextbinX+_binWidth)||(y2>upbinY+_binHeight)){
        cout<<"-------------------------instance usage exceeds the bin size"<<endl;
    }
    // if(exceed>0){
    //     cout<<"有查到"<<endl;
    // }
    return exceed;
}

// update bin usage operations
void Algo::subBinUsage(int x1, int x2, int y1, int y2){
    int nextbinX = nextBinx(x1), upbinY = upBiny(y1);
    int BinX = x1/_binWidth, BinY = y1/_binHeight;
    // if((_binUsagerate[BinY][BinX]/(_binWidth*_binHeight))>(_binMaxUtli/100)){
    //     exceed+=_binUsagerate[BinY][BinX]/(_binWidth*_binHeight)-(_binMaxUtli/100);
    // }
    if(x2<=nextbinX && y2<=upbinY){
        _binUsagerate[BinY][BinX] -= (x2-x1)*(y2-y1);
    }else if(x2<=nextbinX && y2>upbinY){
        _binUsagerate[BinY][BinX] -= (x2-x1)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX] -= (x2-x1)*(y2-upbinY);
        // if(_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
        //     exceed+=_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)-(_binMaxUtli/100);
        // }
    }else if(x2>nextbinX && y2<=upbinY){
        _binUsagerate[BinY][BinX] -= (nextbinX-x1)*(y2-y1);
        _binUsagerate[BinY][BinX+1] -= (x2-nextbinX)*(y2-y1);
        // if(_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
        //     exceed+=_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)-(_binMaxUtli/100);
        // }
    }else{
        // if(_binUsagerate[BinY+1][BinX]/(_binWidth*_binHeight)>_binMaxUtli/100){
        //     exceed+=_binUsagerate[BinY+1][BinX]/(_binWidth*_binHeight)-(_binMaxUtli/100);
        // }
        // if(_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
        //     exceed+=_binUsagerate[BinY][BinX+1]/(_binWidth*_binHeight)-(_binMaxUtli/100);
        // }
        // if(_binUsagerate[BinY+1][BinX+1]/(_binWidth*_binHeight)>_binMaxUtli/100){
        //     exceed+=_binUsagerate[BinY+1][BinX+1]/(_binWidth*_binHeight)-(_binMaxUtli/100);
        // }
        _binUsagerate[BinY][BinX] -= (nextbinX-x1)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX] -= (nextbinX-x1)*(y2-upbinY);
        _binUsagerate[BinY][BinX+1] -= (x2-nextbinX)*(upbinY-y1);
        _binUsagerate[BinY+1][BinX+1] -= (x2-nextbinX)*(y2-upbinY);
        
    }
}

int Algo::nextBinx(int x){
    int nextBinX = (x/_binWidth+1)*_binWidth;
    return nextBinX;
}

int Algo::upBiny(int y){
    int upBinY = (y/_binHeight+1)*_binHeight;
    return upBinY;
}

void Algo::printBinUsage(){
    for (int i = 0; i < _binUsagerate.size(); i++) {        
        for (int j = 0; j < _binUsagerate[0].size(); j++) {
            if(_binUsagerate[i][j]/(_binWidth*_binHeight)>_binMaxUtli/100){
                cout<<"bin "<<i<<" "<<j<<" ";
                cout<<"exceeds the bin size"<<endl;
                cout<<_binUsagerate[i][j]/(_binWidth*_binHeight)<<endl;
            }
        }
    }
}