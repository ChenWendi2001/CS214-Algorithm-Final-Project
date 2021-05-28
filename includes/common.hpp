#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <vector>
#include <string>
#include <utility>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <iomanip>
#include <cmath>
#include <random>
#include "json.hpp"

using json = nlohmann::json;
using std::make_pair;
using std::make_shared;
using std::map;
using std::pair;
using std::priority_queue;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

// data directory
// static const string DIR = "./ToyData/";
static const string DIR = "./";
// static const string DIR = "./scripts/";

// uniformly random int in [mn, mx]
int randInt(int mn, int mx)
{
    static std::mt19937 gen(time(0));
    std::uniform_int_distribution<int> dis(mn, mx);
    return dis(gen);
}

// print error message and terminate
void printError(string msg)
{
    std::cout << "Error: " << msg << std::endl;
    std::exit(0);
}

// print warning message
void printWarning(string msg)
{
    std::cout << "Warning: " << msg << std::endl;
}

struct Graph
{
    // e.g. {"tA3",{"tA1","tA2"}}
    //  tA3 needs the results of tA1 or tA2
    unordered_map<string, set<string>> prev_nodes;

    // e.g. {"tA1",{"tA3","tA4"}}
    //  tA3 and tA4 need result of tA1
    unordered_map<string, set<string>> next_nodes;

    // task belongs to which job
    // e.g {"tA1","A"}
    //  "tA1" belongs to "A"
    unordered_map<string, string> which_job;

    // task's (start_time,run_time)
    map<string, pair<double, double>> task_span;

    // job's finish time
    map<string, double> finish_time;

    // task set of each job
    // e.g {"A",{"tA1","tA2"}}
    //  job "A" has two tasks "tA1" and "tA2"
    unordered_map<string, unordered_set<string>> job_task;

    // e.g, {"tB1","tB2"}
    //  tB2 need the result of tB1
    vector<pair<string, string>> constraint;

    // task's run time
    // e.g. {"tA1", 2}
    //  tA1 takes 2s to execute
    unordered_map<string, double> run_time;

    // resources required by "txx"
    // e.g. require["tA1"]={{"A1",50},{"A2",100}}
    //  tA1 needs A1 and A2
    unordered_map<string,
                  vector<pair<string, double>>>
        require;

    // location of resource "xx"
    // e.g. resource_loc["A1"]=DC1
    //  resource A1 is put in DC1
    unordered_map<string, string> resource_loc;

    // adjacent matrix
    // e.g. edges["DC1"]["DC2"]=1/100
    //  if bandwidth between DC1 and DC2 is 100
    unordered_map<string,
                  unordered_map<string, double>>
        edges;

    // slots of "DCi"
    // e.g. slots["DC1"]={2,{"tA1"}}
    //  the capacity of DC1 is 2 and tA1 is running
    unordered_map<string,
                  pair<int, unordered_set<string>>>
        slots;

