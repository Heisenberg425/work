<template>
  <section>
    <div class="line_02">
      <span>{{ $t('systemStatus.systemStatus') }}</span>
    </div>

    <!-- 最顶行 -->
    <div id="top">
      <el-row :gutter="10">
        <!-- 最顶行左边 -->
        <el-col :md="14" :lg="14" :xl="14">
          <!-- 最顶行左边: 左侧，外网连接状态  -->
          <div class="div_outernet">
            <div :class="[ (monitorInfo.status === 'failed') ? notInUse : inUse ]" class="div_outernet_status">
              <p class="name_title">OpenRT</p>
              <br>
              <p>
                {{ $t('systemStatus.top.outerNetTitle') }}
                <span v-if="monitorInfo.status !== 'failed'">
                  {{ $t('systemStatus.top.connected') }}
                </span>
                <span v-else>
                  {{ $t('systemStatus.top.notConnect') }}
                </span>
              </p>
              <br>
              <p class="info_title">
                {{ $t('systemStatus.top.connectTime') }}:
                {{ monitorInfo.time[0] }}{{ $t('systemStatus.top.day') }}
                {{ monitorInfo.time[1] }}{{ $t('systemStatus.top.hour') }}
                {{ monitorInfo.time[2] }}{{ $t('systemStatus.top.min') }}
                {{ monitorInfo.time[3] }}{{ $t('systemStatus.top.sec') }}
              </p>
            </div>

            <!-- 最顶行左边: 右侧，速率状态  -->
            <div class="div_rate_info" :key="index" @click="pushToRate" style="cursor: pointer;">
              <p >{{ $t('systemStatus.top.rate') }}</p>
              <div class="rate">
                <p>{{ $t('systemStatus.top.upstream') }}:{{ data.up }}</p>
                <p style="font-size: 18px;" />
              </div>
              <div class="rate">
                <p>{{ $t('systemStatus.top.downstream') }}:{{ data.down }}</p>
                <p style="font-size: 18px;" />
              </div>
            </div>
          </div>
        </el-col>

        <!-- 最顶行右边：终端状态显示-->
        <el-col  :md="10"  :lg="10" :xl="10">
          <div class="div_terminal" @click="pushToTerminal" style="cursor: pointer;">
            <p>{{ $t('systemStatus.top.terminalStatus') }}</p>
            <div class="top_right_bottom">
              <div class="top_right_bottom_text">
                <p style="font-size: 24px;">{{ terminalNum }} {{ $t('systemStatus.top.terminalOnline') }}</p>
              </div>
            </div>
          </div>
        </el-col>
      </el-row>
    </div>

    <!-- 中间行 -->
    <div id="middle">
      <el-row :gutter="10">
        <el-col  :md="14"  :lg="14"  :xl="14">

          <!-- 中间行左边：接口状态 -->
          <div class="div_port">

            <!-- 中间行左边：第一行：标题 -->
            <div class="div_port_title">
              <div>
                <p>{{ $t('systemStatus.middle.status') }}</p>
              </div>

              <!-- 中间行左边：第二行：启用数量 -->
              <div class="div_port_info">
                <div class="div_port_info_wan_lan">
                  <p style="font-size: 24px">{{ wanNumber }} {{ $t('systemStatus.middle.WAN') }}</p>
                </div>
                <div class="div_port_info_wan_lan">
                  <p style="font-size: 24px">{{ lanNumber }} {{ $t('systemStatus.middle.LAN') }}</p>
                </div>
              </div>              
            </div>

            <!-- 中间行左边：第三行：接口图标 -->
            <div class="div_port_status">
              <div class="router">
                <div  v-for="(item, index) in ports"  :key="index"  class="port"  @click="pushToINEX">
                  <div>
                    <el-tooltip  class="item"  effect="light">
                      <img  :src="selectUrl(item.linkstatus)"  style="width: 3rem; height: 3rem; border-radius: .3rem;">
                      <div  slot="content"  class="tooltip-content">
                        <p>{{ item.webname }}</p>
                        <el-form  label-position="left"  size="mini">
                          <el-form-item  :label-width="tooltipLabelWidth"  :label="$t('systemStatus.middle.connectStatus')"
                                          style="margin: 0;"> {{ ports[index].linkstatus }}
                          </el-form-item>
                          <el-form-item  :label-width="tooltipLabelWidth"   label="LAN IP:"
                                          style="margin: 0;"> {{ ports[index].ip }}
                          </el-form-item>
                          <el-form-item  :label-width="tooltipLabelWidth"  :label="$t('systemStatus.middle.netmask')">
                                          {{ ports[index].netmask }}
                          </el-form-item>
                        </el-form>
                        <el-form  label-position="left"  size="small">
                          <el-form-item  :label-width="tooltipLabelWidth"  :label="$t('systemStatus.middle.device')"
                                          style="margin: 0;"> {{ ports[index].devname }}
                          </el-form-item>
                          <el-form-item  :label-width="tooltipLabelWidth"  :label="$t('systemStatus.middle.mac')"
                                          style="margin: 0;"> {{ ports[index].mac }}
                          </el-form-item>
                          <el-form-item  :label-width="tooltipLabelWidth"  :label="$t('systemStatus.middle.rate')"
                                          style="margin: 0;"> {{ ports[index].speed }}
                          </el-form-item>
                        </el-form>
                      </div>
                    </el-tooltip>
                    <div class="text-area">
                      <p class="text">
                        <span v-show="item.function !== 'normal'">
                          <svg class="icon">
                            <use :xlink:href="selectIcon(item.function)" />
                          </svg>
                        </span>
                        {{ item.webname }}</p>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </el-col>

        <!-- 中间行右边：暂无信息 -->
        <el-col  :md="10"  :lg="10"  :xl="10">
          <div class="middle_right">
          </div>
        </el-col>
      </el-row>
    </div>

    <!-- 最底行 -->
    <div id="bottom">
      <el-row :gutter="10">
        <el-col  :md="8"  :lg="8"  :xl="8">
          <pie-chart class="div_flow" />
        </el-col>
        <el-col  :md="16" :lg="16" :xl="16">
          <div class="div_rate">
            <line-chart />
          </div>
        </el-col>
      </el-row>
    </div>
  </section>
