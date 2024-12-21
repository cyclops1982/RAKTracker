import datetime
import os
import subprocess


git = subprocess.run(['git', 'rev-parse', 'HEAD'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

VERSION_FILE = "version.h"
VERSION_CONTENTS = f"#pragma once\n#define VERSIONSTRING \"{git}\""

if os.environ.get('PLATFORMIO_INCLUDE_DIR') is not None:
    VERSION_FILE = os.environ.get('PLATFORMIO_INCLUDE_DIR') + os.sep + VERSION_FILE
elif os.path.exists("include"):
    VERSION_FILE = "include" + os.sep + VERSION_FILE
else:
    PROJECT_DIR = os.environ.get("$PROJECT_DIR")
    VERSION_FILE = "include" + os.sep + VERSION_FILE

print(f"Updating {VERSION_FILE} with version/timestamp...")
with open(VERSION_FILE, 'w+') as FILE:
        FILE.write(VERSION_CONTENTS)