#ifndef __DAG_HPP__
#define __DAG_HPP__

#include "common.hpp"

class DAG
{
public:
    // update DAG with finished tasks
    //  from simulator
    void updateDAG(vector<string> finished_tasks)
    {
    }

    // return new tasks that can be submit
    // e.g. {"tA1","tA2"}
    //  put these tasks to scheduler
    vector<string> getSubmit()
    {
    }
};

#endif