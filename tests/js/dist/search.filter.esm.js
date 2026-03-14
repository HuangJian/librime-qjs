var TrieNode = class {
  constructor() {
    this.children = new Map()
    this.isEndOfWord = false
    this.data = []
  }
}
var Trie = class {
  constructor(multipleData = false) {
    this.root = new TrieNode()
    this.multipleData = multipleData
  }
  insert(word, data) {
    let current = this.root
    for (const char of word) {
      if (!current.children.has(char)) {
        current.children.set(char, new TrieNode())
      }
      current = current.children.get(char)
    }
    current.isEndOfWord = true
    if (!current.data.includes(data)) {
      current.data.push(data)
    }
  }
  parseLine(line) {
    const idx = line.indexOf('	')
    if (idx < 1) return null
    return { text: line.substring(0, idx).trim().toLowerCase(), info: line.substring(idx + 1).trim() }
  }
  parseLineRegex(line) {
    const matches = line.match(/^(.*?)\s+(.+)\s*$/)
    if (!matches) return null
    const [, text, info] = matches
    return { text: text.trim().toLowerCase(), info: info.trim() }
  }
  find(word) {
    const node = this._traverse(word)
    const arr = node?.data || []
    return this.multipleData ? arr : arr[0]
  }
  startsWith(prefix) {
    return this._traverse(prefix) !== null
  }
  prefixSearch(prefix) {
    const result = []
    const node = this._traverse(prefix)
    if (node !== null) {
      this._collectWords(node, prefix, result)
    }
    return result
  }
  _traverse(word) {
    let current = this.root
    for (const char of word) {
      if (!current.children.has(char)) {
        return null
      }
      current = current.children.get(char)
    }
    return current
  }
  _collectWords(node, prefix, result) {
    if (node.isEndOfWord) {
      result.push({ text: prefix, info: this.multipleData ? node.data : node.data[0] })
    }
    for (const [char, childNode] of node.children) {
      this._collectWords(childNode, prefix + char, result)
    }
  }
}
var SearchFilter = class {
  dict = new Trie(true)
  selectListeners = []
  constructor(env) {
    console.log('search.filter.js init')
    const dictYamlPath = `${env.userDataDir}/radical_pinyin.dict.yaml`
    let entries = 0
    env
      .loadFile(dictYamlPath)
      .split('\n')
      .filter((it) => !it.startsWith('#'))
      .forEach((line) => {
        const pos = line.indexOf('	')
        if (pos > 0) {
          const char = line.substring(0, pos)
          const code = line.substring(pos + 1).replaceAll("'", '')
          this.dict.insert(code, char)
          ++entries
        }
      })
    console.log(`loaded ${dictYamlPath} with ${entries} entries`)
  }
  finalizer() {
    console.log('search.filter.js finit')
    this.selectListeners.forEach((it) => it.connection.disconnect())
    this.selectListeners = []
  }
  filter(candidates, env) {
    const input = env.engine.context.input
    const pos = input.indexOf(CONDUCTOR_CODE)
    if (pos < 1) return candidates
    this.clearDisconnectedListeners()
    this.connectListenerToRimeContextIfNotYet(env.engine.context, env.id)
    const key = input.substring(pos + 1)
    const entries = (this.dict.prefixSearch(key) || []).flatMap((it) => it.info)
    const matchedCandidates = []
    const others = []
    candidates.forEach((candidate) => {
      if (entries.includes(candidate.text)) {
        matchedCandidates.push(candidate)
      } else {
        others.push(candidate)
      }
    })
    return matchedCandidates.concat(others)
  }
  connectListenerToRimeContextIfNotYet(context, envId) {
    if (!this.selectListeners.some((it) => it.envId === envId)) {
      console.log('connecting listener to rime context', envId)
      const connection = context.selectNotifier.connect(onRimeSelectCallback)
      this.selectListeners.push({ envId, connection })
    }
  }
  clearDisconnectedListeners() {
    for (let i = this.selectListeners.length - 1; i >= 0; --i) {
      if (!this.selectListeners[i].connection.isConnected) {
        this.selectListeners[i].connection.disconnect()
        this.selectListeners.splice(i, 1)
      }
    }
  }
}
var CONDUCTOR_CODE = '`'
function onRimeSelectCallback(context) {
  const input = context.input
  const pos = input.indexOf(CONDUCTOR_CODE)
  if (pos < 1) return
  const inputCode = input.substring(0, pos)
  const preedit = context.preedit.text
  let unselectedCode = preedit.replace(/[\u4e00-\u9fff]/g, '')
  unselectedCode = unselectedCode.substring(0, unselectedCode.indexOf(CONDUCTOR_CODE))
  if (unselectedCode.length > 0) {
    context.input = inputCode + CONDUCTOR_CODE
  } else {
    context.input = inputCode
    context.commit()
  }
}
export { SearchFilter }
