#include "algo.h"

using namespace std;

// findOverlapFeasible used function
bool intersect(const Line &line1, const Line &line2, Point &intersection) {
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

// findOverlapFeasible used function
double pointToSegmentDistance(const Point &p, const Point &v, const Point &w) {
    double l2 = (v.x - w.x) * (v.x - w.x) + (v.y - w.y) * (v.y - w.y);
    if (l2 == 0.0) return hypot(p.x - v.x, p.y - v.y); // v == w case
    double t = max(0.0, min(1.0, ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));
    Point projection = { v.x + t * (w.x - v.x), v.y + t * (w.y - v.y) };
    return hypot(p.x - projection.x, p.y - projection.y);
}

// findOverlapFeasible used function
bool isInside(const Point &p, const vector<Point> &polygon) {
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

// findOverlapFeasible used function
void sortPoints1(vector<Point> &points) {
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

// find the FF placeable region for locationDecision
vector<Point> Algo::findOverlapFeasible(set<Instance*> flipflops){
    vector<vector<Point>> rects;
    for(const auto& ff : flipflops){
        // cout << ff->getInstanceName() << " ";
        FeasibleSquare* fs = ff->getFeasible();
        vector<Point> squareOutline=fs->getDimondOutline();
        rects.emplace_back(squareOutline);
    }
    // cout << endl;

    if(rects.size()==1){
        // cout << "(" << rects[0][0].x << ", " << rects[0][0].y << ") (" << rects[0][1].x << ", " << rects[0][1].y << ") (" << rects[0][2].x << ", " << rects[0][2].y << ") (" << rects[0][3].x << ", " << rects[0][3].y << ")" << endl;
        return rects[0];
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
// cout << "find the intersection point of each edge" << endl;
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
// cout << "check if the points of the dimonds falls in other dimonds" << endl;
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
    // cout << "record the final points" << endl;
    // record the final points
    std::vector<Point> finalPoints;
    // cout << "intersectionPoints.size(): "<<intersectionPoints.size() << endl;
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
    if(finalPoints.size()!=4){
        // cout << "finalPoints.size(): "<<finalPoints.size() << endl;
        return vector<Point>();
    }
    sortPoints1(finalPoints);
    // cout << "(" << finalPoints[0].x << ", " << finalPoints[0].y << ") (" << finalPoints[1].x << ", " << finalPoints[1].y << ") (" << finalPoints[2].x << ", " << finalPoints[2].y << ") (" << finalPoints[3].x << ", " << finalPoints[3].y << ")" << endl;    
    return finalPoints;
}

// main_algo_2, decide "where to place the FFs"
void Algo::locationDecision(set<Instance*> flipflops, FlipFlop* FF){
    // for(const auto& ff : flipflops){
    //     // if(ff->getInstanceName() == "C108685")
    //         cout << ff->getInstanceName() << " ";
    // }
    // cout << endl;

    /* 1.calculate the median of each pin in FF candidates */
    // get all the pins of the set of flipflops 
    double xMedian = 0, yMedian = 0;
    vector<double> xList, yList;
    for(const auto& ff : flipflops){
        for(const auto& pin : ff->getPinList()){
            if(pin->getPinType() == 2)
                continue;
            xList.emplace_back(ff->getX() + pin->getX());
            yList.emplace_back(ff->getY() + pin->getY());
        }
    }

    // calculate the median of x and y
    sort(xList.begin(), xList.end());
    sort(yList.begin(), yList.end());
    // double xTotal = 0, yTotal = 0;
    // for(const auto& x : xList){
    //     xTotal += x;
    // }
    // for(const auto& y : yList){
    //     yTotal += y;
    // }
    // xMedian = xTotal / xList.size();
    // yMedian = yTotal / yList.size();
    if(xList.size() % 2 == 0){
        xMedian = (xList[xList.size()/2 - 1] + xList[xList.size()/2]) / 2.0;
    }
    else{
        xMedian = xList[xList.size()/2];
    }
    if(yList.size() % 2 == 0){
        yMedian = (yList[yList.size()/2 - 1] + yList[yList.size()/2]) / 2.0;
    }
    else{
        yMedian = yList[yList.size()/2];
    }
    // transfer pin medain to the left bottom of the FF
    double xFF = 0, yFF = 0;
    for(const auto& pin: FF->getPinList()){
        xFF += pin->getX();
        yFF += pin->getY();
    }
    xFF = xFF / FF->getPinList().size();
    yFF = yFF / FF->getPinList().size();
    xMedian -= xFF;
    yMedian -= yFF;
    // cout << "xMedian: " << xMedian << " yMedian: " << yMedian << endl;
    /* 1-2.find the closest site to the median position */
    // for y
    // int yId = 0;
    // while(_placementRowArray[yId]->getY() < yMedian && yId < _placementRowArray.size()){
    //     yId++;
    // }
    // if(yId == _placementRowArray.size()){
    //     yId = yId - 1;
    // }
    // else if(_placementRowArray[yId]->getY() - yMedian < yMedian - _placementRowArray[yId-1]->getY()){
        
    // }
    // else{
    //     yId = yId - 1;
    // }
    // // for x
    // int xId = (xMedian - _placementRowArray[yId]->getX()) / _placementRowArray[yId]->getsiteW();
    // if(xId != _placementRowArray[yId]->getSiteArray().size()-1){
    //     if(xMedian - _placementRowArray[yId]->getSiteArray()[xId]->getX() > _placementRowArray[yId]->getSiteArray()[xId+1]->getX() - xMedian){
    //         xId = xId + 1;
    //     }
    // }
    // // get target site
    // Site* targetSite = _placementRowArray[yId]->getSiteArray()[xId];
    /*=========================================================================================================================================*/
        
    /* 2.get the coordinates in the region from median to feasible region */
    vector<Point> overlapRegion = findOverlapFeasible(flipflops);
    // if(flipflops.find(_instanceArray[_instanceName2Id["C98470"]]) != flipflops.end()){
    //     for(auto f : flipflops){
    //         cout << f->getInstanceName() << endl;
    //         FeasibleSquare* fs = f->getFeasible();
    //         for(auto p : fs->getDimondOutline()){
    //             cout << "(" << p.x << ", " << p.y << ") ";
    //         }
    //         for(auto p : fs->getSquareOutline()){
    //             cout << "(" << p.x << ", " << p.y << ") ";
    //         }
    //         cout << endl;
    //     }
    //     cout << "xMedian: " << xMedian << " yMedian: " << yMedian << endl;
    //     for(auto p : overlapRegion){
    //         cout << "(" << p.x << ", " << p.y << ") ";
    //     }
    //     cout << endl;
    // }
    double maxX, maxY, minX, minY;
    int findsize=flipflops.size()*5+20;
    //cout<<"findsize: "<<findsize<<endl;
    if(overlapRegion.size() == 0){
        // TODO: merge 1 FF ??
        maxX = xMedian + _siteWidth * findsize;
        minX = xMedian - _siteWidth * findsize;
        maxY = yMedian + _siteHeight * findsize;
        minY = yMedian - _siteHeight * findsize;
        // cout << "no overlap region" << endl;
    }
    else{
        // double sin45 = sin(-M_PI/4), cos45 = cos(-M_PI/4);
        // vector<Point> overlapRegionR = {
        //     {cos45 * overlapRegion[0].x - sin45 * overlapRegion[0].y, sin45 * overlapRegion[0].x + cos45 * overlapRegion[0].y},
        //     {cos45 * overlapRegion[1].x - sin45 * overlapRegion[1].y, sin45 * overlapRegion[1].x + cos45 * overlapRegion[1].y},
        //     {cos45 * overlapRegion[2].x - sin45 * overlapRegion[2].y, sin45 * overlapRegion[2].x + cos45 * overlapRegion[2].y},
        //     {cos45 * overlapRegion[3].x - sin45 * overlapRegion[3].y, sin45 * overlapRegion[3].x + cos45 * overlapRegion[3].y}
        //     };
        // double medianXR = cos45 * xMedian - sin45 * yMedian, medianYR = sin45 * xMedian + cos45 * yMedian;

        // if((medianXR >= overlapRegionR[3].x && medianXR <= overlapRegionR[1].x) && (medianYR >= overlapRegionR[0].y && medianYR <= overlapRegionR[2].y)){
        //     // if median in feasible region => feasible region boundary
        //     maxX = overlapRegion[1].x;
        //     minX = overlapRegion[3].x;
        //     maxY = overlapRegion[2].y;
        //     minY = overlapRegion[0].y;
        // }
        // else{
        //     // if not => find the furthest point in feasible region boundary
        //     double farX = (abs(overlapRegion[3].x - xMedian) > abs(overlapRegion[1].x - xMedian)) ? overlapRegion[3].x : overlapRegion[1].x;
        //     double farY = (abs(overlapRegion[0].y - yMedian) > abs(overlapRegion[2].y - yMedian)) ? overlapRegion[0].y : overlapRegion[2].y;
        //     maxX = (farX > xMedian) ? farX : xMedian;
        //     minX = (farX < xMedian) ? farX : xMedian;
        //     maxY = (farY > yMedian) ? farY : yMedian;
        //     minY = (farY < yMedian) ? farY : yMedian;
        // }
        maxX = (overlapRegion[1].x + overlapRegion[3].x) / 2 + _siteWidth * findsize;
        minX = (overlapRegion[1].x + overlapRegion[3].x) / 2 - _siteWidth * findsize;
        maxY = (overlapRegion[0].y + overlapRegion[2].y) / 2 + _siteHeight * findsize;
        minY = (overlapRegion[0].y + overlapRegion[2].y) / 2 - _siteHeight * findsize;
        // cout << "overlap" << endl;
    }
    // pair<double, double> top, bottom, left, right;
    // double x1R, x2R, y1R, y2R; // rotated
    // // 2-1.get coordinate of boundary (maxX, maxY, minX, minY)
    // double sin45 = sin(-M_PI/4), cos45 = cos(-M_PI/4);
    // double siteXR = cos45 * targetSite->getX() + sin45 * targetSite->getY();
    // double siteYR = cos45 * targetSite->getY() - sin45 * targetSite->getX();
    // double maxX, maxY, minX, minY;
    // if((siteXR >= x1R && siteXR <= x2R) && (siteYR >= y1R && siteYR <= y2R)){ // if median in feasible region => feasible region boundary
    //     maxX = right.first;
    //     minX = left.first;
    //     maxY = top.second;
    //     minY = bottom.second;
    // }
    // else{ // if not => find the furthest point in feasible region boundary
    //     double farX = (abs(left.first - targetSite->getX()) > abs(right.first - targetSite->getX())) ? left.first : right.first;
    //     double farY = (abs(top.second - targetSite->getY()) > abs(bottom.second - targetSite->getY())) ? top.second : bottom.second;
    //     maxX = (farX > targetSite->getX()) ? farX : targetSite->getX();
    //     minX = (farX < targetSite->getX()) ? farX : targetSite->getX();
    //     maxY = (farY > targetSite->getY()) ? farY : targetSite->getY();
    //     minY = (farY < targetSite->getY()) ? farY : targetSite->getY();
    // }
    // if(flipflops.find(_instanceArray[_instanceName2Id["C83172"]]) != flipflops.end()){
        // cout << "maxX: " << maxX << " maxY: " << maxY << " minX: " << minX << " minY: " << minY << endl;

    // }
    
    // 2-2.find the closest site of the coordinate
    vector<pair<Site*, double>> feasibleSite;
    Site* curSite = NULL;
    double exceedDensity = 0;
    // for each site in the region
    // SHUYUN: calculate delta power and area
    for(const auto& ff : flipflops){
        int x1 = ff->getX(), y1 = ff->getY();
        int x2 = x1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth();
        int y2 = y1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
        subBinUsage(x1, x2, y1, y2);
        // exceedDensity+=subBinUsage(x1, x2, y1, y2);
    }    
    for(int k = 0; k < _placementRowArray.size(); k++){
    // for(const auto& placementrow : _placementRowArray){
        if(_placementRowArray[k]->getY() >= maxY || _placementRowArray[k]->getY() < minY){
            continue;
        }
        int startXid = (minX <= _placementRowArray[k]->getX()) ? 0 : (minX - _placementRowArray[k]->getX()) / _placementRowArray[k]->getsiteW();
        int endXid = (maxX - _placementRowArray[k]->getX()) / _placementRowArray[k]->getsiteW();
        if(endXid >= _placementRowArray[k]->getSiteArray().size()){
            endXid = _placementRowArray[k]->getSiteArray().size() - 1;
        }
        // if(flipflops.find(_instanceArray[_instanceName2Id["C83172"]]) != flipflops.end()){
            // cout << "startXid: " << startXid << " endXid: " << endXid << " ";
        // }

        for(int i = startXid; i <= endXid; i++){
            for(int j = 0; j < ceil(FF->getWidth() / _placementRowArray[k]->getsiteW()); j++){
                bool overlap = false;
                // cout << "i: " << i << " j: " << j << " ";
                // check overlap
                double tmp = FF->getHeight();
                for(int l = k; tmp > 0 && l < _placementRowArray.size(); l++){
                    // cout << "tmp: " << tmp << endl;
                    // if( _placementRowArray[l]->getSiteArray()[i+j]->getX() == 786420 && _placementRowArray[l]->getSiteArray()[i+j]->getY() == 562800){
                    //         if(_placementRowArray[l]->getSiteArray()[i+j]->getOcc()!=NULL)
                    //         cout << _placementRowArray[l]->getSiteArray()[i+j]->getOcc()->getInstanceName() << endl;
                    //         else cout << "N" << endl;
                    //     }
                    if(i + j < _placementRowArray[l]->getSiteArray().size() && _placementRowArray[l]->getSiteArray()[i+j]->getOcc() != NULL  && flipflops.find(_placementRowArray[l]->getSiteArray()[i+j]->getOcc()) == flipflops.end()){ 
                        // if(flipflops.find(_placementRowArray[k]->getSiteArray()[i+j]->getOcc()) == flipflops.end()){ // 不為要合併的FF
                        //     break;
                        // }
                        // if(_placementRowArray[l]->getSiteArray()[i+j]->getOcc()->getInstanceName() == "C111552" && flipflops.find(_instanceArray[_instanceName2Id["C82742"]])!=flipflops.end()){
                        //     cout << _placementRowArray[l]->getSiteArray()[i+j]->getX() << " " << _placementRowArray[l]->getSiteArray()[i+j]->getY() << " " << FF->getLibCellName() << " " << j << endl;
                        // }
                        i = i + j;
                        // cout << "     x ";
                        overlap = true;
                        break;
                    }
                    tmp -= _placementRowArray[k]->getsiteH();
                }
                if(overlap || tmp > 0){
                    break;
                }
                
                if(j == ceil(FF->getWidth() / _placementRowArray[k]->getsiteW()) - 1){
                    curSite = _placementRowArray[k]->getSiteArray()[i];
                    /* 3.calculate the cost for each site */
                    double oriCost = costBeforeMerge(flipflops, FF, curSite, 0);
                    double mergeCost = calAvailableSiteCost(flipflops, FF, curSite, 0, oriCost); // SHUYUN: add delta power and area
                    // cout << setw(6) << mergeCost << " ";
                    if(mergeCost >= 0){
                        feasibleSite.push_back(make_pair(curSite, mergeCost));
                    }
                }
                
            }
        }
        // k=k+1;
        // cout << endl;
    }
    // cout << "endFor" << endl;
    // sort
    for(const auto& ff : flipflops){
        int x1 = ff->getX(), y1 = ff->getY();
        int x2 = x1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth();
        int y2 = y1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
        addBinUsage(x1, x2, y1, y2);
    }
    sort(feasibleSite.begin(), feasibleSite.end(), [](const std::pair<Site*, double>& a, const std::pair<Site*, double>& b) {
        return a.second < b.second;
    });
    /*============================================================================================================================*/
    
    /* 4.place in some nonoverlapping sites with the lowest cost */
    if(feasibleSite.size() > 0){
        // merge FF to the site
        string new_ins_name = "C" + to_string(_instanceArray.size()+1);
       /* if(new_ins_name=="C119092"){
            cout<<"C119092"<<endl;
            for(const auto& ff: feasibleSite){
                cout<<ff.second<<" ";
            }
            cout<<endl;
        }*/
        string lib_cell_name_1b = FF->getLibCellName();
        Instance* merge = new Instance(new_ins_name, lib_cell_name_1b, feasibleSite[0].first->getX(), feasibleSite[0].first->getY());
        MergeInstance(flipflops, merge, FF, feasibleSite[0].first);

        // updateSlack(flipflops, FF, feasibleSite[0].first, merge);
        /* 5.update the site occ */
        for(const auto& ff : flipflops){
            int x1 = ff->getX(), y1 = ff->getY();
            int x2 = x1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getWidth();
            int y2 = y1 + _ffArray[_ffName2Id[ff->getLibCellName()]]->getHeight();
            subBinUsage(x1, x2, y1, y2);
            for(const auto& s : ff->getSite()){
                // if(s->getOcc() != NULL && s->getOcc()->getInstanceName() == "C111552"){
                //     cout << ff->getInstanceName() << endl;
                // }
                s->setOcc(NULL);
            }
            ff->getSite().clear();
        }

        for(const auto& placementrow:_placementRowArray){
            if(placementrow->getY() >= merge->getY() && placementrow->getY() < merge->getY() + FF->getHeight()){
                int startXid = (merge->getX() - placementrow->getX()) / placementrow->getsiteW();
                int siteNum = ceil(FF->getWidth() / placementrow->getsiteW());
                for(int i = 0; i < siteNum; i++){
                    if(startXid + i < placementrow->getSiteArray().size()){
                        // cout << i << endl;
                        placementrow->getSiteArray()[startXid+i]->setOcc(merge);
                        merge->addSite(placementrow->getSiteArray()[startXid+i]);
                        
                    }
                    
                }
            }           
        }
        addBinUsage(merge->getX(), merge->getX() + FF->getWidth(), merge->getY(), merge->getY() + FF->getHeight());
        // feasibleSite[0].first->setOcc(merge);
        // merge->setSite(feasibleSite[0].first);
        merge->setlock(true);

            // if(new_ins_name == "C108695"){
            //     for(const auto& pin: merge->getPinList()){
            //         cout<<"pin name: "<<pin->getPinName()<<endl;
            //     }
            // }
        // if(merge->getInstanceName() == "C111552" || merge->getInstanceName() == "C111941"){
        //     cout << merge->getInstanceName() << endl;
        // }
        // cout << merge->getInstanceName() << endl;

    }
    else{
        // original position
        // cout << flipflops.size() << endl;
        int nextFFSize = 0;

        ///////////////////////////////////////////////
        nextFFSize = _previousSize[flipflops.size()];
        ///////////////////////////////////////////////
        // while(_lowestCostFFMap.find(nextFFSize) == _lowestCostFFMap.end()){ // && selectedFF.size() != 8
        //     nextFFSize--;
        // }
        set<Instance*> tmpFF=flipflops;
        if (nextFFSize !=0 && _lowestCostFFMap.find(nextFFSize) != _lowestCostFFMap.end()) {   
            auto it = tmpFF.begin();
            while (it != tmpFF.end()) {
                // cout<<"拆"<<nextFFSize<<endl;
                set<Instance*> subff;
                for (int i = 0; i < nextFFSize && it != flipflops.end(); i++) {
                    subff.insert(*it);
                    // cout<<(*it)->getInstanceName()<<endl;
                    it++;
                }
                if (!subff.empty()) {
                    string new_ins_name = "C" + to_string(_instanceArray.size() + 1);
                    locationDecision(subff, _lowestCostFFMap[subff.size()]);
                    for (const auto& ff : subff) {
                        flipflops.erase(ff);
                    }
                }
            }
        }
        for(const auto& ff : flipflops){
            string new_ins_name = "C" + to_string(_instanceArray.size()+1);
            FlipFlop* FF1 = _ffArray[_ffName2Id[ff->getLibCellName()]]; // just for 1 bit
            string lib_cell_name_1b = FF1->getLibCellName();
            Instance* merge = new Instance(new_ins_name, lib_cell_name_1b, ff->getX(), ff->getY());
            set<Instance*> ffSet;
            ffSet.insert(ff);
            for(const auto& s : ff->getTimingSlack()){
                /*if(s.second < 0)
                    cout<<s.first<<" "<<s.second<<endl;*/
            }
            MergeInstance(ffSet, merge, FF1, ff->getSite()[0]);
            /*cout<<merge->getInstanceName()<<endl;
            for(const auto& s : merge->getTimingSlack()){
                if(s.second < 0)
                    cout<<s.first<<" "<<s.second<<endl;
            }*/
            // updateSlack(ffSet, FF1, ff->getSite()[0], merge);
           
            /* 5.update the site occ */
            for(const auto& s : ff->getSite()){
                
                s->setOcc(merge);
                merge->addSite(s);
                // if(merge->getInstanceName() == "C111552"){
                //     cout << s->getX() << " " << s->getY() << " " << s->getOcc()->getInstanceName() << endl;
                // }
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // addBinUsage(merge->getX(), merge->getX() + FF1->getWidth(), merge->getY(), merge->getY() + FF1->getHeight());
            // subBinUsage(ff->getX(), ff->getX() + FF1->getWidth(), ff->getY(), ff->getY() + FF1->getHeight());
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            // for(const auto& placementrow:_placementRowArray){
            //     if(placementrow->getY() >= merge->getY() && placementrow->getY() < merge->getY() + FF->getHeight()){
            //         int startXid = (merge->getX() - placementrow->getX()) / placementrow->getsiteW();
            //         int siteNum = ceil(FF->getWidth() / placementrow->getsiteW());
            //         for(int i = 0; i < siteNum; i++){
            //             if(startXid + i < placementrow->getSiteArray().size()){
            //                 placementrow->getSiteArray()[startXid+i]->setOcc(merge);
            //                 merge->addSite(placementrow->getSiteArray()[startXid+i]);
            //             }
            //         }
            //     }           
            // }
            ff->getSite().clear();
            
            // ff->getSite()->setOcc(merge);
            merge->setlock(true);

            // if(new_ins_name == "C108695"){
            //     for(const auto& pin: merge->getPinList()){
            //         cout<<"pin name: "<<pin->getPinName()<<endl;
            //     }
            // }
            // if(merge->getInstanceName() == "C111552" || merge->getInstanceName() == "C111941"){
            //     cout << merge->getInstanceName() << endl;
            // }
            
        }
        

    }
    // cout << "end" << endl;
    /*============================================================================================================================*/
}