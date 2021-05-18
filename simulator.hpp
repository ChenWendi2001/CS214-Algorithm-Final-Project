#include "common.hpp"

class Simulator
{
private:
    Graph graph;

public:
    // get scheduled tasks from scheduler
    // e.g. {{"DC1","tA1"}}
    //  assign tA1 to DC1
    void getScheduled(vector<pair<string, string>>)
    {
    }

    // return current resources to scheduler
    Graph getResources()
    {
    }

    // use finished tasks to update DAG
    // e.g. {"tA1","tA2"} when these tasks are finished
    vector<string> updateDAG()
    {
    }
};