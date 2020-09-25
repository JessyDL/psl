import subprocess

def run_command(command = []):
    proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
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
    possible_authors = run_command(["git shortlog -s -n --all --no-merges"]).split("\n")
    author_exemptions = ["Travis-CI"]
    author_alias = {'JessyDL':'Jessy De Lannoit'}
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
    version = run_command(["git tag -l --sort=-v:refname"]) 
    version = version.split('\n')[0]
    if len(version) == 0:
        return 0, 0, 0
    major, minor, patch = version.split('.')
    return int(major), int(minor), int(patch)

def git_sha1():
    return run_command(["git rev-parse HEAD"]).rstrip()

def git_timestamp():
    return run_command(["git log -1 --pretty=format:%ct"])

def git_log_since(major, minor, patch):
    command = f"git log {major}.{minor}.{patch}..HEAD --format=%B --no-merges"

    if major is 0 and minor is 0 and patch is 0:
        command = "git log --format=%B --no-merges"
    
    logs = run_command([command])
    if not logs:
        return ""
    logs = logs.splitlines()
    logs = [x for x in logs if x]
    logs.sort()
    return "\n".join(logs)

def create_patch(message):
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
    run_command([f'git tag -a {next_major}.{next_minor}.{next_patch} -m "patch release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{current_minor}.{current_patch}:\n{changes}"'])

def create_minor(message):
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
    run_command([f'git tag -a {next_major}.{next_minor}.{next_patch} -m "minor release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{current_minor}.{0}:\n{changes}"'])

def create_major(message):
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
        
    run_command([f'git tag -a {next_major}.{next_minor}.{next_patch} -m "major release {next_major}.{next_minor}.{next_patch}\n{message}\nchanges since {current_major}.{0}.{0}:\n{changes}"'])

# in case we created a patch we no longer want.
def destroy_local_tag(major, minor, patch):
    run_command([f"git tag -d {major}.{minor}.{patch}"])

#if __name__ == '__main__':
#    create_major()