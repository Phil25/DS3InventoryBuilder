# Overview

### Attack values

This directory contains various weapon data that is available [here](http://darksouls3.wikidot.com/links) (Full AR & Damage Simulator).
It's been extracted from the game using some black magic that I don't care to get into, so I'm just using it directly and praying it's up to date.

The spreadsheet contains various tabs, most important of which is `WeaponsBase`, with all the detailed information about each weapon.
In `CalcCorrectGraphDerived` tab, you can find values for different saturation functions for each level; at the end, set of columns labelled `y`.
Finally, `Base` tab can be used to alter desired player attributes and upgrade level (`10` gets converted appropriately for unique weapons).

### Defense values

Values for weapon guard absorption and resistances are parsed directly from [soulsplanner.com](https://soulsplanner.com/darksouls3/weapondef).
Unlike attack values, they do not be very exact (one digit after comma suffices).

### Uniqueness attribute

Couldn't find anything representing uniqueness in the spreadsheet, so I'm just using [soulsplanner.com attack calculator](https://soulsplanner.com/darksouls3/weaponatk) at `+10` to figure out whether a weapon upgrades to `+10` or `+5`.

### Order

The default order for weapons in the game is some hardcoded value (99% sure). I prepared that one myself, so `order.json` is only here.

# Goal

This data is parsed and turned into a JSON file, which is then compressed with bz2 and embedded in the executable.
At runtime it's being decompressed and parsed by the `invbuilder::Database` object.

Saturatons are simply converted to a `std::vector` initializer list.

# Calculate script

The `calculate.py` uses the generated JSON data to calculate AR for the specified weapon and attributes, with hopefully a complete formula.
It is unused in the compilation process and purely there as a tester/PoC.

# Preparing data

**This data is already prepared, but just in case something happens, here's how to do it again:**

### Attack values

1. In `Base` tab, change weapon upgrade level to `X`.
1. Go to `WeaponsBase` and save it as `csv`.
1. Name it `weapons_X.csv` and place it in `./Raw`.
1. Open `weapons_X.csv` and remove lines up to `Fists`, including.
1. Open `weapons_X.csv` and rename `Red and White Round Shield` to `Red and White Shield`
1. Repeat for `X` in `<0, 10>`.

### Saturation functions

1. Go to `CalcCorrectGraphDerived` and save it as `csv`.
1. Name it `saturations.csv` and place it in `./Raw`.

### Defense values

1. Go to [soulsplanner.com defense calculator](https://soulsplanner.com/darksouls3/weapondef).
1. Disable `Show groups`, enable `All groups` and `All infusions`.
1. Set `Reinforce` to `X`.
1. Use a tool to parse the table and save it as `defense_X.tsv` in `./Raw`.
1. Open `defensee_X.tsv` and remove the header lines.
1. Repeat for `X` in `<0, 10>`.

### Uniqueness

1. Go to [soulsplanner.com attack calculator](https://soulsplanner.com/darksouls3/weaponatk).
1. Disable `Show groups`, enable `All groups`.
1. Set `Reinforce` to `+10`.
1. Use a tool to parse the table and save it as `uniqueness.tsv` in `./Raw`.
1. Open `defensee_X.tsv` and remove the header lines.