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
  var TestTranslator = class {
    constructor(env) {
      console.log('translator_test init')
      assertEquals(env.namespace, 'translator_test')
      assert(env.userDataDir.endsWith('qjs/tests/'))
      assert(env.sharedDataDir.endsWith('qjs/tests/'))
      const config = env.engine.schema.config
      assertEquals(config.getString('greet'), 'hello from c++')
      console.log('translator_test init done')
    }
    finalizer() {
      console.log('translator_test finit')
    }
    translate(input, segment, env) {
      console.log('translator_test translate', input)
      assertEquals(env.namespace, 'translator_test')
      const config = env.engine.schema.config
      assertEquals(config.getString('greet'), 'hello from c++')
      const expectedInput = config.getString('expectedInput')
      assertEquals(expectedInput, input)
      if (input === 'test_input') {
        return [
          new Candidate('test', segment.start, segment.end, 'candidate1', 'comment1'),
          new Candidate('test', segment.start, segment.end, 'candidate2', 'comment2'),
          new Candidate('test', segment.start, segment.end, 'candidate3', 'comment3'),
        ]
      }
      return []
    }
  }
  globalThis.iife_instance_translator_test_iife_js = new TestTranslator()
})()
