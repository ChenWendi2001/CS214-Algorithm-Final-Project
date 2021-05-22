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

    // the graph pointer
    shared_ptr<Graph> graph;

    class ArrangeCompare
    {
    public:
        bool operator()(const Arrange &a,
                        const Arrange &b)
        {
            return a.first > b.first;
        }
    };

    // tasks are available but have not been scheduled
    unordered_set<string> ready_queue;

private:
    double count_time(const string &task_name,
                      const string &which_slot)
    {
        vector<pair<string, double>>
            resource_requires = graph->require[task_name];
        double mx = 0;
        for (const auto &resource : resource_requires)
        {
            // resource_position
            string loc = graph->resource_loc[resource.first];
            double bandwidth = graph->edges[loc][which_slot];
            double cost = resource.second * bandwidth;
            mx = std::max(mx, cost);
        }
        return mx;
    }

    // assign tasks using greedy approach
    vector<Arrange> getGreedy()
    {
        priority_queue<Arrange,
                       vector<Arrange>, ArrangeCompare>
            Q;
        // unordered_set<string>::iterator
        for (const auto &task_iter : ready_queue)
        {
            // unordered_map<string, pair<int, unordered_set<string>>>::iterator
            for (const auto &slot : graph->slots)
            {
                if (slot.second.second.size() <
                    slot.second.first) // capacity>0
                {
                    Arrange this_method;
                    this_method.first = count_time(task_iter, slot.first);
                    this_method.second.second = task_iter;
                    this_method.second.first = slot.first;
                    Q.push(this_method);
                }
            }
        }
        vector<Arrange> assignments;
        unordered_map<string, int> used;
        // some slots of graph arranged just now
        while (!Q.empty())
        {
            Arrange assignment = Q.top();
            Q.pop();
            string task = assignment.second.second;
            string DC = assignment.second.first;

            if (ready_queue.find(task) != ready_queue.end())
            {
                // tasks in ready_queue
                // int DC_used = used.find(DC) == used.end() ? 0 : used[DC];
                // map initialize int with 0
                int DC_used = used[DC];
                auto &slot = graph->slots[DC];

                if (slot.first > slot.second.size() + DC_used)
                {
                    // slots has capacity
                    // arrange successfully
                    ready_queue.erase(task); // pop from ready_queue
                    assignments.push_back(assignment);
                    used[DC]++;
                }
            }
        }
        return assignments;
    }

    // assign tasks randomly
    vector<Arrange> getRandom()
    {
        vector<pair<string, int>> available_slot;
        // pair<string, pair<int, unordered_set<string>>>
        for (const auto &slot : graph->slots)
        {
            if (slot.second.second.size() > slot.second.first)
                printError("Slot Overflow in Scheduler");

            int empty_cnt = slot.second.first -
                            slot.second.second.size();
            if (empty_cnt != 0)
            {
                available_slot.push_back(
                    make_pair(slot.first, empty_cnt));
            }
        }

        vector<Arrange> assignments;
        while (!ready_queue.empty() &&
               !available_slot.empty())
        {
            string task = *ready_queue.begin();
            int DC_index = randInt(0,
                                   available_slot.size() - 1);
            // vector<pair<string, int>>::iterator
            auto iter = available_slot.begin() + DC_index;
            string DC = iter->first;
            Arrange assignment;
            assignment.first = count_time(task, DC);
            assignment.second.first = DC;
            assignment.second.second = task;
            assignments.push_back(assignment);
            if (--iter->second == 0)
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

    void initGraph(shared_ptr<Graph> graph)
    {
        this->graph = graph;
    }

    // get new tasks from DAG
    void sumbitTasks(unordered_set<string> tasks)
    {
        for (const auto &task : tasks)
        {
            ready_queue.insert(task);
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