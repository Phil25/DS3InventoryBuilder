import os
import csv
from enum import IntEnum

class Col(IntEnum):
    INFUSION = 0
    PHYSICAL = 9
    MAGIC = 10
    FIRE = 11
    LIGHTNING = 12
    DARK = 13
    TOTAL = 14
    BLEED = 15
    POISON = 16
    FROST = 17

def to_cpp_identifier(name):
    identifier = name.replace(" ", "_")
    identifier = identifier.replace("'", "")
    return identifier.replace("-", "")

def zero_out(value):
    return "0" if value == "-" else value

def generate_test(weapon, infusions, suite, out):
    out.write("\n\n")

    out.write(f"TEST_F({suite}, {to_cpp_identifier(weapon)})")
    out.write("{\n")

    for infusion, damages in infusions.items():
        out.write("\t{\n")
        out.write(f"\tSCOPED_TRACE(\"{infusion}\");\n")
        out.write(f"\tconst auto& [damage, status] = calculator::AttackRating(db, \"{weapon}\", Weapon::Infusion::{infusion}, 10, attribs);\n")

        out.write(f"\tEXPECT_EQ({damages['physical']}, static_cast<int>(damage.physical));\n")
        out.write(f"\tEXPECT_EQ({damages['magic']}, static_cast<int>(damage.magic));\n")
        out.write(f"\tEXPECT_EQ({damages['fire']}, static_cast<int>(damage.fire));\n")
        out.write(f"\tEXPECT_EQ({damages['lightning']}, static_cast<int>(damage.lightning));\n")
        out.write(f"\tEXPECT_EQ({damages['dark']}, static_cast<int>(damage.dark));\n")

        out.write(f"\tEXPECT_EQ({damages['bleed']}, static_cast<int>(status.bleed));\n")
        out.write(f"\tEXPECT_EQ({damages['poison']}, static_cast<int>(status.poison));\n")
        out.write(f"\tEXPECT_EQ({damages['frost']}, static_cast<int>(status.frost));\n")
        out.write("\t}\n")

    out.write("}")

def generate_suite(suite, attribs, filename):
    with open(f"{suite}Test.cpp", "w") as out:
        suite = f"{suite}_{'_'.join(attribs)}"
        attribs = f"{attribs[0]}.f,{attribs[1]}.f,{attribs[2]}.f,{attribs[3]}.f,{attribs[4]}.f"

        out.write("#include <gtest/gtest.h>\n#include <Calculator.h>\nusing namespace invbuilder;\n")
        out.write(f"class {suite} : public ::testing::Test")
        out.write("{\n")
        out.write("protected:\n")
        out.write("static Database db;\n")
        out.write("static PlayerAttributes attribs;\n")
        out.write("};\n")
        out.write(f"Database {suite}::db = Database::Create();\n")
        out.write(f"PlayerAttributes {suite}::attribs = {{{attribs}}};\n")

        weapons = {}

        with open(f"./Data/{filename}", "r") as f:
            reader = csv.reader(f, delimiter="\t")
            for row in reader:
                if row[Col.INFUSION] == "Bare Fists":
                    continue

                split = row[Col.INFUSION].split(" ")

                infusion = split[-2]
                infusion = "None" if not infusion else infusion

                weapon = " ".join(split[:-2])
                if weapon == "":
                    infusion = "None"
                    weapon = "Dark Hand"

                if weapon not in weapons:
                    weapons[weapon] = {}

                weapons[weapon][infusion] = {
                    "physical": zero_out(row[Col.PHYSICAL]),
                    "magic": zero_out(row[Col.MAGIC]),
                    "fire": zero_out(row[Col.FIRE]),
                    "lightning": zero_out(row[Col.LIGHTNING]),
                    "dark": zero_out(row[Col.DARK]),
                    "bleed": zero_out(row[Col.BLEED]),
                    "poison": zero_out(row[Col.POISON]),
                    "frost": zero_out(row[Col.FROST]),
                }

        out.write("\n#ifndef NDEBUG\n")

        for weapon, infusions in weapons.items():
            generate_test(weapon, infusions, suite, out)

        out.write("\n#endif // NDEBUG\n")

for filename in os.listdir("./Data/"):
    split = filename.split("-")
    suite = split[0]
    attribs = split[1:6]
    print("Generating", suite, attribs)
    generate_suite(suite, attribs, filename)