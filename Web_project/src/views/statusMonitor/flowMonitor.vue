<template>
  <section>
    <div class="line_02">
      <span>{{ $t('statusMonitor.flowmonitor') }}</span>
    </div>
    <div id="echartsUp"/>
    <div id="echartsDown"/>
  </section>
</template>

<script>
import { getMonitorInfo } from '../../api/api.js'
import flowMonitor from 'echarts'
require('echarts/theme/walden')

export default {
    name: 'FlowMonitor',
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

        /**
         *  绘制上行流量饼状图。
         *
         *  @param  {none}
         *  @return {none}
         */
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

                this.dataUp[0].name   = 'HTTP协议'
                this.dataUp[1].name   = '邮件服务'
                this.dataUp[2].name   = '网络聊天'
                this.dataUp[3].name   = '文件传输'
                this.dataUp[4].name   = '其他'
                this.dataDown[0].name = 'HTTP协议'
                this.dataDown[1].name = '邮件服务'
                this.dataDown[2].name = '网络聊天'
                this.dataDown[3].name = '文件传输'
                this.dataDown[4].name = '其他'

                option = {
                    title: {
                        text: '上行流量信息',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
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
                    data: ['HTTP', 'Email', 'Chat', 'Transmission', 'Other']
                }
                this.dataUp[0].name   = 'HTTP'
                this.dataUp[1].name   = 'Email'
                this.dataUp[2].name   = 'Chat'
                this.dataUp[3].name   = 'Transmission'
                this.dataUp[4].name   = 'Other'
                this.dataDown[0].name = 'HTTP'
                this.dataDown[1].name = 'Email'
                this.dataDown[2].name = 'Chat'
                this.dataDown[3].name = 'Transmission'
                this.dataDown[4].name = 'Other'
                option = {
                    title: {
                        text: 'UpFlowInfo',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
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

            /**
             *  格式化数据
             *
             *  @param  {none}
             *  @return {none}
             */
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

        /**
         *  绘制下行流量饼状图。
         *
         *  @param  {none}
         *  @return {none}
         */
        drawChartsDown () {
            let myChart = flowMonitor.init(
                document.getElementById('echartsDown'),
                'walden'
            )

            let option  = {}
            if (this.$store.state.app.language === 'zh') {
              option = {
                  title: {
                      text: '下行流量信息',

                      textStyle: {
                          fontSize: 14,
                          fontWeight: 'normal'
                      }
                  },
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
              option = {
                  title: {
                      text: 'DownFlowInfo',

                      textStyle: {
                          fontSize: 14,
                          fontWeight: 'normal'
                      }
                  },
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

          /**
            *  格式化数据
            *
            *  @param  {none}
            *  @return {none}
            */
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

      /**
        *  获取流量监控数据
        *
        *  @param  {none}
        *  @return {none}
        */
        getFlowInfo () {
            let para = Object.assign({}, this.form)
            para.type = 4
            para.page = 1
            getMonitorInfo(para).then(res => {
                if (res.data.code === 200) {
                    this.original = res.data.data
                }
                puthToData(this.original, this.dataUp, this.dataDown)
            }).then(() => {
                this.drawChartsUp()
                this.drawChartsDown()
            }).catch(error => {
                console.log(error)
            })

            /**
             * 当TMT_LINE_APP时，返回Protocol值取:
             * app_type_http = 1,
             * app_type_mail = 2,
             * app_type_chat = 3,
             * app_type_transfer = 4,
             * app_type_others = 5,
             * 总流量 = 6
             */
            function puthToData (val, up, down) {
                for (let i = 0; i < val.length; i++) {
                    let index = val[i].protocol - 1
                    up[index].value   = val[i].recv_total_length
                    down[index].value = val[i].send_total_length
                }
            }
        }
    },

    watch: {
      '$store.state.app.language': function () {
          console.log(this.$store.state.app.language)
          this.getFlowInfo()
      }
    },

    mounted () {
        if (this.$store.state.app.language === 'zh') {
            this.$route.matched[0].name = this.$route.matched[0].meta.zh
            this.$route.matched[1].name = this.$route.matched[1].meta.zh
        } else {
            this.$route.matched[0].name = this.$route.matched[0].meta.en
            this.$route.matched[1].name = this.$route.matched[1].meta.en
        }
        this.getFlowInfo()
    }
}
</script>

<style scoped lang="scss">
#echartsUp,
#echartsDown {
  height: 300px;
  width: 49%;
  border: 1px solid #ddd;
}
#echartsUp {
  float: left;
}
#echartsDown {
  float: right;
}
</style>
