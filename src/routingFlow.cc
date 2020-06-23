#include "routingFlow.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include <iostream>

Route::Route(Pos3d source, Pos3d target)
{
    _grids.insert(source);
    _wavefront = source;
    int z1, x1, y1;
    int z2, x2, y2;
    tie(z1, x1, y1) = source;
    tie(z2, x2, y2) = target;
    int dist = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _cost = _grids.size() + dist;
    // _cost = _grids.size();
}

Route::Route(Route const *route, Pos3d wavefront, Pos3d target)
{
    _grids = route->_grids;
    _grids.insert(wavefront);
    _wavefront = wavefront;
    int z1, x1, y1;
    int z2, x2, y2;
    tie(z1, x1, y1) = wavefront;
    tie(z2, x2, y2) = target;
    int dist = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _cost = _grids.size() + dist;
    // _cost = _grids.size();
}

void Route::print() const
{
    cout<<"Length: "<<_grids.size()<<"\n";
    cout<<"Cost: "<<_cost<<"\n";
    cout<<"wavefront: "<<get_pos_str(_wavefront)<<"\n";
    for(const Pos3d &pos : _grids){
        cout<<get_pos_str(pos)<<" ";
    }
    cout<<"\n";
}

Router::Router(Chip &chip, vector<Net*> &net_list):
    _chip(chip),
    _net_list(net_list)
{}

int Router::get_total_wl() const
{
    int wl = 0;
    for(Net *net : _net_list){
        wl += net->get_route().size();
    }
    return wl;
}

int Router::get_total_hpwl() const
{
    int hpwl = 0;
    for(Net *net : _net_list){
        hpwl += net->get_hpwl();
    }
    return hpwl; 
}

bool sort_net_by_hpwl(Net *n1, Net *n2) {return n1->get_hpwl() > n2->get_hpwl();}

void Router::routing_flow()
{
    cout<<"Rip up all routes\n";
    ripup_all_routes();
    cout<<"Add net on gGrids with pins\n";
    add_net_on_grids();

    // sort net by wl
    sort(_net_list.begin(), _net_list.end(), sort_net_by_hpwl);

    // global route: build a Steiner tree for each net
    cout<<"Global route\n";
    global_route();
    // print_global_route_result();

    // detail route: find a route for each Steiner tree
    cout<<"Detail route\n";
    detail_route();
    print_detail_route_result();
}

void Router::ripup_all_routes()
{
    for(Net *net : _net_list){
        net->get_route().clear();
    }
}

void Router::add_net_on_grids()
{
    // clear net on all grids
    int num_layers = _chip.get_num_layers();
    int num_rows = _chip.get_num_rows();
    int num_cols = _chip.get_num_cols();
    for(int layer = 0; layer < num_layers; layer++){
        for(int row = 0; row < num_rows; row++){
            for(int col = 0; col < num_cols; col++){
                _chip.get_grid(Pos3d(layer, row, col)).get_nets().clear();
            }
        }
    }
    
    // add net on grids
    for(Net *net : _net_list){
        int net_id = net->get_id();
        int num_pins = net->get_num_pins();
        for(int pid = 0; pid < num_pins; pid++){
            Pos3d pos = net->get_pin(pid)->get_pos();
            _chip.get_grid(pos).add_net(net_id);
        }
    }
}

