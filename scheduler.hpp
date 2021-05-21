#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include "common.hpp"
#include <queue>
using std::priority_queue;

class Scheduler
{
private:
    typedef pair<double, pair<string, string>> Arrange;
    // pair<transmit time(2),pair<task(tA1),slots(DC1)>>

    //the graph pointer
    shared_ptr<Graph> graph;
    //tasks are available but have not been scheduled
    class compare
    {
    public:
        bool operator()(const Arrange &a,
                        const Arrange &b)
        {
            return a.first > b.first;
        }
    };
    unordered_set<string> ready_queue;
    priority_queue<Arrange, vector<Arrange>, compare> mapping;
    double count_time(string task_name, string which_slot)
    {
        vector<pair<string, double>> resource_requires =
            (graph->require)[task_name];
        double max = 0;
        for (pair<string, double> resource : resource_requires)
        {
            string resource_position = graph->resource_loc[resource.first];
            double bandwidth = (graph->edges)[resource_position][which_slot];
            double cost = resource.second / bandwidth;
            if (cost > max)
                max = cost;
        }
        return max;
    }

    // only return one
    vector<Arrange> getGreedy()
    {
        vector<Arrange> assignments;
        Arrange assignment;
        while (!mapping.empty())
        {
            assignment = mapping.top();
            mapping.pop();
            string task = assignment.second.second;
            string DC = assignment.second.first;

            if (ready_queue.find(task) != ready_queue.end())
            { //tasks in ready_queue
                if (graph->slots[DC].first != graph->slots[DC].second.size())
                {
                    //slots has capacity

                    //arrange successfully
                    ready_queue.erase(task);
                    //pop from ready_queue
                    assignments.push_back(assignment);
                    return assignments;
                }
            }
        }
        return assignments; //empty,exceptions
    }

public:
    Scheduler(shared_ptr<Graph> g) : graph(g) {}
    // get new tasks from DAG
    void sumbitTasks(unordered_set<string> tasks)
    {
        for (string each_task : tasks)
        {
            ready_queue.insert(each_task);
            //add task into ready_queue
            for (unordered_map<string, pair<int, unordered_set<string>>>::iterator
                     slot = graph->slots.begin();
                 slot != graph->slots.end(); ++slot)
            {
                if ((slot->second).first != 0)
                { //capacity>0
                    Arrange this_method;
                    this_method.first = count_time(each_task, slot->first);
                    this_method.second.second = each_task;
                    this_method.second.first = slot->first;
                    mapping.push(this_method);
                }
            }
        }
    }

    // update current resources from simulator
    // schedule tasks to slots
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Arrange> getScheduled()
    {
        return getGreedy();
    }
};

#endif