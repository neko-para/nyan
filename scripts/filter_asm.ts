import fs from 'fs'

const items = JSON.parse(fs.readFileSync(0, 'utf8')) as {
    file: string
}[]

console.log(
    JSON.stringify(
        items.filter(item => !item.file.endsWith('.s')),
        null,
        4
    )
)
