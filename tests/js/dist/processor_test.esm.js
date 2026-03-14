var totalTests = 0
var passedTests = 0
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
var TestProcessor = class {
  constructor(env) {
    console.log('[processor_test] init')
    const config = env.engine.schema.config
    const initTestValue = config.getString('init_test')
    if (initTestValue) {
      console.log(`[processor_test] init_test value: ${initTestValue}`)
    }
  }
  finalizer() {
    console.log('[processor_test] finit')
  }
  process(keyEvent, env) {
    assertEquals(env.engine.context.lastSegment?.prompt, 'prompt', 'should have lastSegment with prompt')
    console.log(`[processor_test] process: ${keyEvent.repr}`)
    const repr = keyEvent.repr
    if (repr === 'space') {
      return 'kAccepted'
    } else if (repr === 'Return') {
      return 'kRejected'
    }
    return 'kNoop'
  }
}
export { TestProcessor }
