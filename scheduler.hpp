#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include "common.hpp"

class Scheduler
{
public:
    // get new tasks from DAG
    void sumbitTasks(vector<string> tasks)
    {
    }

    // update current resources from simulator
    void updateResource(Graph)
    {
    }

    // schedule tasks to slots
    // e.g. {{"DC1","tA1"}}
    //  assign tA1 to DC1
    vector<pair<string, string>> getScheduled()
    {
    }
};

#endif