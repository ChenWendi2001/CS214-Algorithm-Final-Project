#ifndef __SIMULATOR_HPP__
#define __SIMULATOR_HPP__

#include "common.hpp"

class Simulator
{
private:
    static const double eps = 1e-8;

    // DCi and its slots
    Graph graph;

    // current time of simulator
    double current_time;

    // finish time and name
    // e.g. {4.5, "tA1"}
    priority_queue<pair<double, string>> Q;

    // task's run time
    // e.g. {"tA1", 2}
    //  tA1 takes 2s to execute
    unordered_map<string, double> run_time;

    // location of task
    // e.g. locates["tA1"]="DC1"
    unordered_map<string, string> locates;

    // update current time to next finish
    void updateTime()
    {
    }

public:
    Simulator()
    {
        current_time = 0;
        getTaskTime();
    }

    void getTaskTime(string file_name = "task_time.txt")
    {
        std::ifstream fin(file_name);
        if (!fin.is_open())
            printError(file_name + " not found!");
        string name;
        double t;
        while (fin >> name >> t)
            run_time[name] = t;
    }

    double getTime()
    {
        return current_time;
    }

    // get scheduled tasks from scheduler
    // e.g. {{"DC1","tA1"}}
    //  assign tA1 to DC1
    void getScheduled(vector<pair<string, string>> scheduled_tasks)
    {
        for (const auto &it : scheduled_tasks)
        {
        }
    }

    // return current resources to scheduler
    Graph getResources()
    {
        return graph;
    }

    // get finished tasks and update DAG
    // e.g. {"tA1","tA2"} when these tasks are finished
    vector<string> updateDAG()
    {
        updateTime();
        vector<string> finish_tasks;
        // get finished tasks from Q
        while (!Q.empty() &&
               fabs(Q.top().first - current_time) < eps)
        {
            finish_tasks.emplace_back(Q.top().second);
            Q.pop();
        }
        return finish_tasks;
    }

    // print current status
    void printStatus()
    {
        graph.printStatus();
    }
};

#endif