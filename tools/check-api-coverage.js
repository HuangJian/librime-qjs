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

  // Extract the balanced content inside the outermost parens starting at `startIdx`
  function extractBalancedParens(str, startIdx) {
    let depth = 0
    let started = false
    let result = ''
    for (let i = startIdx; i < str.length; i++) {
      if (str[i] === '(') {
        depth++
        started = true
      } else if (str[i] === ')') {
        depth--
      }
      if (started) {
        result += str[i]
        if (depth === 0) break
      }
    }
    return result.slice(1, -1) // strip outer parens
  }

  // Split a string by top-level commas (respecting nested parens)
  function splitTopLevelArgs(content) {
    if (!content.trim()) return []
    const args = []
    let current = ''
    let depth = 0
    for (const ch of content) {
      if (ch === '(') depth++
      if (ch === ')') depth--
      if (ch === ',' && depth === 0) {
        args.push(current.trim())
        current = ''
      } else {
        current += ch
      }
    }
    if (current.trim()) args.push(current.trim())
    return args
  }

  // Extract JS-visible names from a section's args string.
  // Handles: bare words, tuples (jsName, ...), and nested macros like JS_API_AUTO_PROPERTIES(...).
  function extractNames(argsStr) {
    const rawArgs = splitTopLevelArgs(argsStr)
    const results = []
    for (const arg of rawArgs) {
      // Nested macro: JS_API_AUTO_PROPERTIES(...) or JS_API_CUSTOM_PROPERTIES(...)
      const macroMatch = arg.match(/^JS_API_(?:AUTO|CUSTOM)_PROPERTIES\s*\((.*)\)$/s)
      if (macroMatch) {
        results.push(...extractNames(macroMatch[1]))
        continue
      }
      // Tuple: (jsName, ...) — first identifier is the JS name
      const tupleMatch = arg.match(/^\s*\(\s*([a-zA-Z_]\w*)\s*,[\s\S]*?\)\s*$/s)
      if (tupleMatch) {
        results.push(tupleMatch[1])
        continue
      }
      // Bare identifier
      const wordMatch = arg.match(/^\s*([a-zA-Z_]\w*)\s*$/)
      if (wordMatch) {
        results.push(wordMatch[1])
      }
    }
    return results
  }

  // Extract the args of a named section (e.g. JS_API_WITH_GETTERS) from classBody
  function extractSection(classBody, sectionName) {
    const idx = classBody.indexOf(sectionName)
    if (idx === -1) return []
    return extractNames(extractBalancedParens(classBody, idx + sectionName.length))
  }

  readdirSync(TYPES_DIR)
    .filter((f) => f.startsWith('qjs_') && f.endsWith('.h'))
    .forEach((f) => {
      const content = readFileSync(join(TYPES_DIR, f), 'utf-8')
        .replace(/\/\/.*$/gm, '') // strip C++ line comments

      // Match class definitions: JS_API_EXPORT_CLASS_WITH_RAW_POINTER(...) or _SHARED_POINTER(...)
      const classDefRegex = /JS_API_EXPORT_CLASS_WITH_(?:RAW_POINTER|SHARED_POINTER)\s*\(/g
      let match

      while ((match = classDefRegex.exec(content)) !== null) {
        const classBody = extractBalancedParens(content, match.index + match[0].length - 1)
        const className = splitTopLevelArgs(classBody)[0]?.trim()
        if (!className) continue

        if (!exports.has(className)) {
          exports.set(className, { props: new Set(), getters: new Set(), methods: new Set() })
        }
        const classExports = exports.get(className)

        // Parse properties
        extractSection(classBody, 'JS_API_WITH_PROPERTIES').forEach((p) => classExports.props.add(p))

        // Parse getters
        extractSection(classBody, 'JS_API_WITH_GETTERS').forEach((g) => classExports.getters.add(g))

        // Parse functions
        extractSection(classBody, 'JS_API_WITH_FUNCTIONS').forEach((fn) => classExports.methods.add(fn))

        // Parse constructor: JS_API_WITH_CONSTRUCTOR(funcName) has a non-empty arg
        const ctorIdx = classBody.indexOf('JS_API_WITH_CONSTRUCTOR')
        if (ctorIdx !== -1) {
          const ctorArgs = extractBalancedParens(classBody, ctorIdx + 'JS_API_WITH_CONSTRUCTOR'.length)
          if (ctorArgs.trim()) {
            classExports.methods.add('constructor')
          }
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
