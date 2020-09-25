import versioning
import os
from datetime import datetime

def generate_header(filepath, force = False):
    major, minor, patch = versioning.git_version()
    sha1 = versioning.git_sha1()
    unix_timestamp = versioning.git_timestamp()

    utc_timestamp = datetime.utcfromtimestamp(unix_timestamp).strftime('%Y-%m-%d %H:%M:%S')
    authors = versioning.all_authors()
    if os.path.exists(filepath) and not force:
        fObj = open(filepath, 'r')
        content = fObj.read()
        if content.find(f"// generated from commit sha1 {sha1}."):
            print("header file up to date")
            return
        print("header file out of date, updating...")
        fObj.close()
    fObj = open(filepath, 'w+')
    fObj.write("// *****************************************************************************\n")
    fObj.write("// generated header file don't edit.\n")
    fObj.write("// edit `tools/versioning.py` instead.\n")
    fObj.write(f"// generated from commit sha1 {sha1}.\n")
    fObj.write("// *****************************************************************************\n")
    fObj.write("#include <psl/types.hpp>\n")
    fObj.write("#include <string_view>\n")
    fObj.write("#include <array>\n")
    fObj.write('namespace psl\n')
    fObj.write('{\n')
    fObj.write(f'\tconstexpr std::string_view VERSION_TIME_UTC  {{ "{utc_timestamp}" }};\n')
    fObj.write(f'\tconstexpr std::string_view VERSION_SHA1      {{ "{sha1}" }};\n')
    fObj.write(f'\tconstexpr std::string_view VERSION_FULL      {{ "{major}.{minor}.{patch}.{sha1}" }};\n')
    fObj.write(f'\tconstexpr ui64             VERSION_TIME_UNIX {{ {unix_timestamp} }};\n')
    fObj.write(f'\tconstexpr ui32             VERSION_MAJOR     {{ {major} }};\n')
    fObj.write(f'\tconstexpr ui32             VERSION_MINOR     {{ {minor} }};\n')
    fObj.write(f'\tconstexpr ui32             VERSION_PATCH     {{ {patch} }};\n')
    fObj.write( '\tconstexpr ui32             VERSION           {((VERSION_MAJOR << 22) | (VERSION_MINOR << 12) | VERSION_PATCH)};\n')    
    fObj.write( '\n')
    fObj.write("\tconstexpr static std::array<std::string_view, "+str(len(authors))+ "> PROJECT_CREDITS\n\t{{\n")
    for i, author in enumerate(authors):
        if i < len(authors) - 1:
            fObj.write('\t\t"' + author + '",\n')
        else:
            fObj.write('\t\t"' + author + '"')
    fObj.write("\n\t}};\n")
    fObj.write('}\n')
    #fObj.write("constexpr static psl::string8::view APPLICATION_FULL_NAME {\"PSL "+ version + "." +sha1+ " "+ utc_timestamp +"\"};\n")
    
    fObj.truncate()
    fObj.close()

if __name__ == "__main__":
    generate_header(os.path.dirname(os.path.realpath(__file__)) +"/../include/psl/psl.hpp")