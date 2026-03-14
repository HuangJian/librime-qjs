function greet(name) {
  return `Hello ${name}!`
}
var MyClass = class {
  constructor(value) {
    this.value = value
  }
  myMethod() {
    return this.value + 1
  }
  greet(name) {
    return greet(name)
  }
}
globalThis.funcWithRuntimeError = function () {
  const obj = new MyClass(abcdefg)
  obj.hi()
}
funcWithRuntimeError()
