# DS3InventoryBuilder

WIP subtitle: allows you to quickly build inventory in order to optimize it for hardswarps and look up weapon stats and info.

## Legal, thirdparty and thanks
* Item names and images are properties of FromSoftware Inc.
* [soulsplanner.com](https://soulsplanner.com)'s weapon parameter tables.
* [Table Capture](https://chrome.google.com/webstore/detail/table-capture/iebpjdmgckacbodjpijphcplhebcmeop) Chrome extension to parse said tables.
* [How to Calcualte AR](https://blog.mugenmonkey.com/2016/07/22/how-to-calculate-ar.html) article at MugenMonkey Blog.
* [Post on calcuting AR](https://www.reddit.com/r/darksouls3/comments/4gqrpy/how_to_calculate_attack_rating_of_any_weapon_at/) by `u/Frostitutes`.
* [Spreadsheet with full weapon data](https://www.reddit.com/r/darksouls3/comments/4j3o40/spreadsheet_with_full_ar_calculation/) by `u/monrandira` and `u/pireax`.
* [ds3-ar](https://github.com/Derling/ds3-ar/tree/master/src) project from Derling.

## Building
This project uses the [Conan](https://docs.conan.io/en/latest/installation.html) package manager.
Since it's a Python-based tool, you might want to install it through `pip` in a [virtual environment](https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/#creating-a-virtual-environment).
1. clone and cd to repo,
1. add Bincrafters remote: `conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan`
1. setup CMake and dependencies: `conan install .`,
    * optionally specify compiler by adding `-s compiler=clang -s compiler.version=11`,
    * optionally specify build type by adding `--build_type=Debug`,
    * might require `--build missing` when prebuilt dependencies are not found,
1. build the project: `conan build .`,
    * add `-c` to only generate CMake files and a solution (if on Windows) without building.