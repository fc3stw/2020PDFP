#include "routingFlow.h"
#include "globalRouter.h"
#include <algorithm>

Router::Router(Chip &chip, vector<Net*> &net_list):
    _chip(chip),
    _net_list(net_list),
    _sorted_net_list(net_list)
{}

bool sort_net_by_hpwl(Net *n1, Net *n2) {return n1->get_hpwl() > n2->get_hpwl();}

void Router::routing_flow()
{
    // sort net by wl
    sort(_sorted_net_list.begin(), _sorted_net_list.end(), sort_net_by_hpwl);

    for(Net *net : _sorted_net_list){
        vector<Vertex*> vertex_list;
        for(int i = 0; i<net->get_num_pins(); i++){
            vertex_list.push_back(
                new Vertex(net->get_pin(i)->get_pos())
            );
        }
        // global route: build minimum spanning tree
        GlobalRouter gRouter(vertex_list);
        vector<Edge*> edge_list = gRouter.global_route();
        // detail route: find a route for each 2-pin net
        // compute wl

        // collect candidates for Steiner points
        set<Pos3d> steiner_points = collect_steiner_points(net);
        for(Pos3d sp : steiner_points){
            vertex_list.push_back(new Vertex(sp));
            // global route: build minimum spanning tree
            GlobalRouter gRouter(vertex_list);
            vector<Edge*> edge_list = gRouter.global_route();
            // detail route: find a route for each 2-pin net
            // compute wl
            // remove the steiner points if wl doesn't decrease
            // remove redundant steiner points
        }
    }
}

set<Pos3d> Router::collect_steiner_points(Net *net)
{
    set<Pos3d> steiner_points;
    int num_pins = net->get_num_pins();
    // for each pin pair
    for(int i = 0; i<num_pins-1; i++){
        for(int j = i+1; j<num_pins; j++){
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
                set<int> nets = _chip.get_grid(hp).get_nets();
                int unrouted_net_cnt = 0;
                for(int net_id : nets){
                    if(_net_list.at(net_id)->get_route().empty()) unrouted_net_cnt++;
                }
                if(_chip.get_grid(hp).get_remain_supply() > unrouted_net_cnt) steiner_points.insert(hp);
            }
        }
    }
    return steiner_points;
}
