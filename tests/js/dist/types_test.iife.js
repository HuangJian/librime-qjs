;(() => {
  var totalTests = 0
  var passedTests = 0
  function assert(condition, message = '') {
    totalTests++
    if (condition) {
      passedTests++
      console.log('\u2713 ' + message)
    } else {
      console.log('\u2717 ' + message)
      console.log('  Expected true, but got false')
      throw new Error('Assertion failed' + (message ? ': ' + message : ''))
    }
  }
  function assertEquals(actual, expected, message = '') {
    totalTests++
    const actualStr = JSON.stringify(actual)
    const expectedStr = JSON.stringify(expected)
    if (actualStr === expectedStr) {
      passedTests++
      console.log('\u2713 ' + message)
    } else {
      console.log('\u2717 ' + message)
      console.log('  Expected: ' + expectedStr)
      console.log('  Actual:   ' + actualStr)
      throw new Error('Assertion failed' + (message ? ': ' + message : ''))
    }
  }
  function checkArgument(env) {
    assertEquals(env.namespace, 'namespace')
    assertEquals(env.candidate.text, 'text')
    assertEquals(env.engine.schema.id, '.default')
    env.candidate.text = 'new text'
    const config = env.engine.schema.config
    assertEquals(config.getBool('key'), null)
    assertEquals(config.getBool('key1'), true)
    assertEquals(config.getBool('key2'), false)
    assertEquals(config.getInt('key3'), 666)
    assertEquals(config.getDouble('key4'), 0.999)
    assertEquals(config.getString('key5'), 'string')
    const list = config.getList('list')
    assertEquals(list.getValueAt(0).getString(), 'item1')
    assertEquals(list.getValueAt(1).getString(), 'item2')
    assertEquals(list.getValueAt(2).getString(), 'item3')
    assertEquals(list.getValueAt(3), null)
    assert(!config.getList('none'), 'should not crash if the key does not exist')
    config.setString('greet', 'hello from js')
    const context = env.engine.context
    assertEquals(context.input, 'hello')
    assert(context.preedit !== null, 'preedit should not be null')
    assertEquals(context.preedit.text, 'hello', 'preedit should have text')
    assertEquals(context.preedit.caretPos, 5, 'preedit should have caretPos')
    assertEquals(context.preedit.selectStart, 0, 'preedit should have selectStart')
    assertEquals(context.preedit.selectEnd, 0, 'preedit should have selectEnd')
    context.input = 'world'
    env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)
    env.newCandidate.extraField = 'extra field'
    assertEquals(env.newCandidate.extraField, 'extra field')
    testEnvUtilities(env)
    testTrie(env)
    testLevelDb(env)
    return env
  }
  function testEnvUtilities(env) {
    const info = env.getRimeInfo()
    console.log(`Rime info = ${info}`)
    assert(info.includes('libRime v'))
    assert(info.includes('libRime-qjs v'))
    assert(info.includes('Process RSS Mem: '))
    assert('macOS|Windows|Linux'.includes(env.os.name), 'os.name should be one of macOS|Windows|Linux')
    assert(env.os.version.length > 0, 'os.version should not be empty')
    assert(env.os.architecture.length > 0, 'os.architecture should not be empty')
    console.error(`This is an error message.`)
    env.engine.processKey('Down')
    env.engine.processKey('InvalidKey')
    assertEquals(env.popen(`echo libRime-qjs`).trim(), 'libRime-qjs')
    const ping = env.os.name === 'Windows' ? 'ping 127.0.0.1 -n 10' : 'ping 127.0.0.1 -t 10'
    try {
      const output = env.popen(ping, 500)
      assert(false, `env.popen('ping xxx') should be timed-out, but got ${output}`)
    } catch (e) {
      console.error('expected timed-out error', e)
    }
    assertEquals(env.fileExists(env.currentFolder + '/js/types_test.js'), true)
    assertEquals(env.fileExists(env.currentFolder + '/js/not_found.js'), false)
    const content = env.loadFile(env.currentFolder + '/js/types_test.js')
    assert(content.includes('\u6D4B\u8BD5 UTF-8 \u7F16\u7801'))
  }
  function testTrie(env) {
    const trie = new Trie()
    trie.loadTextFile(env.currentFolder + '/dummy_dict.txt', { lines: 6 })
    checkTrieData(trie)
    trie.saveToBinaryFile(env.currentFolder + '/dumm.bin')
    const trie2 = new Trie()
    trie2.loadBinaryFile(env.currentFolder + '/dumm.bin')
    checkTrieData(trie2)
  }
  function testLevelDb(env) {
    console.log('testLevelDb')
    const levelDb = new LevelDb()
    levelDb.loadTextFile(env.currentFolder + '/dummy_dict.txt', { lines: 6 })
    levelDb.saveToBinaryFile(env.currentFolder + '/dumm.ldb')
    checkTrieData(levelDb)
    levelDb.close()
    const levelDb2 = new LevelDb()
    levelDb2.loadBinaryFile(env.currentFolder + '/dumm.ldb')
    checkTrieData(levelDb2)
  }
  function checkTrieData(trie) {
    const result1 = trie.find('accord')
    assertEquals(
      result1,
      "[\u04D9'k\u0252:d]; n. \u4E00\u81F4, \u8C03\u548C, \u534F\u5B9A\\n vt. \u7ED9\u4E0E, \u4F7F\u4E00\u81F4\\n vi. \u76F8\u7B26\u5408",
    )
    const result2 = trie.find('accordion')
    assertEquals(result2, "[\u04D9'k\u0252:dj\u04D9n]; n. \u624B\u98CE\u7434\\n a. \u53EF\u6298\u53E0\u7684")
    const result3 = trie.find('nonexistent-word')
    assertEquals(result3, null)
    const prefix_results = trie.prefixSearch('accord')
    assertEquals(prefix_results.length, 6)
  }
  globalThis.checkArgument = checkArgument
  var DummyClass = class {}
  globalThis.iife_instance_types_test_iife_js = new DummyClass()
})()
