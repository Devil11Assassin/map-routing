#pragma once
#include "types.h"
#include <vector>
#include <fstream>
#include <iomanip>

class file_io
{
public:
	static void saveOutputs(vector<Query> &queries, vector<Output> &outputs, string mapName);
};

