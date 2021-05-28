#include "common.hpp"

// NOTE: min{cost} max flow

// when constructing nodes
// we assign
//  DC in [0,n-1] (may out of order)
//  task in [n,n+m-1] (may out of order)
//  source to n+m, sink to n+m+1
class NetworkNeck
{
private:
    static constexpr double eps = 1e-8;

    typedef pair<double, pair<string, string>> Assign;

    struct Location
    {
        // {"DC1",2} means DC1 is node 2 in network
        unordered_map<string, int> loc;
        // {2,"DC1"}
        unordered_map<int, string> names;
        int cur_idx;

        int ID(const string &name)
        {
            if (loc.find(name) == loc.end())
            {
                loc[name] = cur_idx;
                names[cur_idx] = name;
                return cur_idx++;
            }
            else
                return loc[name];
        }

        string Name(int k)
        {
            if (names.find(k) == names.end())
                printError("No Such k: " + std::to_string(k));

            return names[k];
        }
    };
    Location DC_id, task_id;
    int task_num, DC_num;

    // -----> Dinic begin
    int source, sink;
    struct Edge
    {
        int v;          // to v
        int ori_cap;    // original capacity
        int cap;        // residual capacity: cap-flow
        int next;       // point at next edge
        double ori_val; // original weight
        double val;     // can be changed into 0 or INF

        // debug
        void print()
        {
            std::cout << "to: " << v << '\n'
                      << "ori_cap: " << ori_cap << '\n'
                      << "ori_val: " << ori_val << '\n'
                      << "val: " << val << std::endl;
        }
    };
    // max among edges' value
    double max_val;
    // adjacent list
    vector<int> head;
    // array of all edges
    // note: i^1 is residual edge
    vector<Edge> edges;
    // current edge optimization
    vector<int> cur_head;
    vector<int> prev;
    // <----- Dinic end

    // task group scheduling
    // e.g. {{"tA1","tA2"}}
    vector<vector<string>> task_group;
    // which job this task belongs to
    unordered_map<string, int> which_job;

    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Assign> assigned;

private:
    void addEdge(int u, int v,
                 int cap, double val)
    {
        Edge e;
        e.next = head[u], e.v = v;
        e.ori_cap = cap;
        e.ori_val = e.val = val;
        edges.emplace_back(e);
        head[u] = edges.size() - 1;
    }

    // u->v with capacity cap and value val
    // note: if we use edges[idx] to store this edge
    //  edges[idx^1] is exactly the reversed edge
    void addEdges(int u, int v,
                  int cap, double val)
    {
        addEdge(u, v, cap, val);
        addEdge(v, u, 0, val);
        max_val = std::max(max_val, val);
    }

    void buildNetwork(const vector<pair<string, int>> &cap_info,
                      const vector<Assign> &assign_info)
    {
        // initialize head with -1
        head = vector<int>(2 + DC_num + task_num, -1);
        prev = vector<int>(2 + DC_num + task_num);

        // link source to DC
        for (const auto &it : cap_info)
        {
            int DC = DC_id.ID(it.first);
            addEdges(source, DC, it.second, 0);
        }

        // link DC to task
        for (const auto &it : assign_info)
        {
            auto item = it.second;
            int DC = DC_id.ID(item.first);
            int task = task_id.ID(item.second);

            // this is an assign edge
            addEdges(DC, task, 1, it.first);
        }

        // link task to sink
        for (const auto &it : task_id.loc)
        {
            int task = it.second;
            addEdges(task, sink, 1, 0);
        }
    }

    bool DinicBFS(vector<int> &d,
                  double val_bound)
    {
        d.assign(d.size(), 0);
        d[source] = 1;

        std::queue<int> Q;
        Q.push(source);
        while (!Q.empty())
        {
            int x = Q.front();
            Q.pop();
            for (int i = head[x]; ~i; i = edges[i].next)
            {
                int to = edges[i].v;
                if (!d[to] &&
                    edges[i].cap > 0 &&
                    edges[i].val <= val_bound)
                {
                    d[to] = d[x] + 1;
                    Q.push(to);
                }
            }
        }
        return d[sink];
    }

    int DinicDFS(const vector<int> &d,
                 double val_bound,
                 int x, int cur_flow)
    {
        if (cur_flow == 0 || x == sink)
            return cur_flow;
        if (d[x] >= d[sink]) // can not reach sink
            return 0;

        int ret = 0;
        for (int &i = cur_head[x]; ~i && cur_flow > 0;
             i = edges[i].next)
        {
            int to = edges[i].v;
            if (d[x] + 1 == d[to] &&
                edges[i].val <= val_bound)
            {
                int new_flow = DinicDFS(d, val_bound, to,
                                        std::min(cur_flow, edges[i].cap));
                edges[i].cap -= new_flow;
                edges[i ^ 1].cap += new_flow;
                ret += new_flow;
                cur_flow -= new_flow;
            }
        }
        return ret;
    }

