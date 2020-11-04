<template>
  <section>

    <!--power button-->
    <div class="power">
      <div class="power-button">
        <img style="width: 60px; height: 50px; border-radius: 5px;" :src="powerButton" />
        <div class="text-area">
          <p class="text">{{ $t('adaptive.debug') }}</p>
        </div>
      </div>
    </div>

    <div class="router">
      <div v-for="(item, index) in ports" class="port" @click="dialog(index)" :key="index">
        <div>
          <el-tooltip class="item" effect="light">
            <img style="width: 60px; height: 50px; border-radius: 5px;" :src=selectUrl(item.linkstatus) />
            <div slot="content" class="tooltip-content">
              <h3></h3>
              <el-form label-position="left" size="mini">
                <el-form-item :label="$t('adaptive.linkstatus')" :label-width="tooltipLabelWidth" style="margin: 0;">
                  {{ports[index].linkstatus}}
                </el-form-item>
                <el-form-item label="LAN IP:" :label-width="tooltipLabelWidth" style="margin: 0;">
                  {{ports[index].ip}}
                </el-form-item>
                <el-form-item :label="$t('adaptive.netmask')" :label-width="tooltipLabelWidth">
                  {{ports[index].netmask}}
                </el-form-item>
              </el-form>
              <el-form label-position="left" size="mini">
                <el-form-item :label="$t('adaptive.devname')" :label-width="tooltipLabelWidth" style="margin: 0;">
                  {{ports[index].devname}}
                </el-form-item>
                <el-form-item :label="$t('adaptive.mac')" :label-width="tooltipLabelWidth" style="margin: 0;">
                  {{ports[index].mac}}
                </el-form-item>
                <el-form-item :label="$t('adaptive.speed')" :label-width="tooltipLabelWidth" style="margin: 0;">
                  {{ports[index].speed}}
                </el-form-item>
              </el-form>
            </div>
          </el-tooltip>
          <div class="text-area">
            <p class="text">
              <span v-show="item.function !== 'normal'">
                <svg class="icon">
                  <use :xlink:href=selectIcon(item.function)></use>
                </svg>
              </span>
              {{ports[index].webname}}</p>
          </div>
        </div>
      </div>

      <div v-if="checked !== 200" class="mask">
        <!-- <div v-if="false" class="mask"> -->
        <!--<div class="mask">-->
        <el-button class="button" type="primary" @click="sortingHandle">{{$t('NAT.adaptive.maskTip')}}</el-button>
      </div>

    </div>

    <el-dialog :title="$t('systemSetting.portadapt')" :visible.sync="sortingVisible" width="50%" :close-on-click-modal="false">

      <div>
        <div class="whole-router-in-dialog">
          <!--power button-->
          <div class="power">
            <div class="power-button">
              <img style="width: 60px; height: 50px; border-radius: 5px;" :src="powerButton" />
              <div class="text-area">
                <p class="text">{{$t('NAT.adaptive.text')}}</p>
              </div>
            </div>
          </div>

          <div class="router">
            <div v-for="(item, index) in ports" class="port" :key="index">
              <img style="width: 60px; height: 50px; border-radius: 5px;" src="../../static/port2.png" />
              <div :class="{ 'triangular': index === portsName.index }"></div>
              <div class="text-area">
                <p class="text" v-if="index < portsName.index">
                  {{localName[index]}}</p>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!--outer dialog-->
      <div style="margin: 10px 0;">
        <span>{{tip1}}</span>
        <span style="font-size: 200%; color: red;">{{this.portsName.index + 1}}</span>
        <span>{{tip2}}</span>
      </div>

      <div style="display: flex; align-items: center;" v-loading="sortLoading">
        <el-form :model="portsName" label-position="left" ref="portsName" size="small">
          <el-form-item :label="$t('NAT.adaptive.portsNameTip')" :label-width="tooltipLabelWidth" style="margin: 0 20px 0 0">
            <el-input v-model="portsName.name" :placeholder="name()"></el-input>
          </el-form-item>
        </el-form>
        <div>{{$t('NAT.adaptive.tipText')}}</div>
      </div>
      <div slot="footer" class="dialog-footer">
        <!-- <el-button type="danger" @click="sortingCancel">{{$t('operation.cancel')}}</el-button> -->
        <el-button v-if="portsName.index < ports.length-1" type="primary" :disabled="sortLoading" @click="sortingNextStep">{{$t('operation.next')}}</el-button>
        <el-button v-else type="primary" :disabled="sortLoading" @click="sortingFinish">{{$t('operation.complete')}}</el-button>
      </div>
    </el-dialog>

    
  </section>