</template>

<script>
import { getPorts, getMonitorInfo, getExtranetStatus } from '../../api/api'
import { pieChart, lineChart } from './components'
import { conversion } from '../../utils/rateUnitExchange.js'
require('echarts/theme/walden')

export default {
    name: 'SystemStatus',
    components: {
        pieChart,
        lineChart
    },

    data () {                                                           // 只在当前界面可见的数据
        return {
            terminalNum: '',                                            // 连接终端数量
            monitorInfo: {                                              // 外网连接信息
                status: '',
                time: ''
            },
            data: {                                                     // 上下行速率以及相关信息
                up: '',
                down: '',
                upStorage: '',
                downStorage: '',
                cpu: '',
                memory: '',
                userNum: ''
            },
            ports: [
                {
                    id: '0',
                    category: '',
                    imgUrl: 'static/port3.png'
                }
            ],
            tooltipLabelWidth: '5rem',
            inUse: 'net_in_use',                                              // css有关变量
            notInUse: 'net_not_in_use'
        }
    },

    computed: {                                                           // 计算属性
       /* 
        * 语言变量 
        */
        lang: {

            /**
             *  获取页面显示的语言.
             *
             *  @param  {none}     
             *  @return {language} 当前页面显示的语言.
             */
            get() {        
                if (this.$store.state.app.language === 'en') {
                    console.log('Current Language: en');
                } else {
                    console.log('Current Language: zh');
                }

                return this.$store.state.app.language;
            },

            /**
             *  设置页面显示的语言.
             *
             *  @param  {lang}     页面显示的语言
             *  @return {none}
             */
            set(lang) {
                console.log('set');
                this.$i18n.locale = lang;
                this.$store.dispatch('setLanguage', lang);
            }
        },

       /* 
        * LAN 口数量
        */
        /**
         *  计算 LAN 口的数量
         *
         *  @param  {none}     
         *  @return LAN 口的数量
         */
        lanNumber: function () {
            let lanSum = 0;
            let i = 0;
            for (i; i < this.ports.length; i++) {
                if (this.ports[i].function === 'lan') {
                    lanSum++;
                }
            }
            return  lanSum;
        },

       /* 
        * WAN 口数量
        */
        /**
         *  计算 WAN 口的数量
         *
         *  @param  {none}     
         *  @return WAN 口的数量
         */
      wanNumber: function () {
          let wanSum = 0;
          let i = 0;
          for (i; i < this.ports.length; i++) {
              if (this.ports[i].function === 'wan') {
                  wanSum++;
              }
          }
          return wanSum;
      },

     /* 
      * 连接状态图标
      */
      /**
       *  获取连接状态图标
       *
       *  @param  {para}     连接状态
       *  @return 连接状态图标
       */
      'item.linkstatus': function (para) {
          if (para === 'off') {
              return 'static/port2.png';
          } else {
              return 'static/port3.png';
          }
        }
    },

    watch: {                                                              // 侦听属性
      '$store.state.app.language': function () {
        console.log(this.$store.state.app.language);
      }
    },

    mounted: function () {
        if (this.$store.state.app.language === 'zh') {
            this.$route.matched[0].name = this.$route.matched[0].meta.zh;
            this.$route.matched[1].name = this.$route.matched[1].meta.zh;
        } else {
            this.$route.matched[0].name = this.$route.matched[0].meta.en;
            this.$route.matched[1].name = this.$route.matched[1].meta.en;
        }
        this.$store.dispatch('setTimer', true);
        this.getData();
    },

    destroyed () {
        clearInterval(this.$store.state.app.timers.intervalOfInterface)
    },

    methods: {

        /**
         *  设置 WAN 口、LAN 口图标。
         *
         *  @param  {para}  off 表示 LAN 口，否则表示 WAN 口
         *  @return {none}
         */
        selectUrl (para) {
            if (para === 'off') {
                return 'static/port2.png';
            } else {
                return 'static/port3.png';
            }
        },

        /**
         *  设置 ADSL、DHCP、STATIC 图标。
         *
         *  @param  {para}  表示接口类型
         *  @return {none}
         */
        selectIcon (para) {
            if (para === 'wan') {
                return '#icon-wan';
            } else if (para === 'lan') {
                return '#icon-pc';
            } else {
                return '#';
            }
        },

        /**
         *  点击 “速率状态” 之后，跳转到 “速率监控” 界面
         *
         *  @param  {none}
         *  @return {none}
         */
        pushToRate() {
            this.$router.push('/rateMonitor');
        },

        /**
         *  点击 “终端在线” 之后，跳转到 “终端监控” 界面
         *
         *  @param  {none}
         *  @return {none}
         */
        pushToTerminal() {
            this.$router.push('/terminalMonitor')
        },

      /**
        *  点击 “端口图标” 之后，跳转到 “内外网设置” 界面
        *
        *  @param  {none}
        *  @return {none}
        */
        pushToINEX () {
          this.$router.push({ path: '../INEX_network' })
        },
    
        /**
         *  获取数据接口，会获取以下信息：
         *  1、getNetInfo:         获取外网连接信息
         *  2、getMonitorSysInfo:  获取上下行速率、CPU、内存使用情况等
         *  3、getPortsInfo:       获取端口信息，比如 IP、MAC 等
         * 
         *  并且，启动一秒钟触发一次的定时器，周期性获取以上数据
         *
         *  @param  {none}
         *  @return {none}
         */
        getData () {
            this.getNetInfo();
            this.getMonitorSysInfo();
            this.getPortsInfo();

            this.$store.state.app.timers.intervalOfInterface = setInterval(() => {
                this.getNetInfo();
                this.getMonitorSysInfo();
                this.getPortsInfo();
            }, 1000)
        },

        /**
         *  获取外网连接信息，包括：
         *  1、外网连接状态，记录到 monitorInfo.status;
         *  2、外网连接时长，记录到 monitorInfo.time。
         *
         *  @param  {none}
         *  @return {none}
         */
        getNetInfo () {
            getExtranetStatus().then(res => {
                if (res.data.code === 200) {
                  this.monitorInfo.status = res.data.status;
                  this.monitorInfo.time   = res.data.time.split(':');
                }
            })
        },

        /**
         *  获取监控信息，这是一个异步函数，连续发出请求后，等待异步返回。
         *
         *  @param  {none}
         *  @return {none}
         */
        async getMonitorSysInfo () {
          
            let paraRate = {};
            paraRate.page = 1;
            paraRate.type = 5;

            /**
             *  获取上下、下行速率。
             *  并通过 pushSystemData 下发。
             */
            await getMonitorInfo(paraRate).then(res => {
                if (res.data.code === 200) {
                    this.data.upStorage   = res.data.data[0].recv_rate;
                    this.data.downStorage = res.data.data[0].send_rate;
                    this.data.up          = conversion(res.data.data[0].recv_rate);
                    this.data.down        = conversion(res.data.data[0].send_rate);
                    this.$store.dispatch('pushSystemData', this.data);
                }
            }).catch(error => {
                console.log(error)
            })

            let paraPerf = {};
            paraPerf.type = 6;

            /**
             *  获取 CPU 使用率、内存使用率、终端在线个数
             *  并通过 pushSystemData 下发。
             */
            await getMonitorInfo(paraPerf).then(res => {
                if (res.data.code === 200) {
                    this.data.cpu    = res.data.cpu_usage.toFixed(0);
                    this.data.memory = res.data.memory_usage.toFixed(0);
                    this.terminalNum = res.data.online_users;
                }
            }).catch(error => {
                console.log(error)
            })
            this.$store.dispatch('pushSystemData', this.data);
          },

        /**
         *  获取端口信息，包括端口的 IP、MAC、连接状态等
         *
         *  @param  {none}
         *  @return {none}
         */
        getPortsInfo: function () {
            getPorts().then(res => {
                let checked = res.data.code;
                if (checked !== 200) {
                    this.$store.dispatch('setAdaptive', checked);
                    this.$router.push('ports');
                }

                this.ports = res.data.interfaces;
            })
        },
    }
}
</script>

