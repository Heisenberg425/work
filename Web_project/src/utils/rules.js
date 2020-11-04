const pattern = {
  name: /^([\w-]+|[\u4e00-\u9fa5]+)$/, // 用户名校验
  passWord: /^\w+$/, // 密码校验
  // ip: /((25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))/,
  // ip: /(?=(\b|\D))(((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))(?=(\b|\D))/,
  // ip: /^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$/,
  // ip: /^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$/,
  ip: /^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$/,
  mac: /^([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2})$/,
  aeskey: /([a-f0-9]){32,64}/,
  number: /^[0-9]*$/,
  time: /^\+?[1-9]\d{0,3}(\.\d*)?$/,
  netmask: /^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/
}

/**
 * 内置规则
 * @param {String} reg - {pattern}中预定义正则名称 or 自定义正则
 * @param {String} msg1 - 正则校验不通过提示
 * @param {String} msg2 - 值为空时提示（可不填）
 */
const validate = (reg, msg1, msg2, msgMin, msgMax) => {
  return (rule, value, callback) => {
    // console.log(reg)
    console.log('in validate ' + JSON.stringify(rule))
    console.log('in validate ' + value)
    if (!value) {
      if (!msg2) {
        callback()
      }
      callback(new Error(msg2))
    }
    reg = pattern[reg] ? pattern[reg] : reg
    if (reg === pattern.mac) {
      value = value.replace(/(^\s+)|(\s+$)/g, '')
    }
    if (!reg.test(value)) {
      callback(new Error(msg1))
    } else {
      callback()
    }

    if (msgMin && msgMax) {
      if (value < msgMin || value > msgMax) {
        callback(new Error('输入范围为' + msgMin + '~' + msgMax))
      }
    } else if (msgMin) {
      if (value < msgMin) {
        callback(new Error('请输入大于等于' + msgMin + '的数'))
      }
    } else if (msgMax) {
      if (value > msgMax) {
        callback(new Error('请输入小于等于' + msgMax + '的数'))
      }
    }
  }
}

export default validate
