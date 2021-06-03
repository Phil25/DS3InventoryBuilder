import csv
import json
import bz2
import os
from pathlib import Path
from enum import IntEnum

# *THIS IS A VERY DISORGANIZED SCRIPT*
# *READ THE README FILE IN THIS DIRECTORY FOR DETAILS*

MINIMIZED = True

# shortens text for smaller JSON sizes if needed (~30% less, from 10mb to ~6mb)
# ex. key "damage" becomes "d"
# generally unneeded since after bz2 it's always <300kb and easier to parse in C++
SHORTEN = False

FILENAME_WEAPON_DATA = "weapon_data.json.bz2.txt"
FILENAME_SATURATIONS = "saturations.txt"
FILENAME_IMAGE_DECLARATIONS = "ImageDeclarations.hpp"
FILENAME_IMAGE_ACCESSOR = "ImageAccessor.hpp"

dir_path = os.path.dirname(os.path.realpath(__file__))

# WeaponDataBase spreadsheet column offset
class WD(IntEnum):
    ID = 0
    NAME = 1
    CLASS = 2
    INFUSION = 3
    BUFFABLE = 4

    WEIGHT = 7

    REQ_STR = 8
    REQ_DEX = 9
    REQ_INT = 10
    REQ_FTH = 11

    LCK_COEFF = 17

    SAT_FUNC_PHYSICAL = 23
    SAT_FUNC_MAGIC = 24
    SAT_FUNC_FIRE = 25
    SAT_FUNC_LIGHTNING = 26
    SAT_FUNC_DARK = 27
    SAT_FUNC_BLEED = 28
    SAT_FUNC_POISON = 29

    LCK_STAT_MOD = 39

    DMG_BLEED = 170
    DMG_POISON = 175
    DMG_FROST = 177

    DMG_PHYSICAL = 178
    DMG_MAGIC = 179
    DMG_FIRE = 180
    DMG_LIGHTNING = 181
    DMG_DARK = 182

    SCL_STR = 194
    SCL_DEX = 195
    SCL_INT = 196
    SCL_FTH = 197
    SCL_LCK = 198

# view definition of `SHORTEN`
def s(text):
    if not SHORTEN:
        return text

    return {
        "id": "id", "name": "n", "class": "c", "buffable": "b", "infusable": "i?", "weight": "w",
        "lck_coeff": "l", "requirements": "r", "order": "o", "unique": "u", "infusion": "i",

        "saturation_index": "s", "level": "l",

        "lck_mod": "l", "status": "s", "damage": "d", "scaling": "sc", "absorption": "a",
        "resistance": "r", "stability": "sb",

        "physical": "p", "magic": "m", "fire": "f", "lightning": "l", "dark": "d",
        "bleed": "b", "poison": "ps", "frost": "fr", "curse": "c",

        "str": "s", "dex": "d", "int": "i", "fth": "f", "lck": "l",
    }[text]

# alternatively: `cat {file} | xxd -p | sed -s 's/\(..\)/0x\1, /g' > {output}`
def to_cpp_byte_array(b):
    return " ".join(map("0x{:02X},".format, b))

def to_cpp_identifier(name):
    identifier = name.replace(" ", "")
    identifier = identifier.replace("'", "")
    return identifier.replace("-", "")

def parse_weapon(out, weapon):
    index = int(weapon[WD.ID][:-4])

    if index not in out:
        out[index] = {}
        out[index]["static"] = {
            s("id"): int(weapon[WD.ID]),
            s("name"): weapon[WD.NAME],
            s("class"): weapon[WD.CLASS],
            s("buffable"): weapon[WD.BUFFABLE] == "TRUE",
            s("weight"): float(weapon[WD.WEIGHT]),
            s("lck_coeff"): float(weapon[WD.LCK_COEFF]), # same across infusions
            s("requirements"): {
                s("str"): int(weapon[WD.REQ_STR]),
                s("dex"): int(weapon[WD.REQ_DEX]),
                s("int"): int(weapon[WD.REQ_INT]),
                s("fth"): int(weapon[WD.REQ_FTH]),
            },
        }
        out[index][s("infusion")] = {}

    out[index][s("infusion")][weapon[WD.INFUSION]] = {
        s("lck_mod"): float(weapon[WD.LCK_STAT_MOD]),
        s("status"): {
            s("bleed"): float(weapon[WD.DMG_BLEED]),
            s("poison"): float(weapon[WD.DMG_POISON]),
            s("frost"): float(weapon[WD.DMG_FROST]),
        },
        s("damage"): {
            s("physical"): float(weapon[WD.DMG_PHYSICAL]),
            s("magic"): float(weapon[WD.DMG_MAGIC]),
            s("fire"): float(weapon[WD.DMG_FIRE]),
            s("lightning"): float(weapon[WD.DMG_LIGHTNING]),
            s("dark"): float(weapon[WD.DMG_DARK]),
        },
        s("scaling"): {
            s("str"): float(weapon[WD.SCL_STR]),
            s("dex"): float(weapon[WD.SCL_DEX]),
            s("int"): float(weapon[WD.SCL_INT]),
            s("fth"): float(weapon[WD.SCL_FTH]),
            s("lck"): float(weapon[WD.SCL_LCK]),
        }
    }

