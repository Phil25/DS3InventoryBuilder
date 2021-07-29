> #### Check out [Dark Souls III Inventory Tool](https://ds3-inventory.nyasu.business/) by sovietspaceship, a tool similar to this one, but web-based.

# DS3InventoryBuilder

This app replicates the Dark Souls 3 weapon inventory for you to preview swaps or just store weapons for your builds.
Every[\*](#known-bugs) sorting method is implemented for you to play around with, all without launching the game and bothering with any limitations therein.
It comes with an AR calculator and a rich weapon browsing tool to easily find anything most relevant for your attributes.
When you're done, save your inventory as a PNG, or encode it as text to share over chat or paste on your soulsplanner.com build.
You can import the inventory code and have your setup back any time.

https://user-images.githubusercontent.com/16500675/121757438-cdaba380-cb1d-11eb-9505-c7e1cb298bc0.mp4

## Full feature list
1. Imitates in-game 5 column inventory layout and automatically sorts every item you add.
2. Settable infusions and levels on all weapons with appropriate icons displayed.
3. Missing requirements icons, with an additional state which shows where a weapon needs to be two handed.
4. All sorting methods both ascending and descending are available ([note `Attack Power` in known bugs](#known-bugs)).
5. Input your character attributes to get exact damage numbers for your weapons and manage missing requirements.
6. Highlighting swaps at L2/R2 distance from the current selection.
7. Simple weapon preview with single selection and switching to advanced comparison view with multiple selection.
8. Weapon preview includes damages, absorption, properties, requirements and scaling along with the hidden luck scaling.
9. Rich weapon browser, with ability to filter by name, classes, infusions, levels and sorting methods.
10. Additional modifiers for sorting the browser, ex. sorting by two handed damage only where two handing is required.
11. Save your inventory as PNG on your computer or copy it directly to clipboard.
12. Export your inventory in an encoded form to share it over chat or paste it on soulsplanner.com along with your build.
13. Import yours and others' inventory codes.
14. ???
15. Profit.

## Known bugs
* Sorting by `Attack Power` is not 100% valid.
When two weapon ARs happen to be similar, DS3 applies some odd comparison rules which I haven't been able to figure out.
At times, even the DS3 ordering is wrong according to the in-game AR display (ex. weapons can be sorted as `61 > 62 > 61`).
This is rare when every weapon is fully upgraded as the damage values are more spread out, but can still happen according to my tests.
There is a in-app popup warning about this, but if you have any details on how DS3 deals with this sorting method, please let me know.

## Special thanks
* [soulsplanner.com](https://soulsplanner.com)'s weapon parameter tables.
* [Table Capture](https://chrome.google.com/webstore/detail/table-capture/iebpjdmgckacbodjpijphcplhebcmeop) Chrome extension to parse said tables.
* [How to Calcualte AR](https://blog.mugenmonkey.com/2016/07/22/how-to-calculate-ar.html) article at MugenMonkey Blog.
* [Post on calcuting AR](https://www.reddit.com/r/darksouls3/comments/4gqrpy/how_to_calculate_attack_rating_of_any_weapon_at/) by `u/Frostitutes`.
* [Spreadsheet with full weapon data](https://www.reddit.com/r/darksouls3/comments/4j3o40/spreadsheet_with_full_ar_calculation/) by `u/monrandira` and `u/pireax`.
* [ds3-ar](https://github.com/Derling/ds3-ar/tree/master/src) project from `Derling`.

## Legal and thirdparty
* Item names and images are properties of FromSoftware Inc
* [wxWidgets](https://www.wxwidgets.org/) for cross-platform native GUI
* [RapidJSON](https://rapidjson.org/)
* [bzip2](https://www.sourceware.org/bzip2/)
* [fmt](https://github.com/fmtlib/fmt)
* [GoogleTest](https://github.com/google/googletest)

## Building
This project uses the [Conan](https://docs.conan.io/en/latest/installation.html) package manager.
Since it's a Python-based tool, you might want to install it through `pip` in a [virtual environment](https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/#creating-a-virtual-environment).
1. clone and cd to repo,
1. add Bincrafters remote: `conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/conan-legacy-bincrafters`
1. setup CMake and dependencies: `conan install .`,
    * optionally specify compiler by adding `-s compiler=clang -s compiler.version=11`,
    * optionally specify build type by adding `--build_type=Debug`,
    * might require `--build missing` when prebuilt dependencies are not found,
1. build the project: `conan build .`,
    * add `-c` to only generate CMake files and a solution (if on Windows) without building.
