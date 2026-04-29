import {
    type Entry,
    type LogLevelVal,
    type Payload,
    type SyscallRoleVal,
    Type,
    type TypeVal
} from './types.ts'

export function* parse(buffer: Buffer): Generator<Entry | null, void, Buffer | null | undefined> {
    let ptr = 0

    /** 丢弃已消费的前缀，等待调用方喂入新数据；返回 false 表示调用方不再提供数据 */
    function* waitForData(need: number): Generator<null, boolean, Buffer | null | undefined> {
        while (ptr + need > buffer.byteLength) {
            // 丢弃已消费部分，保留残余
            buffer = buffer.subarray(ptr)
            ptr = 0
            const next: Buffer | null | undefined = yield null
            if (!next) return false
            buffer = Buffer.concat([buffer, next])
        }
        return true
    }

    for (;;) {
        // 等待足够的 header 数据
        if (!(yield* waitForData(16))) return

        switch (buffer.readUint8(ptr + 14) as TypeVal) {
            case Type.T_Log: {
                const payload: Payload = {
                    ts: buffer.readUint32LE(ptr),
                    eip: buffer.readUint32LE(ptr + 4),
                    pid: buffer.readInt32LE(ptr + 8),
                    len: buffer.readUint16LE(ptr + 12),
                    type: Type.T_Log,
                    logLevel: buffer.readUint8(ptr + 15) as LogLevelVal
                }
                ptr += 16
                // 等待足够的 payload 数据
                if (!(yield* waitForData(payload.len))) return
                yield {
                    payload,
                    log: buffer.subarray(ptr, ptr + payload.len).toString()
                }
                ptr += payload.len
                break
            }
            case Type.T_Syscall: {
                const payload: Payload = {
                    ts: buffer.readUint32LE(ptr),
                    eip: buffer.readUint32LE(ptr + 4),
                    pid: buffer.readInt32LE(ptr + 8),
                    len: buffer.readUint16LE(ptr + 12),
                    type: Type.T_Syscall,
                    syscallRole: buffer.readUint8(ptr + 15) as SyscallRoleVal
                }
                ptr += 16
                // 等待足够的 payload 数据
                if (!(yield* waitForData(payload.len))) return
                yield {
                    payload,
                    content: {
                        ret: buffer.readUint32LE(ptr),
                        eax: buffer.readUint32LE(ptr + 4),
                        args: [
                            buffer.readUint32LE(ptr + 8),
                            buffer.readUint32LE(ptr + 12),
                            buffer.readUint32LE(ptr + 16),
                            buffer.readUint32LE(ptr + 20),
                            buffer.readUint32LE(ptr + 24),
                            buffer.readUint32LE(ptr + 28)
                        ]
                    }
                }
                ptr += payload.len
                break
            }
            default:
                console.log('unknown type', buffer.readUint8(ptr + 14))
                return
        }
    }
}
