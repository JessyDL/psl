import os
import json

def generate(settingspath, filepath, templatepath, force = False):
    if not os.path.exists(settingspath):
        print(f"missing settings file at {settingspath}")
        return
    if not os.path.exists(templatepath):
        print(f"missing template file at {templatepath}")
        return

    settings_modified_time = os.path.getmtime(settingspath)
    template_modified_time = os.path.getmtime(templatepath)

    if os.path.exists(filepath) and not force:
        fObj = open(filepath, 'r')
        content = fObj.read()
        fObj.close()
        if f"// settings updated at {settings_modified_time}." in content and f"// template updated at {template_modified_time}." in content:
            print("config file up to date")
            return
        print("config file out of date, updating...")
    
    settings = json.loads(open(settingspath, 'r').read())

    with open(templatepath,'r') as f:
        template = f.read()
    
    fObj = open(filepath, 'w+')
    fObj.write("// *****************************************************************************\n")
    fObj.write("// generated header file don't edit.\n")
    fObj.write("// edit `settings.json` and `tools/generate_configuration.py` instead.\n")
    fObj.write(f"// settings updated at {settings_modified_time}.\n")
    fObj.write(f"// template updated at {template_modified_time}.\n")
    fObj.write("// *****************************************************************************\n")

    for name in settings:
        value = settings[name]
        if isinstance(value, list) or isinstance(value, dict):
            if name == "includes":
                template = template.replace(f"${name.upper()}", "\n".join(f"#include <{e}>" for e in value))
            continue
        template = template.replace(f"$!{name.upper()}", value.upper())
        template = template.replace(f"${name.upper()}", value)

    fObj.write(template)
    fObj.truncate()
    fObj.close()

if __name__ == "__main__":
    generate(os.path.dirname(os.path.realpath(__file__)) + "/../settings.json", os.path.dirname(os.path.realpath(__file__)) +"/../include/psl/config.hpp", "config.template", True)