    // use Dinic Algorithm to compute max flow
    // note: only use edges with value < val_bound
    int Dinic(double val_bound)
    {
        for (auto &edge : edges)
            edge.cap = edge.ori_cap;

        int ret = 0;
        // used to store layer info
        vector<int> d(2 + DC_num + task_num);
        while (DinicBFS(d, val_bound))
        {
            cur_head = head;
            ret += DinicDFS(d, val_bound, source,
                            std::numeric_limits<int>::max());
        }
        return ret;
    }

    // note: this is not common MCMF(min cost max flow)
    // this is actually min{cost[edge]} with max flow
    // here, we use binary search to find the answer
    double MCMF(int K) // K th iteration
    {
        double L = 0, R = max_val + eps * 10;

        while (fabs(R - L) > eps / 10)
        {
            double mid = (L + R) / 2;
            int flow = Dinic(mid);

            if (flow > task_num - K)
                printError("Compute Max Flow " + std::to_string(flow) +
                           " with Only " + std::to_string(task_num - K) +
                           " Tasks");

            if (flow == task_num - K)
                R = mid;
            else
                L = mid;
        }

        // invoke Dinic again
        // note: to avoid floating point precision
        //  cause some subtle bugs
        if (Dinic(R) < task_num - K)
            printError("No Enough Slots");

        return R;
    }

    // decrease this DC's capacity by 1
    // as we have assigned one task in it
    void decCapacityDC(int DC)
    {
        for (int i = head[source]; ~i;
             i = edges[i].next)
        {
            if (edges[i].v == DC)
            {
                edges[i].ori_cap--;
                return;
            }
        }
        printError("No Such DC!");
    }

    // - change edge with val<bound to 0
    //   no matter where they are, as long as cost not exceed T it's fine
    // - change edge with val>T to INF
    //   these edges will never be used, otherwise T is not bottleneck
    // note: change neck_task's edges to INF
    void updateJob(int job, int neck_task,
                   double val_bound)
    {
        // DC in [0,n-1]
        for (int i = 0; i < DC_num; ++i)
        {
            string DC = DC_id.Name(i);
            static const double INF = std::numeric_limits<double>::max();
            for (int j = head[i]; ~j; j = edges[j].next)
                if (edges[j].v != source)
                {
                    string task = task_id.Name(edges[j].v);

                    if (which_job.find(task) == which_job.end())
                    {
                        edges[j].print();
                        printError("No Such Task!");
                    }
                    if (edges[j].v == neck_task)
                        edges[j].val = edges[j ^ 1].val = INF;
                    else if (which_job[task] == job)
                    {
                        // edges[j].print();
                        edges[j].val = edges[j ^ 1].val =
                            (edges[j].val > val_bound + eps
                                 ? INF
                                 : 0);
                        // edges[j].print();
                    }
                }
        }
    }

    // find bottleneck task
    // then, assign this task, dec DC cap
    //  and update group nodes
    void findBottelneck(double val_bound)
    {
        // DC in [0,n-1]
        for (int i = 0; i < DC_num; ++i)
        {
            string DC = DC_id.Name(i);
            for (int j = head[i]; ~j; j = edges[j].next)
            {
                if (edges[j].v != source &&
                    edges[j].cap == 0 &&
                    fabs(edges[j].val - val_bound) < eps)
                {
                    // this is an assign edge
                    // val==val_bound indicates a bottleneck
                    string task = task_id.Name(edges[j].v);
                    // assign this task
                    auto item = make_pair(DC, task);
                    assigned.emplace_back(
                        make_pair(edges[j].ori_val, item));

                    // this edge should never be choosen
                    // in updateJob() now
                    // edges[j].val = std::numeric_limits<double>::max();

                    // not read by readSched
                    edges[j].cap = edges[j].ori_cap;

                    // DC cap--
                    decCapacityDC(i);
                    // update job
                    int job_id = which_job[task];
                    updateJob(job_id, edges[j].v, val_bound);

                    return;
                }
            }
        }
        printError("Do Not Find Bottleneck!");
    }

    // read (remaining) scheduled tasks from network
    void readSched()
    {
        // DC in [0,n-1]
        for (int i = 0; i < DC_num; ++i)
        {
            string DC = DC_id.Name(i);
            for (int j = head[i]; ~j; j = edges[j].next)
            {
                if (edges[j].v != source &&
                    edges[j].cap == 0)
                {
                    string task = task_id.Name(edges[j].v);
                    auto item = make_pair(DC, task);
                    assigned.emplace_back(
                        make_pair(edges[j].ori_val, item));
                }
            }
        }
    }

