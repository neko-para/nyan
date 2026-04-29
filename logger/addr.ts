import { $ } from 'execa'

const infoCache: Map<number, string> = new Map()

export async function queryAddr(binary: string, addr: number) {
    if (infoCache.has(addr)) {
        return infoCache.get(addr)!
    }
    const info = (
        await $({
            shell: true
        })`llvm-addr2line --obj ${binary} 0x${addr.toString(16)}`
    ).stdout.replace(process.cwd() + '/', '')
    infoCache.set(addr, info)
    return info
}
