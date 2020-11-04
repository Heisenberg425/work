import axios from 'axios'
import MockAdapter from 'axios-mock-adapter'
import { LoginUsers, Users } from './data/user'
import { ports, dialUp } from './data/ports'
import { alias, mapping } from './data/system'
import Mock from 'mockjs'
let _Users = Users
let _ports = ports
let _alias = alias
let _mapping = mapping
let code = 200
// let _dialUp = dialUp;

// console.log(Users);
// console.log(system);

export default {
  /**
   * mock bootstrap
   */
  bootstrap: function () {
    let mock = new MockAdapter(axios)

    // mock success request
    mock.onGet('/success').reply(200, {
      msg: 'success'
    })

    // mock error request
    mock.onGet('/error').reply(500, {
      msg: 'failure'
    })

    // 登录
    mock.onPost('/login').reply(config => {
      let {username, password} = JSON.parse(config.data)
      return new Promise((resolve, reject) => {
        let user = null
        setTimeout(() => {
          let hasUser = LoginUsers.some(u => {
            if (u.username === username && u.password === password) {
              user = JSON.parse(JSON.stringify(u))
              user.password = undefined
              return true
            }
          })

          if (hasUser) {
            resolve([200, {code: 200, msg: '请求成功', user}])
          } else {
            resolve([200, {code: 500, msg: '账号或密码错误'}])
          }
        }, 1000)
      })
    })

    // 获取用户列表
    mock.onGet('/user/list').reply(config => {
      let {name} = config.params
      let mockUsers = _Users.filter(user => {
        return (name && user.name.indexOf(name) === -1)
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            users: mockUsers
          }])
        }, 1000)
      })
    })

    // 获取用户列表（分页）
    mock.onGet('/user/listpage').reply(config => {
      let {page, name} = config.params
      let mockUsers = _Users.filter(user => {
        return (name && user.name.indexOf(name) === -1)
      })
      let total = mockUsers.length
      mockUsers = mockUsers.filter((u, index) => index < 20 * page && index >= 20 * (page - 1))
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            total: total,
            users: mockUsers
          }])
        }, 1000)
      })
    })

    // 删除用户
    mock.onGet('/user/remove').reply(config => {
      let {id} = config.params
      _Users = _Users.filter(u => u.id !== id)
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '删除成功'
          }])
        }, 500)
      })
    })

    // 批量删除用户
    mock.onGet('/user/batchremove').reply(config => {
      let {ids} = config.params
      ids = ids.split(',')
      _Users = _Users.filter(u => !ids.includes(u.id))
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '删除成功'
          }])
        }, 500)
      })
    })

    // 编辑用户
    mock.onGet('/user/edit').reply(config => {
      let {id, name, addr, age, birth, sex} = config.params
      _Users.some(u => {
        console.log(u)
        if (u.id === id) {
          u.name = name
          u.addr = addr
          u.age = age
          u.birth = birth
          u.sex = sex
          return true
        }
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '编辑成功'
          } ])
        }, 500)
      })
    })

    // 新增用户
    mock.onGet('/user/add').reply(config => {
      let {name, addr, age, birth, sex} = config.params
      _Users.push({
        name: name,
        addr: addr,
        age: age,
        birth: birth,
        sex: sex
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '新增成功'
          }])
        }, 500)
      })
    })

    mock.onGet('/system').reply(config => {
      let mockSystem = [
        Mock.mock(
          {
            'CPURate|20-80.1': 1,
            'RAMRate|20-80.1': 1
          }
        )
      ]
      // console.log(_system);
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            system: mockSystem
          }])
        }, 100)
      })
    })

    // ports info
    mock.onPost('/action/interfaceMonitorAction').reply(config => {
      let ports = _ports

      return new Promise((resolve, reject) => {
        resolve([200, {
          code: code,
          interfaces: ports
        }])
      }, 1500)
    })

    // dialup
    mock.onPost('/api/dialUp').reply(config => {
      let {index, account, passwd, status} = config.data
      console.log(index)
      console.log(config)
      console.log(typeof (config))
      console.log(config.data)
      console.log(typeof (config.data))
      console.log(config.data)
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            form: config.data
          }])
        }, 500)
      })
    })

    // alias info
    mock.onPost('/api/alias').reply(config => {
      console.log('config.data is ' + config.data)
      config.data = JSON.parse(config.data)

      let {page} = config.data

      let total = _alias.length
      let mockAlias = _alias.filter((a, id) => id < 5 * page && id >= 5 * (page - 1))

      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            total: total,
            alias: mockAlias
          }])
        }, 1000)
      })
    })

    // edit alias info
    mock.onPost('/api/editAlias').reply(config => {
      console.log('config.data is ' + config.data)
      config.data = JSON.parse(config.data)

      let { id, aliasAddress, initialAddress, endAddress } = config.data
      _alias.some(a => {
        console.log('let see what a is:' + a)
        if (a.id === id) {
          a.aliasAddress = aliasAddress
          a.initialAddress = initialAddress
          a.endAddress = endAddress
          return true
        }
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '编辑成功'
          }])
        }, 500)
      })
    })

    // add alias info
    mock.onPost('/api/addAlias').reply(config => {
      console.log('in add  config.data is ' + config.data)

      config.data = JSON.parse(config.data)
      _alias.unshift({
        id: Mock.Random.guid(),
        aliasAddress: config.data.aliasAddress,
        initialAddress: config.data.initialAddress,
        endAddress: config.data.endAddress
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '新增成功'
          }])
        }, 500)
      })
    })

    // del alias info
    mock.onPost('/api/delAlias').reply(config => {
      config.data = JSON.parse(config.data)
      console.log('in del config.data is ' + config.data.ids)
      let {ids} = config.data
      // ids = ids.split(',');
      _alias = _alias.filter(u => !ids.includes(u.id))
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '删除成功'
          }])
        }, 500)
      })
    })

    // mapping info
    mock.onPost('/api/mapping').reply(config => {
      config.data = JSON.parse(config.data)
      let {page} = config.data

      let total = _mapping.length
      let mockMapping = _mapping.filter((a, id) => id < 5 * page && id >= 5 * (page - 1))

      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            total: total,
            mapping: mockMapping
          }])
        }, 1000)
      })
    })

    // edit mapping info
    mock.onPost('/api/editMapping').reply(config => {
      console.log('config.data is ' + config.data)
      config.data = JSON.parse(config.data)

      let { id, outerPort, innerPort, innerInitialIP, numOfInner, protocol } = config.data
      _mapping.some(m => {
        if (m.id === id) {
          console.log('find it! id is: ' + id)
          m.outerPort = outerPort
          m.innerPort = innerPort
          m.innerInitialIP = innerInitialIP
          m.numOfInner = numOfInner
          m.protocol = protocol
          return true
        }
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '编辑成功'
          }])
        }, 500)
      })
    })

    // add mapping info
    mock.onPost('/api/addMapping').reply(config => {
      console.log('in add  config.data is ' + config.data)

      config.data = JSON.parse(config.data)
      _mapping.unshift({
        id: Mock.Random.guid(),
        outerPort: config.data.outerPort,
        innerPort: config.data.innerPort,
        innerInitialIP: config.data.innerInitialIP,
        numOfInner: config.data.numOfInner,
        protocol: config.data.protocol
      })
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '新增成功'
          }])
        }, 500)
      })
    })

    // del mapping info
    mock.onPost('/api/delMapping').reply(config => {
      config.data = JSON.parse(config.data)
      console.log('in del config.data is ' + config.data.ids)
      let {ids} = config.data
      // ids = ids.split(',');
      _mapping = _mapping.filter(u => !ids.includes(u.id))
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            code: 200,
            msg: '删除成功'
          }])
        }, 500)
      })
    })

    //
    mock.onPost('/api/intAdaHandleAction').reply(config => {
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          let data1 = JSON.parse(config.data)
          console.log('the config.data.flag is ' + data1.flag)
          if (data1.flag === 'last') {
            code = 200
          } else {
            code = 500
          }
          resolve([200, {
            code: 200
          }])
          // reject([500, {
          //   code: 500
          // }])
        }, 300)
      })
    })

    mock.onPost('/action/natAssNodeUseAction').reply(() => {
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve([200, {
            percent: Math.random() * 30 + 20,
            usenode: Math.random() * 1000,
            totalnode: 12228 + Math.random() * 10000
          }])
        }, 100)
      })
    })
  }
}
