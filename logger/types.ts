export const Type = {
    T_Log: 0,
    T_Syscall: 1
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

export type Payload = {
    ts: number
    eip: number
    pid: number
    len: number
} & (
    | {
          type: Type['T_Log']
          logLevel: LogLevelVal
      }
    | {
          type: Type['T_Syscall']
          syscallRole: SyscallRoleVal
      }
)

export type SyscallArgs = [number, number, number, number, number, number]

export type SyscallContent = {
    ret: number
    eax: number
    args: SyscallArgs
}

export type Entry = {
    payload: Payload
    log?: string
    content?: SyscallContent
}

export function isLogEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Log'] }
    log: string
} {
    return entry.payload.type === Type.T_Log
}

export function isSyscallEntry(entry: Entry): entry is {
    payload: Payload & { type: Type['T_Syscall'] }
    content: SyscallContent
} {
    return entry.payload.type === Type.T_Syscall
}
