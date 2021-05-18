#include "common.hpp"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

shared_ptr<Graph> graph;

void init_data();

int main(){
    init_data();
    return 0;
}

void init_data(){
    graph = graph = make_shared<Graph>();
    //
    //initialize graph->resources
    //
    json DC;
    std::ifstream DC_file("./ToyData/DC.json");
    DC_file>>DC;
    
    int num_of_dc = DC["DC"].size();
    for(int i = 0;i<num_of_dc;++i){
        graph->resources[DC["DC"][i]["name"]].insert(DC["DC"][i]["data"].begin(),DC["DC"][i]["data"].end());
    }

    //Debug code

    // for(int i = 0;i<num_of_dc;++i){
    //     auto set = graph->resources[DC["DC"][i]["name"]];
    //     for(auto iter = set.begin();iter!=set.end();++iter){
    //         std::cout<<*iter<<"\t";
    //     }
    //     std::cout<<std::endl;
    // }


    //
    //initialize edges
    //

    json link;
    std::ifstream link_file("./ToyData/link.json");
    link_file >> link;

    for(int i = 0;i<num_of_dc;++i){
        for(int j = 0;j<num_of_dc;++j){
            if(link["link"][i]["bandwidth"][j]==-1)
                graph->edges[link["link"][i]["start"]][link["link"][j]["start"]] = 0xFFFF;
            else 
                graph->edges[link["link"][i]["start"]][link["link"][j]["start"]] = 1/double(link["link"][i]["bandwidth"][j]);
        }
    }

    //Floyd
    for(int i = 0;i<num_of_dc;++i){
        for(int j = 0;j<num_of_dc;++j){
            for(int k = 0;k<num_of_dc;++k){
                auto dc_i = DC["DC"][i]["name"];
                auto dc_j = DC["DC"][j]["name"];
                auto dc_k = DC["DC"][k]["name"];
                if(graph->edges[dc_i][dc_j]>graph->edges[dc_i][dc_k]+graph->edges[dc_k][dc_j]){
                    graph->edges[dc_i][dc_j]=graph->edges[dc_i][dc_k]+graph->edges[dc_k][dc_j];
                }
            }
        }
    }

    //Debug code

    // for(int i = 0;i<num_of_dc;++i){
    //     for(int j = 0;j<num_of_dc;++j){
    //          std::cout<<int(1/graph->edges[DC["DC"][i]["name"]][DC["DC"][j]["name"]])<<"\t";
    //     }
    //     std::cout<<std::endl;
    // }

    //
    //initialize slots
    //
    for(int i = 0;i<num_of_dc;++i){
        graph->slots[DC["DC"][i]["name"]].first = DC["DC"][i]["size"];
    }

    //Debug code
    // for(int i = 0;i<num_of_dc;++i){
    //     std::cout<<graph->slots[DC["DC"][i]["name"]].first<<"\t";
    // }
    
}