def parse_weapons(filename):
    out = {}

    with open(filename) as f:
        reader = csv.reader(f, delimiter=",")
        for weapon in reader:
            parse_weapon(out, weapon)

    for data in out.values():
        data["static"][s("infusable")] = "Heavy" in data[s("infusion")]

    return out

def parse_defense(out, weapon, level):
    def get_name_and_infusion(full_name):
        return full_name.split("+")[0].strip()

    name = f"{get_name_and_infusion(weapon[0])} +{level}"

    out[name] = {
        s("absorption"): {
            s("physical"): float(weapon[5]),
            s("magic"): float(weapon[6]),
            s("fire"): float(weapon[7]),
            s("lightning"): float(weapon[8]),
            s("dark"): float(weapon[9]),
        },
        s("resistance"): {
            s("bleed"): float(weapon[10]),
            s("poison"): float(weapon[11]),
            s("frost"): float(weapon[12]),
            s("curse"): float(weapon[13]),
        },
        s("stability"): float(weapon[14]),
    }

def parse_defenses(filename, level):
    out = {}

    with open(filename) as f:
        reader = csv.reader(f, delimiter="\t")
        for weapon in reader:
            parse_defense(out, weapon, level)

    return out

def assign_defenses(weapons, defenses, level):
    for weapon in weapons.values():
        for infusion in weapon[s("infusion")]:
            name = weapon["static"][s("name")]
            infs = "" if infusion == "-" else f"{infusion} "
            weapon[s("infusion")][infusion][s("absorption")] = defenses[f"{name} {infs}+{level}"][s("absorption")]
            weapon[s("infusion")][infusion][s("resistance")] = defenses[f"{name} {infs}+{level}"][s("resistance")]
            weapon[s("infusion")][infusion][s("stability")] = defenses[f"{name} {infs}+{level}"][s("stability")]

def parse_saturation_indices(filename):
    links = {}

    with open(filename) as f:
        reader = csv.reader(f, delimiter=",")
        for weapon in reader:
            index = int(weapon[WD.ID][:-4])

            if index not in links:
                links[index] = {}

            links[index][weapon[WD.INFUSION]] = {
                s("physical"): int(weapon[WD.SAT_FUNC_PHYSICAL]),
                s("magic"): int(weapon[WD.SAT_FUNC_MAGIC]),
                s("fire"): int(weapon[WD.SAT_FUNC_FIRE]),
                s("lightning"): int(weapon[WD.SAT_FUNC_LIGHTNING]),
                s("dark"): int(weapon[WD.SAT_FUNC_DARK]),
                s("bleed"): int(weapon[WD.SAT_FUNC_BLEED]),
                s("poison"): int(weapon[WD.SAT_FUNC_POISON]),
            }

    return links

def merge_weapons(weapon_groups, saturation_indices):
    merged = {}

    for index, weapon in weapon_groups[0].items():
        name = weapon["static"][s("name")]
        merged[name] = weapon["static"]
        merged[name][s("infusion")] = {}

    for index, weapon in weapon_groups[0].items():
        name = weapon["static"][s("name")]
        for infusion in weapon[s("infusion")]:
            merged[name][s("infusion")][infusion] = {
                s("saturation_index"): saturation_indices[index][infusion]
            }
            merged[name][s("infusion")][infusion][s("level")] = []
            for level in range(11):
                merged[name][s("infusion")][infusion][s("level")].append(weapon_groups[level][index][s("infusion")][infusion])

    return merged

def assign_order_id(weapons, filename):
    with open(filename) as f:
        orders = json.load(f)

    order_id = 1
    for name in orders:
        weapons[name][s("order")] = order_id
        order_id += 1

def assign_uniqueness(weapons, filename):
    def get_name(full_name):
        return full_name.split("+")[0].strip()

    with open(filename) as f:
        reader = csv.reader(f, delimiter="\t")
        for row in reader:
            name = get_name(row[0])
            weapons[name][s("unique")] = row[0][-1] == "5" # +5

def fix_missing_classes(data):
    # Spreasheets don't contain classes unique to DLCs and these have their classes wrong
    data["Giant Door Shield"][s("class")] = "Greatshield (Paired)"
    data["Crow Quills"][s("class")] = "Special Thrusting Sword"
    data["Ringed Knight Paired Greatswords"][s("class")] = "Ultra Greatsword (Paired)"

