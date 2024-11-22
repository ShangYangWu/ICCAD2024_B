#include "module.h"
#include "algo.h"
#include <cmath>
#include <cfloat>

using namespace std;

// intersection of two lines
bool Instance::intersect(const Line &line1, const Line &line2, Point &intersection) {
    double A1 = line1.end.y - line1.start.y;
    double B1 = line1.start.x - line1.end.x;
    double C1 = A1 * line1.start.x + B1 * line1.start.y;

    double A2 = line2.end.y - line2.start.y;
    double B2 = line2.start.x - line2.end.x;
    double C2 = A2 * line2.start.x + B2 * line2.start.y;

    double det = A1 * B2 - A2 * B1;
    if (det == 0) {
        return false; // parallel
    } else {
        intersection.x = (B2 * C1 - B1 * C2) / det;
        intersection.y = (A1 * C2 - A2 * C1) / det;

        // check intersection
        if (min(line1.start.x, line1.end.x) <= intersection.x && intersection.x <= max(line1.start.x, line1.end.x) &&
            min(line1.start.y, line1.end.y) <= intersection.y && intersection.y <= max(line1.start.y, line1.end.y) &&
            min(line2.start.x, line2.end.x) <= intersection.x && intersection.x <= max(line2.start.x, line2.end.x) &&
            min(line2.start.y, line2.end.y) <= intersection.y && intersection.y <= max(line2.start.y, line2.end.y)) {
            return true;
        }
    }
    return false;
}

double Instance::pointToSegmentDistance(const Point &p, const Point &v, const Point &w) {
    double l2 = (v.x - w.x) * (v.x - w.x) + (v.y - w.y) * (v.y - w.y);
    if (l2 == 0.0) return hypot(p.x - v.x, p.y - v.y); // v == w case
    double t = max(0.0, min(1.0, ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));
    Point projection = { v.x + t * (w.x - v.x), v.y + t * (w.y - v.y) };
    return hypot(p.x - projection.x, p.y - projection.y);
}

bool Instance::isInside(const Point &p, const vector<Point> &polygon) {
    const double EPSILON = 1e-9;
    int count = 0;
    int n = polygon.size();

    for (int i = 0; i < n; ++i) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];

        // on line
        if (pointToSegmentDistance(p, p1, p2) < EPSILON) {
            return true;
        }

        if ((p.y > p1.y) != (p.y > p2.y)) {
            double x_intercept = (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x;
            if (p.x < x_intercept) {
                count++;
            }
        }
    }

    return count % 2 == 1;
}

void sortPoints(vector<Point> &points) {
    sort(points.begin(), points.end(), [](const Point &a, const Point &b) {
        return a.y < b.y;
    });

    Point bottom = points[0];
    Point top = points[3];
    Point left = points[1];
    Point right = points[2];

    if (left.x > right.x) std::swap(left, right);

    points = {bottom, right, top, left};
}

