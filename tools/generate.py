import generate_configuration
import generate_project_info
import os


if __name__ == "__main__":
    root = os.path.dirname(os.path.realpath(__file__))
    project = os.path.abspath(os.path.join(root, ".."))
    generate_project_info.generate(os.path.join(project, "include/psl/psl.hpp"))
    generate_configuration.generate(os.path.join(project, "settings.json"), os.path.join(project, "include/psl/config.hpp"), os.path.join(root, "config.template"))