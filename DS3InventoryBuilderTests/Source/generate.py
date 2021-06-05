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

def generate_test(weapon, infusion, row, suite, out):
    out.write("\n\n")

    out.write(f"TEST_F({suite}, {to_cpp_identifier(weapon)}_{infusion})")
    out.write("{\n")

    out.write(f"const auto& [damage, status] = calculator::AttackRating(db, \"{weapon}\", Weapon::Infusion::{infusion}, 10, attribs);")

    out.write(f"EXPECT_EQ({row[Col.PHYSICAL]}, static_cast<int>(damage.physical));")
    out.write(f"EXPECT_EQ({row[Col.MAGIC]}, static_cast<int>(damage.magic));")
    out.write(f"EXPECT_EQ({row[Col.FIRE]}, static_cast<int>(damage.fire));")
    out.write(f"EXPECT_EQ({row[Col.LIGHTNING]}, static_cast<int>(damage.lightning));")
    out.write(f"EXPECT_EQ({row[Col.DARK]}, static_cast<int>(damage.dark));")

    out.write(f"EXPECT_EQ({row[Col.BLEED]}, static_cast<int>(status.bleed));")
    out.write(f"EXPECT_EQ({row[Col.POISON]}, static_cast<int>(status.poison));")
    out.write(f"EXPECT_EQ({row[Col.FROST]}, static_cast<int>(status.frost));")

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

                generate_test(weapon, infusion, [col.replace("-", "0") for col in row], suite, out)

for filename in os.listdir("./Data/"):
    split = filename.split("-")
    suite = split[0]
    attribs = split[1:6]
    print("Generating", suite, attribs)
    generate_suite(suite, attribs, filename)