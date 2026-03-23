#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <cstring>

using namespace std;

int generateKey(const vector<int>& arr1, const vector<int>& arr2) {
    int key = 0;
    for (int idx = 0; idx < 6; idx++) {
        key = key * 6 + arr1[idx];
    }
    for (int idx = 0; idx < 6; idx++) {
        key = key * 6 + arr2[idx];
    }
    return key;
}

void parseKey(int key, vector<int>& arr1, vector<int>& arr2) {
    for (int idx = 5; idx >= 0; idx--) {
        arr2[idx] = key % 6;
        key /= 6;
    }
    for (int idx = 5; idx >= 0; idx--) {
        arr1[idx] = key % 6;
        key /= 6;
    }
}

int main() {
    int cnt1, cnt2, steps;
    cin >> cnt1 >> cnt2 >> steps;
    
    vector<int> state1(6, 0), state2(6, 0);
    
    for (int idx = 0; idx < cnt1; idx++) {
        int val;
        cin >> val;
        state1[val-1]++;
    }
    
    for (int idx = 0; idx < cnt2; idx++) {
        int val;
        cin >> val;
        state2[val-1]++;
    }
    
    map<int, double> curProb;
    curProb[generateKey(state1, state2)] = 1.0;
    
    for (int turn = 0; turn < steps; turn++) {
        map<int, double> nxtProb;
        
        for (map<int, double>::iterator iter = curProb.begin(); iter != curProb.end(); ++iter) {
            int curKey = iter->first;
            double curVal = iter->second;
            
            if (curVal == 0) continue;
            
            vector<int> curState1(6), curState2(6);
            parseKey(curKey, curState1, curState2);
            
            int totalUnits = 0;
            for (int idx = 0; idx < 6; idx++) {
                totalUnits += curState1[idx] + curState2[idx];
            }
            
            if (totalUnits == 0) {
                nxtProb[curKey] += curVal;
                continue;
            }
            
            for (int idx = 0; idx < 6; idx++) {
                if (curState1[idx] > 0) {
                    vector<int> newState1 = curState1;
                    vector<int> newState2 = curState2;
                    
                    if (idx == 0) {
                        newState1[0]--;
                    } else {
                        newState1[idx]--;
                        newState1[idx-1]++;
                    }
                    
                    double weight = (double)curState1[idx] / totalUnits;
                    nxtProb[generateKey(newState1, newState2)] += curVal * weight;
                }
                
                if (curState2[idx] > 0) {
                    vector<int> newState1 = curState1;
                    vector<int> newState2 = curState2;
                    
                    if (idx == 0) {
                        newState2[0]--;
                    } else {
                        newState2[idx]--;
                        newState2[idx-1]++;
                    }
                    
                    double weight = (double)curState2[idx] / totalUnits;
                    nxtProb[generateKey(newState1, newState2)] += curVal * weight;
                }
            }
        }
        
        curProb = nxtProb;
    }
    
    double result = 0.0;
    for (map<int, double>::iterator iter = curProb.begin(); iter != curProb.end(); ++iter) {
        int curKey = iter->first;
        double curVal = iter->second;
        
        vector<int> curState1(6), curState2(6);
        parseKey(curKey, curState1, curState2);
        
        bool allEmpty = true;
        for (int idx = 0; idx < 6; idx++) {
            if (curState2[idx] > 0) {
                allEmpty = false;
                break;
            }
        }
        
        if (allEmpty) {
            result += curVal;
        }
    }
    
    cout << fixed << setprecision(4) << result << endl;
    
    return 0;
}