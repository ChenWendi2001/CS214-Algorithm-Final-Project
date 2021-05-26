#ifndef __DAG_HPP__
#define __DAG_HPP__

#include "common.hpp"

class DAG
{
private:
    shared_ptr<Graph> graph;

    // available task queue
    unordered_set<string> queue;

    // e.g. DAG ["tA1"] = {"tA2","tA3"}
    //  tA1 is directed to tA2 and tA3
    unordered_map<string, unordered_set<string>> DAG;

    // e.g count ["tA2"] = 2
    //  there are 2 tasks are directed to tA2
    unordered_map<string, int> count;

public:
    // update DAG with finished tasks
    //  from simulator

    bool if_finished()
    {
        return count.empty();
    }

    void updateDAG(vector<pair<string, double>> finished_tasks)
    {
        int size = finished_tasks.size();
        for (int i = 0; i < size; ++i)
        {
            string task = std::move(finished_tasks[i].first);
            count.erase(task);
            auto &this_DAG = DAG[task];
            for (const auto &iter : this_DAG)
            {
                if (--count[iter] == 0)
                {
                    queue.insert(iter);
                    //count.erase(*iter);
                }
            }
            DAG.erase(task);
        }
    }

    // return new tasks that can be submit
    // e.g. {"tA1","tA2"}
    //  put these tasks to scheduler
    unordered_set<string> getSubmit()
    {
        unordered_set<string> ret = std::move(queue);
        // std::cout << queue.size();
        if (!queue.empty())
            printError("queue should be clear!");
        return ret;
    }

    void init(shared_ptr<Graph> outergraph)
    {
        this->graph = outergraph;
        for (const auto &iter : graph->require)
        {
            count[iter.first] = 0;
        }
        for (const auto &iter : graph->constraint)
        {
            count[iter.second]++;
            DAG[iter.first].insert(iter.second);
        }

        // unordered_set<string> need_del;
        for (const auto &iter : count)
        {
            if (iter.second == 0)
            {
                queue.insert(iter.first);
                // need_del.insert(iter->first);
            }
        }
        // for (const auto &it : need_del)
        //     count.erase(it);

        //Debug code
        // for(auto iter = count.begin();iter!=count.end();iter++){
        //     if(iter->second==0)
        //         std::cout<<iter->first<<"\n";
        // }
        // for(auto iter = DAG.begin();iter!=DAG.end();iter++){
        //     std::cout<<iter->second.size()<<"\n";
        // }
    }
};

#endif