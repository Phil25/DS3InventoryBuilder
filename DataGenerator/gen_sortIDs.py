import json

# *THIS SCRIPT IS UNUSED*

with open("weapon_data.json") as f:
    j = json.load(f)

print("{")
for name in j.keys():
    print(f"    \"{name}\": ,")
print("}")