#pragma once
#include "search.hpp"
#include <sstream>

namespace routelab {
inline std::string escape_html(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '&') result += "&amp;"; else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;"; else if (c == '"') result += "&quot;";
        else result += c;
    }
    return result;
}
inline std::string map_svg(const Grid& grid, const Result& result) {
    std::ostringstream svg;
    const int tile = 26;
    std::vector<bool> visited(grid.size(),false), path(grid.size(),false);
    for (int id : result.explored) visited[id]=true;
    for (int id : result.path) path[id]=true;
    svg << "<svg role='img' aria-label='" << name(result.algorithm) << " route map' viewBox='0 0 " << grid.cols()*tile << ' ' << grid.rows()*tile << "' xmlns='http://www.w3.org/2000/svg'>";
    svg << "<title>" << name(result.algorithm) << " route: " << (result.found ? "found" : "unreachable") << "</title>";
    for (int id=0; id<grid.size(); ++id) {
        const char cell=grid.at(id);
        const int x=id%grid.cols()*tile, y=id/grid.cols()*tile;
        std::string fill = cell=='#' ? "#182638" : path[id] ? "#65e0bc" : visited[id] ? "#294d67" : grid.cost(id)>1 ? "#66513a" : "#243347";
        if(id==grid.start)fill="#f7c66c";
        if(id==grid.goal)fill="#a7a0ff";
        svg << "<rect x='" << x+1 << "' y='" << y+1 << "' width='24' height='24' rx='4' fill='" << fill << "'/>";
        if(cell=='S'||cell=='G'||(cell>='1'&&cell<='9'))
            svg << "<text x='"<<x+13<<"' y='"<<y+18<<"' text-anchor='middle' fill='"<<(path[id]||cell=='S'||cell=='G'?"#142432":"#ffffff")<<"' font-family='monospace' font-size='13'>"<<cell<<"</text>";
    }
    svg << "</svg>";
    return svg.str();
}
inline void write_report(const Grid& grid, const std::vector<Result>& results, const std::string& title, const std::string& filename) {
    std::ofstream out(filename);
    if(!out)throw std::runtime_error("Cannot write report: "+filename);
    out << R"(<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>RouteLab — route comparison</title><style>
*{box-sizing:border-box}body{margin:0;background:#0d1520;color:#edf4fa;font:16px/1.6 system-ui,sans-serif}main{max-width:1460px;margin:auto;padding:56px 40px}.eyebrow{color:#65e0bc;text-transform:uppercase;letter-spacing:3px;font-size:12px;font-weight:800}h1{font-size:clamp(36px,5vw,66px);letter-spacing:-3px;line-height:1.1;margin:16px 0}h2{font-size:24px;margin:0}.lead{color:#a8b8c9;max-width:740px}.chips{display:flex;gap:10px;flex-wrap:wrap;margin:26px 0 40px}.chip{border:1px solid #34475c;border-radius:20px;padding:6px 14px;font-size:13px}.cards{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:20px}.card{background:#142131;border:1px solid #2a3b50;border-radius:18px;padding:24px;min-width:0}.index{color:#73899e;font-size:12px;letter-spacing:2px}.tag{color:#65e0bc;font-size:13px;margin:4px 0 22px;min-height:42px}.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin:24px 0}.stats b{display:block;font-size:25px}.stats span{font-size:11px;color:#b1c3d5;text-transform:uppercase;letter-spacing:1px}svg{width:100%;height:auto;display:block}.muted{color:#a8b8c9;font-size:13px}.legend{display:flex;gap:22px;flex-wrap:wrap;margin:30px 0}.legend span::before{content:'';display:inline-block;width:10px;height:10px;background:var(--c);border-radius:3px;margin-right:8px}.note{border-top:1px solid #293c51;padding-top:24px;margin-top:26px;color:#a8b8c9;font-size:14px}@media(max-width:950px){.cards{grid-template-columns:1fr}main{padding:30px 20px}.card svg{max-height:440px}}@media print{body{background:white;color:#111}.card{break-inside:avoid}.cards{display:block}.card{margin-bottom:20px}}
</style><main><div class="eyebrow">RouteLab / Algorithm field notes</div><h1>One map. Three decisions.</h1><p class="lead">Compare the shortest route by steps with the cheapest route by terrain cost. Every highlighted path is calculated by the C++ engine.</p><div class="chips"><span class="chip">)";
    out << escape_html(title) << "</span><span class='chip'>" << grid.cols()<<" × "<<grid.rows()<<" grid</span><span class='chip'>4-direction movement</span><span class='chip'>No external assets</span></div><section class='cards'>";
    int index=1;
    for(const auto& r:results){
        out << "<article class='card'><div class='index'>EXPERIMENT 0"<<index++<<"</div><h2>"<<name(r.algorithm)<<"</h2><div class='tag'>"<<(r.algorithm==Algorithm::BFS?"Fewest steps · terrain weights ignored during search":r.algorithm==Algorithm::Dijkstra?"Lowest cost · explores by accumulated distance":"Lowest cost · guided by Manhattan distance")<<"</div>";
        out<<map_svg(grid,r)<<"<div class='stats'><div><b>"<<(r.found?std::to_string(r.cost):"—")<<"</b><span>Route cost</span></div><div><b>"<<(r.found?std::to_string(r.path.size()-1):"—")<<"</b><span>Steps</span></div><div><b>"<<r.explored.size()<<"</b><span>Expanded</span></div></div><p class='muted'>"<<(r.found?"Route found":"No route exists")<<" · "<<r.microseconds<<" μs in this run</p></article>";
    }
    out<<R"(</section><div class="legend"><span style="--c:#65e0bc">Final path</span><span style="--c:#294d67">Expanded node</span><span style="--c:#66513a">Weighted terrain</span><span style="--c:#f7c66c">Start</span><span style="--c:#a7a0ff">Goal</span></div><p class="note">Cost is paid when entering a cell; the start cell is free. Dijkstra and A* find minimum-cost routes because all walkable costs are positive. BFS minimizes steps, not terrain cost. Timing is one measurement on this machine, not a performance guarantee. This offline report contains no scripts, external fonts, or network requests.</p></main></html>)";
    out.flush(); if(!out)throw std::runtime_error("Failed to save report.");
}
}
