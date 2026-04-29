export type ArgType = 'int' | 'uint' | 'pid' | 'fd' | 'ptr' | 'size' | 'str' | 'flags' | 'signo'

export type ArgDecl = { name: string; type: ArgType }

export type SyscallDef = {
    name: string
    ret: ArgType
    args: ArgDecl[]
    noret?: true
}

export const syscallTable: Record<number, SyscallDef> = {
    1: {
        name: 'exit',
        ret: 'int',
        args: [{ name: 'code', type: 'int' }],
        noret: true
    },
    2: {
        name: 'fork',
        ret: 'pid',
        args: []
    },
    3: {
        name: 'read',
        ret: 'int',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'buf', type: 'ptr' },
            { name: 'size', type: 'size' }
        ]
    },
    4: {
        name: 'write',
        ret: 'int',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'buf', type: 'ptr' },
            { name: 'size', type: 'size' }
        ]
    },
    6: {
        name: 'close',
        ret: 'int',
        args: [{ name: 'fd', type: 'fd' }]
    },
    7: {
        name: 'waitpid',
        ret: 'pid',
        args: [
            { name: 'pid', type: 'pid' },
            { name: 'stat_loc', type: 'ptr' },
            { name: 'options', type: 'flags' }
        ]
    },
    11: {
        name: 'execve',
        ret: 'int',
        args: [
            { name: 'pathname', type: 'str' },
            { name: 'argv', type: 'ptr' },
            { name: 'envp', type: 'ptr' }
        ]
    },
    20: { name: 'getpid', ret: 'pid', args: [] },
    37: {
        name: 'kill',
        ret: 'int',
        args: [
            { name: 'pid', type: 'pid' },
            { name: 'sig', type: 'signo' }
        ]
    },
    41: { name: 'dup', ret: 'fd', args: [{ name: 'fd', type: 'fd' }] },
    42: { name: 'pipe', ret: 'int', args: [{ name: 'fds', type: 'ptr' }] },
    45: { name: 'brk', ret: 'ptr', args: [{ name: 'addr', type: 'ptr' }] },
    48: {
        name: 'signal',
        ret: 'ptr',
        args: [
            { name: 'sig', type: 'signo' },
            { name: 'handler', type: 'ptr' }
        ]
    },
    54: {
        name: 'ioctl',
        ret: 'int',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'request', type: 'uint' },
            { name: 'param', type: 'uint' }
        ]
    },
    63: {
        name: 'dup2',
        ret: 'fd',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'newFd', type: 'fd' }
        ]
    },
    91: {
        name: 'munmap',
        ret: 'int',
        args: [
            { name: 'addr', type: 'ptr' },
            { name: 'length', type: 'size' }
        ]
    },
    114: {
        name: 'wait4',
        ret: 'pid',
        args: [
            { name: 'pid', type: 'pid' },
            { name: 'stat_loc', type: 'ptr' },
            { name: 'options', type: 'flags' },
            { name: 'ru', type: 'ptr' }
        ]
    },
    119: { name: 'sigreturn', ret: 'int', args: [] },
    145: {
        name: 'readv',
        ret: 'int',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'iov', type: 'ptr' },
            { name: 'iovcnt', type: 'size' }
        ]
    },
    146: {
        name: 'writev',
        ret: 'int',
        args: [
            { name: 'fd', type: 'fd' },
            { name: 'iov', type: 'ptr' },
            { name: 'iovcnt', type: 'size' }
        ]
    },
    162: {
        name: 'nanosleep',
        ret: 'int',
        args: [
            { name: 'rqtp', type: 'ptr' },
            { name: 'rmtp', type: 'ptr' }
        ]
    },
    174: {
        name: 'rt_sigaction',
        ret: 'int',
        args: [
            { name: 'sig', type: 'signo' },
            { name: 'act', type: 'ptr' },
            { name: 'oldact', type: 'ptr' },
            { name: 'sigsetsize', type: 'size' }
        ]
    },
    175: {
        name: 'rt_sigprocmask',
        ret: 'int',
        args: [
            { name: 'how', type: 'int' },
            { name: 'set', type: 'ptr' },
            { name: 'oldset', type: 'ptr' },
            { name: 'sigsetsize', type: 'size' }
        ]
    },
    192: {
        name: 'mmap2',
        ret: 'ptr',
        args: [
            { name: 'addr', type: 'ptr' },
            { name: 'length', type: 'size' },
            { name: 'prot', type: 'flags' },
            { name: 'flags', type: 'flags' },
            { name: 'fd', type: 'fd' },
            { name: 'pgoffset', type: 'uint' }
        ]
    },
    224: { name: 'gettid', ret: 'pid', args: [] },
    238: {
        name: 'tkill',
        ret: 'int',
        args: [
            { name: 'tid', type: 'pid' },
            { name: 'sig', type: 'signo' }
        ]
    },
    243: {
        name: 'set_thread_area',
        ret: 'int',
        args: [{ name: 'user_desc', type: 'ptr' }]
    },
    252: {
        name: 'exit_group',
        ret: 'int',
        args: [{ name: 'code', type: 'int' }],
        noret: true
    },
    258: {
        name: 'set_tid_address',
        ret: 'int',
        args: [{ name: 'ptr', type: 'ptr' }]
    },
    512: {
        name: 'spawn',
        ret: 'pid',
        args: [
            { name: 'name', type: 'str' },
            { name: 'argv', type: 'ptr' }
        ]
    }
}
