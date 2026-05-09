import {
    type Entry,
    type LogLevelVal,
    type PayloadBase,
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
        if (!(yield* waitForData(20))) {
            return
        }

        const payload: PayloadBase = {
            ts: buffer.readUint32LE(ptr),
            eip: buffer.readUint32LE(ptr + 4),
            pid: buffer.readInt32LE(ptr + 8),
            pgid: buffer.readInt32LE(ptr + 12),
            len: buffer.readUint16LE(ptr + 16)
        }
        const type = buffer.readUint8(ptr + 18) as TypeVal
        const typeExt = buffer.readUint8(ptr + 19)
        ptr += 20

        if (!(yield* waitForData(payload.len))) {
            return
        }

        const content = buffer.subarray(ptr, ptr + payload.len)
        ptr += payload.len

        switch (type) {
            case Type.T_Log:
                yield {
                    payload: {
                        ...payload,
                        type: Type.T_Log,
                        logLevel: typeExt as LogLevelVal
                    },
                    content: content.toString()
                }
                break
            case Type.T_Syscall:
                yield {
                    payload: {
                        ...payload,
                        type: Type.T_Syscall,
                        syscallRole: typeExt as SyscallRoleVal
                    },
                    content: {
                        ret: content.readUint32LE(0),
                        id: content.readUint32LE(4),
                        args: [
                            content.readUint32LE(8),
                            content.readUint32LE(12),
                            content.readUint32LE(16),
                            content.readUint32LE(20),
                            content.readUint32LE(24),
                            content.readUint32LE(28)
                        ]
                    }
                }
                break
            case Type.T_Exception:
                yield {
                    payload: {
                        ...payload,
                        type: Type.T_Exception
                    },
                    content: {
                        num: content.readUint32LE(0),
                        errcode: content.readUint32LE(4),
                        cs: content.readUint32LE(8),
                        eip: content.readUint32LE(12),
                        cr2: content.readUint32LE(16)
                    }
                }
                break
            case Type.T_Fatal:
                yield {
                    payload: {
                        ...payload,
                        type: Type.T_Fatal
                    }
                }
                break
        }
    }
}
