

import subprocess
import json
import os


def get_total_score():
    tot = 0
    res = []

    os.system("cd SDK/cpp_project/build && make")
    
    for i in range(1, 5):
        cmd = "./Robot ./SDK/cpp_project/build/main -m maps/" + \
            str(i) + ".txt -f"
        process = subprocess.Popen(
            cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        process.wait()

        output = process.stdout.read()
        output = output.decode(encoding="utf-8")

        y = json.loads(output)

        res.append(y['score'])
        tot = tot + int(y['score'])

    res.append(tot)
    return res


print(get_total_score())
