import { assert } from './testutils.js'

export class TestFilter {
  constructor(env) {
    console.log('filter_test init')
    // No assertions here
  }
  finalizer() {
    console.log('filter_test finit')
  }
  filter(candidates, env) {
    console.log('filter_test filter', candidates.length)
    return candidates.filter((it) => it.text === 'text2')
  }
}
