#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <windows.h>
#include <conio.h>

using namespace std;

#define START_INTERSECTION 0
#define FILE_NAME "district1data(5).txt"
#define UPDATE_TIME 1000

struct Road {
    int from;
    int to;
    double weight;
    int priority;
    bool plowed;
    double realDistance;
    Road(int f, int t, double d, int p, double w) {
        from = f;
        to = t;
        priority = p;
        weight = w;
        realDistance = d;
        plowed = false;
    }
};

double realTotalDist = 0;

struct Graph {
    vector<vector<int>> adjList;
    vector<vector<double>> adjMatrix;
    vector<Road> roads;
    int numNodes = 0;
    void makeAdjList() {
        adjList.resize(numNodes + 1, vector<int>());
        for (int id = 0; id < roads.size(); id++) {
            adjList[roads[id].from].push_back(id);
            adjList[roads[id].to].push_back(id);
        }
    }
    void makeAdjMatrix() {
        adjMatrix.resize(numNodes + 1, vector<double>(numNodes + 1, -1));
        for (int id = 0; id < roads.size(); id++) {
            adjMatrix[roads[id].from][roads[id].to] = id;
            adjMatrix[roads[id].to][roads[id].from] = id;
        }
    }
    Graph(string filename) {
        ifstream file;
        file.open(filename);
        while (!file.eof()) {
            int from;
            int to;
            double distance;
            int priority;
            double w;
            file >> from >> to >> distance >> priority >> w;
            numNodes = max(numNodes, from);
            numNodes = max(numNodes, to);
            roads.push_back(Road(from, to, distance, priority, w));
        }
        makeAdjList();
        makeAdjMatrix();
    }
    int getN() {
        return roads.size();
    }

};
double getTotalDist(Graph g, int start, bool greedy, bool print);


//check if given intersection has a road with some priority which is not plowed yet
vector<int> checkIntersection(int idx, int prio, Graph& g, bool greedy) {
    vector<int> possible;
    for (int roadIndex : g.adjList[idx]) {
        if (!g.roads[roadIndex].plowed) {
            possible.push_back(roadIndex);
        }
    }
    return possible;
}

double printPath(int start, int end, vector<int>& prev, Graph& g, bool greedy, bool print) {
    double toalDistance = 0;
    vector<int> path;
    while (end != start) {
        path.push_back(end);
        end = prev[end];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    for (int i = 0; i < path.size() - 1; i++) {
        int id = g.adjMatrix[path[i]][path[i + 1]];
        if (!greedy) {
            if (print) {
                cout << path[i] << "->" << path[i + 1] << endl;
                realTotalDist += g.roads[id].realDistance;
            }
            g.roads[id].plowed = true;
        }
        toalDistance += g.roads[id].weight;
    }
    return toalDistance;
}

int getOtherEnd(Graph& g, int currRoad, int currPoint) {
    if (g.roads[currRoad].from == currPoint) return g.roads[currRoad].to;
    return g.roads[currRoad].from;

}

int findNearest(int currInd, int prio, Graph& g, double& distanceCovered, bool greedy, bool print) {
    //return where we end up
    vector<int> minDist(g.numNodes + 1, INT_MAX);
    vector<int> prev(g.numNodes + 1, -1);

    priority_queue<pair<int, int>, vector<pair<int, int>>, less<pair<int, int>>> q;
    int start = currInd;
    q.push({ 0, currInd });
    minDist[currInd] = 0;
    distanceCovered = 0;

    while (!q.empty()) {
        int currDist = q.top().first;
        int currIntersection = q.top().second;
        vector<int> possibleRoad = checkIntersection(currIntersection, prio, g, greedy);
        q.pop();
        int end = currIntersection;

        if (possibleRoad.size() > 0) {
            if (greedy) {
                int currRoad = possibleRoad[rand() % (int)possibleRoad.size()];
                distanceCovered += printPath(start, end, prev, g, greedy, print);
                distanceCovered += g.roads[currRoad].weight;
                g.roads[currRoad].plowed = true;

                if (g.roads[currRoad].from == end) {
                    return g.roads[currRoad].to;
                }
                else {
                    return g.roads[currRoad].from;
                }
            }
            else {

                int bestRoad = -1;
                double bestDistance = INT_MAX;
                double initPath = printPath(start, end, prev, g, greedy, print);

                for (int currRoad : possibleRoad) {
                    double currAns = 0;
                    currAns += g.roads[currRoad].weight;
                    bool initState = g.roads[currRoad].plowed;
                    g.roads[currRoad].plowed = true;
                    currAns += getTotalDist(g, getOtherEnd(g, currRoad, end), true, print);
                    g.roads[currRoad].plowed = initState;

                    if (currAns < bestDistance) {
                        bestRoad = currRoad;
                        bestDistance = currAns;
                    }
                }
                distanceCovered = initPath + g.roads[bestRoad].weight;
                g.roads[bestRoad].plowed = true;

                if (g.roads[bestRoad].from == end) {
                    if (print) {
                        cout << g.roads[bestRoad].from << "->" << g.roads[bestRoad].to << endl;
                        realTotalDist += g.roads[bestRoad].realDistance;
                    }
                }
                else {
                    if (print) {
                        cout << g.roads[bestRoad].to << "->" << g.roads[bestRoad].from << endl;
                        realTotalDist += g.roads[bestRoad].realDistance;

                    }
                }
                return getOtherEnd(g, bestRoad, end);
            }
        }



        for (int nextId : g.adjList[currIntersection]) {
            int nextIntersectoin = getOtherEnd(g, nextId, currIntersection);
            int nextDist = g.roads[nextId].weight;

            if (currDist + nextDist < minDist[nextIntersectoin]) {
                prev[nextIntersectoin] = currIntersection;
                minDist[nextIntersectoin] = currDist + nextDist;
                q.push({ currDist + nextDist , nextIntersectoin });
            }
        }
    }


    return -1;

}

double getTotalDist(Graph g, int start, bool greedy, bool print) {

    int curr = start;
    double totalDist = 0;
    while (true) {
        double distCovered;
        int arrive = findNearest(curr, 0, g, distCovered, greedy, print);
        if (arrive == -1) {
            break;
        }
        totalDist += distCovered;
        curr = arrive;
    }


    return totalDist;
}

int main()
{


    srand(time(0));
    Graph g(FILE_NAME);
    double totalWeight = 0;
    for (int i = 0; i < g.roads.size(); i++) {
        totalWeight += g.roads[i].weight;
    }
    cout << "Total Weights = " << totalWeight << endl;

    double minDist = DBL_MAX;
    int bestSeed;
    int best = INT_MAX;
    for (int i = 1;true; i++) {
        srand(i);
        double currAns = getTotalDist(g, START_INTERSECTION, false, false);
        if (currAns < minDist) {
            minDist = currAns;
            bestSeed = i;
        }
        if (GetAsyncKeyState(0x58)) {
            break;
        }
        if (i % UPDATE_TIME == 0) {
            cout << "############" << endl;
            cout << "Curr Seed: " << i << endl;
            cout << "Best Seed: " << bestSeed << endl;
            cout << "CurrAns / TotalWeight = " << minDist / totalWeight << endl;
        }
    }

    cout << bestSeed << endl;
    cout << minDist << endl;

    srand(bestSeed);
    getTotalDist(g, START_INTERSECTION, false, true);
    cout << realTotalDist << endl;

}
//PLOW ROADS AS YOU GO TO DESTINATION

//PRESS 'X' TO TERMINATE AND SEE RESULT