    void printStatus()
    {
        for (const auto &DC : slots)
        {
            std::cout << std::setw(4)
                      << DC.first << '|';
            const auto tasks = DC.second.second;
            for (const auto &task : tasks)
                std::cout << std::setw(4)
                          << task << '|';
            for (int i = 0; i < DC.second.first - tasks.size(); ++i)
                std::cout << std::setw(4)
                          << ' ' << '|';
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

    // output to screen if file_name is empty
    void printFinishTime(string file_name = "")
    {
        // std::sort(finish_time.begin(), finish_time.end());
        if (file_name.empty())
            for (const auto &it : finish_time)
            {
                std::cout << it.first << ' '
                          << std::setprecision(4) << it.second << std::endl;
            }
        else
        {
            std::ofstream fout;
            fout.open(file_name);
            if (!fout.is_open())
                printError("Can't Open File in printFinishTime");
            for (const auto &it : finish_time)
            {
                fout << it.first << ' '
                     << std::setprecision(4) << it.second << std::endl;
            }
            fout.close();
        }
    }

    // similar to printFinishTime
    // but only have time without name
    // in monotonically increasing order
    void printData(string file_name)
    {
        std::ofstream fout;
        fout.open(file_name);
        if (!fout.is_open())
            printError("Can't Open File in printData");
        vector<double> times;
        for (const auto &it : finish_time)
            times.push_back(it.second);
        std::sort(times.begin(), times.end());
        for (const auto &it : times)
            fout << it << '\n';
        fout << std::endl;
        fout.close();
    }

    // log average time if file_name is not empty
    void printStatistics(string file_name = "")
    {
        double avg = 0, mx = 0;
        for (const auto &it : finish_time)
        {
            avg += it.second;
            mx = std::max(mx, it.second);
        }
        avg /= finish_time.size();
        double var = 0;
        for (const auto &it : finish_time)
            var += (it.second - avg) * (it.second - avg);
        std::cout << "Average: " << avg << '\n'
                  << "Standard Deviation: "
                  << std::sqrt(var / finish_time.size())
                  << std::endl;
        if (!file_name.empty())
        {
            std::ofstream fout;
            fout.open(file_name, std::ios::app);
            if (!fout.is_open())
                printWarning("Can't Open Log File");
            fout << mx << ','
                 << avg << ','
                 << std::sqrt(var / finish_time.size()) << std::endl;
            fout.close();
        }
    }

    // print tasks' span
    void printTasks()
    {
        std::ofstream fout;
        fout.open("tasks.csv");
        for (const auto &it : task_span)
        {
            fout << it.first << ','
                 << it.second.first << ','
                 << it.second.second << std::endl;
        }
        fout.close();
    }
};

// union and find set
// [0,n)
struct UnionFindSet
{
    vector<int> f;
    void init(int n)
    {
        f.resize(n);
        for (int i = 0; i < n; ++i)
            f[i] = i;
    }

    int find(int x)
    {
        return f[x] == x ? x : f[x] = find(f[x]);
    }

    void unite(int u, int v)
    {
        f[find(u)] = find(v);
    }
};

void init_data(shared_ptr<Graph> graph)
{
    // initailize constraint
    json constraint;
    std::ifstream constraint_file(DIR + "constraint.json");
    if (!constraint_file.is_open())
        printError("No constraint.json!");
    constraint_file >> constraint;
    for (const auto &iter : constraint["constraint"])
    {
        // <string,string>
        // u->v
        string prev = iter["start"];
        string next = iter["end"];
        graph->constraint.push_back(
            make_pair(prev, next));
        graph->prev_nodes[next].insert(prev);
        graph->next_nodes[prev].insert(next);
    }
    constraint_file.close();

    // initialize run_time, require and job_task, which job
    json job;
    std::ifstream job_file(DIR + "job_list.json");
    if (!job_file.is_open())
        printError("No job_list.json");
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
    job_file.close();

    // initialize graph->resources
    json DC;
    std::ifstream DC_file(DIR + "DC.json");
    if (!DC_file.is_open())
        printError("No DC.json");
    DC_file >> DC;
    int num_of_dc = DC["DC"].size();
    for (int i = 0; i < num_of_dc; ++i)
    {
        const auto &this_DC = DC["DC"][i];
        int num_of_resource = this_DC["data"].size();
        for (int j = 0; j < num_of_resource; ++j)
            graph->resource_loc[this_DC["data"][j]] = this_DC["name"];
    }
    DC_file.close();

    // initialize edges
    json link;
    std::ifstream link_file(DIR + "link.json");
    if (!link_file.is_open())
        printError("No link.json");
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
    link_file.close();

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

                // #### modify this to change bandwidth
                // d_ij = std::min(d_ij, d_ik + d_kj);
                d_ij = std::min(d_ij, std::max(d_ik, d_kj));
            }
        }
    }

    // initialize slots
    for (int i = 0; i < num_of_dc; ++i)
    {
        const auto &this_DC = DC["DC"][i];
        graph->slots[this_DC["name"]].first = this_DC["size"];
    }
}

#endif