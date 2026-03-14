;(() => {
  var accents =
    '\u0101\xE1\u01CE\xE0\u0113\xE9\u011B\xE8\u012B\xED\u01D0\xEC\u014D\xF3\u01D2\xF2\u016B\xFA\u01D4\xF9\u01D6\u01D8\u01DA\u01DC\xFC'
  var without = 'aaaaeeeeiiiioooouuuuvvvvv'
  var dict = {}
  accents.split('').forEach((char, idx) => (dict[char] = without[idx]))
  function unaccent(str) {
    return str
      .split('')
      .map((char) => {
        return dict[char] || char
      })
      .join('')
  }
  var SortCandidatesByPinyinFilter = class {
    constructor(env) {
      console.log('sort_by_pinyin.js init')
    }
    finalizer() {
      console.log('sort_by_pinyin.js finit')
    }
    #topN = 100
    filter(candidates, env) {
      const userPhrases = []
      const userPhrasesIndices = []
      const candidatesWithPinyin = []
      const candidatesWithPinyinIndices = []
      const input = env.engine.context.input.replace(/\/.*$/, '')
      const size = candidates.length > this.#topN ? this.#topN : candidates.length
      candidates.slice(0, size).forEach((candidate, idx) => {
        const pinyin = this.extractPinyin(candidate.comment)?.replaceAll(' ', '')
        if (candidate.type === 'user_phrase') {
          const weight = this.getWeightByPinyin(pinyin, input, true) + size - idx
          userPhrasesIndices.push(idx)
          userPhrases.push({ candidate, weight })
        } else if (pinyin) {
          const weight = this.getWeightByPinyin(pinyin, input, false) + size - idx
          candidatesWithPinyinIndices.push(idx)
          candidatesWithPinyin.push({ candidate, weight })
        }
      })
      userPhrases.sort((a, b) => b.weight - a.weight)
      userPhrasesIndices.forEach((originalIndex, idx) => {
        candidates[originalIndex] = userPhrases[idx].candidate
      })
      candidatesWithPinyin.sort((a, b) => b.weight - a.weight)
      candidatesWithPinyinIndices.forEach((originalIndex, idx) => {
        candidates[originalIndex] = candidatesWithPinyin[idx].candidate
      })
      return candidates
    }
    getWeightByPinyin(pinyin, input, isInUserPhrase) {
      if (pinyin === input) {
        return 1e4
      }
      if (isInUserPhrase && !pinyin) {
        return 1e4
      }
      if (pinyin?.startsWith(input)) {
        return 5e3
      }
      if (pinyin?.includes(input)) {
        return 1e3 + pinyin.length
      }
      return 0
    }
    extractPinyin(comment) {
      const match = comment.match(/〖(.+?)〗/)
      if (match) {
        return unaccent(match[1])
      }
      const match2 = comment.match(/［(.*?)］/) || []
      return match2[1]
    }
  }
  globalThis.iife_instance_sort_by_pinyin_iife_js = new SortCandidatesByPinyinFilter()
})()
