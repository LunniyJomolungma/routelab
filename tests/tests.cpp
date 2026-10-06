#include "routelab/report.hpp"
#include <iostream>
#include <numeric>
#include <sstream>
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
routelab::Grid parse(const std::string& text){std::istringstream stream(text);return routelab::read_grid(stream);}
void valid_path(const routelab::Grid& grid,const routelab::Result& result){
    check(result.found,"Expected route");check(result.path.front()==grid.start&&result.path.back()==grid.goal,"Wrong endpoints");
    int cost=0;for(std::size_t i=1;i<result.path.size();++i){const auto neighbors=grid.neighbors(result.path[i-1]);check(std::find(neighbors.begin(),neighbors.end(),result.path[i])!=neighbors.end(),"Path crossed wall or jumped");cost+=grid.cost(result.path[i]);}
    check(cost==result.cost,"Cost mismatch");
}
int main(){
    auto weighted=parse("S999G\n.....\n");
    auto bfs=routelab::search(weighted,routelab::Algorithm::BFS);
    auto dijkstra=routelab::search(weighted,routelab::Algorithm::Dijkstra);
    check(bfs.cost==28&&bfs.path.size()==5,"BFS must minimize steps");check(dijkstra.cost==6,"Dijkstra must avoid expensive route");
    auto blocked=parse("S#G\n###\n");
    for(auto a:{routelab::Algorithm::BFS,routelab::Algorithm::Dijkstra,routelab::Algorithm::AStar})check(!routelab::search(blocked,a).found,"Blocked goal must be unreachable");
    for(const auto* invalid:{"", "S..\n.G\n", "SSG", "S?G", "...G", "S..\n\n..G"}){
        bool rejected=false;try{parse(invalid);}catch(const std::exception&){rejected=true;}check(rejected,"Malformed map accepted");
    }
    check(parse("\xEF\xBB\xBFS.G\r\n").cols()==3,"BOM/CRLF handling");
    for(unsigned seed=0;seed<100;++seed){
        auto grid=routelab::generate(12,17,seed);
        check(grid.cells==routelab::generate(12,17,seed).cells,"Generation not reproducible");
        auto d=routelab::search(grid,routelab::Algorithm::Dijkstra),a=routelab::search(grid,routelab::Algorithm::AStar);
        valid_path(grid,d);valid_path(grid,a);check(d.cost==a.cost,"A* differs from Dijkstra");
    }
    check(routelab::escape_html("<script>&\"")=="&lt;script&gt;&amp;&quot;","HTML escaping");
    std::cout<<"All RouteLab tests passed; 100 seeded weighted maps checked.\n";
}
