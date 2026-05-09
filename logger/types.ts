export const Type = {
    T_Log: 0,
    T_Syscall: 1,
    T_Exception: 2,
    T_Fatal: 3
} as const
export type Type = typeof Type
export type TypeVal = Type[keyof Type]

export const LogLevel = {
    LL_Debug: 0,
    LL_Info: 1,
    LL_Warn: 2,
    LL_Error: 3,
    LL_Fatal: 4
} as const
export type LogLevel = typeof LogLevel
export type LogLevelVal = LogLevel[keyof LogLevel]

export const SyscallRole = {
    SR_Enter: 0,
    SR_Leave: 1
} as const
export type SyscallRole = typeof SyscallRole
export type SyscallRoleVal = SyscallRole[keyof SyscallRole]

export type PayloadBase = {
    ts: number
    eip: number
    pid: number
    pgid: number
    len: number
}

export type Payload = PayloadBase &
    (
        | {
              type: Type['T_Log']
              logLevel: LogLevelVal
          }
        | {
              type: Type['T_Syscall']
              syscallRole: SyscallRoleVal
          }
        | {
              type: Type['T_Exception']
          }
        | {
              type: Type['T_Fatal']
          }
    )

export type SyscallArgs = [number, number, number, number, number, number]

export type SyscallContent = {
    ret: number
    id: number
    args: SyscallArgs
}

export type ExceptionContent = {
    num: number
    errcode: number
    cs: number
    eip: number
    cr2: number
}

export type Entry = {
    payload: Payload
    content?: string | SyscallContent | ExceptionContent
}

export function isLogEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Log'] }
    content: string
} {
    return entry.payload.type === Type.T_Log
}

export function isSyscallEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Syscall'] }
    content: SyscallContent
} {
    return entry.payload.type === Type.T_Syscall
}

export function isExceptionEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Exception'] }
    content: ExceptionContent
} {
    return entry.payload.type === Type.T_Exception
}

export function isFatalEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Fatal'] }
    content?: never
} {
    return entry.payload.type === Type.T_Fatal
}
