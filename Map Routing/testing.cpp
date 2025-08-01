#include "testing.h"
#include "map_routing.h"
#include "bonus.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>

const bool Testing::RUN_SAMPLE = 1;
const bool Testing::RUN_MEDIUM = 1;
const bool Testing::RUN_LARGE = 1;
const bool Testing::RUN_BONUS = 0; // bruteforce, medium case runs until no memory

const bool Testing::READ_OUTPUT_ACTUAL = 0;
const bool Testing::COMPARE = 0;
const bool Testing::DETAILED_COMPARISON = 0;
const bool Testing::WRITE_OUTPUT_PREDICTION = 1;

const bool Testing::VISUALIZE = 1;
const bool Testing::MULTI_THREAD = 1;

const vector<Test> Testing::testsSample = {
	Test("[1] Sample Cases", "map1", "queries1", "output1"),
	Test("[1] Sample Cases", "map2", "queries2", "output2"),
	Test("[1] Sample Cases", "map3", "queries3", "output3"),
	Test("[1] Sample Cases", "map4", "queries4", "output4"),
	Test("[1] Sample Cases", "map5", "queries5", "output5"),
	Test("[1] Sample Cases", "map8", "queries8", "output8")
};
const vector<Test> Testing::testsMedium = {
	Test("[2] Medium Cases", "OLMap", "OLQueries", "OLOutput"),
	Test("[2] Medium Cases", "TGMap", "TGQueries", "TGOutput")
};
const vector<Test> Testing::testsLarge = {
	Test("[3] Large Cases", "SFMap", "SFQueries", "SFOutput"),
	Test("[3] Large Cases", "NAMap", "NAQueries", "NAOutput")
};
const vector<Test> Testing::testsBonus = {
	Test("[4] BONUS Test Cases\\[1] Sample Cases", "map1B", "queries1", "output1"),
	Test("[4] BONUS Test Cases\\[2] Medium Cases", "OLMapB", "OLQueries", "OLOutput")
};

vector<Query> Testing::queries;
vector<Output> Testing::outputs;
vector<Output> Testing::outputsActual;

sf::Clock Testing::clock;

int Testing::execTime;
int Testing::execTimeIO;
int Testing::execTimeActual;
int Testing::execTimeIOActual;

void Testing::readMap(string type, string fileName)
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
	file >> numOfEdges;
	map_routing::edges.reserve(numOfEdges);

	for (int i = 0; i < numOfEdges; i++)
	{
		int node1, node2;
		double speed;
		Edge edge{};

		file >> node1 >> node2 >> edge.length >> speed;

		edge.time = edge.length * 60.0 / speed;
		map_routing::edges.push_back(edge);
		map_routing::map[node1].adjs.push_back(make_pair(node2, i));
		map_routing::map[node2].adjs.push_back(make_pair(node1, i));
	}

	file.close();
}

void Testing::readQueries(string type, string fileName)
{
	ifstream file;
	file.open("TEST CASES\\" + type + "\\Input\\" + fileName + ".txt");

	int numOfQueries;
	file >> numOfQueries;
	queries.reserve(numOfQueries);
	outputs.reserve(numOfQueries);

	while (numOfQueries--)
	{
		Query query{};
		file >> query.startX >> query.startY >> query.destX >> query.destY >> query.radius;
		queries.push_back(query);
	}

	file.close();
}

void Testing::readOutput(string type, string fileName)
{
	if (!READ_OUTPUT_ACTUAL)
		return;

	ifstream file;
	file.open("TEST CASES\\" + type + "\\Output\\" + fileName + ".txt");

	int numOfOutputs = queries.size();
	outputsActual.reserve(numOfOutputs);

	while (numOfOutputs--)
	{
		Output output{};
		string path;
		getline(file, path);

		istringstream nodesStream(path);
		while (!nodesStream.eof())
		{
			int nodeID;
			nodesStream >> nodeID;
			output.nodeIDs.push_back(nodeID);
		}

		file >> output.time;
		file.ignore(6);

		file >> output.distTotal;
		file.ignore(4);

		file >> output.distWalk;
		file.ignore(4);

		file >> output.distDrive;
		file.ignore(5);

		outputsActual.push_back(output);
	}

	file >> execTimeActual;
	file.ignore(5);
	file >> execTimeIOActual;

	file.close();
}

