#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include "common.hpp"
#include <queue>
using std::priority_queue;

class Scheduler
{
private:
    typedef pair<double, pair<string, string>> Arrange;
    // pair<transmit time(2),pair<slots(DC1),tasks(tA1)>>
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
    double count_time(string task_name, string which_slot)
    {
        vector<pair<string, double>> resource_requires =
            (graph->require)[task_name];
        double max = 0;
        for (pair<string, double> resource : resource_requires)
        {
            string resource_position = graph->resource_loc[resource.first];
            double bandwidth = (graph->edges)[resource_position][which_slot];
            double cost = resource.second * bandwidth;
            if (cost > max)
                max = cost;
        }
        return max;
    }
    vector<Arrange> getGreedy()
    {
        priority_queue<Arrange, vector<Arrange>, compare> mapping;
        for (unordered_set<string>::iterator task_iter = ready_queue.begin(); task_iter != ready_queue.end(); ++task_iter)
        {
            for (unordered_map<string, pair<int, unordered_set<string>>>::iterator
                     slot = graph->slots.begin();
                 slot != graph->slots.end(); ++slot)
            {
                if ((slot->second).first != 0)
                { //capacity>0
                    Arrange this_method;
                    this_method.first = count_time(*task_iter, slot->first);
                    this_method.second.second = *task_iter;
                    this_method.second.first = slot->first;
                    mapping.push(this_method);
                }
            }
        }
        vector<Arrange> assignments;
        Arrange assignment;
        unordered_map<string, int> used;
        //some slots of graph arranged just now
        while (!mapping.empty())
        {
            assignment = mapping.top();
            mapping.pop();
            string task = assignment.second.second;
            string DC = assignment.second.first;

            if (ready_queue.find(task) != ready_queue.end())
            { //tasks in ready_queue
                int DC_used;
                unordered_map<string, int>::iterator f = used.find(DC);
                if (f == used.end())
                    DC_used = 0;
                else
                    DC_used = f->second;
                if (graph->slots[DC].first > graph->slots[DC].second.size() + DC_used)
                {
                    //slots has capacity

                    //arrange successfully
                    ready_queue.erase(task);
                    //pop from ready_queue
                    assignments.push_back(assignment);
                    if (f == used.end())
                    { //DC not in used
                        used[DC] = 1;
                    }
                    else
                    {
                        ++used[DC];
                    }
                }
            }
        }
        return assignments;
    }
    vector<Arrange> getRandom()
    {
        vector<pair<string, int>> available_slot;
        int number;
        for (pair<string, pair<int, unordered_set<string>>> slot : graph->slots)
        {
            number = slot.second.first - slot.second.second.size();
            if (number != 0)
            {
                available_slot.push_back(make_pair(slot.first, number));
            }
        }
        vector<Arrange> assignments;
        Arrange assignment;
        //unordered_set<string>::iterator ready_queue_begin=ready_queue.
        while ((!ready_queue.empty()) &&
               (!available_slot.empty()))
        {
            string task = *ready_queue.begin();
            int DC_index = randInt(0, available_slot.size() - 1);
            vector<pair<string, int>>::iterator iter = available_slot.begin() + DC_index;
            string DC = iter->first;
            assignment.first = count_time(task, DC);
            assignment.second.first = DC;
            assignment.second.second = task;
            assignments.push_back(assignment);
            if (!(--iter->second))
            {
                available_slot.erase(iter);
            }
            ready_queue.erase(task);
        }
        return assignments;
    }

public:
    enum SchedType
    {
        GREEDY,
        RANDOM
    } sched_type;

    Scheduler(shared_ptr<Graph> g) : graph(g) {}
    // get new tasks from DAG
    void sumbitTasks(unordered_set<string> tasks)
    {
        for (string each_task : tasks)
        {
            ready_queue.insert(each_task);
        }
        //add task into ready_queue
    }

    // legacy
    // void sumbitTasks(unordered_set<string> tasks)
    //{
    //    for (string each_task : tasks)
    //    {
    //        ready_queue.insert(each_task);
    //        //add task into ready_queue
    //        for (unordered_map<string, pair<int, unordered_set<string>>>::iterator
    //                 slot = graph->slots.begin();
    //             slot != graph->slots.end(); ++slot)
    //        {
    //            if ((slot->second).first != 0)
    //            { //capacity>0
    //                Arrange this_method;
    //                this_method.first = count_time(each_task, slot->first);
    //                this_method.second.second = each_task;
    //                this_method.second.first = slot->first;
    //                mapping.push(this_method);
    //            }
    //        }
    //    }
    //}

    // update current resources from simulator
    // schedule tasks to slots
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Arrange> getScheduled()
    {
        switch (sched_type)
        {
        case GREEDY:
            return getGreedy();
        case RANDOM:
            return getRandom();
        }
    }
};

#endif