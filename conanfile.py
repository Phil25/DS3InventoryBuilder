from conans import ConanFile, CMake, tools
import DataGenerator.generate as gen
from pathlib import Path
import shutil

class DS3InventoryBuilder(ConanFile):
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "cmake_installer/3.14.5@conan/stable",
        "wxwidgets/3.1.4@bincrafters/stable",
        "rapidjson/cci.20200410",
        "bzip2/1.0.8"
    )

    def _set_version(self, cmake):
        ver = tools.get_env("APP_VERSION", "undetermined version")
        if not ver:
            ver = "undetermined version"
        cmake.definitions["APP_VERSION"] = str(ver)

    def _generate_data(self):
        out_path = Path.cwd() / "DS3InventoryBuilderCore" / "Source" / "Data"
        gen_path = Path.cwd() / "DataGenerator"

        if (out_path / gen.FILENAME_WEAPON_DATA).exists():
            print(f"Weapon data already generated", flush=True)
        else:
            gen.build_weapon_data(delete_artifacts=True)
            shutil.move(gen_path / gen.FILENAME_WEAPON_DATA, out_path)

        if (out_path / gen.FILENAME_SATURATIONS).exists():
            print(f"Saturations already generated", flush=True)
        else:
            gen.build_saturations(delete_artifacts=True)
            shutil.move(gen_path / gen.FILENAME_SATURATIONS, out_path)

        if (out_path / gen.FILENAME_IMAGE_DECLARATIONS).exists() and (out_path / gen.FILENAME_IMAGE_ACCESSOR).exists():
            print(f"Image data already generated", flush=True)
        else:
            gen.build_images()
            shutil.move(gen_path / gen.FILENAME_IMAGE_DECLARATIONS, out_path / gen.FILENAME_IMAGE_DECLARATIONS)
            shutil.move(gen_path / gen.FILENAME_IMAGE_ACCESSOR, out_path / gen.FILENAME_IMAGE_ACCESSOR)

    def build(self):
        cmake = CMake(self)

        self._set_version(cmake)
        self._generate_data()

        cmake.configure()
        cmake.build()