void Testing::readTest(Test test)
{
	map_routing::map.clear();
	map_routing::edges.clear();
	queries.clear();
	outputs.clear();
	outputsActual.clear();

	readMap(test.type, test.mapFile);
	readQueries(test.type, test.queryFile);
	readOutput(test.type, test.outputFile);
}

void Testing::compareOutputs()
{
	if (!READ_OUTPUT_ACTUAL || !COMPARE)
		return;

	if (outputs.size() != outputsActual.size())
	{
		cout << "ERROR: #outputs != #outputs in file!\n";
		return;
	}

	vector<int> wrongOutputs;
	string detailedReport = "";

	int numOfOutputs = outputs.size();
	for (int i = 0; i < numOfOutputs; i++)
	{
		int wrong = 0;
		string report = to_string(i) + ":\n";

		Output pred = outputs[i];
		Output actual = outputsActual[i];

		if (pred.nodeIDs.size() != actual.nodeIDs.size())
		{
			wrong++;
			report.append("\tnodeIDs(incorrect size, pred=" + to_string(pred.nodeIDs.size()) +
				", actual=" + to_string(actual.nodeIDs.size()) + ")\n");
		}
		else
		{
			int numOfNodes = pred.nodeIDs.size();
			for (int j = 0; j < numOfNodes; j++)
			{
				if (pred.nodeIDs[j] != actual.nodeIDs[j])
				{
					wrong++;
					report.append("\tnodeIDs(wrong id, j=" + to_string(j) +
						", pred=" + to_string(pred.nodeIDs[j]) +
						", actual=" + to_string(actual.nodeIDs[j]) + ")\n");
					break;
				}
			}
		}

		if (math::round2(pred.time) != math::round2(actual.time))
		{
			wrong++;
			report.append("\ttime(pred=" + to_string(pred.time) + ", actual=" + to_string(actual.time) + ")\n");
		}
		if (math::round2(pred.distTotal) != math::round2(actual.distTotal))
		{
			wrong++;
			report.append("\tdistTotal(pred=" + to_string(pred.distTotal) + ", actual=" + to_string(actual.distTotal) + ")\n");
		}

		if (math::round2(pred.distWalk) != math::round2(actual.distWalk))
		{
			wrong++;
			report.append("\tdistWalk(pred=" + to_string(pred.distWalk) + ", actual=" + to_string(actual.distWalk) + ")\n");
		}

		if (math::round2(pred.distDrive) != math::round2(actual.distDrive))
		{
			wrong++;
			//cout << std::fixed << std::setprecision(30) << pred.distDrive << " " << actual.distDrive << "\n";
			report.append("\tdistDrive(pred=" + to_string(pred.distDrive) + ", actual=" + to_string(actual.distDrive) + ")\n");
		}

		if (wrong)
		{
			wrongOutputs.push_back(i);
			detailedReport.append(report + "\n");
		}
	}
	
	if (!wrongOutputs.size())
	{
		cout << "EVALUATION: 100%\n";
	}
	else
	{
		cout << "#WRONG: " << wrongOutputs.size() << "\nINDICES: ";
		for (auto i : wrongOutputs)
			cout << i << " ";
		cout << "\n";
		
		if (DETAILED_COMPARISON)
		{
			cout << detailedReport << "\n";
		}
	}
}

void Testing::writeOutputs(Test test)
{
	if (!WRITE_OUTPUT_PREDICTION)
		return;
	
	ofstream file;
	file.open("TEST CASES\\" + test.type + "\\Output\\" + test.outputFile + "_meow.txt");

	int numOfOutputs = outputs.size();
	for (int i = 0; i < numOfOutputs; i++)
	{
		int numOfNodes = outputs[i].nodeIDs.size();
		file << outputs[i].nodeIDs[0];
		for (int j = 1; j < numOfNodes; j++)
			file << " " << outputs[i].nodeIDs[j];

		file << std::fixed << std::setprecision(2) << "\n";
		file << outputs[i].time << " mins\n";
		file << outputs[i].distTotal << " km\n";
		file << outputs[i].distWalk << " km\n";
		file << outputs[i].distDrive << " km\n\n";
	}

	file << execTime << " ms\n\n";
	execTimeIO = clock.getElapsedTime().asMilliseconds();
	file << execTimeIO << " ms\n";

	file.close();
}

void Testing::visualize(Test test)
{
	if (VISUALIZE)
		visualizer::visualize(outputs, outputsActual, queries, test.type + "\\" + test.queryFile);
}

