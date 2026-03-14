;(() => {
  var FilterWithIsApplicable = class {
    isApplicable(env) {
      console.log('filter_test isApplicable')
      return false
    }
    filter(candidates, env) {
      throw new Error('should not be called as isApplicable returns false')
    }
  }
  globalThis.iife_instance_filter_is_applicable_iife_js = new FilterWithIsApplicable()
})()
