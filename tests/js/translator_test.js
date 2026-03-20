import { assert, assertEquals } from './testutils.js'

export class TestTranslator {
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

    // Check if the input matches the expected input from the test
    const expectedInput = config.getString('expectedInput')
    assertEquals(expectedInput, input)

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