<style scoped lang="scss">
p {
  color: grey;
}

// 外网信息
.div_outernet {
  height: 150px;
  display: flex;
  margin-top: 1rem;
  border: 1px solid lightgrey;
  
  // 外网连接状态
  .div_outernet_status {
    .name_title {
      height: 3rem;
      line-height: 3rem;
      font-size: 1.5rem;
    }
    .info_title {
      margin-top: 1rem;
    }
  }

   // 外网未连接时显示的颜色
  .net_not_in_use {
    width: 60%;
    background-color: lightgrey;
    p {
      margin: auto 20px;
      color: white;
    }
  }

  // 外网连接时显示的颜色
  .net_in_use {
    width: 60%;
    background-color: rgb(138, 199, 130);
    p {
      margin: auto 20px;
      color: white;
    }
  }

  // 外网连接速率
  .div_rate_info {
    padding-left: 1rem;
    width: 40%;
    .rate {
      p {
        margin: 1rem auto;
        display: inline-block;
      }
    }
  }
}

.div_terminal {
  height: 150px;
  border: 1px solid lightgrey;
  padding-left: 1rem;
  margin-top: 1rem;
  font-family: Roboto-Thin;
  .top_right_bottom {
    display: flex;
    .top_right_bottom_text {
      width: 25%;
      p {
        margin: 1rem auto;
      }
    }
  }
}

