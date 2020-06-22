#include "routingGraph.h"
#include <tuple>
#include <vector>

using namespace std;

class GlobalRouter{
    RoutingGraph _original_graph;
    RoutingGraph _mst;
public:
    GlobalRouter(const RoutingGraph &original_graph);

    RoutingGraph get_mst() const;

    void route();
};