#!/bin/sh

sed -i 's|http://192.168.1.104:5000/mirrors/json|https://github.com/nlohmann/json|' ./.gitmodules
sed -i 's|http://192.168.1.104:5000/mirrors/doctest|https://github.com/doctest/doctest|' ./.gitmodules
git submodule update --init --recursive
