<template>
  <section>

    <div class="line_02">
      <span>{{ $t('statusMonitor.performanceMonitor') }}</span>
    </div>

    <el-table
      :data='form'
      :header-cell-style="headerStyle">
      <el-table-column
        prop="cpu"
        :label="$t('statusMonitor.cpu')"
        min-width="120"/>
      <el-table-column
        prop="memory"
        :label="$t('statusMonitor.memory')"
        min-width="120"/>
    </el-table>
    <el-col>
      <div class="line_02">
        <span>{{ $t('statusMonitor.terminalNum') }}</span>
      </div>
      <el-row :span="24">
        <div
          id="chartLine1"
          style="width:100%; height:250px; margin-top: 20px"/>
      </el-row>

    </el-col>

  </section>
</template>

<script>
import performanceMonitor from 'echarts'
require('echarts/theme/walden')

let option = {}
let myChart

export default {
  name: 'PerformanceMonitor',
  data () {
    return {
      form: [{
        cpu: Number,
        memory: Number
      }]
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    drawLineChart1: function () {
      myChart = performanceMonitor.init(
        document.getElementById('chartLine1'),
        'walden'
      )

      if (this.$store.state.app.language === 'zh') {
        option = {
          title: {
            text: '',
            subtext: '',

            textStyle: {
              fontSize: 14,
              fontWeight: 'normal'
            }
          },
          tooltip: {
            trigger: 'axis',
            axisPointer: {
              type: 'cross',
              label: {
                backgroundColor: 'blue'
              }
            }
          },
          legend: {
            data: ['在线终端数']
          },
          grid: {
            left: '5%',
            right: '5%',
            bottom: '10%',
            containLabel: false
          },
          toolbox: {
            show: true
          // feature: {
          // dataView: {readOnly: false},
          // restore: {},
          // saveAsImage: {}
          // }
          },
          dataZoom: {
            show: false,
            start: 0,
            end: 100
          },
          xAxis: [
            {
              type: 'category',
              boundaryGap: true,
              data: (function () {
                let now = new Date()
                let res = []
                let len = 12
                while (len--) {
                  res.unshift(now.toLocaleTimeString().replace(/^\D*/, ''))
                  now = new Date(now - 2000)
                }
                return res
              })()
            }
          /*
            {
              type: 'category',
              boundaryGap: true,
              data: (function (){
                let res = [];
                let len = 0;
                while (len < 12) {
                  res.push(len + 1);
                  len++;
                }
                return res;
              })()
            }
            */
          ],

          yAxis: [
            {

              type: 'value',
              scale: false,
              minInterval: 1,
              name: '数量',
              boundaryGap: [0.2, 0.2],
              axisLabel: {

                interval: 1

              }
            }
          /*
            {
              type: 'value',
              scale: true,
              name: '终端在线人数',
              max: 1200,
              min: 0,
              boundaryGap: [0.2, 0.2]
            }
            */
          ],
          series: [
          /*
            {
              name:'终端在线人数',
              type:'bar',
              xAxisIndex: 1,
              yAxisIndex: 1,
              data:(function (){
                let res = [];
                let len = 12;
                while (len--) {
                  res.push(Math.round(Math.random() * 1000));
                }
                return res;
              })()
            },
            */
            {
              name: '终端在线数',
              type: 'line',
              smooth: true,
              data: []
            }
          ]
        }
      } else {
        option = {
          title: {
            text: '',
            subtext: '',

            textStyle: {
              fontSize: 14,
              fontWeight: 'normal'
            }
          },
          tooltip: {
            trigger: 'axis',
            axisPointer: {
              type: 'cross',
              label: {
                backgroundColor: 'blue'
              }
            }
          },
          legend: {
            data: ['OnlineNum']
          },
          grid: {
            left: '5%',
            right: '5%',
            bottom: '10%',
            containLabel: false
          },
          toolbox: {
            show: true
          // feature: {
          // dataView: {readOnly: false},
          // restore: {},
          // saveAsImage: {}
          // }
          },
          dataZoom: {
            show: false,
            start: 0,
            end: 100
          },
          xAxis: [
            {
              type: 'category',
              boundaryGap: true,
              data: (function () {
                let now = new Date()
                let res = []
                let len = 12
                while (len--) {
                  res.unshift(now.toLocaleTimeString().replace(/^\D*/, ''))
                  now = new Date(now - 2000)
                }
                return res
              })()
            }
          /*
            {
              type: 'category',
              boundaryGap: true,
              data: (function (){
                let res = [];
                let len = 0;
                while (len < 12) {
                  res.push(len + 1);
                  len++;
                }
                return res;
              })()
            }
            */
          ],

          yAxis: [
            {
              type: 'value',
              scale: false,
              minInterval: 1,
              name: 'Num',
              boundaryGap: [0.2, 0.2]
            }
          /*
            {
              type: 'value',
              scale: true,
              name: '终端在线人数',
              max: 1200,
              min: 0,
              boundaryGap: [0.2, 0.2]
            }
            */
          ],
          series: [
          /*
            {
              name:'终端在线人数',
              type:'bar',
              xAxisIndex: 1,
              yAxisIndex: 1,
              data:(function (){
                let res = [];
                let len = 12;
                while (len--) {
                  res.push(Math.round(Math.random() * 1000));
                }
                return res;
              })()
            },
            */
            {
              name: 'TerminalNum',
              type: 'line',
              smooth: true,
              data: []
            }
          ]
        }
      }
      myChart.setOption(option)

      window.addEventListener('resize', function () {
        myChart.resize()
      })
    },

    drawCharts () {
      this.drawLineChart1()
    },

    async update () {
      let axisData = new Date().toLocaleTimeString().replace(/^\D*/, '')

      let HorAxis = option.series[0].data
      if (HorAxis.length > 11) {
        HorAxis.shift()
      }
      HorAxis.push(this.$store.state.app.systemData.userNum)

      option.xAxis[0].data.shift()
      option.xAxis[0].data.push(axisData)

      myChart.setOption(option)
    },
    updateCPU () {
      this.form[0].cpu = (this.$store.state.app.systemData.cpu) + '%'
      this.form[0].memory = (this.$store.state.app.systemData.memory) + '%'
    },
    getInfo () {
      this.update()
      setInterval(this.update, 2000)
      setInterval(this.updateCPU, 100)
    }

    // getSystemInfo() {
    //   let para = {}
    //   para.type = 6
    //   getMonitorInfo(para).then((res) => {
    //     if (res.data.code === 200) {
    //       this.form[0].cpu = (res.data.cpu_usage).toFixed(2) + '%'
    //       this.form[0].memory = (res.data.memory_usage).toFixed(2) + '%'
    //     }
    //   })
    // }

  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)

      this.drawCharts()
    }
  },
  mounted: function () {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
    this.getInfo()
    this.drawCharts()
    // 18.09.17 pky 太烦，生成环境恢复
    // this.getSystemInfo()
  }
}
</script>

<style scoped>
</style>