</template>

<script>
import {
  getPorts,
  sendSorting,
  sendWANLAN,
  sortingCancel
} from '@/api/api.js'

let stopSignal1, stopSignal3, stopSignal4
export default {
  name: 'portadaptive',
  data() {
    return {
      dialogFormVisible: false, // LAN,WAN设置
      sortingVisible: false, // 端口自适应排序显示控制

      form: {
        number: '', // 端口编号，唯一，通过index确认端口
        use: '', // 选择功能LAN/WAN
        IP: '',
        mask: '', // 子网掩码
        gateway: '', // 网关
        mode: '1', // 工作模式
        rate: '1', // 网卡速率 lan
        control: '', // lan互访控制
        accessMethod: '1', // 接入方式
        primaryDNS: '', // 首选DNS
        secondaryDNS: '', // 备选DNS
        status: '未连接' // 连接状态，ASDL使用该属性
      },
      sendForm: {
        webnetif: '', // 端口编号，唯一，通过index确认端口
        use: '', // 选择功能LAN/WAN
        handle: '' // 添加为1，解绑为0
      },
      WANForm: {
        index: '',  // 端口编号，唯一，通过index确认端口
        use: '',    // 选择功能LAN/WAN
        handle: ''  // 添加为1，解绑为0
      },

      portsName: {
        index: '',
        name: '',
        flag: ''
      },

      localName: [],

      ports: [],
      checked: '', // 检查是否已自适应端口排序

      powerButton: 'static/port3.png',
      tips: [
        '请只接通第',
        '个网口，并为其设置网口名',
        '请只接通第',
        '个网口',
        '请插入网线，连通当前网口'
      ],
      tip1: '请只接通第',
      tip2: '个网口，并为其设置网口名',

      wanLimit: '',
      lanLimit: '',
      wanCount: 0,
      lanCount: 0,
      unbindEnable: false, // 绑定LAN/WAN口时，判断当前端口是否已被占用。是否可以解绑
      isInuseLAN: false, // 绑定LAN/WAN口时，判断当前端口是否已被占用。加上LAN上限是否超过
      isInuseWAN: false, // 绑定LAN/WAN口时，判断当前端口是否已被占用。加上LAN上限是否超过

      WANLANLoading: false,
      sortLoading: false,

      tooltipLabelWidth: '100px',
      formLabelWidth: '130px'
    }
  },
  watch: {
    checked: function() {
      if (this.checked !== 200) {
        this.$confirm('即将进行网口定位操作, 是否继续?', '提示', {
          confirmButtonText: '确定',
          cancelButtonText: '取消',
          type: 'warning'
        }).then(() => {
          this.$message({
            type: 'success',
            message: '开始重新定位!'
          })
          this.sortingHandle()
        }).catch(() => {
          this.$message({
            type: 'info',
            message: '已取消'
          })
        })
        this.getPortsInfoBegin()
      }
    }
  },
  methods: {
    modifyChecked() {
      this.checked = 0
    },

    interval() {
      stopSignal1 = terval(this.getPortsInfo, 5000)
    },

    // 获取ports信息
    getPortsInfo: function() {
      getPorts()
        .then(res => {
          if (res.data.code === 200) {
            this.wanLimit = res.data.wancount
            this.lanLimit = res.data.lancount
            let lanCount = 0
            let wanCount = 0
            for (let i = 0; i < res.data.interfaces.length; i++) {
              if (res.data.interfaces[i].function.toLowerCase() === 'wan') {
                wanCount++
                console.log('wanCount is ' + wanCount)
              }
              if (res.data.interfaces[i].function.toLowerCase() === 'lan') {
                lanCount++
                console.log('lanCount is ' + lanCount)
              }
            }
            this.lanCount = lanCount
            this.wanCount = wanCount

            res.data.interfaces.sort(sortNumber)

            // console.log("get feedback, res.data.interfaces is: "+res.data.interfaces);
            this.checked = JSON.parse(JSON.stringify(res.data.code))
            this.$store.dispatch('setAdaptive', this.checked)
            console.log('commit adaptive ' + this.checked)
            this.ports = JSON.parse(JSON.stringify(res.data.interfaces))
            console.log(
              'after parse, the port[] is: ' + JSON.stringify(res.data.interfaces)
            )
          }
        })
        .catch(error => {
          console.log(error)
        })
      // 根据webindex排序，确保端口顺序正确
      function sortNumber(a, b) {
        return a.webindex - b.webindex
      }
    },
    getPortsInfoBegin: function() {
      getPorts()
        .then(res => {
          this.wanLimit = res.data.wancount
          this.lanLimit = res.data.lancount
          let lanCount = 0
          let wanCount = 0
          for (let i = 0; i < res.data.interfaces.length; i++) {
            if (res.data.interfaces[i].function.toLowerCase() === 'wan') {
              wanCount++
              console.log('wanCount is ' + wanCount)
            }
            if (res.data.interfaces[i].function.toLowerCase() === 'lan') {
              lanCount++
              console.log('lanCount is ' + lanCount)
            }
          }
          this.lanCount = lanCount
          this.wanCount = wanCount

          // console.log("get feedback, res.data.interfaces is: "+res.data.interfaces);
          this.checked = JSON.parse(JSON.stringify(res.data.code))
          // console.log("code is "+JSON.stringify(res.data.code));
          this.ports = JSON.parse(JSON.stringify(res.data.interfaces))
          console.log(
            'after parse, the port[] is: ' + JSON.stringify(res.data.interfaces)
          )
        })
        .catch(error => {
          console.log(error)
        })
    },
    // 显示选择功能的dialog
    dialog: function(para) {
      console.log(para)
      this.WANForm.index = para
      this.WANForm.use = ''

      this.unbindEnable = this.ports[para].function === 'normal'
      this.isInuseLAN = this.lanLimit <= this.lanCount || !this.unbindEnable
      this.isInuseWAN = this.wanLimit <= this.wanCount || !this.unbindEnable
    },
    // 选择LAN,WAN口对应图标
    selectUrl(para) {
      // if(para === "空闲")
      if (para === 'off') {
        return 'static/port2.png'
      } else {
        return 'static/port3.png'
      }
    },
    // 选择router页面里的图标
    selectIcon(para) {
      if (para === 'wan') {
        return '#icon-wan'
      } else if (para === 'lan') {
        return '#icon-pc'
      } else {
        return false
      }
    },
    // 选择端口对应名称
    name: function() {
      return 'eth' + this.portsName.index
    },

    // 显示端口自适应排序页面
    sortingHandle: function() {
      this.sortingVisible = true

      this.portsName.index = 0
      this.portsName.name = ''
      this.portsName.flag = ''
    },
    // 网口自适应功能 “下一步”按钮调用函数
    sortingNextStep: function() {
      // form validate
      if (this.portsName.name.length > 10) {
        this.$message({
          message: '名称长度请控制在10个字以内',
          type: 'error'
        })
        return
      } else if (this.portsName.name.length === 0) {
        this.$message({
          message: '请输入名称',
          type: 'error'
        })
        return
      }

      this.sortLoading = true
      this.tip = this.tips[5]

      if (this.portsName.flag !== 'middle') {
        this.portsName.flag = 'first'
      }
      let para = Object.assign({}, this.portsName)
      console.log('name is: ' + JSON.stringify(this.portsName))

      stopSignal3 = setInterval(() => {
        sendSorting(para)
          .then(res => {
            if (this.portsName.index === res.data.index) {
              if (res.data.code === 200) {
                clearInterval(stopSignal3)
                console.log('interval stop: ' + stopSignal3)

                this.localName[res.data.index] = this.portsName.name

                this.sortLoading = false
                this.portsName.index = res.data.index + 1
                // this.portsName.index++;
                this.portsName.flag = 'middle'
                this.tip1 = this.tips[0]
                this.tip2 = this.tips[1]

                console.log(
                  'in sorting interval: http status is ' +
                  res.status +
                  ' and the index is ' +
                  this.portsName.index
                )
                console.log(res)
              } else if (res.data.code === 500) {
                this.tip1 = this.tips[2]
                this.tip2 = this.tips[3]
                console.log(
                  'in sorting interval: http status is ' +
                  res.status +
                  ' and the index is ' +
                  this.portsName.index
                )
                console.log(res)
              }
            }
          })
          .catch(error => {
            console.log(error)
          })
      }, 1000)
    },
    // 网口自适应功能 “完成”按钮调用函数
    sortingFinish: function() {
      this.sortLoading = true
      this.portsName.flag = 'last'

      let para = Object.assign({}, this.portsName)

      stopSignal4 = setInterval(() => {
        sendSorting(para)
          .then(res => {
            if (res.data.index === this.portsName.index) {
              if (res.data.code === 200) {
                this.sortLoading = false
                this.portsName.name = ''
                this.portsName.flag = ''
                this.portsName.index = 0
                clearInterval(stopSignal4)
                console.log('in cancel interval: ' + res.status)
                console.log(res)
                this.sortingVisible = false

                this.tip1 = this.tips[0]
                this.tip2 = this.tips[1]

                this.getPortsInfo()

                setTimeout(() => {
                  this.getPortsInfo()
                }, 1000)
              } else if (res.data.code === 500) {
                this.tip1 = this.tips[2]
                this.tip2 = this.tips[3]
                console.log(
                  'in sorting interval: http status is ' +
                  res.status +
                  ' and the index is ' +
                  this.portsName.index
                )
                console.log(res)
              }
            }
          })
          .catch(error => {
            console.log(error)
          })
      }, 1000)
      this.$emit('finishSorting', 'finish')
    },
    // 网口自适应功能 “取消”按钮调用函数
    sortingCancel: function() {
      if (stopSignal3 != null) {
        clearInterval(stopSignal3)
      } else {
        clearInterval(stopSignal4)
      }
      this.sortingVisible = false
      sortingCancel().then(res => {
        if (res.status === 200) {
          return true
        }
      })
      this.sortLoading = false
      this.portsName.index = 0
      this.portsName.name = ''
      this.portsName.flag = ''

      this.tip1 = this.tips[0]
      this.tip2 = this.tips[1]

      this.$emit('finishSorting', 'cancel')
    }
  },
  mounted() {
    this.getPortsInfoBegin()
    // this.interval();
  },
  beforeDestroy() {
    clearInterval(stopSignal1)
    clearInterval(stopSignal3)
    clearInterval(stopSignal4)
  }
}
</script>

