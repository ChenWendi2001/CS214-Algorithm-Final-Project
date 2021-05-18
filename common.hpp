#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <iomanip>
#include <cmath>

using std::make_pair;
using std::make_shared;
using std::pair;
using std::priority_queue;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct Graph
{
    // resources held by "DCi"
    // e.g. resources["DC1"]={"A1","A2","A3"}
    unordered_map<string,
                  unordered_set<string>>
        resources;

    // adjacent matrix
    // e.g. edges["DC1"]["DC2"]=1/100
    //  if bandwidth between DC1 and DC2 is 100
    unordered_map<string,
                  unordered_map<string, double>>
        edges;

    // slots of "DCi"
    // e.g. slots["DC1"]={2,{"tA1"}}
    //  the capacity of DC1 is 2 and tA1 is running
    unordered_map<string,
                  pair<int, unordered_set<string>>>
        slots;

    void printStatus()
    {
        for (const auto &DC : slots)
        {
            std::cout << std::setw(4)
                      << DC.first << '|';
            const auto tasks = DC.second.second;
            for (const auto &task : tasks)
                std::cout << std::setw(4)
                          << task << '|';
            for (int i = 0; i < DC.second.first - tasks.size(); ++i)
                std::cout << std::setw(4)
                          << ' ' << '|';
            std::cout << std::endl;
        }
    }
};

// print error message and terminate
void printError(string msg)
{
    std::cout << "Error: " << msg << std::endl;
    std::exit(0);
}

// print warning message
void printWarning(string msg)
{
    std::cout << "Warning: " << msg << std::endl;
}

#endif
