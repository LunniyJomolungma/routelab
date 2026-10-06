#include "routelab/report.hpp"
#include <iomanip>
#include <iostream>
#include <limits>

unsigned number(const std::string& text) {
    if(text.empty() || text.size()>10 || !std::all_of(text.begin(),text.end(),[](char c){return c>='0'&&c<='9';}))throw std::runtime_error("Expected a nonnegative integer.");
    const auto value=std::stoull(text);
    if(value>std::numeric_limits<unsigned>::max())throw std::runtime_error("Integer is too large.");
    return static_cast<unsigned>(value);
}
void usage(){std::cout<<"RouteLab | weighted route experiments\n\n  routelab compare MAP [REPORT.html]\n  routelab solve MAP bfs|dijkstra|astar\n  routelab generate ROWS COLS SEED OUTPUT.map\n\nMap: S=start G=goal #=wall .=cost 1 digits 1..9=terrain cost\n";}
int main(int argc,char* argv[]){
    try{
        if(argc==1 || (argc==2 && std::string(argv[1])=="--help")){usage();return 0;}
        const std::string command=argv[1];
        if(command=="generate" && argc==6){
            const auto rows=number(argv[2]),cols=number(argv[3]);
            if(rows>100||cols>100)throw std::runtime_error("Generated dimensions must be 3..100.");
            const auto grid=routelab::generate(static_cast<int>(rows),static_cast<int>(cols),number(argv[4]));
            routelab::save_grid(grid,argv[5]);std::cout<<"Saved reproducible map to "<<argv[5]<<'\n';return 0;
        }
        if(!((command=="compare"&&(argc==3||argc==4))||(command=="solve"&&argc==4)))throw std::runtime_error("Invalid arguments. Run routelab --help.");
        const auto grid=routelab::load_grid(argv[2]);
        std::vector<routelab::Algorithm> algorithms;
        if(command=="compare")algorithms={routelab::Algorithm::BFS,routelab::Algorithm::Dijkstra,routelab::Algorithm::AStar};
        else{
            const std::string selected=argv[3];
            if(selected=="bfs")algorithms={routelab::Algorithm::BFS};
            else if(selected=="dijkstra")algorithms={routelab::Algorithm::Dijkstra};
            else if(selected=="astar")algorithms={routelab::Algorithm::AStar};
            else throw std::runtime_error("Algorithm must be bfs, dijkstra or astar.");
        }
        std::vector<routelab::Result> results;
        std::cout<<std::left<<std::setw(12)<<"Algorithm"<<std::setw(12)<<"Cost"<<std::setw(12)<<"Steps"<<"Expanded\n";
        for(auto algorithm:algorithms){
            auto result=routelab::search(grid,algorithm);
            std::cout<<std::setw(12)<<routelab::name(algorithm)<<std::setw(12)<<(result.found?std::to_string(result.cost):"unreachable")<<std::setw(12)<<(result.found?std::to_string(result.path.size()-1):"-")<<result.explored.size()<<'\n';
            results.push_back(result);
        }
        if(command=="compare"){
            const std::string report=argc==4?argv[3]:"report.html";
            routelab::write_report(grid,results,argv[2],report);std::cout<<"\nOpen "<<report<<" in a browser.\n";
        }else if(results[0].found){
            auto display=grid.cells;
            for(int id:results[0].path)if(id!=grid.start&&id!=grid.goal)display[id/grid.cols()][id%grid.cols()]='*';
            std::cout<<'\n';for(const auto& row:display)std::cout<<row<<'\n';
        }
        return 0;
    }catch(const std::exception& error){std::cerr<<"Error: "<<error.what()<<'\n';return 1;}
}
