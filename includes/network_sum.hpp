#include "common.hpp"

// NOTE: min cost sum max flow

// when constructing nodes
// we assign
//  DC in [0,n-1] (may out of order)
//  task in [n,n+m-1] (may out of order)
//  source to n+m, sink to n+m+1
class NetworkSum
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
    int assign_num;

    // -----> MCMF begin
    int source, sink;
    struct Edge
    {
        int v;    // to v
        int cap;  // residual capacity: cap-flow
        int next; // point at next edge
        double cost;

        // debug
        void print()
        {
            std::cout << "to: " << v << '\n'
                      << "val: " << cost << std::endl;
        }
    };
    // adjacent list
    vector<int> head;
    // array of all edges
    // note: i^1 is residual edge
    vector<Edge> edges;
    vector<int> prev;
    // <----- MCMF end

    // e.g. {{4,{"DC1","tA1"}}}
    //  assign tA1 to DC1, takes 4s to transfer data
    vector<Assign> assigned;

private:
    void addEdge(int u, int v,
                 int cap, double val)
    {
        Edge e;
        e.next = head[u], e.v = v;
        e.cap = cap, e.cost = val;
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
                double cost = edges[i].cost;
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
                        make_pair(edges[j].cost, item));
                }
            }
        }
    }

    // minimize cost sum and find max flow
    void schedule()
    {
        MCMF();
        readSched();
        if (assigned.size() != assign_num)
            printError("Incorrect Assigned Size!");
    }

public:
    // e.g. {{"DC1",4}} in cap_info
    //  the capacity of DC1 is 4
    // e.g. {{4,{"DC1","tA1"}}} in assign_info
    //  if we assign tA1 to DC1, then it takes 4s to transfer data
    void initNetwork(int task_num,
                     int assign_num, // maybe no enough slots
                     vector<pair<string, int>> cap_info,
                     vector<Assign> assign_info)
    {
        DC_num = cap_info.size();
        this->task_num = task_num;
        this->assign_num = assign_num;

        DC_id.cur_idx = 0;
        task_id.cur_idx = DC_num;
        source = DC_num + task_num;
        sink = source + 1;

        buildNetwork(cap_info, assign_info);
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