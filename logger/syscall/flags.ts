function parse_trivial_bit_flags(flags: number, map: Record<number, string>) {
    const res: string[] = []
    for (const [val, name] of Object.entries(map)) {
        if (flags & parseInt(val)) {
            res.push(name)
        }
    }
    return res
}

export function parse_open_flags(flags: number) {
    const res: string[] = []
    res.push(
        {
            0: 'O_RDONLY',
            1: 'O_WRONLY',
            2: 'O_RDWR'
        }[flags & 3] ?? ''
    )
    res.push(
        ...parse_trivial_bit_flags(flags, {
            0o100: 'O_CREAT',
            0o200: 'O_EXCL',
            0o400: 'O_NOCTTY',
            0o1000: 'O_TRUNC',
            0o2000: 'O_APPEND',
            0o4000: 'O_NONBLOCK',
            0o10000: 'O_DSYNC',
            0o20000: 'O_DIRECTORY',
            0o40000: 'O_DIRECT',
            0o100000: 'O_LARGEFILE',
            0o400000: 'O_NOFOLLOW',
            0o1000000: 'O_NOATIME',
            0o2000000: 'O_CLOEXEC',
            0o4000000: 'O_SYNC',
            0o10000000: 'O_PATH',
            0o20200000: 'O_TMPFILE'
        })
    )
    return res.join('|')
}

export function parse_ioctl_cmd(cmd: number) {
    return (
        {
            0x5401: 'TCGETS',
            0x540f: 'TIOCGPGRP',
            0x5410: 'TIOCSPGRP',
            0x5413: 'TIOCGWINSZ'
        }[cmd] ?? ''
    )
}

export function parse_fcntl64_cmd(cmd: number) {
    return (
        {
            0: 'F_DUPFD',
            1: 'F_GETFD',
            2: 'F_SETFD',
            1030: 'F_DUPFD_CLOEXEC'
        }[cmd] ?? ''
    )
}

export function parse_mmap_pgoff_prot(prot: number) {
    const res: string[] = []
    if ((prot & 7) === 0) {
        res.push('PROT_NONE')
    }
    res.push(
        ...parse_trivial_bit_flags(prot, {
            1: 'PROT_READ',
            2: 'PROT_WRITE',
            4: 'PROT_EXEC',
            0x01000000: 'PROT_GROWSDOWN',
            0x02000000: 'PROT_GROWSUP'
        })
    )
    return res.join('|')
}

export function parse_mmap_pgoff_flags(flags: number) {
    const res: string[] = []
    res.push(
        {
            0: 'MAP_FILE',
            1: 'MAP_SHARED',
            2: 'MAP_PRIVATE',
            3: 'MAP_SHARED_VALIDATE'
        }[flags & 0xf] ?? ''
    )
    res.push(
        ...parse_trivial_bit_flags(flags, {
            0x10: 'MAP_FIXED',
            0x20: 'MAP_ANON',
            0x100: 'MAP_GROWSDOWN',
            0x800: 'MAP_DENYWRITE',
            0x1000: 'MAP_EXECUTABLE',
            0x2000: 'MAP_LOCKED',
            0x4000: 'MAP_NORESERVE',
            0x8000: 'MAP_POPULATE',
            0x10000: 'MAP_NONBLOCK',
            0x20000: 'MAP_STACK',
            0x40000: 'MAP_HUGETLB',
            0x80000: 'MAP_SYNC',
            0x100000: 'MAP_FIXED_NOREPLACE'
        })
    )
    return res.join('|')
}

export const all_parsers = {
    parse_open_flags,
    parse_ioctl_cmd,
    parse_fcntl64_cmd,
    parse_mmap_pgoff_prot,
    parse_mmap_pgoff_flags
} as Partial<Record<string, (val: number) => string>>
