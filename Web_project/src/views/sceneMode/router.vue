<template>
  <div class="container">

    <div class="header">
      <title-line class="title_line" />
    </div>

    <div class="body">
      <left-col
        class="left_col"
        @changebox="changeBox" />

      <main-box
        class="content"
        :websocketdata="websocketData[0]"
        :kidvpndata="websocketData[6]"
        v-if="functionChoosen === mainBox && websocketData[0]" />

      <internet-box
        class="content"
        :internet-info="websocketData[5]"
        @getinternetinfo="getNetInfo"
        v-if="functionChoosen === internetBox" />

      <terminal-box
        class="content"
        v-if="functionChoosen === terminalBox" />

    </div>

  </div>
</template>

<script>
import {
  mainBox,
  internetBox,
  terminalBox,
  leftCol,
  titleLine
} from './components/index.js'
import { websocketIpGetAction, getMonitorInfo } from '@/api/api.js'
let websocket = ''

export default {
  name: 'SceneRouter',
  components: {
    mainBox,
    internetBox,
    terminalBox,
    leftCol,
    titleLine
  },
  data () {
    return {
      functionChoosen: '',
      data: {

      },
      mainBox: 'mainBox',
      internetBox: 'internetBox',
      terminalBox: 'terminalBox',

      websocketData: [],

      timer: ''
    }
  },

  methods: {
    changeBox (val) {
      this.functionChoosen = val
      console.log('functionChoosen' + this.functionChoosen)
      console.log('websocketData[0]' + this.websocketData[0])

      console.log('切换' + val)
    },

    initListener () {
      this.websocket.addEventListener(
        'message',
        function (event) {
          console.log(event.data)
        }
      )
    },

    removeListener () {
      this.websocket.removeEventListener(
        'message',
        function (event) {
          console.log(event.data)
        }
      )
    },

    initWebsocket (url) {
      // Create WebSocket connection.
      websocket = new WebSocket('ws://' + url + ':8001')

      // Connection opened
      websocket.addEventListener('open', function (event) {
        setTimeout(() => {
          websocket.send('0')
        }, 0)
        setTimeout(() => {
          websocket.send('5')
        }, 20)
        setTimeout(() => {
          websocket.send('6')
        }, 40)
        setTimeout(() => {
          websocket.send('2')
        }, 60)
      })

      // Listen for messages
      websocket.addEventListener('message', event => {
        console.log('Message from server ', event.data)
        let data = JSON.parse(event.data)

        if (data.mode) {
          this.websocketData[0] = data
        }
        if (data.webname) {
          this.websocketData[1] = data
        }
        if (data.hasOwnProperty('arp_defense')) {
          this.websocketData[5] = data
          console.log('wan_ip')
          console.log(data)
        }
        if (data.hasOwnProperty('public_ip') && (data['public_ip'] !== '')) {
          // this.websocketData[6] = data
          this.$set(this.websocketData, 6, data)
          console.log('get main info')
        }
        console.log('get new data')
        console.log(this.websocketData)
      })
    },
    getcpu() {
      let para1 = {}
      para1.type = 6
      getMonitorInfo(para1)
        .then(res => {
          if (res.data.code === 200) {
            this.CPU = res.data.cpu_usage.toFixed(0)
            this.RAM = res.data.memory_usage.toFixed(0)
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    update () {
      let para1 = {}
      para1.type = 6
      getMonitorInfo(para1)
        .then(res => {
          if (res.data.code === 200) {
            this.data.cpu = res.data.cpu_usage.toFixed(0)
            this.data.memory = res.data.memory_usage.toFixed(0)
            this.data.userNum = res.data.online_users
            // this.data.upStorage = 0
            // this.data.downStorage = 0
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.$store.dispatch('pushSystemData', this.data)
      console.log('this.$store.state.app.systemData.cpu')
      console.log(this.$store.state.app.systemData.cpu)
      console.log(this.$store.state.app.systemData.memory)
    }

  },

  mounted () {
    if (this.$store.state.app.systemData.cpu === '') {
      this.update()
      this.timer = setInterval(() => {
        this.update()
      }, 2000)
    }

    setTimeout(() => {
      this.functionChoosen = 'mainBox'
    }, 300)
    websocketIpGetAction()
      .then(res => {
        this.initWebsocket(res.data.websockip)
      })
      .catch(err => {
        console.log(err)
      })
  },
  destroyed () {
    console.log('close websocket')
    websocket.close()
    if (this.timer) {
      clearInterval(this.timer)
    }
  }
}
</script>

<style scoped lang="scss">
.container {
  height: 100vh;
  width: 100vw;
  // background: rgb(197, 224, 179);
  .header {
    overflow: hidden;
    .title_line {
      height: 4rem;
    }
  }
  @media (min-width: 80rem) {
    .body {
      height: 90%;
      width: 60%;
      padding-left: 20vw;
      padding-right: 20vw;
      display: flex;
      margin: auto;
      .left_col {
        position: relative;
        width: 10vw;
        height: 100%;
        margin-left: -10vw;
        right: 2vw;
      }
      .content {
        width: 100%;
        height: 100%;
        overflow: auto;
      }
    }
  }
  @media (max-width: 80rem) {
    .body {
      height: 90%;
      width: 90%;
      display: flex;
      float: right;
      // margin: auto;
      .left_col {
        position: relative;
        width: 10vw;
        height: 100%;
        margin-left: -10vw;
        right: 2vw;
      }
      .content {
        width: 100%;
        min-width: 8rem;
        height: 100%;
        overflow: auto;
      }
    }
  }
}

::-webkit-scrollbar {
  width: 0;
}
</style>
