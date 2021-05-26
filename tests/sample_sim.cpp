#include "common.hpp"
#include "simulator.hpp"
#include "scheduler.hpp"
Simulator sim;
shared_ptr<Graph> graph;
shared_ptr<Scheduler> sch;
void test_scheduler()
{
    shared_ptr<Graph> graph = make_shared<Graph>();
    Scheduler sch(graph);

    //edge initial
    unordered_map<string, double> edge;
    edge.insert(make_pair("DC2", 90));
    edge.insert(make_pair("DC3", 70));
    edge.insert(make_pair("DC1", 1000));
    graph->edges.insert(make_pair("DC1", edge));
    edge.clear();
    edge.insert(make_pair("DC1", 100));
    edge.insert(make_pair("DC2", 1000));
    edge.insert(make_pair("DC3", 130));
    graph->edges.insert(make_pair("DC2", edge));
    edge.clear();
    edge.insert(make_pair("DC1", 50));
    edge.insert(make_pair("DC2", 110));
    edge.insert(make_pair("DC3", 1000));
    graph->edges.insert(make_pair("DC3", edge));
    edge.clear();

    //require  initial
    vector<pair<string, double>> req;
    //tA1
    req.push_back(make_pair("A1", 200));
    req.push_back(make_pair("A2", 300));
    req.push_back(make_pair("A3", 100));
    graph->require.insert(make_pair("tA1", req));
    //tA2
    req.push_back(make_pair("A1", 300));
    req.push_back(make_pair("A2", 100));
    req.push_back(make_pair("A3", 200));
    graph->require.insert(make_pair("tA2", req));

    //loc initial
    graph->resource_loc.insert(make_pair("A1", "DC1"));
    graph->resource_loc.insert(make_pair("A2", "DC2"));
    graph->resource_loc.insert(make_pair("A3", "DC3"));

    //slots initial
    unordered_set<string> res;
    res.clear();
    graph->slots.insert(make_pair("DC1", make_pair(2, res)));
    graph->slots.insert(make_pair("DC2", make_pair(1, res)));
    graph->slots.insert(make_pair("DC3", make_pair(2, res)));

    unordered_set<string> ready;
    ready.insert("tA1");
    ready.insert("tA2");
    sch.sumbitTasks(ready);
    vector<pair<double, pair<string, string>>> ret1 = sch.getScheduled();
    vector<pair<double, pair<string, string>>> ret2 = sch.getScheduled();
    return;
}
int main()
{
    test_scheduler();
    // Graph graph;
    // graph.slots["DC1"] =
    //     std::make_pair<int, vector<string>>(
    //         3, {"tA1", "tA2"});
    // graph.slots["DC2"] =
    //     std::make_pair<int, vector<string>>(
    //         4, {"tB1"});
    // graph.slots["DC3"] =
    //     std::make_pair<int, vector<string>>(
    //         3, {});
    // graph.printStatus();

    graph = make_shared<Graph>();
    graph->slots["DC1"] =
        std::make_pair<int, unordered_set<string>>(
            5, {"tA2"});
    graph->printStatus();
    graph->readTaskTime();
    sim.updateGraph(graph);
    sim.printStatus();
    std::cout << sim.getTime() << std::endl;

    vector<pair<double,
                pair<string, string>>>
        sched = {{4, {"DC1", "tA1"}}};
    sim.updateScheduled(sched); // put into run queue
    sim.printStatus();          // check

    if (!sim.isEmpty())
    {
        sim.forwardTime(); // forward time

        auto finished = sim.getFinished();
        for (const auto &it : finished)
            std::cout << it << ' ';
        std::cout << std::endl;

        sim.printStatus();
        std::cout << sim.getTime() << std::endl;
    }
    return 0;
}