<template>
  <section class="chart-container">
    <div class="line_02"><span>{{ $t('statusMonitor.rateMonitor') }}</span></div>
    <div>
      <div id="chartLine1"/>
    </div>
    <div>
      <div id="chartLine2"/>
    </div>
  </section>
</template>

<script>
/**
 * @getMonitorInfo 参数：{type: Number， page: Number} 获取速率(type: 4) 共计1种请求
 */
import { getMonitorInfo } from '../../api/api.js'
import rateMonitor from 'echarts'
require('echarts/theme/walden')

let myChartUp, myChartDown
let optionUp, optionDown

export default {
    data () {
        return {
            tooltip: {
                trigger: 'axis',
                axisPointer: {
                    type: 'cross',
                    label: {
                        backgroundColor: 'blue'
                    }
                }
            },
            grid: {
                left: '3%',
                right: '3%',
                bottom: '3%',
                containLabel: true
            },
            seriesUp: [
                {
                    name: 'HTTP协议',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '邮件服务',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '网络聊天',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '文件传输',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '其他',
                    type: 'line',
                    smooth: true,
                    data: []
                }
            ],
            seriesDown: [
                {
                    name: 'HTTP协议',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '邮件服务',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '网络聊天',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '文件传输',
                    type: 'line',
                    smooth: true,
                    data: []
                },
                {
                    name: '其他',
                    type: 'line',
                    smooth: true,
                    data: []
                }
            ],

            dataUp: [
                {value: 0, name: 'HTTP协议'},
                {value: 0, name: '邮件服务'},
                {value: 0, name: '网络聊天'},
                {value: 0, name: '文件传输'},
                {value: 0, name: '其他'}
            ],
            dataDown: [
                {value: 0, name: 'HTTP协议'},
                {value: 0, name: '邮件服务'},
                {value: 0, name: '网络聊天'},
                {value: 0, name: '文件传输'},
                {value: 0, name: '其他'}
            ],

            Axis: (function () {
                let now = new Date()
                let res = []
                let len = 12
                while (len--) {
                    res.unshift(now.toLocaleTimeString().replace(/^\D*/, ''))
                    now = new Date(now - 2000)
                }
                return res
            })(),

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
            original: [],
            timer:0
        }
    },

    methods: {

        /**
         *  绘制上行速率折线图。
         *
         *  @param  {none}
         *  @return {none}
         */
        drawLineChart1: function () {
            myChartUp = rateMonitor.init(document.getElementById('chartLine1'), 'walden')
            let option = {}
            if (this.$store.state.app.language === 'zh') {
                this.seriesUp[0].name   = 'HTTP协议'
                this.seriesUp[1].name   = '邮件服务'
                this.seriesUp[2].name   = '网络聊天'
                this.seriesUp[3].name   = '文件传输'
                this.seriesUp[4].name   = '其他'
                this.seriesDown[0].name = 'HTTP协议'
                this.seriesDown[1].name = '邮件服务'
                this.seriesDown[2].name = '网络聊天'
                this.seriesDown[3].name = '文件传输'
                this.seriesDown[4].name = '其他'
                this.dataUp[0].name     = 'HTTP协议'
                this.dataUp[1].name     = '邮件服务'
                this.dataUp[2].name     = '网络聊天'
                this.dataUp[3].name     = '文件传输'
                this.dataUp[4].name     = '其他'
                this.dataDown[0].name   = 'HTTP协议'
                this.dataDown[1].name   = '邮件服务'
                this.dataDown[2].name   = '网络聊天'
                this.dataDown[3].name   = '文件传输'
                this.dataDown[4].name   = '其他'

                option = {
                    title: {
                        text: '上行速率',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: this.tooltip,
                    legend: {
                        data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他'],
                        selected: {
                            'HTTP协议': true,
                            '邮件服务': false,
                            '网络聊天': false,
                            '文件传输': false,
                            '其他': false
                        }
                    },
                    grid: this.grid,
                    toolbox: {
                      show: true
                    },
                    dataZoom: {
                        show: false,
                        start: 0,
                        end: 100
                    },
                    xAxis: {
                      type: 'category',
                      boundaryGap: true,
                      data: this.Axis
                    },
                    yAxis: [
                    {
                        type: 'value',
                        scale: true,
                        min: 0,
                        boundaryGap: [0.2, 0.2],
                        axisLabel: {
                            formatter: function (value, index) {
                                const k = 1000
                                const t = 1024
                                let i = value
                                if (i >= k * t * t) {
                                    return (i / (t * t * t)).toFixed(0) + 'GB/s'
                                } else if (i >= t * k) {
                                    return (i / (t * t)).toFixed(0) + 'MB/s'
                                } else if (i >= k) {
                                    return (i / t).toFixed(0) + 'KB/s'
                                } else {
                                    return i.toFixed(0) + 'B/s'
                                }
                            }
                        }
                    }
                    ],
                    series: this.series
                }
            } else {
                this.seriesUp[0].name   = 'HTTP'
                this.seriesUp[1].name   = 'Email'
                this.seriesUp[2].name   = 'Chat'
                this.seriesUp[3].name   = 'Transmission'
                this.seriesUp[4].name   = 'Other'
                this.seriesDown[0].name = 'HTTP'
                this.seriesDown[1].name = 'Email'
                this.seriesDown[2].name = 'Chat'
                this.seriesDown[3].name = 'Transmission'
                this.seriesDown[4].name = 'Other'
                this.dataUp[0].name     = 'HTTP协议'
                this.dataUp[1].name     = 'Email'
                this.dataUp[2].name     = 'Chat'
                this.dataUp[3].name     = 'Transmission'
                this.dataUp[4].name     = 'Other'
                this.dataDown[0].name   = 'HTTP'
                this.dataDown[1].name   = 'Email'
                this.dataDown[2].name   = 'Chat'
                this.dataDown[3].name   = 'Transmission'
                this.dataDown[4].name   = 'Other'

                option = {
                    title: {
                        text: 'Up Rate',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: this.tooltip,
                    legend: {
                        data: ['HTTP', 'Email', 'Chat', 'Transmission', 'Other'],
                        selected: {
                            'HTTP': true,
                            'Email': false,
                            'Chat': false,
                            'Transmission': false,
                            'Other': false
                        }
                    },
                    grid: this.grid,
                    toolbox: {
                        show: true
                    },
                    dataZoom: {
                        show: false,
                        start: 0,
                        end: 100
                    },
                    xAxis: {
                        type: 'category',
                        boundaryGap: true,
                        data: this.Axis
                    },
                    yAxis: [
                    {
                        type: 'value',
                        scale: true,
                        min: 0,
                        boundaryGap: [0.2, 0.2],
                        axisLabel: {
                            formatter: function (value, index) {
                                const k = 1000
                                const t = 1024
                                let i = value
                                if (i >= k * t * t) {
                                  return (i / (t * t * t)).toFixed(0) + 'GB/s'
                                } else if (i >= t * k) {
                                  return (i / (t * t)).toFixed(0) + 'MB/s'
                                } else if (i >= k) {
                                  return (i / t).toFixed(0) + 'KB/s'
                                } else {
                                  return i.toFixed(0) + 'B/s'
                                }
                            }
                        }
                    }
                    ],
                    series: this.series
                }
            }

            myChartUp.setOption(option)

            window.addEventListener('resize', function () {
                myChartUp.resize()
            })
        },

        /**
         *  绘制下行速率折线图。
         *
         *  @param  {none}
         *  @return {none}
         */
        drawLineChart2: function () {
            myChartDown = rateMonitor.init(document.getElementById('chartLine2'), 'walden')
            let option = {}
            if (this.$store.state.app.language === 'zh') {
                option = {
                    title: {
                        text: '下行速率',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: this.tooltip,
                    legend: {
                        data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他'],
                        selected: {
                            'HTTP协议': true,
                            '邮件服务': false,
                            '网络聊天': false,
                            '文件传输': false,
                            '其他': false
                        }
                    },
                    grid: this.grid,
                    toolbox: {
                        show: true
                    },
                    dataZoom: {
                        show: false,
                        start: 0,
                        end: 100
                    },
                    xAxis: {
                        type: 'category',
                        boundaryGap: true,
                        data: this.Axis
                    },
                    yAxis: [
                        {
                            type: 'value',
                            scale: true,
                            min: 0,
                            boundaryGap: [0.2, 0.2],
                            axisLabel: {
                                formatter: function (value, index) {
                                    const k = 1000
                                    const t = 1024
                                    let i = value
                                    if (i >= k * t * t) {
                                        return (i / (t * t * t)).toFixed(0) + 'GB/s'
                                    } else if (i >= t * k) {
                                        return (i / (t * t)).toFixed(0) + 'MB/s'
                                    } else if (i >= k) {
                                        return (i / t).toFixed(0) + 'KB/s'
                                    } else {
                                        return i.toFixed(0) + 'B/s'
                                    }
                                }
                              }
                          }
                    ],
                    series: this.series
                }
            } else {
                option = {
                    title: {
                        text: 'Down Rate',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: this.tooltip,
                    legend: {
                        data: ['HTTP', 'Email', 'Chat', 'Transmission', 'Other'],
                        selected: {
                            'HTTP': true,
                            'Email': false,
                            'Chat': false,
                            'Transmission': false,
                            'Other': false
                        }
                    },
                    grid: this.grid,
                    toolbox: {
                        show: true
                    },
                    dataZoom: {
                        show: false,
                        start: 0,
                        end: 100
                    },
                    xAxis: {
                        type: 'category',
                        boundaryGap: true,
                        data: this.Axis
                    },
                    yAxis: [
                        {
                            type: 'value',
                            scale: true,
                            min: 0,
                            boundaryGap: [0.2, 0.2],
                            axisLabel: {
                                formatter: function (value, index) {
                                    const k = 1000
                                    const t = 1024
                                    let i = value
                                    if (i >= k * t * t) {
                                        return (i / (t * t * t)).toFixed(0) + 'GB/s'
                                    } else if (i >= t * k) {
                                        return (i / (t * t)).toFixed(0) + 'MB/s'
                                    } else if (i >= k) {
                                        return (i / t).toFixed(0) + 'KB/s'
                                    } else {
                                        return i.toFixed(0) + 'B/s'
                                    }
                                }
                            }
                        }
                    ],
                    series: this.series
                }
            }

            myChartDown.setOption(option)

            window.addEventListener('resize', function () {
                myChartDown.resize()
            })
        },

        /**
         *  获取速率信息。
         *
         *  @param  {none}
         *  @return {none}
         */
        async getRateInfo () {
            let para = Object.assign({}, this.form)
            para.type = 4
            para.page = 1
            await getMonitorInfo(para)
                .then((res) => {
                    if (res.data.code === 200) {
                        this.original = res.data.data
                    }
                    pushToData(this.original, this.dataUp, this.dataDown)
                }).catch(error => {
                    console.log(error)
                })

            function pushToData (val, up, down) {
                for (let i = 0; i < val.length; i++) {
                    let index = val[i].protocol - 1
                    up[index].value   = val[i].recv_rate
                    down[index].value = val[i].send_rate
                }
            }
        },

        /**
         *  更新界面函数。
         *
         *  @param  {none}
         *  @return {none}
         */
        async update () {
            optionUp = {
              xAxis:
                  {
                      type: 'category',
                      boundaryGap: true,
                      data: this.Axis
                  },
              series: this.seriesUp
            }
            optionDown = {
                xAxis:
                    {
                        type: 'category',
                        boundaryGap: true,
                        data: this.Axis
                    },
                series: this.seriesDown
            }

            await this.getRateInfo()
            let axisData = (new Date()).toLocaleTimeString().replace(/^\D*/, '')
            optionUp.xAxis.data.shift()
            optionUp.xAxis.data.push(axisData)
            for (let i = 0; i < this.dataUp.length; i++) {
                if (optionUp.series[i].data.length > 11) {
                    optionUp.series[i].data.shift()
                    optionDown.series[i].data.shift()
                }
                optionUp.series[i].data.push(this.dataUp[i].value)
                optionDown.series[i].data.push(this.dataDown[i].value)
            }
            myChartUp.setOption(optionUp)
            myChartDown.setOption(optionDown)
        },

        /**
         *  启动更新函数。
         *
         *  @param  {none}
         *  @return {none}
         */
        startUpdate () {
            this.update();
            this.timer = setInterval(() => {
                this.update();
            }, 2000)
        },

        drawLineChart () {
            this.drawLineChart1()
            this.drawLineChart2()
            this.startUpdate()
        }
    },

    watch: {
        '$store.state.app.language': function () {
            console.log(this.$store.state.app.language)
            this.drawLineChart()
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
        this.drawLineChart()
    },

    destroyed () {
        clearInterval(this.timer)
    },
}
</script>

<style scoped>
#chartLine1, #chartLine2 {
  width: 100%;
  height: 300px;
  margin-top: 30px;
}
</style>
