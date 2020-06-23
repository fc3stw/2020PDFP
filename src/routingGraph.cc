#include "routingGraph.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>

RoutingGraph::RoutingGraph()
{
    _vertex_list.clear();
    _edge_list.clear();
}

RoutingGraph::RoutingGraph(const RoutingGraph *input_graph, bool vertex_only)
{
    int num_vertices = input_graph->get_num_vertices();
    for(int i = 0; i<num_vertices; i++){
        add_vertex(input_graph->get_vertex(i)->get_pos());
    }
    if(!vertex_only){
        int num_edges = input_graph->get_num_edges();
        for(int i = 0; i<num_edges; i++){
            add_edge(input_graph->get_edge(i)->get_v1()->get_id(), input_graph->get_edge(i)->get_v2()->get_id());
        }
    }
}

RoutingGraph::~RoutingGraph()
{
    for(Vertex *v : _vertex_list){
        delete v;
    }
    for(Edge *e : _edge_list){
        delete e;
    }
}

int RoutingGraph::get_num_vertices() const {return _vertex_list.size();}

int RoutingGraph::get_num_edges() const {return _edge_list.size();}

Vertex* RoutingGraph::get_vertex(int id) const {return _vertex_list.at(id);}

Edge* RoutingGraph::get_edge(int id) const {return _edge_list.at(id);}

int RoutingGraph::get_total_edge_weights() const
{
    int weight = 0;
    for(Edge *e : _edge_list){
        weight += e->get_weight();
    }
    return weight;
}

void RoutingGraph::add_vertex(Pos3d pos)
{
    int id = _vertex_list.size();
    _vertex_list.push_back(new Vertex(id, pos));
}

void RoutingGraph::add_edge(int v1, int v2)
{
    assert(v1 < get_num_vertices() && v1 >= 0);
    assert(v2 < get_num_vertices() && v2 >= 0);
    int id = _edge_list.size();
    int z1, x1, y1;
    int z2, x2, y2;
    tie(z1, x1, y1) = _vertex_list.at(v1)->get_pos();
    tie(z2, x2, y2) = _vertex_list.at(v2)->get_pos();
    int weight = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    Edge *new_edge = new Edge(id, _vertex_list.at(v1), _vertex_list.at(v2), weight);
    _edge_list.push_back(new_edge);
    _vertex_list.at(v1)->add_edge(new_edge);
    _vertex_list.at(v2)->add_edge(new_edge);
}

void RoutingGraph::remove_vertex(int id)
{
    // remove edges related to the vertex
    vector<int> edge_to_remove;
    for(Edge *e : _edge_list){
        if(e->get_v1()->get_id()==id || e->get_v2()->get_id()==id){
            edge_to_remove.push_back(e->get_id());
        }
    }

    // remove edges in reversed order
    for(int i = edge_to_remove.size()-1; i >= 0; i--){
        remove_edge(edge_to_remove.at(i));
    }
    
    // remove vertex
    Vertex *v = _vertex_list.at(id);
    delete v;
    _vertex_list.erase(_vertex_list.begin() + id);

    // fix vertex index
    int new_id = 0;
    for(Vertex *v : _vertex_list){
        v->set_id(new_id);
        new_id++;
    }
}

void RoutingGraph::remove_edge(int id) {
    // remove the edge from related vertex
    Edge *edge = _edge_list.at(id);
    for(Vertex *v : _vertex_list){
        set<Edge*> &edges = v->get_edges();
        set<Edge*>::iterator it = edges.find(edge);
        if(it!=edges.end()) edges.erase(it);
    }
    // remove the edge
    delete edge;
    _edge_list.erase(
        _edge_list.begin() + id);
    // fix edge index
    int new_id = 0;
    for(Edge *e : _edge_list){
        e->set_id(new_id);
        new_id++;
    }
}

void RoutingGraph::print() const
{
    cout<<"#vertices: "<<get_num_vertices()<<"\n";
    for(Vertex *v : _vertex_list){
        cout<<"  #"<<v->get_id()<<" ";
    }
    cout<<"\n";
    cout<<"#edges: "<<get_num_edges()<<"\n";
    for(Edge *e : _edge_list){
        cout<<"  #"<<e->get_id()<<" "<<e->get_v1()->get_id()<<"->"<<e->get_v2()->get_id()<<"\n";
    }
}

Vertex::Vertex(int id, const Pos3d &pos):
    _id(id),
    _pos(pos)
{}

int Vertex::get_id() const {return _id;}

Pos3d Vertex::get_pos() const {return _pos;}

set<Edge*>& Vertex::get_edges() {return _edges;}

void Vertex::set_id(int val) {_id = val;}

void Vertex::add_edge(Edge *e) {_edges.insert(e);}

Edge::Edge(int id, Vertex* v1, Vertex* v2, int weight):
    _id(id),
    _v1(v1),
    _v2(v2),
    _weight(weight)
{}

int Edge::get_id() const {return _id;}

Vertex* Edge::get_v1() const {return _v1;}

Vertex* Edge::get_v2() const {return _v2;}

int Edge::get_weight() const {return _weight;}

void Edge::set_id(int val) {_id = val;}
