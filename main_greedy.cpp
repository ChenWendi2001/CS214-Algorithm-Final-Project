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
    Scheduler scheduler(graph);
    Simulator sim;

    dag.init(graph);
    sim.updateGraph(graph);
    sim.printStatus();
    int task_cnt = 0;
    while (!dag.if_finished())
    {

        scheduler.sumbitTasks(dag.getSubmit());
        auto sched = scheduler.getScheduled();
        sim.updateScheduled(sched);

        //sim.printStatus();
        sim.forwardTime();
        std::cout << sim.getTime() << "\n";
        auto finished = sim.getFinished();
        task_cnt += finished.size();
        dag.updateDAG(finished);
    }

    //Debug code
    // vector<string> temp = dag.getSubmit();
    // for(auto iter = temp.begin();iter!=temp.end();iter++){
    //     std::cout<<*iter<<" ";
    // }
    return 0;
}

void init_data()
{
    graph = graph = make_shared<Graph>();
    //
    //initailize constraint
    //
    json constraint;
    std::ifstream constraint_file("./ToyData/constraint.json");
    constraint_file >> constraint;
    for (auto iter = constraint["constraint"].begin(); iter != constraint["constraint"].end(); iter++)
    {
        graph->constraint.push_back(std::make_pair<string, string>((*iter)["start"], (*iter)["end"]));
    }

    //Debug code
    // for(auto iter = graph->constraint.begin();iter!=graph->constraint.end();iter++){
    //     std::cout<<(*iter).first<<" "<<(*iter).second<<"\n";
    // }

    //
    //initialize run_time, require and job_task
    //
    json job;
    std::ifstream job_file("./ToyData/job_list.json");
    job_file >> job;

    int num_of_jobs = job["job"].size();
    for (int i = 0; i < num_of_jobs; ++i)
    {
        int num_of_task = job["job"][i]["task"].size();
        unordered_set<string> &task = graph->job_task[job["job"][i]["name"]];
        for (int j = 0; j < num_of_task; ++j)
        {
            string task_name = job["job"][i]["task"][j]["name"];
            //run time
            graph->run_time[task_name] = job["job"][i]["task"][j]["time"];
            //job list
            task.insert(task_name);
            int num_of_resource = job["job"][i]["task"][j]["resource"].size();
            for (int k = 0; k < num_of_resource; ++k)
            {
                graph->require[job["job"][i]["task"][j]["name"]].push_back(
                    std::make_pair(job["job"][i]["task"][j]["resource"][k]["name"], job["job"][i]["task"][j]["resource"][k]["size"]));
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
        int num_of_resource = DC["DC"][i]["data"].size();
        for (int j = 0; j < num_of_resource; ++j)
            graph->resource_loc[DC["DC"][i]["data"][j]] = DC["DC"][i]["name"];
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

    for (int i = 0; i < num_of_dc; ++i)
    {
        for (int j = 0; j < num_of_dc; ++j)
        {
            if (link["link"][i]["bandwidth"][j] == -1)
                graph->edges[link["link"][i]["start"]][link["link"][j]["start"]] = 0xFFFF;
            else
                graph->edges[link["link"][i]["start"]][link["link"][j]["start"]] = 1 / double(link["link"][i]["bandwidth"][j]);
        }
    }

    //Floyd
    for (int i = 0; i < num_of_dc; ++i)
    {
        for (int j = 0; j < num_of_dc; ++j)
        {
            for (int k = 0; k < num_of_dc; ++k)
            {
                auto dc_i = DC["DC"][i]["name"];
                auto dc_j = DC["DC"][j]["name"];
                auto dc_k = DC["DC"][k]["name"];
                if (graph->edges[dc_i][dc_j] > graph->edges[dc_i][dc_k] + graph->edges[dc_k][dc_j])
                {
                    graph->edges[dc_i][dc_j] = graph->edges[dc_i][dc_k] + graph->edges[dc_k][dc_j];
                }
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
        graph->slots[DC["DC"][i]["name"]].first = DC["DC"][i]["size"];
    }

    //Debug code
    // for(int i = 0;i<num_of_dc;++i){
    //     std::cout<<graph->slots[DC["DC"][i]["name"]].first<<"\t";
    // }
}