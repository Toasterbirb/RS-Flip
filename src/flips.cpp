#include <fstream>
#include <filesystem>
#include <iostream>
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
		done 		= j["done"];
	}

	Flip::Flip(const std::string& item, const int& buy_price, const int& sell_price, const int& buy_amount)
	{
		this->item 			= item;
		this->buy_price 	= buy_price;
		this->sell_price 	= sell_price;
		this->sold_price 	= 0;
		this->buylimit 		= buy_amount;
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

		j["item"] 	= item;
		j["buy"] 	= buy_price;
		j["sell"] 	= sell_price;
		j["sold"] 	= sold_price;
		j["limit"] 	= buylimit;
		j["done"] 	= done;

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
		std::ofstream file(data_file);
		file << std::setw(4) << json_data << std::endl;
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

		/* Add existing flips to the array */
		for (int i = 0; i < json_data["flips"].size(); i++)
		{
			flips.push_back(json_data["flips"][i]);
		}
	}

	void PrintStats()
	{
		Init();

		std::cout << "## Stats ##\n";
		std::cout << "Total profit: " << Utils::RoundBigNumbers(json_data["stats"]["profit"]) << std::endl;
		std::cout << "Flips done: " << Utils::RoundBigNumbers(json_data["stats"]["flips_done"]) << std::endl;
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

	void List()
	{
		std::cout << "## On-going flips ##" << std::endl;
		int index = 0;
		for (int i = 0; i < json_data["flips"].size(); i++)
		{
			/* Check if the flip is done yet */
			if (json_data["flips"][i]["done"] == true)
				continue;

			std::cout << "[" << index << "] " << json_data["flips"][i]["item"] << " | Buy: " << json_data["flips"][i]["buy"] << " | Sell: " << json_data["flips"][i]["sell"] << std::endl;
			index++;
		}
	}

	void Add(Flip flip)
	{
		Init();
		flips.push_back(flip.ToJson());

		json_data["flips"] = flips;
		WriteJson();
	}

	void Sell(const int& index, const int& sell_value, int sell_amount)
	{
		Init();
		int undone_index = 0;
		int result;
		bool result_found = false;
		for (int i = index; i < json_data["flips"].size(); i++)
		{
			/* Skip flips that are already done */
			if (json_data["flips"][i]["done"] == true)
				continue;

			if (undone_index != index)
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
			std::cout << "No items matching ID [" << index << "] were found!" << std::endl;
			return;
		}

		/* Update the flip values */
		if (sell_amount == 0)
			sell_amount = json_data["flips"][result]["limit"];
		else
			json_data["flips"][result]["limit"] = sell_amount;

		json_data["flips"][result]["done"] 	= true;
		json_data["flips"][result]["sold"] 	= sell_value;

		/* Update the stats */
		int flips_done = json_data["stats"]["flips_done"];
		flips_done++;
		json_data["stats"]["flips_done"] = flips_done;

		int buy_price = json_data["flips"][result]["buy"];
		int total_profit = json_data["stats"]["profit"];
		int profit = ((buy_price - sell_value) * sell_amount);
		total_profit += profit;
		json_data["stats"]["profit"] = profit;

		std::cout << "Flip complete!" << std::endl;
		std::cout << "Profit: " << profit << " (" << Utils::RoundBigNumbers(profit) << ")" << std::endl;
		std::cout << "Total profit so far: " << total_profit << " (" << Utils::RoundBigNumbers(total_profit) << ")" << std::endl;

		/* Update the json file */
		WriteJson();
	}
}