void Testing::solveThread(int start, int end)
{
	thread_local Graph map = map_routing::map;
	thread_local vector<Edge> edges = map_routing::edges;

	for (int i = start; i < end; i++)
		outputs[i] = map_routing::solveQuery(queries[i], map, edges);
}

void Testing::multiThread(int n)
{
	outputs.resize(n);

	vector<thread> threads;
	int threadCount = thread::hardware_concurrency();
	threadCount = (n < threadCount) ? n : threadCount;
	int bucketSize = n / threadCount;

	for (int i = 0; i < threadCount; i++)
	{
		int start = i * bucketSize;
		int end = (i == threadCount - 1) ? n : start + bucketSize;
		threads.emplace_back(solveThread, start, end);
	}

	for (auto& t : threads)
		t.join();
}

void Testing::test(vector<Test> tests)
{
	for (const auto& test : tests)
	{
		clock.restart();

		readTest(test);
		execTimeIO = clock.getElapsedTime().asMilliseconds();

		int numOfQueries = queries.size();

		if (MULTI_THREAD && map_routing::map.size() > 1000)
			multiThread(numOfQueries);
		else
		{
			for (int i = 0; i < numOfQueries; i++)
				outputs.push_back(map_routing::solveQuery(queries[i], map_routing::map, map_routing::edges));
		}

		compareOutputs();
		execTime = clock.getElapsedTime().asMilliseconds() - execTimeIO;

		writeOutputs(test);

		visualize(test);
		cout << "Runtime: " + to_string(execTime) + "\nw/ IO: " + to_string(execTimeIO) + "\n\n";
	}
}

void Testing::runTests()
{
	if (RUN_SAMPLE)
	{
		cout << "===SAMPLE TEST===\n\n";
		test(testsSample);
	}
	if (RUN_MEDIUM)
	{
		cout << "===MEDIUM TEST===\n\n";
		test(testsMedium);
	}
	if (RUN_LARGE)
	{
		cout << "===LARGE TEST===\n\n";
		test(testsLarge);
	}
	if (RUN_BONUS)
	{
		cout << "===BONUS TEST===\n\n";
		runTestsBonus();
	}
}

void Testing::solveThreadBonus(int start, int end)
{
	thread_local Graph map = map_routing::map;
	thread_local vector<EdgeVar> edges = bonus::edgesVar;

	for (int i = start; i < end; i++)
		outputs[i] = bonus::solveQuery(queries[i], map, edges);
}

void Testing::runTestsBonus()
{
	for (const auto& test : testsBonus)
	{
		clock.restart();
		{
			map_routing::map.clear();
			bonus::edgesVar.clear();
			queries.clear();
			outputs.clear();
			outputsActual.clear();

			bonus::readMap(test.type, test.mapFile);
			readQueries(test.type, test.queryFile);
			readOutput(test.type, test.outputFile);
		}
		execTimeIO = clock.getElapsedTime().asMilliseconds();

		int numOfQueries = queries.size();

		if (MULTI_THREAD && map_routing::map.size() > 1000)
		{
			int n = numOfQueries;
			outputs.resize(n);

			vector<thread> threads;
			int threadCount = thread::hardware_concurrency();
			threadCount = (n < threadCount) ? n : threadCount;
			int bucketSize = n / threadCount;

			for (int i = 0; i < threadCount; i++)
			{
				int start = i * bucketSize;
				int end = (i == threadCount - 1) ? n : start + bucketSize;
				threads.emplace_back(solveThreadBonus, start, end);
			}

			for (auto& t : threads)
				t.join();;
		}
		else
		{
			for (int i = 0; i < numOfQueries; i++)
				outputs.push_back(bonus::solveQuery(queries[i], map_routing::map, bonus::edgesVar));
		}

		compareOutputs();
		execTime = clock.getElapsedTime().asMilliseconds() - execTimeIO;

		writeOutputs(test);

		map_routing::edges.clear();
		map_routing::edges.reserve(bonus::edgesVar.size());

		for (const auto& edge : bonus::edgesVar)
		{
			map_routing::edges.push_back(Edge(edge.length, 0.0));
		}

		visualize(test);
		cout << "Runtime: " + to_string(execTime) + "\nw/ IO: " + to_string(execTimeIO) + "\n\n";
	}

}