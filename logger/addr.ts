import { $ } from 'execa'

const infoCache: Map<number, string> = new Map()

type LLVMSymbolizerOutput = {
    Symbol: [
        {
            FileName: string
            FunctionName: string
            Column: number
            Line: number
        }
    ]
}[]

export async function queryAddr(binary: string, addr: number) {
    if (infoCache.has(addr)) {
        return infoCache.get(addr)!
    }
    const info = JSON.parse(
        (
            await $({
                shell: true
            })`llvm-symbolizer --output-style=JSON --obj ${binary} 0x${addr.toString(16)}`
        ).stdout
    ) as LLVMSymbolizerOutput
    const symbol = info[0].Symbol[0]
    const final = `${symbol.FileName.replace(process.cwd() + '/', '')}:${symbol.Line}`
    infoCache.set(addr, final)
    return final
}
