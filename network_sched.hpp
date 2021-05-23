#include "common.hpp"

// when construct nodes
// we assign source to 0, sink to 1
//  DC in [2,n+1] with n DC in total, (may out of order)
//  and task in [n+2,...] (may out of order)
class NetworkSched
{
private:
    static const double eps = 1e-8;

    struct Location
    {
        // {"DC1",2} means DC1 is node 2 in network
        unordered_map<string, int> loc;
        int cur_idx;

        int get(const string &name)
        {
            if (loc.find(name) == loc.end())
                loc[name] = cur_idx++;
            else
                return loc[name];
        }
    };
    Location DC_id, task_id;
    int task_num, DC_num;

    // -----> Dinic begin
    const int source, sink;
    struct Edge
    {
        int v;      // to v
        int cap;    // remaining capacity: cap-flow
        int next;   // point at next edge
        double val; // weight
    };
    // max and min among edges' value
    double max_val, min_val;
    // adjacent list
    vector<int> head;
    // array of all edges
    // note: i^1 is residual edge
    vector<Edge> edges;
    // current edge optimization
    vector<int> cur_head;
    // <----- Dinic end

    // e.g. {{"DC1","tA1"}}
    //  assign tA1 to DC1
    vector<pair<string, string>> assigned;

private:
    void addEdge(int u, int v,
                 int cap, double val)
    {
        Edge e;
        e.next = head[u], e.v = v;
        e.cap = cap, e.val = val;
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
        min_val = std::min(min_val, val);
        max_val = std::max(max_val, val);
    }

    void buildNetwork(const vector<pair<string, int>> &cap_info,
                      const vector<pair<double, pair<string, string>>> &assign_info)
    {
        // initialize head with -1
        head = vector<int>(2 + DC_num + task_num, -1);

        // link source to DC
        for (const auto &it : cap_info)
        {
            int DC = DC_id.get(it.first);
            addEdges(source, DC, it.second, 0);
        }

        // link DC to task
        for (const auto &it : assign_info)
        {
            auto loc = it.second;
            int DC = DC_id.get(loc.first);
            int task = task_id.get(loc.second);

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
        for (auto &di : d)
            di = 0;
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
                    edges[i].val < val_bound)
                {
                    d[to] = d[x] + 1;
                    Q.push(to);
                }
            }
        }
        return d[sink];
    }

    int DinicDFS(const vector<int> &d,
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
            if (d[x] + 1 == d[to])
            {
                int new_flow = DinicDFS(d, to,
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
        int ret;
        // layer
        vector<int> d(2 + DC_num + task_num, 0);
        while (DinicBFS(d, val_bound))
        {
            cur_head = head;
            ret += DinicDFS(d, source,
                            std::numeric_limits<int>::max());
        }
        return ret;
    }

    // note: this is not common MCMF(min cost max flow)
    // this is actually min{cost[edge]} with max flow
    // here, we use binary search to find the answer
    double MCMF()
    {
        double L = min_val, R = max_val;
        while (fabs(R - L) > eps)
        {
            double mid = (L + R) / 2;
            if (Dinic(mid) == task_num)
                R = mid;
            else
                L = mid;
        }
        return R;
    }

    // read scheduled tasks from network
    void readSched()
    {
    }

    // main function schedule all tasks
    void schedule()
    {
        // while (assigned_task<total)
        // {
        //      find a solution with min{cost[edge]}
        //      find and remove bottleneck tasks, say need T time
        //      update network according to bottleneck tasks
        //          1. assign bottleneck to DC
        //          2. change edges' value of tasks in bottleneck group
        //             - change edge with val<T to 0
        //               no matter where they are, as long as cost not exceed T it's fine
        //             - change edge with val>T to INF
        //               these edges will never be used, otherwise T is not bottleneck
        //      update assigned tasks
        // }

        while (assigned.size() < task_num)
        {
        }
    }

public:
    NetworkSched() : source(0), sink(1)
    {
        min_val = std::numeric_limits<double>::max();
        max_val = 0;
    }

    // e.g. {{"tA1","tA2"},{"tB2"}} in task_group
    //  tA1 and tA2 belong to same job
    // e.g. {{"DC1",4}} in cap_info
    //  the capacity of DC1 is 4
    // e.g. {{4,{"DC1","tA1"}}} in assign_info
    //  if we assign tA1 to DC1, then it takes 3.5s to transfer data
    void initNetwork(int DC_num, int task_num,
                     vector<vector<string>> task_group,
                     vector<pair<string, int>> cap_info,
                     vector<pair<double, pair<string, string>>> assign_info)
    {
        DC_id.cur_idx = 2;
        task_id.cur_idx = DC_num + 1;
        this->DC_num = DC_num;
        this->task_num = task_num;

        buildNetwork(cap_info, assign_info);

        // ####### todo
        // initTaskGroup()
    }

    // schedule tasks to slots
    // e.g. {{"DC1","tA1"}}
    //  assign tA1 to DC1
    vector<pair<string, string>> getSched()
    {
        schedule();
        return assigned;
    }
};