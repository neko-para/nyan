import fs from 'fs/promises'
import path from 'path'
import { parse } from 'yaml'

const res = parse(await fs.readFile(path.join(import.meta.dirname, 'sysdef.yml'), 'utf8')) as {
    [id: number]: {
        name: string
        ret: string
        args?: {
            [name: string]: string
        }
    }
}

const headerRows: string[] = [
    `#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <nyan/errno.h>
#include <nyan/signal.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

namespace nyan::interrupt {
struct SyscallFrame;
}`,
    '',
    'namespace nyan::syscall {',
    ''
]

for (const [id, info] of Object.entries(res)) {
    headerRows.push(`// ${id}`)
    let decl = ''
    if (info.ret === 'never') {
        decl += '[[noreturn]] void '
    } else {
        decl += info.ret.replace(/ @.+$/, '') + ' '
    }
    decl += info.name
    const args = Object.entries(info.args ?? {}).map(([name, type]) => {
        if (type === 'frame') {
            return 'interrupt::SyscallFrame* frame'
        } else {
            return type.replace(/ @.+$/, '') + ' ' + name
        }
    })
    decl += `(${args.join(', ')});`
    headerRows.push(decl, '')
}

headerRows.push('}  // namespace nyan::syscall', '')

await fs.writeFile(
    path.join(import.meta.dirname, '../include/nyan/syscall.h'),
    headerRows.join('\n')
)

const dispatchRows = ['#pragma mark - BEGIN']
for (const [id, info] of Object.entries(res)) {
    dispatchRows.push(`        case ${id}:
            CALL(${info.name});
            break;`)
}
dispatchRows.push('#pragma mark - END')

const oldDispatchContents = await fs.readFile(
    path.join(import.meta.dirname, '../kernel/interrupt/impl.cpp'),
    'utf8'
)
await fs.writeFile(
    path.join(import.meta.dirname, '../kernel/interrupt/impl.cpp'),
    oldDispatchContents.replace(
        /#pragma mark - BEGIN[\s\S]+#pragma mark - END/,
        dispatchRows.join('\n')
    )
)
