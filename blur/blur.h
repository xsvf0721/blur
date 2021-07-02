#pragma once

#include <string>

#include "rendering.h"

class c_blur {
private:
	std::vector<std::string> get_files(int& argc, char* argv[]);

public:
	std::string create_temp_path(const std::string& video_path);
	void remove_temp_path(const std::string& path);

public:
	void run(int argc, char* argv[]);
};

inline c_blur blur;