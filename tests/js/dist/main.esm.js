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
globalThis.MyClass = MyClass
var obj = new MyClass(10)
console.log(obj.greet('QuickJS'))
console.log(obj.myMethod())
console.log(obj.greet?.name.includes('greet'))
console.log(obj.hello?.name.includes('greet'))
