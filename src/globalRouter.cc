#include "globalRouter.h"
#include <cmath>

GlobalRouter::GlobalRouter(RoutingGraph &original_graph):
    _original_graph(original_graph, false),
    _mst(original_graph, true)
{}

RoutingGraph GlobalRouter::get_mst() const {return _mst;}

void GlobalRouter::global_route()
{}
