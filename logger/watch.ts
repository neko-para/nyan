import chalk from 'chalk'
import { watch } from 'node:fs'
import { open, stat } from 'node:fs/promises'

import { queryAddr } from './addr.ts'
import { errnoMap, getErrno, isError } from './errno.ts'
import { parse } from './parser.ts'
import { signalMap } from './signal.ts'
import { type ArgDecl, type ArgType, syscallTable } from './syscall.ts'
import {
    type Entry,
    type Payload,
    type SyscallArgs,
    type SyscallContent,
    SyscallRole,
    isExceptionEntry,
    isLogEntry,
    isSyscallEntry
} from './types.ts'

function pad(num: number, len: number, fill = '0') {
    return num.toString().padStart(len, fill)
}

function entryPrefix(payload: Payload) {
    const hour = Math.floor(payload.ts / 3600000)
    const min = Math.floor(payload.ts / 60000) % 60
    const sec = Math.floor(payload.ts / 1000) % 60
    const msec = payload.ts % 1000
    return `[${pad(hour, 2)}:${pad(min, 2)}:${pad(sec, 2)}.${pad(msec, 3)} ${pad(payload.pid, 3, ' ')}]`
}

function formatValue(val: number, type: ArgType) {
    switch (type) {
        case 'int':
            return (val | 0).toString()
        case 'uint':
            return val.toString()
        case 'pid':
            return (val | 0).toString()
        case 'fd':
            return (val | 0).toString()
        case 'ptr':
            return '0x' + val.toString(16).padStart(8, '0')
        case 'size':
            return val.toString()
        case 'str':
            return '0x' + val.toString(16).padStart(8, '0')
        case 'flags':
            return '0x' + val.toString(16)
        case 'signo':
            return signalMap[val] ?? `SIG(${val})`
    }
    return `${type}(${val})`
}

function formatReturn(val: number, type: ArgType) {
    if (isError(val)) {
        return errnoMap[getErrno(val)]
    }
    return formatValue(val, type)
}

function buildCall(args: ArgDecl[], content: SyscallContent) {
    const result: string[] = []
    args.forEach((decl, idx) => {
        result.push(`${decl.name}=${formatValue(content.args[idx], decl.type)}`)
    })
    return result.join(', ')
}

const pendingSyscall: Map<
    number,
    {
        ts: number
        id: number
        args: SyscallArgs
    }
> = new Map()

async function handleEntry(entry: Entry) {
    const prefix = chalk.dim(entryPrefix(entry.payload))
    if (isLogEntry(entry)) {
        const info = await queryAddr(binaryPath, entry.payload.eip)
        console.log(`${prefix} ${info} ${chalk.bold(entry.content.trim())}`)
    } else if (isSyscallEntry(entry)) {
        const def = syscallTable[entry.content.id]
        if (!def) {
            console.log(
                `${prefix} unknown syscall ${entry.content.id} ${entry.payload.syscallRole === SyscallRole.SR_Enter ? 'enter' : 'leave'}`
            )
            return
        }
        if (entry.payload.syscallRole === SyscallRole.SR_Enter) {
            if (def.noret) {
                console.log(`${prefix} ${def.name}(${buildCall(def.args, entry.content)})`)
            } else {
                pendingSyscall.set(entry.payload.pid, {
                    ts: entry.payload.ts,
                    id: entry.content.id,
                    args: entry.content.args
                })
            }
        } else {
            const prev = pendingSyscall.get(entry.payload.pid)
            pendingSyscall.delete(entry.payload.pid)
            if (!prev || prev.id !== entry.content.id) {
                console.log(
                    `${prefix} ${def.name}(${buildCall(def.args, entry.content)}) = ${formatReturn(entry.content.ret, def.ret)}`
                )
            } else {
                const duration = chalk.dim(`${entry.payload.ts - prev.ts}ms`)
                console.log(
                    `${prefix} ${def.name}(${buildCall(def.args, entry.content)}) = ${formatReturn(entry.content.ret, def.ret)} ${duration}`
                )
            }
        }
    } else if (isExceptionEntry(entry)) {
        // TODO: support track addr for user addr
        switch (entry.content.num) {
            case 13: {
                const { errcode: sel, eip } = entry.content
                // GPF
                console.log(
                    `${prefix} ${chalk.bold('#GPF')} sel=${formatValue(sel, 'flags')} eip=${formatValue(eip, 'ptr')}`
                )
                break
            }
            case 14: {
                // PF
                const { errcode: flags, eip, cr2 } = entry.content
                let type = ''
                if (flags & 1) {
                    type += 'P'
                }
                if (flags & 2) {
                    type += 'W'
                }
                if (flags & 4) {
                    type += 'U'
                }
                console.log(
                    `${prefix} ${chalk.bold('#PF')} ${type.trim()} addr=${formatValue(cr2, 'ptr')} eip=${formatValue(eip, 'ptr')}`
                )
                break
            }
            default:
                console.log(
                    `${prefix} ${chalk.bold(`#${entry.content.num}`)} eip=${formatValue(entry.content.eip, 'ptr')}`
                )
        }
    }
}

async function drain(gen: ReturnType<typeof parse>, data?: Buffer | null) {
    let result = gen.next(data)
    while (!result.done) {
        // yield null 表示等待更多数据，暂停消费
        if (result.value === null) return
        await handleEntry(result.value)
        result = gen.next()
    }
}

async function watchFile(path: string): Promise<void> {
    const fd = await open(path, 'r')
    let offset = 0
    let reading = false

    const gen = parse(Buffer.alloc(0))
    // 启动 generator，推进到第一个 yield
    await drain(gen)

    async function readNew(): Promise<void> {
        if (reading) return
        reading = true
        try {
            const { size } = await stat(path)
            if (size <= offset) return

            const readLen = size - offset
            const buf = Buffer.alloc(readLen)
            const { bytesRead } = await fd.read(buf, 0, readLen, offset)
            if (bytesRead === 0) return
            offset += bytesRead

            await drain(gen, buf.subarray(0, bytesRead))
        } finally {
            reading = false
        }
    }

    // 初始读取已有内容
    await readNew()

    // 监听文件变化，增量读取
    watch(path, async eventType => {
        if (eventType === 'change') {
            await readNew()
        }
    })

    console.log(`Watching ${path} ...`)
}

const filePath = process.argv[2]
const binaryPath = process.argv[3]
if (!filePath || !binaryPath) {
    console.error('Usage: watch.ts <log-file> <kernel-file>')
    process.exit(1)
}

watchFile(filePath)