void Router::global_route()
{
    GlobalRouter global_router;
    _steiner_trees.clear();
    for(Net const *net : _net_list){
        // build a routing graph for each net
        RoutingGraph *graph = new RoutingGraph;
        int num_pins = net->get_num_pins();
        for(int pid = 0; pid < num_pins; pid++){
            Pin *pin = net->get_pin(pid);
            graph->add_vertex(pin->get_pos());
        }
        for(int i = 0; i < num_pins-1; i++){
            for(int j = i+1; j < num_pins; j++){
                graph->add_edge(i, j);
            }
        }

        // global route: build minimum spanning tree
        global_router.set_graph(graph);
        global_router.route();
        RoutingGraph *mst = new RoutingGraph(global_router.get_mst(), false);
        global_router.clear_graph();
        
        // compute wl: sum of edge weight
        int min_wl = mst->get_total_edge_weights();

        // collect candidates for Steiner points
        set<Pos3d> steiner_points;
        steiner_points = collect_steiner_points(net);
        
        // ignore net with less than 3 pins (no need for Steiner point)
        if(net->get_num_pins()<3) steiner_points.clear();
        // steiner_points.clear();

        for(Pos3d sp : steiner_points){
            // add a steiner point
            graph->add_vertex(sp);
            int num_vertices = graph->get_num_vertices();
            for(int i = 0; i < num_vertices-1; i++){
                graph->add_edge(i, num_vertices-1);
            }

            // global route: build minimum spanning tree
            global_router.set_graph(graph);
            global_router.route();
            RoutingGraph *new_mst = new RoutingGraph(global_router.get_mst(), false);
            global_router.clear_graph();

            // remove the steiner points if wl doesn't decrease
            int new_wl = new_mst->get_total_edge_weights();
            if(new_wl < min_wl){
                delete mst;
                mst = new_mst;
                min_wl = new_wl;
            }
            else{
                delete new_mst;
                int last_vertex_id = graph->get_num_vertices()-1;
                graph->remove_vertex(last_vertex_id);
            }

            // remove redundant steiner points
            num_vertices = mst->get_num_vertices();
            // in reversed order
            for(int sid = num_vertices-1; sid >= num_pins; sid--){
                Vertex *v = graph->get_vertex(sid);
                int vertex_degree = v->get_edges().size();
                // redundant steiner point if degree < 3
                if(vertex_degree < 3){
                    graph->remove_vertex(sid);
                }
            }
        }
        delete graph;

        // add net id to the corresponding gGrid of steiner points
        int num_vertices = mst->get_num_vertices();
        for(int sid = num_pins; sid < num_vertices; sid++){
            Pos3d pos = mst->get_vertex(sid)->get_pos();
            _chip.get_grid(pos).add_net(net->get_id());
        }
        
        _steiner_trees.push_back(mst);
    }
}

void Router::detail_route()
{
    int num_nets = _net_list.size();
    for(int net_id = 0; net_id < num_nets; net_id++){
        Net *net = _net_list.at(net_id);
        net->print();
        RoutingGraph *graph = _steiner_trees.at(net_id);

        // find a route for each edge on the steiner tree
        set<Pos3d> net_route;
        int num_edges = graph->get_num_edges();
        for(int edge_id = 0; edge_id < num_edges; edge_id++){
            Edge *edge = graph->get_edge(edge_id);
            Pos3d source = edge->get_v1()->get_pos();
            Pos3d target = edge->get_v2()->get_pos();
            Route *edge_route = Astar_search(net_id, source, target);
            net_route.insert(edge_route->_grids.begin(), edge_route->_grids.end());
            delete edge_route;
        }

        // remove redundant routing grid by steiner point
        // TODO

        for(Pos3d pos : net_route){
            net->add_route(pos);
            bool ret = _chip.get_grid(pos).add_demand(1);
            assert(
                ret && "Fail to add net demand on gGrid"
            );
        }
    }
}

set<Pos3d> Router::collect_steiner_points(Net const *net)
{
    set<Pos3d> steiner_points;
    int num_pins = net->get_num_pins();
    // for each pin pair
    for(int i = 0; i<num_pins-1; i++){
        for(int j = i+1; j<num_pins; j++){
            assert(i < net->get_num_pins());
            assert(j < net->get_num_pins());
            Pin *p1 = net->get_pin(i);
            Pin *p2 = net->get_pin(j);

            // Hanan points only occurs if two pins have common surface
            // Hanan points will not occur if two pins have common line
            int z1, x1, y1;
            int z2, x2, y2;
            tie(z1, z1, y1) = p1->get_pos();
            tie(z2, x2, y2) = p2->get_pos();
            vector<Pos3d> hanan_points(2,Pos3d(0,0,0)); // at most 2 points
            
            if(z1==z2){
                if(x1==x2 || y1==y2) continue;
                hanan_points[0] = Pos3d(z1, x1, y2);
                hanan_points[1] = Pos3d(z1, x2, y1);
            }
            else if(x1==x2){
                if(z1==z2 || y1==y2) continue;
                hanan_points[0] = Pos3d(z1, x1, y2);
                hanan_points[1] = Pos3d(z2, x1, y1);
            }
            else if(y1==y2){
                if(z1==z2 || x1==x2) continue;
                hanan_points[0] = Pos3d(z1, x1, y1);
                hanan_points[1] = Pos3d(z2, x2, y1);
            }
            else{
                // no common surface
                continue;
            }

            // check supply on the Hanan point
            for(Pos3d hp : hanan_points){
                // ignore hanan point out of chip
                if(!_chip.has_grid(hp)) continue;
                gGrid &grid = _chip.get_grid(hp);
                int net_demand = grid.get_nets().size();
                if(_chip.get_grid(hp).get_remain_supply() > net_demand) steiner_points.insert(hp);
            }
        }
    }
    return steiner_points;
}

