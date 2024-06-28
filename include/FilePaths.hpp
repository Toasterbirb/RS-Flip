#pragma once

#include <string>

namespace file_paths
{
	static inline const std::string user_home = (std::string)getenv("HOME");
	static inline const std::string data_path = user_home + "/.local/share/rs-flip";
	static inline const std::string data_file = data_path + "/flips.json";
	static inline const std::string item_blacklist_file = data_path + "/item_blacklist.txt";
}