// init pin->feasible
void Algo::initfeasible(){
    // find all feasible region
    for(const auto& ins : _instanceArray){
        if(ins->getInstType()==0){
            // cout << ins->getInstanceName() << endl;
            vector<vector<Point>> rects;
            FeasibleSquare* initfeas = new FeasibleSquare(
                {ins->getX(), ins->getY()},
                {ins->getX(), ins->getY()},
                {ins->getX(), ins->getY()},
                {ins->getX(), ins->getY()}
            );
            initfeas->setSquareOutline();
            ins->initFeasibleSquare(initfeas);
            // Dpin
            for(const auto& Dpin: ins->getInPinList()){
                Pin* setpin = ins->getPin(Dpin->getPinName());
                double slack = ins->getTimingSlack()[setpin];
                Point center;
                for(const auto& p: _netArray[_netName2Id[Dpin->getNetName()]]->getPinList()){
                    if(p->getPinName()!=Dpin->getPinName()){
                        if(p->getPinName()[0]=='I' || p->getPinName()[0]=='i'){ // PI
                            center.x = p->getX();
                            center.y = p->getY();
                            slack += _displaymentDelay*(abs((center.x)-(Dpin->getX()+ins->getX()))+abs((center.y)-(Dpin->getY()+ins->getY())));
                        }else{ // FF or Gate
                            Instance* prev = _instanceArray[_instanceName2Id[p->getInstName()]];
                            center.x = p->getX() + prev->getX();
                            center.y = p->getY() + prev->getY();
                            slack += _displaymentDelay*(abs((center.x)-(Dpin->getX()+ins->getX()))+abs((center.y)-(Dpin->getY()+ins->getY())));
                        }
                    break;
                    }
                }

                if(slack>0){
                    FeasibleSquare* f = new FeasibleSquare(
                        {center.x, center.y-slack/_displaymentDelay},
                        {center.x+slack/_displaymentDelay, center.y},
                        {center.x, center.y+slack/_displaymentDelay},
                        {center.x-slack/_displaymentDelay, center.y}
                    );
                    rects.emplace_back(f->getDimondOutline());
                }else{
                     if(slack==0){
                        FeasibleSquare* f = new FeasibleSquare(
                            {center.x, center.y-slack/_displaymentDelay},
                            {center.x+slack/_displaymentDelay, center.y},
                            {center.x, center.y+slack/_displaymentDelay},
                            {center.x-slack/_displaymentDelay, center.y}
                        );
                        ins->setFeasibleSquare(f);
                        rects.emplace_back(f->getDimondOutline());
                     }else{
                        FeasibleSquare* f = new FeasibleSquare(
                            {center.x, center.y},
                            {center.x, center.y},
                            {center.x, center.y},
                            {center.x, center.y}
                        );
                        ins->setFeasibleSquare(f);
                        rects.emplace_back(f->getDimondOutline());
                     }
                }
            }

            // Qpin
            unordered_map<Pin*, double> slack;
            for(const auto& out: ins->getOutPinList()){
                slack[out] = DBL_MAX;
            }
            for(const auto& next_ff: ins->getNextFF()){
                for(const auto& s: next_ff->getTimingSlack()){
                    if(s.second < slack[ins->getNeighborFFMapping(next_ff)->getStart().first]){
                        slack[ins->getNeighborFFMapping(next_ff)->getStart().first] = s.second;
                    }
                }
            }
            for(const auto& out: ins->getOutPinList()){
                Point center;
                if(slack[out]>100000000){ // PO
                    continue;
                }else{
                    for(const auto& p: _netArray[_netName2Id[out->getPinName()]]->getPinList()){
                        if(p->getPinName()!=out->getPinName()){
                            Instance* next = _instanceArray[_instanceName2Id[p->getInstName()]];
                            center.x = p->getX() + next->getX();
                            center.y = p->getY() + next->getY();
                            slack[out] += _displaymentDelay*(abs((center.x)-(out->getX()+ins->getX()))+abs((center.y)-(out->getY()+ins->getY())));
                            if(slack[out]>=0){
                                FeasibleSquare* f = new FeasibleSquare(
                                    {center.x, center.y-slack[out]/_displaymentDelay},
                                    {center.x+slack[out]/_displaymentDelay, center.y},
                                    {center.x, center.y+slack[out]/_displaymentDelay},
                                    {center.x-slack[out]/_displaymentDelay, center.y}
                                );
                                rects.emplace_back(f->getDimondOutline());
                            }
                        }
                    }
                }
                              
            }
            if(rects.size()>=1){
                ins->setFFfeasible(rects);
            }
        }
    }
}

// set FF feaible region = intersection of Dpin feasible region
void Instance::setFFfeasible(vector<vector<Point>>& rects){
    // only one FF selected
    if(rects.size()==1){
        _feasibleSquare->setOutline(rects[0][0], rects[0][1], rects[0][2], rects[0][3]);
        return;
    }

    // get the edges of each feasible region
    vector<vector<Line>> edges;
    for(const auto& rect : rects){
        vector<Line> e;
        for(size_t i=0; i<4; ++i){
            e.push_back({rect[i], rect[(i + 1) % 4]});
        }
        edges.emplace_back(e);
    }

    // find the intersection point of each edge
    std::vector<Point> intersectionPoints;
    for (size_t i = 0; i < edges.size(); ++i) {
        for (size_t j = i + 1; j < edges.size(); ++j) {
            for (const auto &edge1 : edges[i]) {
                for (const auto &edge2 : edges[j]) {
                    Point intersection;
                    if (intersect(edge1, edge2, intersection)) {
                        intersectionPoints.emplace_back(intersection);
                    }
                }
            }
        }
    }

    // check if the points of the dimonds falls in other dimonds
    for (size_t i = 0; i < rects.size(); ++i) {
        for (const auto &p : rects[i]) {
            bool insideAll = true;
            for (size_t j = 0; j < rects.size(); ++j) {
                if (i == j) continue;
                if (!isInside(p, rects[j])) {
                    insideAll = false;
                    break;
                }
            }
            if (insideAll) {
                intersectionPoints.push_back(p);
            }
        }
    }

    // record the final points
    std::vector<Point> finalPoints;
    for (const auto &p : intersectionPoints) {
        bool insideAll = true;
        for (const auto &rect : rects) {
            if (!isInside(p, rect)) {
                insideAll = false;
                break;
            }
        }
        if (insideAll) {
            finalPoints.push_back(p);
        }
    }
    if(finalPoints.size()!=4){ // 4 points to form a sqaure
        return;
    }

    sortPoints(finalPoints); // set the outline in order
    _feasibleSquare->setOutline(finalPoints[0], finalPoints[1], finalPoints[2], finalPoints[3]);
}

