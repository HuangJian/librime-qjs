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
var TestFilter = class {
  constructor(env) {
    console.log('filter_test init')
    assert(env.namespace === 'filter_test')
    assert(env.userDataDir.endsWith('qjs/tests/'))
    console.log(`env = ${env}`)
    console.log(`env.engine.schema = ${env.engine.schema}`)
    console.log(`env.engine.schema.config = ${env.engine.schema.config}`)
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
  }
  finalizer() {
    console.log('filter_test finit')
  }
  filter(candidates, env) {
    console.log('filter_test filter', candidates.length)
    assert(env.namespace === 'filter_test')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
    const expectingText = config.getString('expectingText')
    assert(expectingText === 'text2')
    return candidates.filter((it) => it.text === expectingText)
  }
}
export { TestFilter }
