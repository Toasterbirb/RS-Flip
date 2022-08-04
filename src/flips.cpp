#include "Stats.hpp"
#include "AvgStat.hpp"
#include "Flips.hpp"
#include "Utils.hpp"

#define DEFAULT_DATA_FILE "{\"stats\":{\"profit\":0,\"flips_done\":0},\"flips\":[]}\n"

namespace Flips
{
	Flip::Flip()
	{
		item 		= "null";
		buy_price 	= 0;
		sell_price 	= 0;
		sold_price 	= 0;
		buylimit 	= 0;
		done 		= false;
	}

	Flip::Flip(nlohmann::json j)
	{
		item 		= j["item"];
		buy_price 	= j["buy"];
		sell_price 	= j["sell"];
		sold_price 	= j["sold"];
		buylimit 	= j["limit"];
		cancelled 	= j["cancelled"];
		done 		= j["done"];
	}

	Flip::Flip(const std::string& item, const int& buy_price, const int& sell_price, const int& buy_amount)
	{
		this->item 			= item;
		this->buy_price 	= buy_price;
		this->sell_price 	= sell_price;
		this->sold_price 	= 0;
		this->buylimit 		= buy_amount;
		this->cancelled 	= false;
		this->done 			= false;
	}

	void Flip::Sell(const int& sell_price)
	{
		this->sell_price 	= sell_price;
		this->done 			= true;
	}

	nlohmann::json Flip::ToJson()
	{
		nlohmann::json j;

		j["item"] 		= item;
		j["buy"] 		= buy_price;
		j["sell"] 		= sell_price;
		j["sold"] 		= sold_price;
		j["limit"] 		= buylimit;
		j["cancelled"] 	= cancelled;
		j["done"] 		= done;

		return j;
	}

	std::string ReadFile(const std::string& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			std::cerr << "File " << filepath << " couldn't be opened!\n";
			return "";
		}

