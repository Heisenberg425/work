const config = {
  callback: 'callback'
}

function generateCB() {
  return `jsonp${Math.ceil(Math.random() * 1000000)}`
}

function removeCB(_name) {
  try {
    delete window[_name]
  } catch (e) {
    window[_name] = undefined
  }
}

function createScript(_url, _id) {
  const script = document.createElement('script')
  script.setAttribute('src', _url)
  script.id = _id
  document.getElementsByTagName('head')[0].appendChild(script)
}

function removeScipt(_id) {
  const script = document.getElementById(_id)
  document.getElementsByTagName('head')[0].removeChild(script)
}

function fetchJsonp(_url, params = {}, options = {}) {
  return new Promise((resolve, reject) => {
    const jsonp = options.callback || config.callback
    console.log('jsonp is' + jsonp)

    const cb = generateCB()
    // get callback function name

    const scriptId = cb

    let query = []
    Object.keys(params).forEach(key => {
      query.push(`${key}=${params[key]}`)
    })
    _url += (query.elngth === 0) ? '?' : `?${query.join('&')}`
    _url += `&${jsonp}=${cb}`

    // register the callback function
    window[cb] = (res) => {
      console.log('in js, the res is ' + res)
      resolve(res)
      reject(res)
      removeScipt(scriptId)
      removeCB(cb)
    }

    createScript(_url, scriptId)
  })
}

export default fetchJsonp
