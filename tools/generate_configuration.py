import os
import json

def generate_configuration(settingspath, filepath, templatepath, force = False):
    if not os.path.exists(settingspath):
        print(f"missing settings file at {settingspath}")
        return
    if not os.path.exists(templatepath):
        print(f"missing template file at {templatepath}")
        return

    settings_modified_time = os.path.getmtime(settingspath)

    if os.path.exists(filepath) and not force:
        fObj = open(filepath, 'r')
        content = fObj.read()
        fObj.close()
        if content.find(f"// settings updated at {settings_modified_time}."):
            print("header file up to date")
            return
        print("header file out of date, updating...")
    
    settings = json.loads(open(settingspath, 'r').read())

    with open(templatepath,'r') as f:
        template = f.read()
    
    fObj = open(filepath, 'w+')
    fObj.write("// *****************************************************************************\n")
    fObj.write("// generated header file don't edit.\n")
    fObj.write("// edit `tools/generate_configuration.py` instead.\n")
    fObj.write(f"// settings updated at {settings_modified_time}.\n")
    fObj.write("// *****************************************************************************\n")

    for name in settings:
        value = settings[name]
        template = template.replace(f"$!{name.upper()}", value.upper())
        template = template.replace(f"${name.upper()}", value)

    fObj.write(template)
    fObj.truncate()
    fObj.close()

if __name__ == "__main__":
    generate_configuration(os.path.dirname(os.path.realpath(__file__)) + "/../settings.json", os.path.dirname(os.path.realpath(__file__)) +"/../include/psl/config.hpp", "config.template", True)