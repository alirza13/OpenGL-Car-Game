#pragma once

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>

#include "Texture.h"

class PNGLoader
{
public:
	static Texture load(const std::string& filename);
	~PNGLoader();

private:
	PNGLoader();
};