.div_port {
  margin-top: 1rem;
  height: 15rem;
  border: 1px solid lightgrey;
  font-family: Roboto-Thin;
  .div_port_title {
    height: 100px;
    padding-left: 20px;
    .div_port_info {
      display: flex;
      .div_port_info_wan_lan {
        width: 50%;
        p {
          margin: 0 auto;
        }
      }
    }
  }
}
.middle_right {
  margin-top: 1rem;
  height: 240px;
  border: 1px solid lightgrey;
  font-family: Roboto-Thin;
  p {
    margin-left: 20px;
  }
  span {
    margin-left: 0;
    color: grey;
  }
}

#bottom {
  .div_flow {
    margin-top: 1rem;
    height: 350px;
    width: 100%;
    border: 1px solid lightgrey;
  }
  .div_rate {
    margin-top: 1rem;

    border: 1px solid lightgrey;
    height: 350px;
  }
}

.router {
  height: 4.2rem;
  width: 92%;
  margin: 0.6rem 1.2rem;
  display: inline-flex;
  justify-content: space-around;
  background-color: lightgoldenrodyellow;
  text-align: center;
  border: 1px solid lightgrey;
  border-radius: 0.6rem;
  .port {
    width: 3rem;
    height: 3rem;
    border-radius: 0.3rem;
    background-color: grey;
    text-align: center;
    margin: 0.6rem 0.2rem 0.2rem 0;
    img:hover {
      cursor: pointer;
    }
  }
}

.text-area p {
  display: flex;
}

.icon {
  width: 1em;
  height: 1em;
  vertical-align: -0.15em;
  fill: currentColor;
  overflow: hidden;
}

@font-face {
  font-family: Roboto-Thin;
  src: url("../../assets/font/Roboto-Thin.ttf");
}
</style>
