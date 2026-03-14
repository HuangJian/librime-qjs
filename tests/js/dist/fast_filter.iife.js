;(() => {
  var FastFilterWithGenerator = class {
    *filter(iter, env) {
      for (let idx = 0, candidate; (candidate = iter.next()); ++idx) {
        console.log(`checking candidate at index = ${idx}`)
        if (idx % 2 === 0) {
          yield candidate
        }
      }
    }
  }
  globalThis.iife_instance_fast_filter_iife_js = new FastFilterWithGenerator()
})()
