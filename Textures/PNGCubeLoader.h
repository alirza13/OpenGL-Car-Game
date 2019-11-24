#pragma once
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>

#include "TextureCube.h"

class PNGCubeLoader
{
public:
	static TextureCube load(const std::string& filename);
	~PNGCubeLoader();

private:
	PNGCubeLoader();
};
