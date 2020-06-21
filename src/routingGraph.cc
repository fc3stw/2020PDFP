#include "routingGraph.h"
#include <cmath>
#include <algorithm>

RoutingGraph::RoutingGraph(const RoutingGraph &input_graph, bool vertex_only)
{
    int num_vertices = input_graph.get_num_vertices();
    for(int i = 0; i<num_vertices; i++){
        add_vertex(input_graph.get_vertex(i)->get_pos());
    }
    if(vertex_only) return;
    int num_edges = input_graph.get_num_edges();
    for(int i = 0; i<num_edges; i++){
        add_edge(input_graph.get_edge(i)->get_v1(), input_graph.get_edge(i)->get_v2());
    }
}

int RoutingGraph::get_num_vertices() const {return _vertex_list.size();}

int RoutingGraph::get_num_edges() const {return _edge_list.size();}

Vertex* RoutingGraph::get_vertex(int id) const {return _vertex_list.at(id);}

Edge* RoutingGraph::get_edge(int id) const {return _edge_list.at(id);}

void RoutingGraph::add_vertex(Pos3d pos)
{
    int id = _vertex_list.size();
    _vertex_list.push_back(new Vertex(id, pos));
}

void RoutingGraph::add_edge(int v1, int v2)
{
    int id = _edge_list.size();
    int z1, x1, y1;
    int z2, x2, y2;
    tie(z1, x1, y1) = _vertex_list.at(v1)->get_pos();
    tie(z2, x2, y2) = _vertex_list.at(v2)->get_pos();
    int weight = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _edge_list.push_back(new Edge(id, v1, v2, weight));
    _vertex_list.at(v1)->add_edge(id);
    _vertex_list.at(v2)->add_edge(id);
}

void RoutingGraph::remove_edge(int id) {
    _edge_list.erase(
        _edge_list.begin() + id);
}

Vertex::Vertex(int id, const Pos3d &pos):
    _id(id),
    _pos(pos)
{}

Pos3d Vertex::get_pos() const {return _pos;}

set<int>& Vertex::get_edges() {return _edge_ids;}

void Vertex::add_edge(int edge_id) {_edge_ids.insert(edge_id);}

Edge::Edge(int id, int v1, int v2, int weight):
    _id(id),
    _v1(v1),
    _v2(v2),
    _weight(weight)
{}

int Edge::get_v1() const {return _v1;}

int Edge::get_v2() const {return _v2;}

int Edge::get_weight() const {return _weight;}
