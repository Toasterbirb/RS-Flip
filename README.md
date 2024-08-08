# RS-Flip
RS-Flip is a CLI utility to help with flipping in RuneScape. The core features include calculating the profits and saving on-going and past flips.

## Usage
Check the output of `flip help`

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
