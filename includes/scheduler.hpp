#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include "common.hpp"
#include "network_neck.hpp"
#include "network_sum.hpp"

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
    unordered_set<string> ready_set;
    // same as ready_set but FIFO
    //  used by NetworkSched
    std::deque<string> ready_queue;

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
        for (const auto &task_iter : ready_set)
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

        // for K_GREEDY
        // skip first k choices
        // (k=0 for normal greedy)
        unordered_map<string, int> k_val;

        vector<Arrange> assignments;
        unordered_map<string, int> used;
        // some slots of graph arranged just now
        while (!Q.empty())
        {
            Arrange assignment = Q.top();
            Q.pop();
            string task = assignment.second.second;
            string DC = assignment.second.first;

            if (ready_set.find(task) != ready_set.end())
            {
                // tasks in ready_queue
                // int DC_used = used.find(DC) == used.end() ? 0 : used[DC];
                // map initialize int with 0
                int DC_used = used[DC];
                auto &slot = graph->slots[DC];

                if (slot.first > slot.second.size() + DC_used)
                {
                    if (sched_type == K_GREEDY)
                    {
                        // skip 0~2 choices
                        if (k_val.find(task) == k_val.end())
                            k_val[task] = randInt(0, 2);
                        if (k_val[task] != 0)
                        {
                            k_val[task]--;
                            continue;
                        }
                    }
                    // slots with enough capacity
                    // arrange successfully
                    ready_set.erase(task); // pop from ready_queue
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
        while (!ready_set.empty() &&
               !available_slot.empty())
        {
            string task = *ready_set.begin();
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
            ready_set.erase(task);
        }
        return assignments;
    }

    // use NetworkSum
    vector<Arrange> getNetworkSum()
    {
        NetworkSum net_sum;
        // e.g. {{"DC1",2}}
        vector<pair<string, int>> cap_info;
        // e.g. {{4,{"DC1","tA1"}}}
        vector<pair<double,
                    pair<string, string>>>
            assign_info;

        int slots_cnt = 0;
        for (const auto &slot : graph->slots)
        {
            int cap = slot.second.first -
                      slot.second.second.size();
            if (cap > 0)
                cap_info.emplace_back(
                    make_pair(slot.first, cap));
            slots_cnt += cap;
        }

        // DC is full
        if (cap_info.empty())
            return vector<Arrange>();

        for (const auto &task : ready_set)
        {
            for (const auto &slot : graph->slots)
            {
                string DC = slot.first;
                if (slot.second.second.size() <
                    slot.second.first)
                {
                    double ti = count_time(task, slot.first);
                    assign_info.emplace_back(
                        make_pair(ti, make_pair(DC, task)));
                }
            }
        }

        net_sum.initNetwork(ready_set.size(),
                            std::min(slots_cnt,
                                     (int)ready_set.size()),
                            cap_info,
                            assign_info);
        auto assigned = net_sum.getSched();
        for (const auto &it : assigned)
            ready_set.erase(it.second.second);
        return assigned;
    }

    // use NetworkNeck
    vector<Arrange> getNetworkNeck()
    {
        NetworkNeck net_neck;
        NetworkSum net_sum;

        // e.g. {{"tA1","tA2"},{"tB1"}}
        vector<vector<string>> task_group;
        // e.g. {{"DC1",2}}
        vector<pair<string, int>> cap_info;
        // e.g. {{4,{"DC1","tA1"}}}
        vector<pair<double,
                    pair<string, string>>>
            assign_info;

        int slots_cnt = 0;
        for (const auto &slot : graph->slots)
        {
            int cap = slot.second.first -
                      slot.second.second.size();
            if (cap > 0)
                cap_info.emplace_back(
                    make_pair(slot.first, cap));
            slots_cnt += cap;
        }

        // DC is full
        if (cap_info.empty())
            return vector<Arrange>();

        for (const auto &task : ready_set)
        {
            for (const auto &slot : graph->slots)
            {
                string DC = slot.first;
                if (slot.second.second.size() <
                    slot.second.first)
                {
                    double ti = count_time(task, slot.first);
                    assign_info.emplace_back(
                        make_pair(ti, make_pair(DC, task)));
                }
            }
        }

        net_sum.initNetwork(ready_set.size(),
                            std::min(slots_cnt,
                                     (int)ready_set.size()),
                            cap_info,
                            assign_info);
        auto assigned = net_sum.getSched();
        vector<string> assign_queue;
        for (const auto &it : assigned)
            assign_queue.push_back(it.second.second);

        assign_info.clear();
        for (const auto &task : assign_queue)
        {
            for (const auto &slot : graph->slots)
            {
                string DC = slot.first;
                if (slot.second.second.size() <
                    slot.second.first)
                {
                    double ti = count_time(task, slot.first) +
                                graph->run_time[task];
                    assign_info.emplace_back(
                        make_pair(ti, make_pair(DC, task)));
                }
            }
        }
        // initialize task group
        if (neck_type == SAME_TASK)
        {
            // tasks in same job belong to same group
            unordered_map<string, int> job_id;
            for (const auto &task : assign_queue)
            {
                string job = graph->which_job[task];
                if (job_id.find(job) == job_id.end())
                {
                    job_id[job] = task_group.size();
                    task_group.push_back({task});
                }
                else
                    task_group[job_id[job]].push_back(task);
            }
        }

        if (neck_type == SAME_NEXT)
        {
            // tasks with same succeed nodes belong to same group
            UnionFindSet g;
            g.init(assign_queue.size());
            unordered_map<string, int> task_id;
            for (int i = 0; i < assign_queue.size(); ++i)
                task_id[assign_queue[i]] = i;
            for (const auto &task : assign_queue)
            {
                // next of this
                for (const auto &next : graph->next_nodes[task])
                    // prev of next
                    for (const auto &prev : graph->prev_nodes[next])
                        if (task_id.find(prev) != task_id.end())
                        {
                            g.unite(task_id[prev], task_id[task]);
                        }
            }
            unordered_map<int, int> group_id;
            for (int i = 0; i < assign_queue.size(); ++i)
            {
                int id = g.find(i);
                if (group_id.find(id) == group_id.end())
                {
                    group_id[id] = task_group.size();
                    task_group.push_back({assign_queue[i]});
                }
                else
                    task_group[group_id[id]].push_back(
                        assign_queue[i]);
            }
        }

        net_neck.sched_type = NetworkNeck::FAIR;
        net_neck.initNetwork(assign_queue.size(),
                             task_group,
                             cap_info,
                             assign_info);

        assigned = net_neck.getSched();
        for (auto &it : assigned)
        {
            string task = it.second.second;
            it.first -= graph->run_time[task];
            ready_set.erase(task);
        }
        return assigned;
    }

public:
    enum SchedType
    {
        GREEDY,
        K_GREEDY,
        RANDOM,
        NETWORK_SUM,
        NETWORK_NECK
    } sched_type;

    enum NeckType
    {
        SAME_TASK,
        SAME_NEXT
    } neck_type;

    void initGraph(shared_ptr<Graph> graph)
    {
        this->graph = graph;
    }

    int taskSize()
    {
        switch (sched_type)
        {
        case GREEDY:
        case K_GREEDY:
        case RANDOM:
        case NETWORK_SUM:
        case NETWORK_NECK:
            return ready_set.size();
            // case NETWORK_NECK:
            // return ready_queue.size();
        }
    }

    // get new tasks from DAG
    void sumbitTasks(unordered_set<string> tasks)
    {
        for (const auto &task : tasks)
        {
            switch (sched_type)
            {
            case GREEDY:
            case K_GREEDY:
            case RANDOM:
            case NETWORK_SUM:
            case NETWORK_NECK:
                ready_set.insert(task);
                break;
                // case NETWORK_NECK:
                //     ready_queue.push_back(task);
                //     break;
            }
        }
        //add task into ready_queue
    }

    // update current resources from simulator
    // schedule tasks to slots
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Arrange> getScheduled()
    {
        switch (sched_type)
        {
        case GREEDY:
        case K_GREEDY:
            return getGreedy();
        case RANDOM:
            return getRandom();
        case NETWORK_NECK:
            return getNetworkNeck();
        case NETWORK_SUM:
            return getNetworkSum();
        }
    }
};

#endif