		std::string contents( 	(std::istreambuf_iterator<char>(file)),
								(std::istreambuf_iterator<char>()));
		file.close();
		return contents;
	}

	void WriteFile(const std::string& filepath, const std::string text)
	{
		std::ofstream file(filepath);
		if (!file.is_open())
		{
			std::cerr << "Can't open the data file!" << std::endl;
			return;
		}

		file << text;
		file.close();
	}

	void CreateDefaultDataFile()
	{
		WriteFile(data_file, DEFAULT_DATA_FILE);
	}

	void WriteJson()
	{
		/* Backup the file before writing anything */
		std::filesystem::copy_file(data_file, data_file + "_backup", std::filesystem::copy_options::overwrite_existing);

		std::ofstream file(data_file);
		file << std::setw(4) << json_data << std::endl;
	}

	void LoadFlipArray()
	{
		flips.clear();

		/* Add existing flips to the array */
		for (int i = 0; i < json_data["flips"].size(); i++)
		{
			/* Don't load cancelled flips */
			if (json_data["flips"][i]["cancelled"] == true)
				continue;

			flips.push_back(json_data["flips"][i]);
		}
	}

	void Init()
	{
		if (!std::filesystem::exists(data_path))
			std::filesystem::create_directories(data_path);

		if (!std::filesystem::exists(data_file))
			CreateDefaultDataFile();

		/* Read the json data file */
		std::string json_string = ReadFile(data_file);
		json_data = nlohmann::json::parse(json_string);

		LoadFlipArray();
	}

	void ApplyFlipArray()
	{
		json_data["flips"] = flips;
		WriteJson();
	}

	int FindLongestName(std::vector<Stats::AvgStat> stats)
	{
		if (stats.size() == 0)
			return 0;

		int result = stats[0].name.size();

		for (int i = 1; i < stats.size(); i++)
			if (result < stats[i].name.size())
				result = stats[i].name.size();

		return result;
	}

	int FindLongestName(std::vector<nlohmann::json> flip_list)
	{
		if (flip_list.size() == 0)
			return 0;

		std::string element = flip_list[0]["item"];
		int result = element.size();

		for (int i = 1; i < flip_list.size(); i++)
		{
			element = flip_list[i]["item"];
			if (result < element.size())
				result = element.size();
		}

		return result;
	}

	void PrintTableSep(const int& name_length)
	{
		for (int i = 0; i < name_length + 2; i++)
			std::cout << "-";

		std::cout << "|";

		for (int i = 0; i < 18; i++)
			std::cout << "-";

		std::cout << "\n";
	}

	void PrintStats(const int& topValueCount)
	{
		Init();

		Utils::PrintTitle("Stats");
		std::cout << "Total profit: " << Utils::RoundBigNumbers(json_data["stats"]["profit"]) << std::endl;
		std::cout << "Flips done: " << Utils::RoundBigNumbers(json_data["stats"]["flips_done"]) << std::endl;
		std::cout << "\n";

		/* Quit if zero flips done */
		if (json_data["stats"]["flips_done"] == 0)
			return;

		/* Print top performing flips */
		std::vector<Stats::AvgStat> stats = Stats::FlipsToAvgstats(flips);
		int name_length = 0;


		Utils::PrintTitle("Top flips by ROI-%");
		std::vector<Stats::AvgStat> topROI = Stats::SortFlipsByROI(stats);
		name_length = FindLongestName(topROI);

		PrintTableSep(name_length);
		std::cout << std::setw(name_length + 1) << "Item name" << " | ROI-%" << std::endl;
		PrintTableSep(name_length);
		for (int i = 0; i < Utils::Clamp(topROI.size(), 0, topValueCount); i++)
		{
			std::cout << " " << std::setw(name_length) << topROI[i].name << " | " << std::to_string(topROI[i].AvgROI()) + "%" << std::endl;
		}
		PrintTableSep(name_length);
		std::cout << "\n";

		Utils::PrintTitle("Top flips by stability");
		std::vector<Stats::AvgStat> topStability = Stats::SortFlipsByStability(stats);
		name_length = FindLongestName(topStability);

		PrintTableSep(name_length);
		std::cout << std::setw(name_length + 1) << "Item name" << " | Flip instability" << std::endl;
		PrintTableSep(name_length);
		for (int i = 0; i < Utils::Clamp(topStability.size(), 0, topValueCount); i++)
		{
			std::cout << " " << std::setw(name_length) << topStability[i].name << " | " << Utils::CleanDecimals(std::round(topStability[i].FlipStability())) << std::endl;
		}
		PrintTableSep(name_length);
		std::cout << "\n";

		Utils::PrintTitle("Top flips by Profit");
		std::vector<Stats::AvgStat> topProfit = Stats::SortFlipsByProfit(stats);
		name_length = FindLongestName(topProfit);

		PrintTableSep(name_length);
		std::cout << std::setw(name_length + 1) << "Item name" << " | Average profit" << std::endl;
		PrintTableSep(name_length);
		for (int i = 0; i < Utils::Clamp(topProfit.size(), 0, topValueCount); i++)
		{
			std::cout << " " << std::setw(name_length) << topProfit[i].name << " | " << Utils::RoundBigNumbers(topProfit[i].AvgProfit()) << std::endl;
		}
		PrintTableSep(name_length);
	}

	void FixStats()
	{
		Init();
		std::cout << "Recalculating statistics..." << std::endl;

		int total_profit = 0;
		int flip_count = 0;

		for (int i = 0; i < json_data["flips"].size(); i++)
		{
			/* Check if the flip is done */
			if (json_data["flips"][i]["done"] == true)
			{
				flip_count++;

				/* Flip is done, but the sold price is missing */
				if (json_data["flips"][i]["sold"] == 0)
					json_data["flips"][i]["sold"] = json_data["flips"][i]["sell"];

				/* Calculate the profit */
				int buy_price 	= json_data["flips"][i]["buy"];
				int sell_price 	= json_data["flips"][i]["sold"];
				int limit 		= json_data["flips"][i]["limit"];
				total_profit += (sell_price - buy_price) * limit;
			}
		}

		/* Update the stats values */
		json_data["stats"]["profit"] 		= total_profit;
		json_data["stats"]["flips_done"] 	= flip_count;

		/* Update the data file */
		WriteJson();
	}

	void RestoreBackup()
	{
		std::cout << "Restoring the backup. Make sure to create a new one if you want to keep it, because the old one is lost" << std::endl;

		/* Check if the backup exists */
		if (std::filesystem::exists(data_file + "_backup"))
			std::filesystem::rename(data_file + "_backup", data_file);
	}

	void List()
	{
		Init();

		std::vector<nlohmann::json> undone_flips;

		for (int i = 0; i < flips.size(); i++)
		{
			/* Check if the flip is done yet */
			if (flips[i]["done"] == true)
				continue;

			undone_flips.push_back(flips[i]);
		}

		int name_length = FindLongestName(undone_flips);
		std::cout << "Name length: " << name_length << std::endl;
		Utils::PrintTitle("On-going flips");
		for (int i = 0; i < undone_flips.size(); i++)
		{
			std::string flip_name = undone_flips[i]["item"];
			std::cout << "[" << i << "] " << undone_flips[i]["item"] << std::setw(name_length - flip_name.size() + 10) << " | Count: " << undone_flips[i]["limit"] << " | Buy: " << undone_flips[i]["buy"] << " | Estimated sell: " << undone_flips[i]["sell"] << std::endl;
		}
	}

	int FindRealIDWithUndoneID(const int& undone_ID)
	{
		int undone_index = 0;
		int result;
		bool result_found = false;
		for (int i = 0; i < flips.size(); i++)
		{
			/* Skip flips that are already done */
			if (flips[i]["done"] == true)
				continue;

			if (undone_index != undone_ID)
				undone_index++;
			else
			{
				result = i;
				result_found = true;
				break;
			}
		}

		if (!result_found)
		{
			std::cout << "No items matching ID [" << undone_ID << "] were found!" << std::endl;
			return -1;
		}
		else
		{
			return result;
		}
	}

	void Add(Flip flip)
	{
		Init();
		flips.push_back(flip.ToJson());
		ApplyFlipArray();
	}

	void Cancel(const int& ID)
	{
		Init();

		/* Mark the flip as cancelled. It will be removed when the flip array
		 * is loaded next time around and saved */
		int flip_to_cancel = FindRealIDWithUndoneID(ID);
		flips[flip_to_cancel]["cancelled"] = true;

		std::cout << "Flip [" << flips[flip_to_cancel]["item"] << "] cancelled!\n";

		ApplyFlipArray();
	}

	void Sell(const int& index, int sell_value, int sell_amount)
	{
		Init();
		int result = FindRealIDWithUndoneID(index);
		if (result == -1)
			return;

		/* Update the flip values */
		if (sell_amount == 0)
			sell_amount = flips[result]["limit"];
		else
			flips[result]["limit"] = sell_amount;

		flips[result]["done"] = true;

		if (sell_value == 0)
			sell_value = flips[result]["sell"];

		flips[result]["sold"] = sell_value;

		/* Update the stats */
		int flips_done = json_data["stats"]["flips_done"];
		flips_done++;
		json_data["stats"]["flips_done"] = flips_done;

		int total_profit = json_data["stats"]["profit"];
		int buy_price = flips[result]["buy"];
		int profit = ((sell_value - buy_price) * sell_amount);
		total_profit += profit;
		json_data["stats"]["profit"] = total_profit;

		Utils::PrintTitle("Flip complete");
		std::cout << "Item: " << flips[result]["item"] << std::endl;
		std::cout << "Profit: " << profit << " (" << Utils::RoundBigNumbers(profit) << ")" << std::endl;
		std::cout << "Total profit so far: " << total_profit << " (" << Utils::RoundBigNumbers(total_profit) << ")" << std::endl;

		/* Update the json file */
		ApplyFlipArray();
	}
}
