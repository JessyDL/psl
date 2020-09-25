import versioning
import os
from datetime import datetime

def generate_header(filepath, force = False):
    major, minor, patch = versioning.git_version()
    sha1 = versioning.git_sha1()
    unix_timestamp = int(versioning.git_timestamp())

    utc_timestamp = datetime.utcfromtimestamp(unix_timestamp).strftime('%Y-%m-%d %H:%M:%S')
    #authors = all_authors()
    if os.path.exists(filepath) and not force:
        fObj = open(filepath, 'r')
        content = fObj.read()
        if content.find("#define VERSION_SHA1 " + sha1):
            print("header file up to date")
            return
        print("header file out of date, updating...")
        fObj.close()
    fObj = open(filepath, 'w+')
    fObj.write("// *****************************************************************************\n")
    fObj.write("// generated header file don't edit.\n")
    fObj.write("// edit `tools/versioning.py` instead.\n")
    fObj.write("// *****************************************************************************\n")
    fObj.write("#include <cstdint>\n")
    fObj.write(f'#define VERSION_TIME_UTC "{utc_timestamp}"\n')
    fObj.write(f'#define VERSION_TIME_UNIX "{unix_timestamp}"\n')
    fObj.write(f'#define VERSION_MAJOR "{major}"\n')
    fObj.write(f'#define VERSION_MINOR "{minor}"\n')
    fObj.write(f'#define VERSION_PATCH "{patch}"\n')
    fObj.write(f'#define VERSION_SHA1 "{sha1}"\n')

    fObj.write(f'#define VERSION "{major}.{minor}.{patch}"\n')
    fObj.write(f'#define VERSION_FULL "{major}.{minor}.{patch}.{sha1}"\n')
    fObj.write('namespace psl\n')
    fObj.write('{\n')
    fObj.write(f'\tconstexpr std::uint64_t VERSION_TIME_UNIX {{ {unix_timestamp} }};\n')
    fObj.write(f'\tconstexpr std::uint32_t VERSION_MAJOR {{ {major} }};\n')
    fObj.write(f'\tconstexpr std::uint32_t VERSION_MINOR {{ {minor} }};\n')
    fObj.write(f'\tconstexpr std::uint32_t VERSION_PATCH {{ {patch} }};\n')
    fObj.write( '\tconstexpr std::uint32_t VERSION {((VERSION_MAJOR << 22) | (VERSION_MINOR << 12) | VERSION_PATCH)};\n')
    fObj.write('}\n')
    #fObj.write("constexpr static psl::string8::view APPLICATION_NAME {\"PSL\"};\n")
    #fObj.write("constexpr static psl::string8::view APPLICATION_FULL_NAME {\"PSL "+ version + "." +sha1+ " "+ utc_timestamp +"\"};\n")
    #fObj.write("\n constexpr static std::array<psl::string8::view, "+str(len(authors))+ "> APPLICATION_CREDITS\n{{\n")
    #for i, author in enumerate(authors):
    #    if i < len(authors) - 1:
    #        fObj.write('\t"' + author + '",')
    #    else:
    #        fObj.write('\t"' + author + '"')
    #fObj.write("\n}};")
    fObj.truncate()
    fObj.close()

if __name__ == "__main__":
    generate_header(os.path.dirname(os.path.realpath(__file__)) +"/../include/psl/psl.hpp", True)