#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

struct Graph
{
    // resources held by "DCi"
    // e.g. resources["DC1"]={"A1","A2","A3"}
    unordered_map<string, vector<string>> resources;

    // adjacent matrix
    // e.g. edges["DC1"]["DC2"]=1/100
    //  if bandwidth between DC1 and DC2 is 100
    unordered_map<string, unordered_map<string, double>> edges;

    // slots of "DCi"
    // e.g. slots["DC1"]={2,{"tA1"}}
    //  the capacity of DC1 is 2 and tA1 is running
    unordered_map<string, pair<int, vector<string>>> slots;
};

#endif
