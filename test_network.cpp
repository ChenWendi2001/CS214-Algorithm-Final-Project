#include "network_sched.hpp"

using std::min;

vector<vector<string>> task_group;
vector<pair<string, int>> cap_info;
vector<pair<double,
            pair<string, string>>>
    assign_info;

// // sample from paper
// void initData()
// {
//     task_group.push_back({"tA1", "tA2"});
//     task_group.push_back({"tB1", "tB2"});

//     cap_info.emplace_back(make_pair("DC1", 2));
//     cap_info.emplace_back(make_pair("DC2", 2));
//     cap_info.emplace_back(make_pair("DC3", 1));

//     assign_info.emplace_back(
//         make_pair(200.0 / 100,
//                   std::move(make_pair("DC1", "tA1"))));
//     assign_info.emplace_back(
//         make_pair(min(100.0 / 80, 200.0 / 160),
//                   std::move(make_pair("DC2", "tA1"))));
//     assign_info.emplace_back(
//         make_pair(100.0 / 150,
//                   std::move(make_pair("DC3", "tA1"))));

//     assign_info.emplace_back(
//         make_pair(200.0 / 100,
//                   std::move(make_pair("DC1", "tA2"))));
//     assign_info.emplace_back(
//         make_pair(min(100.0 / 80, 200.0 / 160),
//                   std::move(make_pair("DC2", "tA2"))));
//     assign_info.emplace_back(
//         make_pair(100.0 / 150,
//                   std::move(make_pair("DC3", "tA2"))));

//     assign_info.emplace_back(
//         make_pair(min(200.0 / 80, 200.0 / 100),
//                   std::move(make_pair("DC1", "tB1"))));
//     assign_info.emplace_back(
//         make_pair(200.0 / 160,
//                   std::move(make_pair("DC2", "tB1"))));
//     assign_info.emplace_back(
//         make_pair(200.0 / 120,
//                   std::move(make_pair("DC3", "tB1"))));

//     assign_info.emplace_back(
//         make_pair(min(200.0 / 80, 300.0 / 100),
//                   std::move(make_pair("DC1", "tB2"))));
//     assign_info.emplace_back(
//         make_pair(300.0 / 160,
//                   std::move(make_pair("DC2", "tB2"))));
//     assign_info.emplace_back(
//         make_pair(200.0 / 120,
//                   std::move(make_pair("DC3", "tB2"))));
// }

// min-max case
void initData()
{
    task_group.push_back({"tA1", "tA2"});
    task_group.push_back({"tB1", "tB2"});

    cap_info.emplace_back(make_pair("DC1", 2));
    cap_info.emplace_back(make_pair("DC2", 1));
    cap_info.emplace_back(make_pair("DC3", 1));

    assign_info.emplace_back(
        make_pair(2,
                  std::move(make_pair("DC1", "tB1"))));
    assign_info.emplace_back(
        make_pair(2,
                  std::move(make_pair("DC3", "tB2"))));
    assign_info.emplace_back(
        make_pair(2.5,
                  std::move(make_pair("DC1", "tB2"))));
    assign_info.emplace_back(
        make_pair(4,
                  std::move(make_pair("DC2", "tA1"))));
    assign_info.emplace_back(
        make_pair(3.5,
                  std::move(make_pair("DC1", "tA2"))));
    assign_info.emplace_back(
        make_pair(3,
                  std::move(make_pair("DC3", "tA2"))));
}

int main()
{
    initData();
    NetworkSched f;
    // std::shuffle(assign_info.begin(),
    //              assign_info.end(),
    //              std::mt19937(time(0)));
    f.sched_type = NetworkSched::FAIR;
    f.initNetwork(4, task_group,
                  cap_info, assign_info);

    auto ans = f.getSched();
    for (const auto &it : ans)
        std::cout << it.first << ' '
                  << it.second.first << ' '
                  << it.second.second << std::endl;
    return 0;
}