#include "common.hpp"

// when constructing nodes
// we assign
//  DC in [0,n-1] (may out of order)
//  task in [n,n+m-1] (may out of order)
//  source to n+m, sink to n+m+1
class NetworkSched
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
        int cap;        // remaining capacity: cap-flow
        int next;       // point at next edge
        double ori_val; //original weight
        double val;     // weight
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
    // <----- Dinic end

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
        // layer
        vector<int> d(2 + DC_num + task_num, 0);
        while (DinicBFS(d, val_bound))
        {
            cur_head = head;
            ret += DinicDFS(d, val_bound, source,
                            std::numeric_limits<int>::max());
        }
        if (ret > task_num)
            printError("Compute Max Flow " + std::to_string(ret) +
                       " with Only " + std::to_string(task_num) + " Tasks");
        return ret;
    }

    // note: this is not common MCMF(min cost max flow)
    // this is actually min{cost[edge]} with max flow
    // here, we use binary search to find the answer
    double MCMF()
    {
        double L = 0, R = max_val;
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
        assigned.clear();
        // DC in [0,n-1]
        for (int i = 0; i < DC_num; ++i)
        {
            string DC = DC_id.Name(i);
            for (int j = head[i]; ~j; j = edges[j].next)
            {
                if (edges[j].v != source &&
                    edges[j].cap == 0)
                {
                    // this is an assign edge
                    // ## todo this is not correct
                    // !!!!!!!!!
                    string task = task_id.Name(edges[j].v);
                    auto item = make_pair(DC, task);
                    assigned.emplace_back(
                        make_pair(edges[j].ori_val, item));
                }
            }
        }
    }

    // main function schedule all tasks
    void schedule()
    {
        // while (assigned_task<total)
        // {
        //      find a solution with MCMF
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

        // not correct
        MCMF();
        readSched();

        while (assigned.size() < task_num)
        {
        }
    }

public:
    NetworkSched() : max_val(0) {}

    // e.g. {{"tA1","tA2"},{"tB2"}} in task_group
    //  tA1 and tA2 belong to same job
    // e.g. {{"DC1",4}} in cap_info
    //  the capacity of DC1 is 4
    // e.g. {{4,{"DC1","tA1"}}} in assign_info
    //  if we assign tA1 to DC1, then it takes 4s to transfer data
    void initNetwork(int DC_num, int task_num,
                     //  vector<vector<string>> task_group,
                     vector<pair<string, int>> cap_info,
                     vector<Assign> assign_info)
    {
        DC_id.cur_idx = 0;
        task_id.cur_idx = DC_num;
        source = DC_num + task_num;
        sink = source + 1;

        this->DC_num = DC_num;
        this->task_num = task_num;

        buildNetwork(cap_info, assign_info);

        // ####### todo
        // initTaskGroup()
    }

    // schedule tasks to slots
    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Assign> getSched()
    {
        schedule();
        return assigned;
    }
};