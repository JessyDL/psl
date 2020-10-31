import subprocess

def run_command(command = []):
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    data = ""
    while proc.poll() is None:
        output = proc.stdout.readline()
        if output:
            data = data + output.decode("utf-8") 
            
    output = proc.communicate()[0]
    if output:
        data = data + output.decode("utf-8") 
        
    return data

def all_authors():
    possible_authors = run_command(["git", "shortlog", "-s", "-n", "--all", "--no-merges"]).split("\n")
    author_exemptions = ["Travis-CI"]
    author_alias = {'JessyDL':'Jessy De Lannoit', 'Jessy':'Jessy De Lannoit'}
    authors = {}
    for author in possible_authors:
        if author and not any(s in author for s in author_exemptions):
            author = author.strip()
            number, name = author.split(None,1)
            if name in author_alias.keys():
                name = author_alias[name]
                
            if name not in authors:
                authors[name] = number
            else:
                authors[name] += number
            
    list(sorted(authors.items()))
    return authors.keys()

def git_version():
    version = run_command(["git", "tag", "-l", "--sort=-v:refname"]) 
    version = version.split('\n')[0]
    if len(version) == 0:
        return 0, 0, 0
    major, minor, patch = version.split('.')
    return int(major), int(minor), int(patch)

def git_sha1():
    return run_command(["git", "rev-parse", "HEAD"]).rstrip()

def git_timestamp():
    res = run_command(["git", "log", "-1", "--pretty=format:%ct"])
    if not res:
        return 0
    return int(res)

def git_log_since(major, minor, patch):
    if major == 0 and minor == 0 and patch == 0:
        logs = run_command(["git", "log", "--format=%B", "--no-merges"])
    else:
        logs = run_command(["git", "log",  f"{major}.{minor}.{patch}..HEAD", "--format=%B", "--no-merges"])
    if not logs:
        return ""
    logs = logs.splitlines()
    logs = [x for x in logs if x]
    logs.sort()
    return "\n".join(logs)

def create_patch(message = None):
    current_major, current_minor, current_patch = git_version()
    next_major = current_major
    next_minor = current_minor
    next_patch = current_patch + 1
    changes = git_log_since(current_major, current_minor, current_patch)
    if not changes or len(changes) < 3:
        print("unlikely small changes in patch version, please verify this is what you want to do")
        return
    if message:
        message = f"\n{message}\n"
    create_version(next_major, next_minor, next_patch, f'patch release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{current_minor}.{current_patch}:\n{changes}"')

def create_minor(message = None):
    current_major, current_minor, _ = git_version()
    next_major = current_major
    next_minor = current_minor + 1
    next_patch = 0
    changes = git_log_since(current_major, current_minor, 0)
    if not changes or len(changes) < 3:
        print("unlikely small changes in ninor version, please verify this is what you want to do")
        return
    if message:
        message = f"\n{message}\n"
    create_version(next_major, next_minor, next_patch, f'minor release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{current_minor}.{0}:\n{changes}"')

def create_major(message = None):
    current_major, _, _ = git_version()
    next_major = current_major + 1
    next_minor = 0
    next_patch = 0
    changes = git_log_since(current_major, 0, 0)
    if not changes or len(changes) < 3:
        print("unlikely small changes in major version, please verify this is what you want to do")
        return
    if message:
        message = f"\n{message}\n"       
    create_version(next_major, next_minor, next_patch, f'major release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{0}.{0}:\n{changes}"')

def create_version(major, minor, patch, message):
    run_command(["git", "tag", '-a', f'{major}.{minor}.{patch}', '-m', message])

# in case we created a patch we no longer want.
def destroy_local_tag(major, minor, patch):
    run_command(["git", "tag", '-d', f'{major}.{minor}.{patch}'])

if __name__ == '__main__':
    destroy_local_tag(1,0,0)