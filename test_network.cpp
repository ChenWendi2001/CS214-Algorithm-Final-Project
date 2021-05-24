#include "network_sched.hpp"

using std::min;

vector<pair<string, int>> cap_info;
vector<pair<double,
            pair<string, string>>>
    assign_info;

// sample from paper
void initData()
{
    cap_info.emplace_back(make_pair("DC1", 2));
    cap_info.emplace_back(make_pair("DC2", 2));
    cap_info.emplace_back(make_pair("DC3", 1));

    assign_info.emplace_back(
        make_pair(200.0 / 100,
                  std::move(make_pair("DC1", "tA1"))));
    assign_info.emplace_back(
        make_pair(min(100.0 / 80, 200.0 / 160),
                  std::move(make_pair("DC2", "tA1"))));
    assign_info.emplace_back(
        make_pair(100.0 / 150,
                  std::move(make_pair("DC3", "tA1"))));

    assign_info.emplace_back(
        make_pair(200.0 / 100,
                  std::move(make_pair("DC1", "tA2"))));
    assign_info.emplace_back(
        make_pair(min(100.0 / 80, 200.0 / 160),
                  std::move(make_pair("DC2", "tA2"))));
    assign_info.emplace_back(
        make_pair(100.0 / 150,
                  std::move(make_pair("DC3", "tA2"))));

    assign_info.emplace_back(
        make_pair(min(200.0 / 80, 200.0 / 100),
                  std::move(make_pair("DC1", "tB1"))));
    assign_info.emplace_back(
        make_pair(200.0 / 160,
                  std::move(make_pair("DC2", "tB1"))));
    assign_info.emplace_back(
        make_pair(200.0 / 120,
                  std::move(make_pair("DC3", "tB1"))));

    assign_info.emplace_back(
        make_pair(min(200.0 / 80, 300.0 / 100),
                  std::move(make_pair("DC1", "tB2"))));
    assign_info.emplace_back(
        make_pair(300.0 / 160,
                  std::move(make_pair("DC2", "tB2"))));
    assign_info.emplace_back(
        make_pair(200.0 / 120,
                  std::move(make_pair("DC3", "tB2"))));
}

int main()
{
    initData();
    NetworkSched f;
    f.initNetwork(3, 4,
                  cap_info, assign_info);

    f.getSched();
    return 0;
}