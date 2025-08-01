#include "bonus.h"
#include "map_routing.h"
#include <fstream>
#include <queue>

double bonus::interval;
int bonus::numOfSpeeds;
vector<EdgeVar> bonus::edgesVar;

unordered_map<int, double> bonus::getFilteredNodes(double x, double y, double r, Graph& map)
{
	r = r / 1000.f;
	double xLower = x - r, xUpper = x + r;
	double yLower = y - r, yUpper = y + r;

	list<int> nodesRecheck;

	int numOfNodes = map.size();
	for (int i = 0; i < numOfNodes; i++)
	{
		if (map[i].x <= xUpper && map[i].x >= xLower &&
			map[i].y <= yUpper && map[i].y >= yLower)
			nodesRecheck.push_back(i);
	}

	r *= r;
	unordered_map<int, double> filteredNodes;
	filteredNodes.reserve(nodesRecheck.size());

	auto nodesRecheckEnd = nodesRecheck.end();
	for (auto node = nodesRecheck.begin(); node != nodesRecheckEnd; node++)
	{
		double xTemp = x - map[*node].x;
		double yTemp = y - map[*node].y;
		double distSq = (xTemp * xTemp) + (yTemp * yTemp);
		if (distSq <= r)
			filteredNodes.insert(make_pair(*node, sqrt(distSq)));
	}

	return filteredNodes;
}

struct ParentInfo {
	int parentOfParent;
	int index;
	double time;
	ParentInfo(int parentOfParent, int index, double time)
	{
		this->parentOfParent = parentOfParent;
		this->index = index;
		this->time = time;
	}
};

struct QueueInfo
{
	int cur;
	int parent;
	int curIndex;
	QueueInfo(int cur, int parent, int curIndex)
	{
		this->cur = cur;
		this->parent = parent;
		this->curIndex = curIndex;
	}
};

typedef unordered_map<int, vector<ParentInfo>> Parents;
typedef pair<double, QueueInfo> QueueData;
struct QueueComparator {
	bool operator()(const QueueData& a, const QueueData& b) const {
		return a.first > b.first;
	}
};

Output bonus::dijkstra(Graph& map, vector<EdgeVar>& edgesVar)
{
	vector<Parents> parent(map.size());

	int start = map.size() - 2;
	int dest = map.size() - 1;
	parent[start][-1].push_back(ParentInfo(-1, -1, 0.0));

	priority_queue<QueueData, vector<QueueData>, QueueComparator> pqueue;

	int doneParentID = -1;
	ParentInfo doneParent(-1, -1, -1.0);

	pqueue.push(make_pair(0.0, QueueInfo(start, -1, 0)));
	while (pqueue.size())
	{
		auto node = pqueue.top();
		pqueue.pop();

		if (node.second.cur == dest)
		{
			doneParentID = node.second.parent;
			doneParent = parent[node.second.cur][node.second.parent][node.second.curIndex];
			break;
		}

		for (const auto& adj : map[node.second.cur].adjs)
		{
			if (adj.first != node.second.parent)
			{
				double edgeTime = edgesVar[adj.second].durations[static_cast<int>(node.first / interval) % numOfSpeeds];
				edgeTime += parent[node.second.cur][node.second.parent][node.second.curIndex].time;

				int newCurIndex = parent[adj.first][node.second.cur].size();
				parent[adj.first][node.second.cur].push_back(ParentInfo(node.second.parent, node.second.curIndex, edgeTime));
				pqueue.push(make_pair(edgeTime, QueueInfo(adj.first, node.second.cur, newCurIndex)));
			}
		}
	}

	Output output;

	output.nodeIDs.reserve(100);

	int curNode = dest;
	output.distTotal = 0;
	output.time = doneParent.time;

	while (doneParentID != -1)
	{
		output.nodeIDs.push_back(curNode);
		output.distTotal += edgesVar[map[curNode].getEdgeID(doneParentID)].length;

		int tempParentID = doneParentID;

		curNode = doneParentID;
		doneParentID = doneParent.parentOfParent;

		doneParent = parent[tempParentID][doneParent.parentOfParent][doneParent.index];
	}

	reverse(output.nodeIDs.begin(), output.nodeIDs.end());
	output.nodeIDs.pop_back();

	output.distWalk = edgesVar[map[start].getEdgeID(output.nodeIDs.front())].length + edgesVar[map[dest].getEdgeID(output.nodeIDs.back())].length;
	output.distDrive = output.distTotal - output.distWalk;

	return output;
}

