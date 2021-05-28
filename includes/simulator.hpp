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
    typedef pair<double, string> Task;
    priority_queue<Task, vector<Task>, std::greater<Task>> Q;

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
        if (Q.empty())
            printError("Q is Empty!");
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
            auto &slot = graph->slots[DC];
            unordered_set<string> &tasks = slot.second;

            if (slot.first <= tasks.size())
                printError("No Available Slots on " + DC);

            tasks.insert(task);
            locates[task] = DC;
            double finish_time = current_time;
            finish_time += it.first;
            finish_time += graph->run_time[task];
            Q.push(make_pair(finish_time, task));
        }
    }

    // get finished tasks and update DAG
    // e.g. {{"tA1",9.5},{"tA2",5}} when these tasks are finished
    vector<pair<string, double>> getFinished()
    {
        vector<pair<string, double>> finish_tasks;
        // get finished tasks from Q
        static const double eps = 1e-8;
        while (!Q.empty() &&
               Q.top().first < current_time + eps)
        {
            string task = Q.top().second;
            double finish_time = Q.top().first;
            graph->slots[locates[task]].second.erase(task);

            // update job finish time
            string job = graph->which_job[task];
            auto &tasks = graph->job_task[job];
            tasks.erase(task);
            if (tasks.empty())
                graph->finish_time[job] = finish_time;

            graph->task_span[task] = make_pair(finish_time - graph->run_time[task],
                                               graph->run_time[task]);
            finish_tasks.emplace_back(make_pair(task, finish_time));
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