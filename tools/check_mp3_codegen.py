# SPDX-License-Identifier: BSD-2-Clause
"""Reject unprotected multiplier state and unsupported runtime helpers."""
from pathlib import Path
import re
import sys


def check(path):
    errors = []
    for line, text in enumerate(Path(path).read_text().splitlines(), 1):
        code = text.split('//', 1)[0].split(';', 1)[0]
        if re.search(r'^\s*(?:mpy\w*|mac\w*|maa\w*|msu\w*)\b|\bp[01]\b|\bp\*', code):
            errors.append(f'{line}: {text.strip()}')
        if re.search(r'\bcall\s+__(?:[us]?(?:mul|div|mod)|float|fix|.*[ds]f)', code):
            errors.append(f'{line}: unexpected runtime arithmetic: {text.strip()}')
    if errors:
        raise ValueError('unsafe MP3 C code generation:\n' + '\n'.join(errors))
    print('MP3 C codegen: no live product registers or software floating-point helpers')


if __name__ == '__main__':
    check(sys.argv[1])
