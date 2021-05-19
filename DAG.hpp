#ifndef __DAG_HPP__
#define __DAG_HPP__

#include "common.hpp"

class DAG
{
private:

shared_ptr<Graph> graph;

//available task queue
unordered_set<string> queue;

//e.g. DAG ["tA1"] = {"tA2","tA3"}
//tA1 is directed to tA2 and tA3
unordered_map<string,unordered_set<string>> DAG;


//e.g DAG ["tA2"] = 2
//there are 2 tasks are directed to tA2
unordered_map <string,int> count;

public:
    // update DAG with finished tasks
    //  from simulator
    void updateDAG(vector<string> finished_tasks)
    {
        int size = finished_tasks.size();
        for(int i = 0;i<size;++i){
            for(auto iter = DAG[finished_tasks[i]].begin();iter!=DAG[finished_tasks[i]].end();iter++){
                count[*iter] --;
                if(count[*iter]==0){
                    queue.insert(*iter);
                }
            }
            DAG.erase(finished_tasks[i]);
        }
    }

    // return new tasks that can be submit
    // e.g. {"tA1","tA2"}
    //  put these tasks to scheduler
    vector<string> getSubmit()
    {
        return vector<string>(queue.begin(),queue.end());
    }

    void init(shared_ptr<Graph> outergraph)
    {
        this->graph = outergraph;
        for(auto iter = graph->require.begin();iter!=graph->require.end();iter++){
            count[iter->first] = 0;
        }
        for(auto iter = graph->constraint.begin();iter!=graph->constraint.end();iter++){
            count[iter->second]++;
            DAG[iter->first].insert(iter->second);
        }

        for(auto iter = count.begin();iter!=count.end();iter++){
            if(iter->second==0)
                queue.insert(iter->first);
        }
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