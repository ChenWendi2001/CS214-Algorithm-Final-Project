#ifndef __SIMULATOR_HPP__
#define __SIMULATOR_HPP__

#include "common.hpp"

class Simulator
{
private:
    // DCi and its slots
    shared_ptr<Graph> graph;

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

public:
    Simulator()
    {
        current_time = 0;
    }

    // whether there are running tasks
    // note: check this before tick time
    bool isEmpty()
    {
        bool ret = true;
        for (const auto &it : graph->slots)
        {
            const auto &slot = it.second;
            ret &= slot.second.empty();
        }
        return ret;
    }

    // only initialized once
    // read task's run time from file
    void readTaskTime(string file_name = "task_time.txt")
    {
        std::ifstream fin(file_name);
        if (!fin.is_open())
            printError(file_name + " not found!");
        string name;
        double t;
        while (fin >> name >> t)
            run_time[name] = t;
    }

    // only initialized once
    void updateGraph(shared_ptr<Graph> graph)
    {
        this->graph = graph;
    }

    double getTime()
    {
        return current_time;
    }

    // current_time += t
    void tickTime(double t)
    {
        current_time += t;
    }

    // forward time to next completion
    void forwardTime()
    {
        current_time = Q.top().first;
    }

    // get scheduled tasks from scheduler
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    void updateScheduled(vector<pair<double,
                                     pair<string, string>>>
                             scheduled_tasks)
    {
        for (const auto &it : scheduled_tasks)
        {
            string DC = it.second.first;
            string task = it.second.second;
            auto slots = graph->slots[DC];
            unordered_set<string> &tasks = graph->slots[DC].second;

            if (graph->slots[DC].first <= tasks.size())
                printError("No Available Slots on " + DC);

            tasks.insert(task);
            locates[task] = DC;
            double finish_time = current_time;
            finish_time += it.first;
            finish_time += run_time[task];
            Q.push(make_pair(finish_time, task));
        }
    }

    // get finished tasks and update DAG
    // e.g. {"tA1","tA2"} when these tasks are finished
    vector<string> getFinished()
    {
        vector<string> finish_tasks;
        // get finished tasks from Q
        static const double eps = 1e-8;
        while (!Q.empty() &&
               fabs(Q.top().first - current_time) < eps)
        {
            string task = Q.top().second;
            graph->slots[locates[task]].second.erase(task);
            finish_tasks.emplace_back(task);
            Q.pop();
        }
        return finish_tasks;
    }

    // print current status
    void printStatus()
    {
        graph->printStatus();
    }
};

#endif