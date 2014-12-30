#pragma once
#include <iostream>
#include <initializer_list>
#include <map>
#include <set>

namespace sal {


template <typename V>
struct WEdge {
	V source;
	V dest;
	int weight;
	// can't maintain as POD because V might not be POD
	WEdge(V u, V v, int w) : source{u}, dest{v}, weight{w} {} 
};
template <typename V>
struct UEdge {
	V source;
	V dest;
	UEdge(V u, V v) : source{u}, dest{v} {} 
};

// iterate over neighbours of a vertex 
// for adjacency lists just a wrapper around map's iterator
template <typename Iter>
struct Adjacent_iterator {
	using CR = const Adjacent_iterator<Iter>&;
	using V = typename Iter::value_type::first_type;
	Iter cur;

	void operator++() {++cur;}
	void operator--() {--cur;}
	V operator*() {return cur->first;}
	bool operator==(CR other) {return other.cur == cur;}
	bool operator!=(CR other) {return !(*this == other);}
	V& dest() {return cur->first;}
	int& weight() {return cur->second;}
};
template <typename Iter>
struct Adjacent_const_iterator {
	using CR = const Adjacent_const_iterator<Iter>&;
	using V = typename Iter::value_type::first_type;
	Iter cur;

	void operator++() {++cur;}
	void operator--() {--cur;}
	V operator*() {return cur->first;}
	bool operator==(CR other) {return other.cur == cur;}
	bool operator!=(CR other) {return !(*this == other);}
	V dest() {return cur->first;}
	int weight() {return cur->second;}
};

// iterate over vertices of a graph
// for adjacency lists just a wrapper around map's iterator
template <typename Iter>
struct Vertex_iterator {
	using CR = const Vertex_iterator<Iter>&;
	using V = typename Iter::value_type::first_type;
	using adjacent_iterator = Adjacent_iterator<typename Iter::value_type::second_type::iterator>;
	Iter cur;

	void operator++() {++cur;}
	void operator--() {--cur;}
	V operator*() {return cur->first;}
	bool operator==(CR other) {return other.cur == cur;}
	bool operator!=(CR other) {return !(*this == other);}
	std::pair<adjacent_iterator, adjacent_iterator> adjacent() {
		return {{cur->second.begin()}, {cur->second.end()}};
	}
	adjacent_iterator begin() 	{return {cur->second.begin()};}
	adjacent_iterator end() 	{return {cur->second.end()};}
};
template <typename Iter>
struct Vertex_const_iterator {
	using CR = const Vertex_const_iterator<Iter>&;
	using V = typename Iter::value_type::first_type;
	using adjacent_const_iterator = Adjacent_const_iterator<typename Iter::value_type::second_type::const_iterator>;
	Iter cur;

	void operator++() {++cur;}
	void operator--() {--cur;}
	V operator*() {return cur->first;}
	bool operator==(CR other) {return other.cur == cur;}
	bool operator!=(CR other) {return !(*this == other);}
	std::pair<adjacent_const_iterator, adjacent_const_iterator> adjacent() const {
		return {{cur->second.begin()}, {cur->second.end()}};
	}
	adjacent_const_iterator begin() const 	{return {cur->second.begin()};}
	adjacent_const_iterator end() const 	{return {cur->second.end()};}
};


// undirected weighted graph implementation
template <typename V = int, typename Edges = std::map<V, int>>
class Adjacency_list {
protected:
	// destination and weight
	// internally vertices can be integers/indices since they
	// can be labeled vertex 0, vertex 1, vertex 2, ...
	using Adj = std::map<V, Edges>;
	Adj adj;
public:
	using vertex_type = V;
	using adjacent_iterator = Adjacent_iterator<typename Edges::iterator>;
	using adjacent_const_iterator = Adjacent_const_iterator<typename Edges::const_iterator>;
	using iterator = Vertex_iterator<typename Adj::iterator>;
	using const_iterator = Vertex_const_iterator<typename Adj::const_iterator>;
	using reverse_iterator = Vertex_iterator<typename Adj::reverse_iterator>;
	using const_reverse_iterator = Vertex_iterator<typename Adj::const_reverse_iterator>;
	// constructors
	Adjacency_list() = default;
	Adjacency_list(const std::initializer_list<UEdge<V>>& l) {
		for (const auto& edge : l) {
			adj[edge.source][edge.dest] = 1;
			adj[edge.dest][edge.source] = 1;
		}		
	}	
	Adjacency_list(const std::initializer_list<WEdge<V>>& l) {
		for (const auto& edge : l) {
			adj[edge.source][edge.dest] = edge.weight;
			adj[edge.dest][edge.source] = edge.weight;
		}		
	}
	template <typename Iter_edgelist>
	Adjacency_list(Iter_edgelist begin, const Iter_edgelist end) {
		while (begin != end) {
			adj[begin->source][begin->dest] = begin->get_weight();
			adj[begin->dest][begin->source] = begin->get_weight();
			++begin;
		}
	}
	Adjacency_list(Adjacency_list&& g) : adj{std::move(g.adj)} {}
	~Adjacency_list() = default;

