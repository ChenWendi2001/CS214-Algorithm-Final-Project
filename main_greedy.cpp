#include "common.hpp"
#include "DAG.hpp"
#include "scheduler.hpp"
#include "simulator.hpp"

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

    return 0;
}
