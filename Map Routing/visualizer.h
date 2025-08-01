#pragma once
#include "types.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

class visualizer
{
public:
	static void visualize(vector<Output> &outputs, vector<Output>& outputsActual, vector<Query>& queries, string curQuery);
};
