#include "common.hpp"
#include "json.hpp"
#include <fstream>
#include "DAG.hpp"
#include "scheduler.hpp"
#include "simulator.hpp"

using json = nlohmann::json;

shared_ptr<Graph> graph;

void init_data();

int main()
{
    init_data();
    DAG dag;
    Scheduler scheduler;
    scheduler.sched_type = scheduler.GREEDY;
    Simulator sim;

    dag.init(graph);
    scheduler.initGraph(graph);
    sim.updateGraph(graph);
    sim.printStatus();
    int task_cnt = 0;
    while (!dag.if_finished())
    {

        scheduler.sumbitTasks(dag.getSubmit());
        auto sched = scheduler.getScheduled();
        sim.updateScheduled(sched);

        // sim.printStatus();
        sim.forwardTime();
        // std::cout << sim.getTime() << "\n";
        auto finished = sim.getFinished();

        // for (const auto &it : finished)
        //     std::cout << it.first << ' ';
        // std::cout << std::endl;

        task_cnt += finished.size();
        dag.updateDAG(finished);
    }
    std::cout << sim.getTime() << "\n";

    graph->printFinishTime();

    //Debug code
    // vector<string> temp = dag.getSubmit();
    // for(auto iter = temp.begin();iter!=temp.end();iter++){
    //     std::cout<<*iter<<" ";
    // }
    return 0;
}

void init_data()
{
    // WTF?
    // graph = graph = make_shared<Graph>();
    graph = make_shared<Graph>();

    //
    //initailize constraint
    //
    json constraint;
    std::ifstream constraint_file("./ToyData/constraint.json");
    constraint_file >> constraint;
    for (const auto &iter : constraint["constraint"])
    {
        // <string,string>
        graph->constraint.push_back(
            make_pair(iter["start"], iter["end"]));
    }

    //Debug code
    // for(auto iter = graph->constraint.begin();iter!=graph->constraint.end();iter++){
    //     std::cout<<(*iter).first<<" "<<(*iter).second<<"\n";
    // }

    //
    //initialize run_time, require and job_task, which job
    //
    json job;
    std::ifstream job_file("./ToyData/job_list.json");
    job_file >> job;

    int num_of_jobs = job["job"].size();
    for (int i = 0; i < num_of_jobs; ++i)
    {
        const auto this_job = job["job"][i];
        int num_of_task = this_job["task"].size();
        string job_name = this_job["name"];
        unordered_set<string> &job_task = graph->job_task[job_name];

        for (int j = 0; j < num_of_task; ++j)
        {
            const auto &this_task = this_job["task"][j];
            string task_name = this_task["name"];

            graph->run_time[task_name] = this_task["time"]; // run time
            job_task.insert(task_name);                     // job list
            graph->which_job[task_name] = job_name;         // which job

            int num_of_resource = this_task["resource"].size();
            for (int k = 0; k < num_of_resource; ++k)
            {
                graph->require[task_name].push_back(
                    make_pair(this_task["resource"][k]["name"],
                              this_task["resource"][k]["size"]));
            }
        }
    }

    //Debug code

    // for(int i = 0;i<num_of_jobs;++i){
    //     int num_of_task = job["job"][i]["task"].size();
    //     for(int j = 0;j<num_of_task;++j){
    //         std::cout<<job["job"][i]["task"][j]["name"]<<":" <<graph->run_time[job["job"][i]["task"][j]["name"]];
    //         std::cout<<"\n";
    //         int num_of_resource = job["job"][i]["task"][j]["resource"].size();
    //         for(int k = 0;k<num_of_resource;++k){
    //             for(auto iter = graph->require[job["job"][i]["task"][j]["name"]].begin();iter !=graph->require[job["job"][i]["task"][j]["name"]].end();iter++)
    //                 std::cout<<(*iter).first<<" "<<(*iter).second<<"\n";
    //         }
    //     }
    // }

    //
    //initialize graph->resources
    //
    json DC;
    std::ifstream DC_file("./ToyData/DC.json");
    DC_file >> DC;

    int num_of_dc = DC["DC"].size();
    for (int i = 0; i < num_of_dc; ++i)
    {
        const auto &this_DC = DC["DC"][i];
        int num_of_resource = this_DC["data"].size();
        for (int j = 0; j < num_of_resource; ++j)
            graph->resource_loc[this_DC["data"][j]] = this_DC["name"];
    }

    //Debug code

    // for(int i = 0;i<num_of_dc;++i){
    //     int num_of_resource = DC["DC"][i]["data"].size();
    //     for(int j = 0;j<num_of_resource;++j)
    //         std::cout<<DC["DC"][i]["data"][j]<<":"<<graph->resource_loc[DC["DC"][i]["data"][j]]<<'\n';
    // }

    //
    //initialize edges
    //

    json link;
    std::ifstream link_file("./ToyData/link.json");
    link_file >> link;

    const double INF = 1e6;
    for (int i = 0; i < num_of_dc; ++i)
    {
        for (int j = 0; j < num_of_dc; ++j)
        {
            int bandwidth = link["link"][i]["bandwidth"][j];
            string u = link["link"][i]["start"];
            string v = link["link"][j]["start"];
            graph->edges[u][v] = bandwidth == -1
                                     ? INF
                                     : 1 / double(bandwidth);
        }
    }

    // Floyd
    for (int k = 0; k < num_of_dc; ++k)
    {
        auto dc_k = DC["DC"][k]["name"];

        for (int i = 0; i < num_of_dc; ++i)
        {
            auto dc_i = DC["DC"][i]["name"];
            double d_ik = graph->edges[dc_i][dc_k];

            for (int j = 0; j < num_of_dc; ++j)
            {
                auto dc_j = DC["DC"][j]["name"];
                double d_kj = graph->edges[dc_k][dc_j];
                double &d_ij = graph->edges[dc_i][dc_j];

                d_ij = std::min(d_ij, d_ik + d_kj);
            }
        }
    }

    //Debug code

    // for(int i = 0;i<num_of_dc;++i){
    //     for(int j = 0;j<num_of_dc;++j){
    //          std::cout<<int(1/graph->edges[DC["DC"][i]["name"]][DC["DC"][j]["name"]])<<"\t";
    //     }
    //     std::cout<<std::endl;
    // }

    //
    //initialize slots
    //
    for (int i = 0; i < num_of_dc; ++i)
    {
        const auto &this_DC = DC["DC"][i];
        graph->slots[this_DC["name"]].first = this_DC["size"];
    }

    //Debug code
    // for(int i = 0;i<num_of_dc;++i){
    //     std::cout<<graph->slots[DC["DC"][i]["name"]].first<<"\t";
    // }
}