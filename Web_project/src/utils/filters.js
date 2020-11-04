const RATE = 1024
const LIMIT = 1000
const HOUR = 24
const MINUTE = 60

const filters = {
  conversion: function (val) {
    let unit = ''
    if (val > (RATE * RATE * LIMIT)) {
      unit = 'GB/s'
      val = val / (RATE * RATE * RATE)
      val = val.toFixed(2)
    } else if (val > RATE * LIMIT) {
      unit = 'MB/s'
      val = val / (RATE * RATE)
      val = val.toFixed(2)
    } else if (val <= RATE * LIMIT) {
      unit = 'KB/s'
      val = val / RATE
      val = val.toFixed(2)
    }
    return val + unit
  },
  conversionUnit: function (val) {
    let unit = ''
    if (val > (RATE * RATE * LIMIT)) {
      unit = 'GB'
      val = val / (RATE * RATE * RATE)
      val = val.toFixed(2)
    } else if (val > RATE * LIMIT) {
      unit = 'MB'
      val = val / (RATE * RATE)
      val = val.toFixed(2)
    } else if (val < RATE * LIMIT) {
      unit = 'KB'
      val = val / RATE
      val = val.toFixed(2)
    }
    return val + unit
  },
  time: function (val) {
    let unit = ''
    let result = ''
    if (val > MINUTE * HOUR) {
      let t = Math.floor(val / (MINUTE * HOUR))
      val = val % (MINUTE * HOUR)
      unit = '天'
      result = t + unit
    } if (val > MINUTE) {
      let t = Math.floor(val / MINUTE)
      val = val % MINUTE
      unit = '小时'
      result = result + t + unit
    } if (val <= MINUTE) {
      unit = '分钟'
      result = result + val + unit
    }
    return result
  }
}

export default filters
