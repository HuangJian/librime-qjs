// usage: `node ./check-api-coverage.js`

import { readdirSync, readFileSync } from 'node:fs'
import { join, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = dirname(fileURLToPath(import.meta.url))
const TYPES_DIR = join(__dirname, '../src/types')
const DTS_FILE = join(__dirname, '../contrib/rime.d.ts')

// 1. Parse C++ files for exposed APIs
function parseCppExports() {
  const exports = new Map()

  readdirSync(TYPES_DIR)
    .filter((f) => f.startsWith('qjs_') && f.endsWith('.h'))
    .forEach((f) => {
      const content = readFileSync(join(TYPES_DIR, f), 'utf-8')

      // Match class definitions
      const classDefRegex = /EXPORT_CLASS_WITH_(?:RAW_POINTER|SHARED_POINTER)\s*\(\s*\n*(\w+),(\n.+)+\)/gs
      let classMatch

      while ((classMatch = classDefRegex.exec(content)) !== null) {
        const [_, className, classBody] = classMatch

        if (!exports.has(className)) {
          exports.set(className, { props: new Set(), getters: new Set(), methods: new Set() })
        }
        const classExports = exports.get(className)

        // Parse properties
        const propsMatch = classBody.match(/WITH_PROPERTIES\s*\((.+?)\)/s)
        if (propsMatch) {
          const props = propsMatch[1].split(',').map((p) => p.trim())
          props.forEach((p) => classExports.props.add(p))
        }

        // Parse getters
        const gettersMatch = classBody.match(/WITH_GETTERS\s*\((.+?)\)/s)
        if (gettersMatch) {
          const getters = gettersMatch[1].split(',').map((g) => g.trim())
          getters.forEach((g) => classExports.getters.add(g))
        }

        // Parse functions
        const functionsMatch = classBody.match(/WITH_FUNCTIONS\s*\((.+?)\)/s)
        if (functionsMatch) {
          const functions = functionsMatch[1]
            .split(',')
            .filter((_, idx) => idx % 2 === 0)
            .map((f) => f.trim())

          functions.forEach((f) => classExports.methods.add(f))
        }

        // Parse constructor
        if (classBody.includes('WITH_CONSTRUCTOR')) {
          classExports.methods.add('constructor')
        }
      }
    })

  return exports
}

// 2. Parse TypeScript declarations
function parseDtsDeclarations() {
  const dtsContent = readFileSync(DTS_FILE, 'utf-8')
  const declarations = new Map()

  const blocks = dtsContent.split(/(?=interface|declare)/g)
  blocks.forEach((block) => {
    const interfaceMatch = block?.replace(/extends \w+/g, '').match(/\s*(\w+)\s*{(.+)}/s)
    if (interfaceMatch) {
      const [_, name, content] = interfaceMatch

      const props = new Set()
      const getters = new Set()
      const methods = new Set()

      content.split('\n').forEach((line) => {
        const propMatch = line.match(/^\s*(\w+)\??:/)
        if (propMatch && !line.includes('readonly')) props.add(propMatch[1])

        const getterMatch = line.match(/^\s+readonly\s(\w+)\??:/)
        if (getterMatch) getters.add(getterMatch[1])

        const methodMatch = line.match(/^\s*(new|\*?\w+)(\s*|\?)\(/)
        if (methodMatch) methods.add(methodMatch[1].replace('new', 'constructor'))
      })
      declarations.set(name, { props, getters, methods })
    }
  })

  return declarations
}

// 3. Compare and report discrepancies
function compareExports(declaration, otherDeclaration, errorType) {
  let errorCount = 0

  for (const [className, { props, getters, methods }] of declaration) {
    const otherExports = otherDeclaration.get(className) || {
      props: new Set(),
      getters: new Set(),
      methods: new Set(),
    }

    // Check missing properties
    ;[...props]
      .filter((p) => !otherExports.props.has(p))
      .forEach((p) => {
        console.error(`${errorType} property in ${className}: ${p}`)
        errorCount++
      })

    // Check missing getters
    ;[...getters]
      .filter((p) => !otherExports.getters.has(p))
      .forEach((p) => {
        console.error(`${errorType} getter in ${className}: ${p}`)
        errorCount++
      })

    // Check missing methods
    ;[...methods]
      .filter((m) => !otherExports.methods.has(m))
      .forEach((m) => {
        console.error(`${errorType} method in ${className}: ${m}`)
        errorCount++
      })
  }
  return errorCount
}

// Main execution
const cppExports = parseCppExports()

const interfacesOutsideTypes = [
  {
    name: 'Module',
    methods: ['constructor', 'finalizer'],
  },
  {
    name: 'Processor',
    methods: ['process'],
  },
  {
    name: 'Translator',
    methods: ['translate'],
  },
  {
    name: 'Filter',
    methods: ['filter', 'isApplicable'],
  },
  {
    name: 'FastFilter',
    methods: ['*filter', 'isApplicable'],
  },
  {
    name: 'ParseTextFileOptions',
    props: ['delimiter', 'comment', 'lines', 'isReversed', 'charsToRemove', 'onDuplicatedKey', 'concatSeparator'],
  },
]

interfacesOutsideTypes.forEach((i) =>
  cppExports.set(i.name, {
    props: new Set(i.props),
    getters: new Set(i.getters),
    methods: new Set(i.methods),
  }),
)

const dtsDeclarations = parseDtsDeclarations()
const errors =
  compareExports(cppExports, dtsDeclarations, 'rime.d.ts missed') +
  compareExports(dtsDeclarations, cppExports, 'rime.d.ts extra')

console.log('Validated classes: ', [...cppExports.keys()].join(', '))
console.log(`\nValidation complete. Found ${errors} discrepancies.`)
process.exit(errors > 0 ? 1 : 0)