	// cardinality of vertex set and edge set
	size_t num_vertex() const {return adj.size();}
	virtual size_t num_edge() const {
		size_t edges {};
		for (auto v = adj.begin(); v != adj.end(); ++v)
			edges += v->second.size();
		return edges >> 1;	// divide by 2 for unweighted
	} 

	// check for existence of vertex and edge
	bool is_vertex(V v) const {return (adj.find(v) != adj.end());}
	bool is_edge(V u, V v) const {
		auto u_itr = adj.find(u);
		if (u_itr == adj.end()) return false;
		auto v_itr = u_itr->second.find(v);
		return !(v_itr == u_itr->second.end());
	}

	// weight of edge, 1/0 for unweighted edge
	int weight(V u, V v) const {
		auto u_itr = adj.find(u);
		if (u_itr == adj.end()) return 0;
		auto v_itr = u_itr->second.find(v);
		return (v_itr == u_itr->second.end())? 0 : v_itr->second;		
	}
	size_t degree(V v) const {
		auto v_itr = adj.find(v); 
		return (v_itr == adj.end())? 0 : v_itr->second.size();
	}

	// begin and end
	std::pair<adjacent_iterator, adjacent_iterator> adjacent(V v) {
		auto v_itr = adj.find(v);
		if (v_itr != adj.end()) 
			return {{v_itr->second.begin()}, {v_itr->second.end()}};
		else return {{},{}};
	}	
	std::pair<adjacent_const_iterator, adjacent_const_iterator> adjacent(V v) const {
		auto v_itr = adj.find(v);
		if (v_itr != adj.end()) 
			return {{v_itr->second.begin()}, {v_itr->second.end()}};
		else return {{},{}};
	}
	// vertex iteration
	iterator vertex(V v) 		 		{return {adj.find(v)};}
	iterator begin() 					{return {adj.begin()};}
	iterator end() 						{return {adj.end()};}
	const_iterator vertex(V v) const 	{return {adj.find(v)};}
	const_iterator begin() const 		{return {adj.begin()};}
	const_iterator end() const   		{return {adj.end()};}
	reverse_iterator rbegin() 			 	{return {adj.rbegin()};}
	reverse_iterator rend() 				{return {adj.rend()};}
	const_reverse_iterator rbegin() const 	{return {adj.rbegin()};}
	const_reverse_iterator rend() const  	{return {adj.rend()};}


	V min_vertex() const {
		if (adj.empty()) return {};	// default
		return adj.begin()->first;
	}

	// modifier interface
	void add_vertex(V v) {adj[v];}
	// undirected
	virtual void add_edge(V u, V v, int weight = 1) {
		adj[u][v] = weight;
		adj[v][u] = weight;
	}

	// printing
	friend ostream& operator<<(ostream& os, const Adjacency_list& g) {
		for (const auto& vertex : g.adj) {
			os << vertex.first << '(';
			if (!vertex.second.empty()) {
				for (const auto& neighbour : vertex.second)
					os << neighbour.first << ':' << neighbour.second << ',';
				// backspace to remove extra comma
				os << '\b';
			}
			os << ") ";
		}
		return os;
	}
};

// directed adjacency list
template <typename V = int, typename Edges = std::map<V, int>>
class Adjacency_list_directed : public Adjacency_list<V, Edges> {
	using Adjacency_list<V, Edges>::adj;
public:
	using vertex_type = V;
	using adjacent_iterator = typename Adjacency_list<V, Edges>::adjacent_iterator;
	using adjacent_const_iterator = typename Adjacency_list<V, Edges>::adjacent_const_iterator;
	using iterator = typename Adjacency_list<V, Edges>::iterator;
	using const_iterator = typename Adjacency_list<V, Edges>::const_iterator;
	// constructors
	Adjacency_list_directed() = default;
	Adjacency_list_directed(const std::initializer_list<WEdge<V>>& l) {
		for (auto& edge : l) {
			adj[edge.source][edge.dest] = edge.weight;
			adj[edge.dest];
		}		
	}	
	Adjacency_list_directed(const std::initializer_list<UEdge<V>>& l) {
		for (auto& edge : l) {
			adj[edge.source][edge.dest] = 1;
			adj[edge.dest];
		}		
	}
	template <typename Iter_edgelist>
	Adjacency_list_directed(Iter_edgelist begin, const Iter_edgelist end) {
		while (begin != end) {
			adj[begin->source][begin->dest] = begin->get_weight();
			adj[begin->dest];	// default construct destination vertex
			++begin;
		}
	}
	Adjacency_list_directed(Adjacency_list_directed&& g) {adj = std::move(g.adj);}
	~Adjacency_list_directed() = default;

	// most operations the same as undirected, degree is outdegree
	size_t num_edge() const override {
		size_t edges {};
		for (auto v = adj.begin(); v != adj.end(); ++v)
			edges += v->second.size();
		return edges;
	} 
	// add edge only to source vertex
	void add_edge(V u, V v, int weight = 1) override {
		adj[u][v] = weight;
		adj[v]; // default construct destination vertex
	}
};


}	// end namespace sal