Output bonus::getShortestPath(Query query, unordered_map<int, double> startNodes, unordered_map<int, double> destNodes,
	Graph& map, vector<EdgeVar>& edgesVar)
{
	int mapCount = map.size();
	int edgeCount = edgesVar.size();
	int startCount = startNodes.size();
	int destCount = destNodes.size();

	Node start;
	start.x = query.startX;
	start.y = query.startY;
	start.adjs.reserve(startCount);

	for (const auto& edge : startNodes)
	{
		EdgeVar temp;
		temp.length = edge.second;
		temp.durations.reserve(numOfSpeeds);
		double duration = (edge.second / 5.0) * 60.0;

		for (int i = 0; i < numOfSpeeds; i++)
			temp.durations.push_back(duration);

		edgesVar.push_back(temp);
		start.adjs.push_back(make_pair(edge.first, edgesVar.size() - 1));
		map[edge.first].adjs.push_back(make_pair(mapCount, edgesVar.size() - 1));
	}

	map.push_back(start);

	Node dest;
	dest.x = query.destX;
	dest.y = query.destY;
	dest.adjs.reserve(destCount);

	for (const auto& edge : destNodes)
	{
		EdgeVar temp;
		temp.length = edge.second;
		temp.durations.reserve(numOfSpeeds);
		double duration = (edge.second * 60.0) / 5.0;

		for (int i = 0; i < numOfSpeeds; i++)
			temp.durations.push_back(duration);

		edgesVar.push_back(temp);
		dest.adjs.push_back(make_pair(edge.first, edgesVar.size() - 1));
		map[edge.first].adjs.push_back(make_pair(mapCount + 1, edgesVar.size() - 1));
	}

	map.push_back(dest);

	Output output = dijkstra(map, edgesVar);

	for (const auto& node : map[mapCount].adjs)
		map[node.first].adjs.pop_back();

	for (const auto& node : map[mapCount + 1].adjs)
		map[node.first].adjs.pop_back();

	map.resize(mapCount);
	edgesVar.resize(edgeCount);

	return output;
}

Output bonus::solveQuery(Query query, Graph& map, vector<EdgeVar>& edgesVar)
{
	unordered_map<int, double> startNodes = getFilteredNodes(query.startX, query.startY, query.radius, map);
	unordered_map<int, double> destNodes = getFilteredNodes(query.destX, query.destY, query.radius, map);

	return getShortestPath(query, startNodes, destNodes, map, edgesVar);
}

void bonus::readMap(string type, string fileName)
{
	ifstream file;
	file.open("TEST CASES\\" + type + "\\Input\\" + fileName + ".txt");

	int numOfNodes;
	file >> numOfNodes;
	map_routing::map.reserve(numOfNodes + 2);

	while (numOfNodes--)
	{
		int index;
		Node node{};

		file >> index >> node.x >> node.y;
		map_routing::map.push_back(node);
	}

	int numOfEdges;
	file >> numOfEdges >> numOfSpeeds >> interval;
	edgesVar.reserve(numOfEdges);

	for (int i = 0; i < numOfEdges; i++)
	{
		int node1, node2;
		EdgeVar edge{};

		file >> node1 >> node2 >> edge.length;

		edge.durations.reserve(numOfSpeeds);
		for (int j = 0; j < numOfSpeeds; j++)
		{
			double speed;
			file >> speed;
			edge.durations.push_back((edge.length * 60.0) / speed);
		}

		edgesVar.push_back(edge);

		map_routing::map[node1].adjs.push_back(make_pair(node2, i));
		map_routing::map[node2].adjs.push_back(make_pair(node1, i));
	}

	file.close();
}