Route* Router::Astar_search(int net_id, Pos3d source, Pos3d target)
{
    Route *best_route;
    Route *new_route = new Route(source, target);
    if(source==target) return new_route;

    priority_queue<Route*, vector<Route*>, SortRouteByCost> pq_route;
    pq_route.push(new_route);
    while(!pq_route.empty()){
        Route *route = pq_route.top();
        pq_route.pop();

        // check destination
        if(route->_wavefront == target){
            best_route = route;
            break;
        }

        // identify layer
        int layer, row, col;
        tie(layer, row, col) = route->_wavefront;
        Direction dir = _chip.get_layer_dir(layer);
        // propagate wave front to 2 adjacent layer and 2 adjacent grid
        // up
        Pos3d wavefront = Pos3d(layer+1, row, col);
        if(check_new_route(net_id, route, wavefront)){
            // cout<<"add wavefront "<<get_pos_str(wavefront)<<endl;
            new_route = new Route(route, wavefront, target);
            pq_route.push(new_route);
        }
        // down
        wavefront = Pos3d(layer-1, row, col);
        if(check_new_route(net_id, route, wavefront)){
            new_route = new Route(route, wavefront, target);
            pq_route.push(new_route);
        }
        // left or back
        if(dir==H) wavefront = Pos3d(layer, row, col-1);
        else wavefront = Pos3d(layer, row-1, col);
        if(check_new_route(net_id, route, wavefront)){
            new_route = new Route(route, wavefront, target);
            pq_route.push(new_route);
        }
        // right or front
        if(dir==H) wavefront = Pos3d(layer, row, col+1);
        else wavefront = Pos3d(layer, row+1, col);
        if(check_new_route(net_id, route, wavefront)){
            new_route = new Route(route, wavefront, target);
            pq_route.push(new_route);
        }
        
        delete route;
    }

    // clear remaining routes in pq
    while(!pq_route.empty()){
        delete pq_route.top();
        pq_route.pop();
    }

    return best_route;
}

bool Router::check_new_route(int net_id, Route *route, Pos3d wavefront)
{
    // check wavefront
    if(!_chip.has_grid(wavefront)) return false;
    // check redundant wavefront
    if(route->_grids.find(wavefront)!=route->_grids.end()) return false;
    // check remain supply
    set<int> &net_ids = _chip.get_grid(wavefront).get_nets();
    int unrouted_net_cnt = 0;
    for(int nid : net_ids){
        if(nid==net_id) continue;
        if(!_net_list.at(nid)->get_route().empty()) continue;
        unrouted_net_cnt++;
    }
    if(_chip.get_grid(wavefront).get_remain_supply() - unrouted_net_cnt <= 0) return false;
    
    return true;
}

void Router::print_global_route_result() const
{
    int net_id = 0;
    for(RoutingGraph *graph : _steiner_trees){
        cout<<"Net #"<<_net_list.at(net_id)->get_id()<<"\n";
        graph->print();
        net_id++;
    }
}

void Router::print_detail_route_result() const
{
    for(Net *net : _net_list){
        cout<<"Net #"<<net->get_id()<<" wire length: "<<net->get_route().size()<<"\n";;
    }
}
