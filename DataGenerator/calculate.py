import json

def load_data(filename):
    with open(filename) as f:
        return json.load(f)

def get_ar(stats, name, infusion, level, two_handing=False):
    data = load_data("weapon_data.json")
    sats = load_data("saturations.json")

    lck_coeff = data[name]["lck_coeff"] / 100
    infusion_info = data[name]["infusion"][infusion]
    level_info = infusion_info["level"][level]

    sat_ids = infusion_info["saturation_index"]
    status = level_info["status"]
    damages = level_info["damage"]
    scaling = level_info["scaling"]
    lck_stat_mod = level_info["lck_mod"]

    statStr = stats["str"]

    if two_handing:
        statStr = min(statStr * 1.5, 99)

    return (
        damages["physical"] * (1 +
            scaling["str"] / 100 * sats[sat_ids["physical"]][statStr] / 100 +
            scaling["dex"] / 100 * sats[sat_ids["physical"]][stats["dex"]] / 100 +
            scaling["lck"] / 100 * sats[sat_ids["physical"]][stats["lck"]] / 100 +
            scaling["fth"] / 100 * sats[sat_ids["physical"]][stats["fth"]] / 100 * int(infusion == "Blessed")
        ),
        damages["magic"] * (1 +
            scaling["int"] / 100 * sats[sat_ids["magic"]][stats["int"]] / 100
        ),
        damages["fire"] * (1 +
            scaling["int"] / 100 * sats[sat_ids["fire"]][stats["int"]] / 100 +
            scaling["fth"] / 100 * sats[sat_ids["fire"]][stats["fth"]] / 100
        ),
        damages["lightning"] * (1 +
            scaling["fth"] / 100 * sats[sat_ids["lightning"]][stats["fth"]] / 100
        ),
        damages["dark"] * (1 +
            scaling["int"] / 100 * sats[sat_ids["dark"]][stats["int"]] / 100 +
            scaling["fth"] / 100 * sats[sat_ids["dark"]][stats["fth"]] / 100
        ),
        status["bleed"] * (
            1 + lck_coeff * lck_stat_mod * sats[sat_ids["bleed"]][stats["lck"]] / 100
        ),
        status["poison"] * (
            1 + lck_coeff * lck_stat_mod * sats[sat_ids["poison"]][stats["lck"]] / 100
        ),
        status["frost"]
    )

def main():
    stats = {"str": 30, "dex": 60, "int": 11, "fth": 11, "lck": 7}
    name = "Crow Quills"
    infusion = "Sharp"
    level = 9
    ar = get_ar(stats, name, infusion, level, False)

    print(f"{infusion} {name}+{level} -> {ar}")

if __name__ == "__main__":
    main()