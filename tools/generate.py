import generate_configuration
import generate_project_info
import os


if __name__ == "__main__":
    root = os.path.dirname(os.path.realpath(__file__))
    generate_project_info.generate(root +"/../include/psl/psl.hpp")
    generate_configuration.generate(root + "/../settings.json", root +"/../include/psl/config.hpp", root + "/config.template")