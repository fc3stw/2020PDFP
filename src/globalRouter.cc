#include "globalRouter.h"
#include <cmath>

Vertex::Vertex(const Pos &pos):
    _pos(pos)
{}

Pos Vertex::get_pos() const {return _pos;}

Edge::Edge(Vertex *v1, Vertex *v2):
    _v1(v1),
    _v2(v2)
{}

Vertex* Edge::get_v1() const {return _v1;}

Vertex* Edge::get_v2() const {return _v2;}

int Edge::get_wl() const
{
    return abs(get<0>(_v1->get_pos()) - get<0>(_v2->get_pos()))
        + abs(get<1>(_v1->get_pos()) - get<1>(_v2->get_pos()))
        + abs(get<2>(_v1->get_pos()) - get<2>(_v2->get_pos()));
}

GlobalRouter::GlobalRouter(const vector<Vertex*> &vertex_list)
{
    _vertex_list = vertex_list;
    for(int i = 0; i<_vertex_list.size()-1; i++){
        for(int j = i+1; j<_vertex_list.size(); j++){
            _edge_list.push_back(
                new Edge(_vertex_list[i], _vertex_list[j]));
        }
    }
}

vector<Edge*>& GlobalRouter::global_route()
{
    return _min_spanning_tree;
}
