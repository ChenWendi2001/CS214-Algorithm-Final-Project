#include "includes/common.hpp"
#include "includes/DAG.hpp"
#include "includes/scheduler.hpp"
#include "includes/simulator.hpp"

int main()
{
    shared_ptr<Graph> graph;
    graph = make_shared<Graph>();
    init_data(graph);
    DAG dag;
    Scheduler scheduler;
    scheduler.sched_type = scheduler.GREEDY;
    Simulator sim;

    dag.init(graph);
    scheduler.initGraph(graph);
    sim.updateGraph(graph);
    // sim.printStatus();
    int task_cnt = 0;
    while (!dag.if_finished())
    {

        scheduler.sumbitTasks(dag.getSubmit());
        auto sched = scheduler.getScheduled();
        sim.updateScheduled(sched);

        // sim.forwardTime();
        sim.tickTime(0.01);

        // std::cout << sim.getTime() << "\n";
        auto finished = sim.getFinished();
        // sim.printStatus();

        // for (const auto &it : finished)
        //     std::cout << it.first << ' ';
        // std::cout << std::endl;

        task_cnt += finished.size();
        dag.updateDAG(finished);
    }
    std::cout << "GREEDY: " << sim.getTime() << "\n";
    graph->printStatistics("greedy.log");
    graph->printFinishTime("greedy.txt");
    graph->printData("greedy_data.txt");
    std::cout << std::endl;
    return 0;
}
