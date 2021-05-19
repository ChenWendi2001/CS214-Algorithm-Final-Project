#include "common.hpp"
#include "simulator.hpp"

Simulator sim;
shared_ptr<Graph> graph;

int main()
{
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
        sim.tickTime(); // forward time

        auto finished = sim.getFinished();
        for (const auto &it : finished)
            std::cout << it << ' ';
        std::cout << std::endl;

        sim.printStatus();
        std::cout << sim.getTime() << std::endl;
    }
    return 0;
}