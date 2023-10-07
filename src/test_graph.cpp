#include "graph.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <functional>
#include <exception>

static std::ostream &error(std::ostream &stream) {
  stream << "\x1b[1;31mERROR:\x1b[0m ";
  return stream;
}



void test(bool ok)	{
	static int numTest = 1;

	std::cout << "Test " << numTest << " : ";
	if (ok) std::cout << "OK" << std::endl;
	else std::cout<< "FAIL" << std::endl;

	numTest++;
}

void testTopo(Graph<int>& g)	{
	// labels should be from 0 to n-1
	const int n = g.nodes.size();

	std::vector<int> topo = topological(g);

	bool ok = (topo.size() == n);

	int location [n];
	for (int i = 0; i < n; i++)	location[i] = -1;
	for (int i = 0; i < n; i++)	{
		if (topo[i] < 0 || topo[i] >= n)	ok = false;
		else location[topo[i]] = i;
	}
	for (int i = 0; i < n; i++)
		if (location[i] == -1)	ok = false;

	if (ok)	{
		for (Node<int> node : g.nodes)
			for (Node<int> child : node.link_to)
				if (location[node.label] > location[child.label])	ok = false;
	}
	test(ok);
}




int main(int argc, const char *argv[]) {

	// cycles :
	Graph<int> H = mk_graph<int>();
	add_node(H,0);
	add_node(H,1);
	add_edge(H,0,1);
	test(!has_cycle(H));
	testTopo(H);
	add_edge(H,1,0);
	test(has_cycle(H));

	Graph<int> G = mk_graph<int>();
	add_node(G,1);	add_node(G,2);	add_node(G,3);	add_node(G,4);	add_node(G,0);
	test(!has_cycle(G));
	add_edge(G,1,2);	add_edge(G,1,4);	add_edge(G,1,0);
	add_edge(G,2,3);	add_edge(G,3,4);	add_edge(G,2,4);
	test(!has_cycle(G));
	add_edge(G,0,2);
	test(!has_cycle(G));
	testTopo(G);
	add_edge(G,3,1);
	test(has_cycle(G));

	Graph<int> K = mk_graph<int>();
	add_node(K,0);	add_node(K,1);	add_node(K,2);	add_node(K,3);	add_node(K,4);	add_node(K,5);
	add_node(K,6);	add_node(K,7);	add_node(K,8);	add_node(K,9);	add_node(K,10);	add_node(K,11);
	add_edge(K,6,0);	add_edge(K,9,6);	add_edge(K,4,3);	add_edge(K,7,0);add_edge(K,0,11); add_edge(K,8,2);
	test(!has_cycle(K));	testTopo(K);
	add_edge(K, 10,1);	add_edge(K, 9,10); add_edge(K, 1,6); add_edge(K, 6, 5); add_edge(K, 7, 4); add_edge(K,4,0); add_edge(K,11,3); add_edge(K,6,8);
	test(!has_cycle(K));	testTopo(K);

	add_edge(K, 11,1);
	test(has_cycle(K));

	bool cycleDetected = false;
	try	{topological(K);}	catch (const char* e) {if (e == "HasCycle") cycleDetected = true;}
	test(cycleDetected);


}