    // set edges with 0 val to ori val
    // may assign tasks to DC with high bandwidth
    void resetEdges()
    {
        for (auto &edge : edges)
            if (edge.val < eps)
                edge.val = edge.ori_val;
    }

    // use SPFA to find augmenting path
    int SPFA()
    {
        // cost
        vector<double> dis(2 + DC_num + task_num,
                           std::numeric_limits<double>::max());
        // capacity
        vector<int> cap(2 + DC_num + task_num, 0);
        vector<bool> inq(2 + DC_num + task_num, false);
        std::queue<int> Q;

        dis[source] = 0;
        cap[source] = std::numeric_limits<int>::max();
        inq[source] = true;
        Q.push(source);

        while (!Q.empty())
        {
            int x = Q.front();
            Q.pop(), inq[x] = false;
            for (int i = head[x]; ~i; i = edges[i].next)
            {
                int to = edges[i].v;
                double cost = edges[i].val;
                if (edges[i].cap > 0 &&
                    dis[x] + cost < dis[to])
                {
                    dis[to] = dis[x] + cost;
                    prev[to] = i; // edge id
                    cap[to] = std::min(cap[x], edges[i].cap);
                    if (!inq[to])
                        Q.push(to), inq[to] = true;
                }
            }
        }
        return cap[sink];
    }

    // update path
    void updateEdge(int flow)
    {
        for (int i = sink; i != source;
             i = edges[prev[i] ^ 1].v)
        {
            edges[prev[i]].cap -= flow;
            edges[prev[i] ^ 1].cap += flow;
        }
    }

    // min cost sum max flow
    int MCMF()
    {
        int ret = 0, new_flow;
        while (new_flow = SPFA())
        {
            ret += new_flow;
            updateEdge(new_flow);
        }
        return ret;
    }

    // main function schedule all tasks
    void scheduleFair()
    {
        // repeat until every job has one bottleneck task
        // {
        //      find a solution with MCMF
        //      find and remove bottleneck tasks, say need T time
        //      update network according to bottleneck tasks
        //          1. assign bottleneck to DC
        //          2. change edges' value of tasks in bottleneck group
        //             - change edge with val < T to 0
        //               no matter where they are, as long as cost not exceed T it's fine
        //             - change edge with val > T to INF
        //               these edges will never be used, otherwise T is not bottleneck
        //      update assigned tasks
        // }

        for (int i = 0; i < task_group.size(); ++i)
        {
            double val_bound = MCMF(i);

            // debug
            // readSched();
            // return;

            findBottelneck(val_bound);
        }
        // after find all bottlenecks
        // find min cost sum for remaining tasks
        resetEdges();
        MCMF();

        // assign rest tasks
        readSched();
        if (assigned.size() != task_num)
            printError("Incorrect Assigned Size!");
    }

    // only run MCMF one time
    // do not use task_group
    // minimize worse time of all tasks
    void scheduleSimple()
    {
        MCMF(0);
        readSched();
        if (assigned.size() != task_num)
            printError("Incorrect Assigned Size!");
    }

public:
    enum SchedType
    {
        FAIR,
        SIMPLE
    } sched_type;

    NetworkNeck() : max_val(0) {}

    // e.g. {{"tA1","tA2"},{"tB2"}} in task_group
    //  tA1 and tA2 belong to same job
    // e.g. {{"DC1",4}} in cap_info
    //  the capacity of DC1 is 4
    // e.g. {{4,{"DC1","tA1"}}} in assign_info
    //  if we assign tA1 to DC1, then it takes 4s to transfer data
    void initNetwork(int task_num,
                     vector<vector<string>> task_group,
                     vector<pair<string, int>> cap_info,
                     vector<Assign> assign_info)
    {
        DC_num = cap_info.size();
        this->task_num = task_num;

        DC_id.cur_idx = 0;
        task_id.cur_idx = DC_num;
        source = DC_num + task_num;
        sink = source + 1;

        // initialize job
        this->task_group = task_group;
        for (int i = 0; i < task_group.size(); ++i)
            for (const auto &task : task_group[i])
                which_job[task] = i;

        buildNetwork(cap_info, assign_info);
    }

    // schedule tasks to slots
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Assign> getSched()
    {
        switch (sched_type)
        {
        case SIMPLE:
            scheduleSimple();
            break;
        case FAIR:
            scheduleFair();
            break;
        }
        return assigned;
    }
};