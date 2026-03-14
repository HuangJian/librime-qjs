;(() => {
  var BadTranslator = class {
    translate() {
      console.log('no return')
    }
  }
  globalThis.iife_instance_translator_no_return_iife_js = new BadTranslator()
})()
