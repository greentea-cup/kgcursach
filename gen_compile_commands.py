import subprocess
import json
import re

find_source = re.compile(r'\b([\w/]*\.c|[\w/]*\.cpp)\b')
find_out = re.compile(r'-o\s*([\w/]*\.o)')
known_compilers = ("gcc", "g++", "clang", "clang++", "cl")

make_commands = subprocess.run(["make", "--always-make", "--dry-run", "all"], capture_output=True).stdout.decode().split('\n')
result = []
for cmd in make_commands:
    if cmd.startswith(known_compilers) and ('.cpp' in cmd or '.c' in cmd):
        sources = find_source.findall(cmd)
        outs = find_out.findall(cmd)
        if not outs:
            continue
        out = outs[0]
        for src in sources:
            result.append({'directory': '.', 'command': cmd, 'file': src, "output": out})

with open("compile_commands.json", 'w') as f:
    json.dump(result, f)