<style scoped>
.whole-router .router .port img:hover {
  cursor: pointer;
}
.whole-router {
  display: flex;
  position: relative;
  border: 1px solid lightgrey;
  background-color: ghostwhite;
  color: #909399;
}
.whole-router .router .port {
  width: 60px;
  height: 50px;
  margin: 25px auto;
  flex-grow: 1;
  border-radius: 5px;
}
.power {
  border: 1px solid lightgrey;
  border-radius: 10px;
  text-align: center;
  margin: auto;
  background-color: dodgerblue;
}
.router {
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin: 10px 20px;
  background-color: lightgoldenrodyellow;
  display: flex;
  flex-wrap: wrap;
  text-align: center;
}
.whole-router-in-dialog {
  width: 100%;
  border: 1px solid lightgrey;
  background-color: ghostwhite;
  color: #909399;
  display: flex;
  margin-bottom: 30px;
  position: relative;
}

.whole-router-in-dialog .power {
  border: 1px solid lightgrey;
  border-radius: 10px;
  text-align: center;
  margin: 10px 0 10px 20px;
  background-color: dodgerblue;
}

.whole-router-in-dialog .power .power-button {
  width: 60px;
  height: 50px;
  margin: 25px 20px;
}

.whole-router-in-dialog .router {
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin: 10px 20px;
  background-color: lightgoldenrodyellow;
  display: flex;
  flex-wrap: wrap;
  text-align: center;
}

.whole-router-in-dialog .router .port {
  width: 60px;
  height: 50px;
  margin: 25px auto;
  flex-grow: 1;
  border-radius: 5px;
}

.mask {
  position: absolute;
  top: 0;
  left: 0;
  background-color: rgba(70, 70, 70, 0.5);
  width: 100%;
  height: 100%;
}

.mask .button {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
}

.power-button {
  width: 60px;
  height: 50px;
  margin: 25px 20px;
}
.text {
  font-weight: bold;
}
.text-area p {
  margin: 0;
}

.icon {
  width: 1em;
  height: 1em;
  vertical-align: -0.15em;
  fill: currentColor;
  overflow: hidden;
}

.triangular {
  border: 10px solid lightgoldenrodyellow;
  border-bottom-color: #f00;
  width: 0;
  height: 0;
  margin: auto;
}
</style>
