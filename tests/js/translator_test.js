export class TestTranslator {
  constructor(env) {
    console.log('translator_test init')
    assert(env.namespace === 'translator_test')
    assert(env.userDataDir === 'tests/js')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
  }

  finalizer(env) {
    console.log('translator_test finit')
    assert(env.namespace === 'translator_test')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')
  }

  translate(input, segment, env) {
    console.log('translator_test translate', input)
    assert(env.namespace === 'translator_test')
    const config = env.engine.schema.config
    assert(config.getString('greet') === 'hello from c++')

    // Check if the input matches the expected input from the test
    const expectedInput = config.getString('expectedInput')
    assert(expectedInput === input)

    // Return candidates based on the input
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

function assert(condition, msg) {
  if (!condition) {
    throw new Error('assertion failed: ' + msg)
  }
}