// roughly same as setFFfeasible
void Algo::updateFeasible(set<Instance*> target){
    // update targets feasible region
    for(const auto& ins: target){
        vector<vector<Point>> rects;

        // step1: init target feasible region
        ins->getFeasible()->setOutline(
            {ins->getX(), ins->getY()},
            {ins->getX(), ins->getY()},
            {ins->getX(), ins->getY()},
            {ins->getX(), ins->getY()}
        );
        // step2: Dpin feasible region 
        for(const auto& Dpin: ins->getInPinList()){
            Pin* setpin = ins->getPin(Dpin->getPinName());
            double slack = ins->getTimingSlack()[setpin];
            Point center;
            for(const auto& p: _netArray[_netName2Id[Dpin->getNetName()]]->getPinList()){
                if(p->getPinName()!=Dpin->getPinName()){
                    if(p->getPinName()[0]=='I' || p->getPinName()[0]=='i'){ // PI
                        center.x = p->getX();
                        center.y = p->getY();
                        slack += _displaymentDelay*(abs((center.x)-(Dpin->getX()+ins->getX()))+abs((center.y)-(Dpin->getY()+ins->getY())));
                    }else{ // FF or Gate
                        Instance* prev = _instanceArray[_instanceName2Id[p->getInstName()]];
                        center.x = p->getX() + prev->getX();
                        center.y = p->getY() + prev->getY();
                        slack += _displaymentDelay*(abs((center.x)-(Dpin->getX()+ins->getX()))+abs((center.y)-(Dpin->getY()+ins->getY())));
                    }
                break;
                }
            }

            if(slack>0){
                FeasibleSquare* f = new FeasibleSquare(
                    {center.x, center.y-slack/_displaymentDelay},
                    {center.x+slack/_displaymentDelay, center.y},
                    {center.x, center.y+slack/_displaymentDelay},
                    {center.x-slack/_displaymentDelay, center.y}
                );
                rects.emplace_back(f->getDimondOutline());
            }else{
                    if(slack==0){
                    FeasibleSquare* f = new FeasibleSquare(
                        {center.x, center.y-slack/_displaymentDelay},
                        {center.x+slack/_displaymentDelay, center.y},
                        {center.x, center.y+slack/_displaymentDelay},
                        {center.x-slack/_displaymentDelay, center.y}
                    );
                    ins->setFeasibleSquare(f);
                    rects.emplace_back(f->getDimondOutline());
                    }else{
                    FeasibleSquare* f = new FeasibleSquare(
                        {center.x, center.y},
                        {center.x, center.y},
                        {center.x, center.y},
                        {center.x, center.y}
                    );
                    ins->setFeasibleSquare(f);
                    rects.emplace_back(f->getDimondOutline());
                    }
            }
        }

        // step3: Qpin feasible region
        unordered_map<Pin*, double> slack;
        for(const auto& out: ins->getOutPinList()){
            slack[out] = DBL_MAX;
        }
        for(const auto& next_ff: ins->getNextFF()){
            for(const auto& s: next_ff->getTimingSlack()){
                if(s.second < slack[ins->getNeighborFFMapping(next_ff)->getStart().first]){
                    slack[ins->getNeighborFFMapping(next_ff)->getStart().first] = s.second;
                }
            }
        }
        for(const auto& out: ins->getOutPinList()){
            Point center;
            if(slack[out]>100000000){ // PO
                continue;
            }else{
                for(const auto& p: _netArray[_netName2Id[out->getPinName()]]->getPinList()){
                    if(p->getPinName()!=out->getPinName()){
                                Instance* next = _instanceArray[_instanceName2Id[p->getInstName()]];
                                center.x = p->getX() + next->getX();
                                center.y = p->getY() + next->getY();
                                slack[out] += _displaymentDelay*(abs((center.x)-(out->getX()+ins->getX()))+abs((center.y)-(out->getY()+ins->getY())));
                                if(slack[out]>=0){
                                    FeasibleSquare* f = new FeasibleSquare(
                                        {center.x, center.y-slack[out]/_displaymentDelay},
                                        {center.x+slack[out]/_displaymentDelay, center.y},
                                        {center.x, center.y+slack[out]/_displaymentDelay},
                                        {center.x-slack[out]/_displaymentDelay, center.y}
                                    );
                                    rects.emplace_back(f->getDimondOutline());
                                }
                    }
                }
            }
                            
        }
        if(rects.size()>=1){
            ins->setFFfeasible(rects);
        }        
    }
}