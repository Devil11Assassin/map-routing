#pragma once
#include "types.h"
#include "visualizer.h"

class Testing
{
	static const bool RUN_SAMPLE;
	static const bool RUN_MEDIUM;
	static const bool RUN_LARGE;
	static const bool RUN_BONUS;
	static const bool VISUALIZE;
	static const bool MULTI_THREAD;
	static const bool READ_OUTPUT_ACTUAL;
	static const bool COMPARE;
	static const bool DETAILED_COMPARISON;
	static const bool WRITE_OUTPUT_PREDICTION;

	static const vector<Test> testsSample;
	static const vector<Test> testsMedium;
	static const vector<Test> testsLarge;
	static const vector<Test> testsBonus;

	static vector<Query> queries;
	static vector<Output> outputs;
	static vector<Output> outputsActual;

	static sf::Clock clock;

	static int execTime, execTimeIO;
	static int execTimeActual, execTimeIOActual;

	static void readMap(string type, string fileName);
	static void readQueries(string type, string fileName);
	static void readOutput(string type, string fileName);
	static void readTest(Test test);

	static void compareOutputs();
	static void writeOutputs(Test test);
	
	static void visualize(Test test);

	static void solveThread(int start, int end);
	static void multiThread(int n);
	
	static void test(vector<Test> tests);

	static void solveThreadBonus(int start, int end);
	static void runTestsBonus();

public:
	static void runTests();
};
