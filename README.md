# RS-Flip
RS-Flip is a CLI utility to help with flipping in RuneScape. The core features include calculating the profits and saving on-going and past flips.

## Usage
```
calc 	Calculate the margin for an item and possible profits
	-b [Insta buy price]
	-s [Insta sell price]
	-l [Buy limit for the item]

add 	Add a flip to the database
	-i [Item name]
	-b [Buying price]
	-s [Assumed future selling price]
	-l [Buy limit for the item]

sold  Finish an on-going flip
	-i [ID] 			The ID number can be found with the `--list` command
	-s [Selling price] 	Optional. This argument is for cases where final sell value changed
	-l [Amount sold]  	Optional. This argument is for cases where the full buy limit didn't buy or the amount sold was partial.

cancel [ID] 	Cancels an on-going flip and removes it from the database
list 		Lists all on-going flips with their IDs, buy and sell values
stats  	Prints out profit statistics
repair 	Attempts to repair the statistics from the flip data in-case of some bug. Also restores the backup
```

## Compiling
```sh
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 3rd party libs
- [doctest](https://github.com/doctest/doctest)
- [json](https://github.com/nlohmann/json)
