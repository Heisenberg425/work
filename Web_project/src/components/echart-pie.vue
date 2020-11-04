<template>
   <section>
   
    
  </section>
</template>

<script>
import { getMonitorInfo } from '@/api/api.js'
import flowMonitor from 'echarts'
require('echarts/theme/walden')
export default {
  name: 'PieChart',
  // props: {
  //   dataType: {type: String, default: 'default'}
  // },
  data () {
    return {
      legend: {
        orient: 'vertical',
        x: 'right',
        data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他']
      },
      dataUp: [
        { value: 0, name: 'HTTP协议' },
        { value: 0, name: '邮件服务' },
        { value: 0, name: '网络聊天' },
        { value: 0, name: '文件传输' },
        { value: 0, name: '其他' }
      ],
      dataDown: [
        { value: 0, name: 'HTTP协议' },
        { value: 0, name: '邮件服务' },
        { value: 0, name: '网络聊天' },
        { value: 0, name: '文件传输' },
        { value: 0, name: '其他' }
      ],

      form: {
        type: Number,
        enable: '',
        protocol: '',
        port: '',
        ip: '',
        mac: '',
        alias: '',
        ifname: '',
        duration: Number,
        send_rate: Number,
        recv_rate: Number,
        send_total_length: Number,
        recv_total_length: Number,
        send_total_packets: Number,
        recv_total_packets: Number
      },
      original: []
    }
  },
  methods: {
    drawChartsUp () {
      let myChart = flowMonitor.init(
        document.getElementById('echartsUp'),
        'walden'
      )
      let option = {}
      if (this.$store.state.app.language === 'zh') {
        this.legend = {
          orient: 'vertical',
          x: 'right',
          data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他']
        }

        this.dataUp[0].name = 'HTTP协议'
        this.dataUp[1].name = '邮件服务'
        this.dataUp[2].name = '网络聊天'
        this.dataUp[3].name = '文件传输'
        this.dataUp[4].name = '其他'
        this.dataDown[0].name = 'HTTP协议'
        this.dataDown[1].name = '邮件服务'
        this.dataDown[2].name = '网络聊天'
        this.dataDown[3].name = '文件传输'
        this.dataDown[4].name = '其他'
        // this.option.series[0].name = '协议流量分布'
        // this.option.title.text = '上行流量信息'
        option = {

          tooltip: {
            trigger: 'item',
            formatter: function(p) {
              console.log('p')
              console.log(p)
              let str = p.seriesName + '<br/>' + p.name + ': ' + formatNum(p.value) + 'Bit' + '(' + p.percent + '%' + ')'

              return  str
            }
          },
          legend: this.legend,
          series: [
            {
              name: '协议流量分布',
              type: 'pie',
              radius: ['80%', '90%'],
              avoidLabelOverlap: false,
              label: {
                normal: {
                  show: false,
                  position: 'center'
                },
                emphasis: {
                  show: true,
                  textStyle: {
                    fontSize: '20',
                    fontWeight: 'bold'
                  }
                }
              },
              labelLine: {
                normal: {
                  show: false
                }
              },
              data: this.dataUp
            }
          ]
        }
      } else {
        this.legend = {
          orient: 'vertical',
          x: 'right',
          data: ['HTTP', 'Email', 'Chat', 'Transmission', 'Others']
        }
        this.dataUp[0].name = 'HTTP'
        this.dataUp[1].name = 'Email'
        this.dataUp[2].name = 'Chat'
        this.dataUp[3].name = 'Transmission'
        this.dataUp[4].name = 'Others'
        this.dataDown[0].name = 'HTTP'
        this.dataDown[1].name = 'Email'
        this.dataDown[2].name = 'Chat'
        this.dataDown[3].name = 'Transmission'
        this.dataDown[4].name = 'Others'
        // this.option.series[0].name = 'Protocol traffic distribution'
        // this.option.title.text = 'UpFlowInfo'
        option = {

          tooltip: {
            trigger: 'item',
            formatter: function(p) {
              console.log('p')
              console.log(p)
              let str = p.seriesName + '<br/>' + p.name + ': ' + formatNum(p.value) + 'Bit' + '(' + p.percent + '%' + ')'

              return  str
            }
          },
          legend: this.legend,
          series: [
            {
              name: 'Protocol traffic distribution',
              type: 'pie',
              radius: ['80%', '90%'],
              avoidLabelOverlap: false,
              label: {
                normal: {
                  show: false,
                  position: 'center'
                },
                emphasis: {
                  show: true,
                  textStyle: {
                    fontSize: '20',
                    fontWeight: 'bold'
                  }
                }
              },
              labelLine: {
                normal: {
                  show: false
                }
              },
              data: this.dataUp
            }
          ]
        }
      }
      function formatNum(strNum) {
        console.log('strNum')
        console.log(strNum)
        if (strNum.length <= 3) {
          return strNum
        }
        if (!/^(\+|-)?(\d+)(\.\d+)?$/.test(strNum)) {
          return strNum
        }
        var a = RegExp.$1

        var b = RegExp.$2

        var c = RegExp.$3
        var re = new RegExp()
        re.compile('(\\d)(\\d{3})(,|$)')
        while (re.test(b)) {
          b = b.replace(re, '$1,$2$3')
        }
        return a + '' + b + '' + c
      }
      console.log('option')
      console.log(option)
      myChart.setOption(option)

      window.addEventListener('resize', function () {
        myChart.resize()
      })
    },
    pushToData (val, up, down) {
      for (let i = 0; i < val.length; i++) {
        // if (val[i].protocol !== 6) {
        console.log('val[i]')
        console.log(val[i])
        let index = val[i].protocol - 1
        // if (this.dataType === 'recv') {
        console.log('recv1')
        up[index].value = val[i].recv_total_length
        // } else if (this.dataType === 'send') {
        console.log('send1')
        down[index].value = val[i].send_total_length
        // }

        // }
      }
      console.log('down')
      console.log(down)
    },
    drawChartsDown () {
      let myChart = flowMonitor.init(
        document.getElementById('echartsDown'),
        'walden'
      )
      let option  = {}
      if (this.$store.state.app.language === 'zh') {
        // this.option.title.text = '下行流量信息'
        // this.option.series[0].name = '协议流量分布'
        this.legend = {
          orient: 'vertical',
          x: 'right',
          data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他']
        }

        this.dataUp[0].name = 'HTTP协议'
        this.dataUp[1].name = '邮件服务'
        this.dataUp[2].name = '网络聊天'
        this.dataUp[3].name = '文件传输'
        this.dataUp[4].name = '其他'
        this.dataDown[0].name = 'HTTP协议'
        this.dataDown[1].name = '邮件服务'
        this.dataDown[2].name = '网络聊天'
        this.dataDown[3].name = '文件传输'
        this.dataDown[4].name = '其他'
        option = {

          tooltip: {
            trigger: 'item',
            formatter: function(p) {
              console.log('p')
              console.log(p)
              let str = p.seriesName + '<br/>' + p.name + ': ' + formatNum(p.value) + 'Bit' + '(' + p.percent + '%' + ')'

              return  str
            }
          },
          legend: this.legend,
          series: [
            {
              name: '协议流量分布',
              type: 'pie',
              radius: ['80%', '90%'],
              avoidLabelOverlap: false,
              label: {
                normal: {
                  show: false,
                  position: 'center'
                },
                emphasis: {
                  show: true,
                  textStyle: {
                    fontSize: '20',
                    fontWeight: 'bold'
                  }
                }
              },
              labelLine: {
                normal: {
                  show: false
                }
              },
              data: this.dataDown
            }
          ]
        }
      } else {
        // this.option.series[0].name = 'Protocol traffic distribution'
        // this.option.title.text = 'DownFlowInfo'
        this.legend = {
          orient: 'vertical',
          x: 'right',
          data: ['HTTP', 'Email', 'Chat', 'Transmission', 'Others']
        }
        this.dataUp[0].name = 'HTTP'
        this.dataUp[1].name = 'Email'
        this.dataUp[2].name = 'Chat'
        this.dataUp[3].name = 'Transmission'
        this.dataUp[4].name = 'Others'
        this.dataDown[0].name = 'HTTP'
        this.dataDown[1].name = 'Email'
        this.dataDown[2].name = 'Chat'
        this.dataDown[3].name = 'Transmission'
        this.dataDown[4].name = 'Others'
        option = {

          tooltip: {
            trigger: 'item',
            formatter: function(p) {
              console.log('p')
              console.log(p)
              let str = p.seriesName + '<br/>' + p.name + ': ' + formatNum(p.value) + 'Bit' + '(' + p.percent + '%' + ')'

              return  str
            }
          },
          legend: this.legend,
          series: [
            {
              name: 'Protocol traffic distribution',
              type: 'pie',
              radius: ['80%', '90%'],
              avoidLabelOverlap: false,
              label: {
                normal: {
                  show: false,
                  position: 'center'
                },
                emphasis: {
                  show: true,
                  textStyle: {
                    fontSize: '20',
                    fontWeight: 'bold'
                  }
                }
              },
              labelLine: {
                normal: {
                  show: false
                }
              },
              data: this.dataDown
            }
          ]
        }
      }
      function formatNum(strNum) {
        console.log('strNum')
        console.log(strNum)
        if (strNum.length <= 3) {
          return strNum
        }
        if (!/^(\+|-)?(\d+)(\.\d+)?$/.test(strNum)) {
          return strNum
        }
        var a = RegExp.$1

        var b = RegExp.$2

        var c = RegExp.$3
        var re = new RegExp()
        re.compile('(\\d)(\\d{3})(,|$)')
        while (re.test(b)) {
          b = b.replace(re, '$1,$2$3')
        }
        return a + '' + b + '' + c
      }
      myChart.setOption(option)

      window.addEventListener('resize', function () {
        myChart.resize()
      })
    },

    getFlowInfo () {
      let para = Object.assign({}, this.form)
      para.type = 4
      para.page = 1
      // if (this.dataType !== 'default') {
      getMonitorInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.original = res.data.data
            console.log('this.original ')
            console.log(this.original)
          }

          this.pushToData(this.original, this.dataUp, this.dataDown)
          this.drawChartsDown()
          this.drawChartsUp()
        })

        .catch(error => {
          console.log(error)
        })
      // }
      /**
       * 当TMT_LINE_APP时，返回Protocol值取:
       * app_type_http = 1,
       * app_type_mail = 2,
       * app_type_chat = 3,
       * app_type_transfer = 4,
       * app_type_others = 5,
       * 总流量 = 6
       */
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.getFlowInfo()
    }
  },
  mounted () {
    this.getFlowInfo()
  }
}
</script>

<style scoped lang="scss">
#echartsUp,
#echartsDown {
  height: 200px;
  width: 90%;
  
}

</style>