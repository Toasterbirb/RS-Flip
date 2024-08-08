# RS-Flip
RS-Flip is a CLI utility to help with flipping in RuneScape. The core features include calculating the profits and saving on-going and past flips.

## Usage
```
SYNOPSIS
        rs-flip tips [-t <profit>] [-c <count>] [-r <count>] [-g]
        rs-flip calc -b <price> -s <price> -l <limit>
        rs-flip add -i <name> -b <price> -s <price> -l <limit> [-a <account>]
        rs-flip sold -i <id> [-s <price>] [-l <count>]
        rs-flip cancel -i <id>
        rs-flip update -i <id> [-b <price>] [-s <price>] [-l <count>] [-a <account>]
        rs-flip list [<account>]
        rs-flip filter ([-i <name>] | [-c <count>])
        rs-flip stats [-c <count>]
        rs-flip repair
        rs-flip help
        rs-flip test

OPTIONS
        recommend flips based on past flipping data
            tips              mode
            -t <profit>       profit threshold
            -c <count>        maximum result count
            -r <count>        maximum random flip suggestion count (def: 0)
            -g                print the results in ge-inspector pre-filter list format

        calculate the margin for an item and possible profits
            calc              mode
            -b <price>        insta buy price
            -s <price>        insta sell price
            -l <limit>        buy limit for the item

        add a flip to the database
            add               mode
            -i <name>         item name
            -b <price>        buying price
            -s <price>        assumed future selling price
            -l <limit>        item count to buy (usually the buy limit or slightly below)
            -a <account>      the name of the account used for the flip

        finish an on-going flip
            sold              mode
            -i <id>           the id number can be found with the 'list' command
            -s <price>        final selling price
            -l <count>        final amount of items sold

        cancels an on-going flip and removes it from the database
            cancel            mode
            -i <id>           the id of the flip to cancel

        update the details of an on-going flip
            update            mode
            -i <id>           the id of the flip to update
            -b <price>        new buying price
            -s <price>        new selling price
            -l <count>        new item count
            -a <account>      new account name

        list all on-going flips with their ids, buy and sell values
            list              mode
            <account>         list only flips made with this account

        look for items with filters
            filter            mode
            -i <name>         find stats for a specific item
            -c <count>        find flips that have been done count <= times

        print out profit statistics
            stats             mode
            -c <count>        set the amount of values to show

        repair                attempts to repair the statistics from the flip data in-case of some
                              bug

        help                  show help
        test                  run unit tests
```

To ignore specific item recommendations, add the item names one per line to `~/.local/share/rs-flip/item_blacklist.txt`

## Dependencies
- [doctest](https://github.com/doctest/doctest)
- [json](https://github.com/nlohmann/json)

## Compiling
```sh
mkdir build
cd build
cmake ..
make -j$(nproc)
```
