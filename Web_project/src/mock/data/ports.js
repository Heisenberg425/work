import Mock from 'mockjs'

const ports = []

let portsNumber = parseInt(Math.random() * 3 + 4, 10)

for (let i = 0; i < portsNumber; i++) {
  ports.push(Mock.mock({
    enname: i,
    devname: 'el',
    webname: 'eTHX',  // 用户自定义的名称
    function: Mock.Random.pick(['wan', 'lan', 'normal']),
    linkstatus: 'off',
    ip: Mock.Random.ip(),
    netmask: Mock.Random.ip(),
    mac: '00:00:00:00:00:00:00:00',
    speed: '0'
    // webindex:
  }))
}

const dialUp = {
  index: '',
  account: '',
  passwd: '',
  status: ''
}

export { dialUp, ports }