def parse_saturations(filename):
    sats = []

    # 16 seems like a stopping point despite MugenMonkey blog saying only 11 are used
    INDEX_LIMIT = 16

    class Sat(IntEnum):
        INDEX = 1
        LEVELS_BEGIN = 319

    with open(filename) as f:
        reader = csv.reader(f, delimiter=",")
        for sat in reader:
            index = int(sat[Sat.INDEX])
            if index > INDEX_LIMIT:
                break

            sats.append([0.])
            for row in range(Sat.LEVELS_BEGIN, Sat.LEVELS_BEGIN + 99):
                sats[index].append(float(sat[row]))

    return sats

def build_weapon_data(delete_artifacts=False):
    print(f"Parsing weapon data (shortened={SHORTEN})...", flush=True)

    weaps = []
    for level in range(11):
        w = parse_weapons(f"{dir_path}/Raw/weapons_{level}.csv")
        d = parse_defenses(f"{dir_path}/Raw/defense_{level}.tsv", level)
        assign_defenses(w, d, level)
        weaps.append(w)

    saturation_indices = parse_saturation_indices(f"{dir_path}/Raw/weapons_0.csv")
    weapon_data = merge_weapons(weaps, saturation_indices)
    assign_order_id(weapon_data, f"{dir_path}/Raw/order.json")
    assign_uniqueness(weapon_data, f"{dir_path}/Raw/uniqueness.tsv")
    fix_missing_classes(weapon_data)

    print(f"Writing base JSON weapon data (minimized={MINIMIZED})...", flush=True)
    with open(f"{dir_path}/weapon_data.json", "w", encoding="utf-8") as f:
        if MINIMIZED:
            json.dump(weapon_data, f, ensure_ascii=False, separators=(",", ":"))
        else:
            json.dump(weapon_data, f, ensure_ascii=False, indent=4)

    print("Compressing to bz2...", flush=True)
    with open(f"{dir_path}/weapon_data.json", "rb") as raw:
        with bz2.open(f"{dir_path}/weapon_data.json.bz2", "wb") as out:
            out.write(raw.read())

    print(f"Preparing C++ byte format for weapon data (filename=\"{FILENAME_WEAPON_DATA}\")...", flush=True)
    with open(f"{dir_path}/weapon_data.json.bz2", "rb") as f:
        with open(f"{dir_path}/{FILENAME_WEAPON_DATA}", "w") as out:
            out.write(to_cpp_byte_array(f.read()))

    print("Weapon data generation successful!", flush=True)

    if delete_artifacts:
        print("Deleting artifacts...", flush=True)
        os.unlink(f"{dir_path}/weapon_data.json")
        os.unlink(f"{dir_path}/weapon_data.json.bz2")

def build_saturations(delete_artifacts=False):
    saturations = parse_saturations(f"{dir_path}/Raw/saturations.csv")

    if not delete_artifacts: # unneeded when deleting
        print("Writing JSON saturation data...", flush=True)
        with open(f"{dir_path}/saturations.json", "w", encoding="utf-8") as f:
            json.dump(saturations, f, ensure_ascii=False, indent=4)

    print("Preparing C++ array format for saturation functions...", flush=True)
    with open(f"{dir_path}/{FILENAME_SATURATIONS}", "w", encoding="utf-8") as f:
        for sat in saturations:
            f.write("{")
            for value in sat:
                f.write(f"{value}f,")
            f.write("},")

    print("Saturations generation successful!", flush=True)

def build_images():
    images = os.listdir(f"{dir_path}/Raw/Image/")
    images = [Path(i).stem for i in images]

    print(f"Preparing C++ {FILENAME_IMAGE_DECLARATIONS}...", flush=True)
    with open(f"{dir_path}/{FILENAME_IMAGE_DECLARATIONS}", "w") as out:
        for name in images:
            identifier = to_cpp_identifier(name)
            out.write(f"static uint8_t {identifier}[]={{")
            with open(f"{dir_path}/Raw/Image/{name}.png", "rb") as image:
                out.write(to_cpp_byte_array(image.read()))
            out.write("};")

    print(f"Preparing C++ {FILENAME_IMAGE_ACCESSOR}...", flush=True)
    with open(f"{dir_path}/{FILENAME_IMAGE_ACCESSOR}", "w") as out:
        out.write("uint8_t* GetImageBytes(unsigned int& size, std::string& name){\n")
        index = 0
        out.write(f"static int index={index};switch(++index){{\n")
        for name in images:
            index += 1
            identifier = to_cpp_identifier(name)
            out.write(f"case {index}: size=sizeof({identifier}); name=\"{name}\"; return {identifier};\n")
        out.write("}return nullptr;}")

    print("Images generation successful!", flush=True)

if __name__ == "__main__":
    build_weapon_data()
    build_saturations()
    build_images()