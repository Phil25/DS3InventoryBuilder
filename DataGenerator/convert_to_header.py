import json

# *THIS SCRIPT IS UNUSED*

def get_weapon_class(name):
    return {
        "Fist": "Fist",
        "Dagger": "Dagger",
        "Dagger (Paired)": "Dagger",
        "Straight Sword": "StraightSword",
        "Sword (Paired)": "StraightSword",
        "Curved Sword": "CurvedSword",
        "Curved Sword (Paired)": "CurvedSword",
        "Greatsword": "Greatsword",
        "Thrusting Sword": "ThrustingSword",
        "Curved Greatsword": "CurvedGreatsword",
        "Katana": "Katana",
        "Katana (Paired)": "Katana",
        "Ultra Greatsword": "UltraGreatsword",
        "Ultra Greatsword (Unique)": "UltraGreatsword",
        "Axe": "Axe",
        "Axe (Paired)": "Axe",
        "Greataxe": "Greataxe",
        "Halberd": "Halberd",
        "Hammer": "Hammer",
        "Hammer (Paired)": "Hammer",
        "Great Hammer": "GreatHammer",
        "Spear": "Spear",
        "Spear (Paired)": "Spear",
        "Pike": "Pike",
        "Reaper": "Reaper",
        "Claw": "Claw",
        "Whip": "Whip",
        "Talisman": "Talisman",
        "Sacred Chime": "SacredChime",
        "Staff": "Staff",
        "Pyromancy Flame": "PyromancyFlame",
        "Bow": "Bow",
        "Greatbow": "Greatbow",
        "Crossbow": "Crossbow",
        "Small Shield": "SmallShield",
        "Shield": "Shield",
        "Greatshield": "Greatshield",
        "Torch": "Torch",
    }[name]

def get_bool(value):
    return "true" if value else "false"

def get_infusion_name(name):
    return "None" if name == "-" else name

def write_requirements(f, requirements):
    f.write('{')
    f.write(f'{requirements["str"]},')
    f.write(f'{requirements["dex"]},')
    f.write(f'{requirements["int"]},')
    f.write(f'{requirements["fth"]},')
    f.write('0')
    f.write('},')

def write_saturation_indices(f, saturation_index):
    f.write('{')
    f.write(f'{saturation_index["physical"]},')
    f.write(f'{saturation_index["magic"]},')
    f.write(f'{saturation_index["fire"]},')
    f.write(f'{saturation_index["lightning"]},')
    f.write(f'{saturation_index["dark"]},')
    f.write(f'{saturation_index["bleed"]},')
    f.write(f'{saturation_index["poison"]},')
    f.write('},')

def write_level_data(f, level):
    f.write('{')

    f.write(f'{level["lck_mod"]},')
    f.write(f'{level["stability"]},')

    f.write('{')
    f.write(f'{level["status"]["bleed"]},')
    f.write(f'{level["status"]["poison"]},')
    f.write(f'{level["status"]["frost"]},')
    f.write('},')

    f.write('{')
    f.write(f'{level["damage"]["physical"]},')
    f.write(f'{level["damage"]["magic"]},')
    f.write(f'{level["damage"]["fire"]},')
    f.write(f'{level["damage"]["lightning"]},')
    f.write(f'{level["damage"]["dark"]},')
    f.write('},')

    f.write('{')
    f.write(f'{level["scaling"]["str"]},')
    f.write(f'{level["scaling"]["dex"]},')
    f.write(f'{level["scaling"]["int"]},')
    f.write(f'{level["scaling"]["fth"]},')
    f.write(f'{level["scaling"]["lck"]},')
    f.write('},')

    f.write('{')
    f.write(f'{level["absorption"]["physical"]},')
    f.write(f'{level["absorption"]["magic"]},')
    f.write(f'{level["absorption"]["fire"]},')
    f.write(f'{level["absorption"]["lightning"]},')
    f.write(f'{level["absorption"]["dark"]},')
    f.write('},')

    f.write('{')
    f.write(f'{level["resistance"]["bleed"]},')
    f.write(f'{level["resistance"]["poison"]},')
    f.write(f'{level["resistance"]["frost"]},')
    f.write(f'{level["resistance"]["curse"]},')
    f.write('},')

    f.write('},')

def write_infusion_data(f, infusion):
    f.write('{')
    write_saturation_indices(f, infusion["saturation_index"])
    f.write('{{')
    for level in range(11):
        write_level_data(f, infusion["level"][level])
    f.write('}}')
    f.write('}')

def write_infusions(f, infusions):
    f.write('{')
    for name in infusions:
        f.write('{')
        f.write(f'Weapon::Infusion::{get_infusion_name(name)},')
        write_infusion_data(f, infusions[name])
        f.write('},')

    f.write('}')
    pass

def write_weapon_data(f, weapon):
    f.write(f'"{weapon["name"]}",')
    f.write(f'Weapon::Type::{get_weapon_class(weapon["class"])},')
    f.write(f'{weapon["order"]},')
    f.write(f'{weapon["lck_coeff"]},')
    f.write(f'{get_bool(weapon["buffable"])},')
    f.write(f'{get_bool(weapon["infusable"])},')
    f.write(f'{get_bool(weapon["unique"])},')
    f.write(f'{weapon["weight"]},')
    write_requirements(f, weapon["requirements"])
    write_infusions(f, weapon["infusion"])

def main():
    with open("weapon_data.json") as f:
        data = json.load(f)

    with open("weapon_data.txt", "w") as f:
        for name in data:
            f.write(f'{{ "{name}", {{')
            write_weapon_data(f, data[name])
            f.write('} },')

if __name__ == "__main__":
    main()