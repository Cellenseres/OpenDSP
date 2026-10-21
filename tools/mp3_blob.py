# SPDX-License-Identifier: BSD-2-Clause
"""Pack checked-in generated MP3 tables; no generator/parser dependencies."""
from pathlib import Path
import re
import struct
import sys

ROOT=Path(__file__).resolve().parents[1]


def pack():
    words=[]
    for name in ('tables.h','hybrid.inc'):
        text=(ROOT/'firmware/mp3'/name).read_text()
        for match in re.finditer(r'#define (\w+) \(\*\(const (u16|s32) \(\*\)((?:\[\d+\])+)\)0x([0-9a-f]+)\)',text):
            symbol,typ,shape,address=match.groups()
            shape=[int(x) for x in re.findall(r'\d+',shape)]
            at=text.index('#else',match.end())
            start=text.index('{',at);end=text.index(';',start)
            data=text[start:end]
            if len(shape)>1:
                values=[]
                for row in re.findall(r'\{([^{}]*)\}',data):
                    entries=[int(x) for x in re.findall(r'-?\d+',row)]
                    values+=entries+[0]*(shape[-1]-len(entries))
            else: values=[int(x) for x in re.findall(r'-?\d+',data)]
            count=1
            for n in shape:count*=n
            if len(values)!=count: raise ValueError((symbol,'shape',len(values),count))
            offset=int(address,16)-0xc000
            if offset<len(words): raise ValueError((symbol,'overlap'))
            words += [0]*(offset-len(words))
            for value in values:
                if typ=='s32':words.append(value>>16&65535)
                words.append(value&65535)
    expected=int(re.search(r'MP3_TABLE_WORDS (\d+)',(ROOT/'firmware/mp3/table_layout.h').read_text()).group(1))
    if len(words)!=expected or len(words)>11358: raise ValueError('table footprint mismatch')
    return struct.pack('<'+'H'*len(words),*words)


if __name__=='__main__':
    output=Path(sys.argv[1]);output.parent.mkdir(parents=True,exist_ok=True)
    output.write_bytes(pack());print(f'{output}: {output.stat().st_size} bytes')
