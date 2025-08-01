#include "map_routing.h"
#include <queue>
#include <unordered_set>

Graph map_routing::map;
vector<Edge> map_routing::edges;

// x: X-Axis position
// y: Y-Axis position 
// returns: IDs of nodes within r distance of (x,y)
vector<pair<int, double>> map_routing::getFilteredNodes(double x, double y, double r, Graph& map)
{
	vector<pair<int, double>> filteredNodes;
	filteredNodes.reserve(map.size());

	r /= 1000.0;
	double rSqr = r * r;

	double xLower = x - r, xUpper = x + r;
	double yLower = y - r, yUpper = y + r;

	int nodeCount = map.size();
	for (int i = 0; i < nodeCount; i++)
	{
		if (map[i].x >= xLower && map[i].x <= xUpper &&
			map[i].y >= yLower && map[i].y <= yUpper)
		{
			double xNode = x - map[i].x;
			double yNode = y - map[i].y;
			double distSq = (xNode * xNode) + (yNode * yNode);

			if (distSq <= rSqr)
				filteredNodes.emplace_back(i, sqrt(distSq));
		}
	}

	return filteredNodes;
}
/* map_routing.cpp */
Output map_routing::dijkstra(Graph& map, vector<Edge>& edges)
{
	vector<int> parent(map.size(), -1);
	vector<int> edgeID(map.size(), -1);
	vector<double> time(map.size(), DBL_MAX);
	vector<bool> visited(map.size());

	int start = map.size() - 2;
	int dest = map.size() - 1;
	time[start] = 0;

	priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pqueue;

	pqueue.push(make_pair(0, start));
	while (pqueue.size())
	{
		int node = pqueue.top().second;
		double nodeTime = pqueue.top().first;
		pqueue.pop();

		if (node == dest)
			break;

		if (visited[node])
			continue;
		visited[node] = true;

		for (const auto& adj : map[node].adjs)
		{
			if (edges[adj.second].time + time[node] < time[adj.first])
			{
				time[adj.first] = edges[adj.second].time + time[node];
				parent[adj.first] = node;
				edgeID[adj.first] = adj.second;
				pqueue.push(make_pair(time[adj.first], adj.first));
			}
		}
	}

	Output output;
	output.time = time[dest];
	output.distTotal = 0;

	int curNode = dest;
	while (parent[curNode] != -1)
	{
		output.distTotal += edges[edgeID[curNode]].length;
		output.nodeIDs.push_back(curNode);
		curNode = parent[curNode];
	}

	reverse(output.nodeIDs.begin(), output.nodeIDs.end());

	if (parent[dest] == -1)
	{
		output.distWalk = DBL_MAX;
		output.distDrive = DBL_MAX;
	}
	else
	{
		output.nodeIDs.pop_back();
		output.distWalk = edges[map[start].getEdgeID(output.nodeIDs.front())].length + edges[map[dest].getEdgeID(output.nodeIDs.back())].length;
		output.distDrive = output.distTotal - output.distWalk;
	}

	return output;
}

// query: Current Query [use start(x,y) and destination(x,y)]
// startNodes: Nodes that can be reached from start(x,y)
// destNodes: Nodes that can be reached from destination(x,y)
// returns: Output (check Output fields for more info)
// NOTE: 
//		Walking is always at 5 km/h
Output map_routing::getShortestPath(Query query, vector<pair<int, double>> startNodes, vector<pair<int, double>> destNodes, Graph& map, vector<Edge>& edges)
{
	int mapCount = map.size();
	int edgeCount = edges.size();
	int startCount = startNodes.size();
	int destCount = destNodes.size();

	Node start;
	start.x = query.startX;
	start.y = query.startY;
	start.adjs.reserve(startCount);

	for (const auto& edge : startNodes)
	{
		edges.push_back(Edge(edge.second, (edge.second * 60.0) / 5.f));
		start.adjs.push_back(make_pair(edge.first, edges.size() - 1));
		map[edge.first].adjs.push_back(make_pair(mapCount, edges.size() - 1));
	}

	map.push_back(start);

	Node dest;
	dest.x = query.destX;
	dest.y = query.destY;
	dest.adjs.reserve(destCount);

	for (const auto& edge : destNodes)
	{
		edges.push_back(Edge(edge.second, (edge.second * 60.0) / 5.f));
		dest.adjs.push_back(make_pair(edge.first, edges.size() - 1));
		map[edge.first].adjs.push_back(make_pair(mapCount + 1, edges.size() - 1));
	}

	map.push_back(dest);

	Output output = dijkstra(map, edges);

	for (const auto& node : map[mapCount].adjs)
		map[node.first].adjs.pop_back();

	for (const auto& node : map[mapCount + 1].adjs)
		map[node.first].adjs.pop_back();

	map.resize(mapCount);
	edges.resize(edgeCount);

	return output;
}

Output map_routing::solveQuery(Query query, Graph& map, vector<Edge>& edges)
{
	vector<pair<int, double>> startNodes = getFilteredNodes(query.startX, query.startY, query.radius, map);
	vector<pair<int, double>> destNodes = getFilteredNodes(query.destX, query.destY, query.radius, map);

	return getShortestPath(query, startNodes, destNodes, map, edges);
}