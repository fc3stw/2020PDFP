#include "module.h"
#include "globalRouter.h"

struct Route{
    set<Pos3d> _grids;
    Pos3d _wavefront;
    int cost;

    Route(){}
    Route(Pos3d source, Pos3d target);
    Route(Route *route, Pos3d wavefront, Pos3d target);
};

struct SortRouteByCost{
    bool operator() (const Route *r1, const Route *r2) const{
        return r1->cost < r2->cost;
    }
};

class Router{
    Chip &_chip;
    vector<Net*> _net_list;
    vector<RoutingGraph*> _steiner_trees;
public:
    Router(Chip &chip, vector<Net*> &net_list);

    int get_total_wl() const;

    void routing_flow();
    void ripup_all_routes();
    void add_net_on_grids();
    void global_route();
    void detail_route();
    set<Pos3d> collect_steiner_points(Net *net);
    Route* Astar_search(int net_id, Pos3d source, Pos3d target);
    bool check_new_route(int net_id, Route *route, Pos3d wavefront);
};
