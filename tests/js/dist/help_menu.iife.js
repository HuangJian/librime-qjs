;(() => {
  var menus = [
    ['\u5E2E\u52A9\u83DC\u5355', '\u2192 /help'],
    ['\u5FEB\u6377\u6307\u4EE4', '\u2192 /deploy /screenshot'],
    ['\u65B9\u6848\u9009\u5355', '\u2192 F4'],
    [
      '\u5FEB\u901F\u8BA1\u7B97',
      '\u2192 /calc \u6216 /js \u7EC4\u5408\u952E\uFF0C\u5982`/calcsin(pi/2)`\u5019\u9009`1.0`',
    ],
    [
      '\u62C6\u5B57\u53CD\u67E5',
      '\u2192 uU \u7EC4\u5408\u952E\uFF0C\u5982`uUguili`\u53CD\u67E5\u51FA`\u9B51\u3018ch\u012B\u3019`',
    ],
    [
      '\u6C49\u8BD1\u82F1\u4E0A\u5C4F',
      '\u2192 /e* \u7EC4\u5408\u952E\uFF0C\u5982`shuxue/en`\u4E0A\u5C4F`mathematics`',
    ],
    [
      '\u62FC\u97F3\u4E0A\u5C4F',
      '\u2192 /p* \u7EC4\u5408\u952E\uFF0C\u5982`pinyin/py1`\u4E0A\u5C4F`p\u012Bn y\u012Bn`',
    ],
    [
      '\u5FEB\u6377\u6309\u952E',
      "\u2192 \u4E8C\u4E09\u5019\u9009 ;' \xA7 \u4E0A\u4E0B\u7FFB\u9875 ,. \xA7 \u4EE5\u8BCD\u5B9A\u5B57 []",
    ],
    ['\u5355\u8BCD\u5927\u5199', '\u2192 AZ \u5927\u5199\u5B57\u6BCD\u89E6\u53D1'],
    ['\u65E5\u671F\u65F6\u95F4', '\u2192 rq | sj | xq | dt | ts | nl'],
    [
      '\u4E2D\u6587\u6570\u5B57',
      '\u2192 R\u5FEB\u6377\u952E\uFF0C\u5982`R666`\u5019\u9009`\u516D\u767E\u516D\u5341\u516D\u5143\u6574`',
    ],
  ]
  var HelpMenuTranslator = class {
    constructor(env) {
      console.log('help_menu.js init')
      if (env.os.name === 'macOS') {
        menus.splice(2, 0, ['\u8BCD\u5178\u6E05\u9664', '\u2192 Fn + \u21E7 + \u232B \u7EC4\u5408\u952E'])
      } else {
        const idx = menus.findIndex(([text, comment]) => text === '\u5FEB\u6377\u6307\u4EE4')
        menus.splice(idx, 1)
      }
    }
    finalizer() {
      console.log('help_menu.js finit')
    }
    translate(input, segment, env) {
      if (input.length < 3 || !'/help'.startsWith(input)) return []
      segment.prompt = '\u3014\u5E2E\u52A9\u83DC\u5355\u3015'
      const ret = menus.map(
        ([text, comment]) => new Candidate('help', segment.start, segment.end, text, comment),
      )
      ret.unshift(
        new Candidate('help', segment.start, segment.end, '\u7248\u672C\u72B6\u6001', env.getRimeInfo()),
      )
      return ret
    }
  }
  globalThis.iife_instance_help_menu_iife_js = new HelpMenuTranslator()
})()
