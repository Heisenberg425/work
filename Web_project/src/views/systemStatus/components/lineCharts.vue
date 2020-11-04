<template>
  <div>
    <div id="chartLine1" style="width:100%; height:160px; margin-top: 10px"/>
    <div id="chartLine2" style="width:100%; height:160px; margin-top: 10px"/>
  </div>
</template>

<script>

import lineCharts from 'echarts'

let myChart1;
let myChart2;
let option1 = {};
let option2 = {};

export default {
    name: 'LineCharts',
    data () {
        return {
            option: {
                title: {
                    text: '上行速率',
                    textStyle: {
                        fontWeight: 'normal',
                        fontSize: 14
                    }
                },
                title1: {
                    text: '下行速率',
                    textStyle: {
                        fontWeight: 'normal',
                        fontSize: 14
                    }
                },
                tooltip: {
                    trigger: 'axis',
                    formatter: function (params) {
                        params = params[0];

                        const k = 1000;
                        const t = 1024;
                        let i = params.value[1]
                        if (i >= k * t * t) {
                            i = (i / (t * t * t)).toFixed(2) + 'GB/s';
                        } else if (i >= t * k) {
                            i = (i / (t * t)).toFixed(2) + 'MB/s';
                        } else if (i >= k) {
                            i = (i / t).toFixed(2) + 'KB/s';
                        } else {
                            i = i.toFixed() + 'B/s';
                        }

                        let date = new Date(params.name)
                        return (date.getHours() + ':' + date.getMinutes() + ' : ' + i)
                    },
                    axisPointer: {
                        animation: false
                    }
                },
                grid: {
                    left: '6%',
                    right: '3%',
                    bottom: '12%',
                    containLabel: false
                },
                xAxis: {
                    type: 'time',
                    splitLine: {
                        show: false
                    },
                    axisLine: {
                        lineStyle: {
                            color: '#CD0000'
                        }
                    }
                },
                yAxis: {
                    type: 'value',
                    boundaryGap: [0, '0%'],
                    splitLine: {
                        show: false
                    },
                    splitNumber: 3, // 分割 端数
                    axisLabel: {
                        formatter: function (value, index) {
                            const k = 1000
                            const t = 1024
                            let i = value
                            if (i >= k * t * t) {
                                return (i / (t * t * t)).toFixed(2) + 'GB/s';
                            } else if (i >= t * k) {
                                return (i / (t * t)).toFixed(2) + 'MB/s';
                            } else if (i >= k) {
                                return (i / t).toFixed(2) + 'KB/s';
                            } else {
                                return i.toFixed() + 'B/s';
                            }
                        }
                    }
                },
                series: [
                    {
                        name: '下行速率',
                        type: 'line',
                        lineStyle: {
                            width: 0.5,
                            type: 'dotted'
                        },
                        showSymbol: false,
                        hoverAnimation: false,
                    }
                ]
            }
        }
    },

    methods: {

        /**
         *  绘制折线图 1
         *
         *  @param  {none}
         *  @return {none}
         */
        drawLineChart1: function () {
            myChart1 = lineCharts.init(
              document.getElementById('chartLine1'),
              'walden'
            )

            let para = this.$store.state.app.systemData
            let data = []
            let now = new Date()

            for (let i = 0; i < 300; i++) {
                data.splice(0, 0, newData1(0))
            }
            console.log('data: ')
            console.log(data)
            
            if (this.$store.state.app.language === 'en') {
                option1 = {
                    title: this.option.title,
                    tooltip: this.option.tooltip,
                    grid: this.option.grid,
                    xAxis: this.option.xAxis,
                    yAxis: this.option.yAxis,
                    series: [
                    {
                        name: 'Up Rate',
                        type: 'line',
                        lineStyle: {
                            width: 0.5,
                            type: 'dotted'
                        },
                        showSymbol: false,
                        hoverAnimation: false,
                        data: data
                    }
                    ]
                }
          } else {
              option1 = {
                  title: this.option.title,
                  tooltip: this.option.tooltip,
                  grid: this.option.grid,
                  xAxis: this.option.xAxis,
                  yAxis: this.option.yAxis,
                  series: [
                      {
                        name: '上行速率',
                        type: 'line',
                        lineStyle: {
                          width: 0.5,
                          type: 'dotted'
                        },
                        showSymbol: false,
                        hoverAnimation: false,
                        data: data
                      }
                  ]
                }
            }

            function newData1 (val) {
                now = new Date(+now - 1000);
                return {
                    name: now.toString(),
                    value: [
                        now,
                        val
                    ]
                }
            }
     
            function newData2 (val) {
                now = new Date()
                return {
                    name: now.toString(),
                    value: [
                        now,
                        val
                    ]
                }
            }
            
            console.log('option12')
            console.log(option1)
            myChart1.setOption(option1)

            let mychart1 = setInterval(function () {
                data.shift()
                console.log('para.up')
                console.log(para.up)
                data.push(newData2(para.up))
                myChart1.setOption(option1)
            }, 2000)

            this.$store.state.app.timers.mychart1 = mychart1

            window.addEventListener('resize', function () {
            myChart1.resize()
        })
    },

    /**
     *  绘制折线图 2
     *
     *  @param  {none}
     *  @return {none}
     */
    drawLineChart2: function () {
        myChart2 = lineCharts.init(
            document.getElementById('chartLine2'),
            'walden'
        )

        let para = this.$store.state.app.systemData
        let data = []
        let now = new Date()

        for (let i = 0; i < 300; i++) {
           data.splice(0, 0, newData1(0))
        }

        function newData1 (val) {
            now = new Date(+now - 1000)
            return {
                name: now.toString(),
                value: [
                    now,
                    val
                ]
            }
        }

        function newData2 (val) {
            now = new Date()
            return {
                name: now.toString(),
                value: [
                    now,
                    val
                ]
            }
        }

        if (this.$store.state.app.language === 'en') {
            option2 = {
                title: this.option.title1,
                tooltip: this.option.tooltip,
                grid: this.option.grid,
                xAxis: this.option.xAxis,
                yAxis: this.option.yAxis,
                series: [
                    {
                      name: 'Down Rate',
                      type: 'line',
                      showSymbol: false,
                      hoverAnimation: false,
                      data: data
                    }
                ]
            }
        } else {
            option2 = {
                title: this.option.title1,
                tooltip: this.option.tooltip,
                grid: this.option.grid,
                xAxis: this.option.xAxis,
                yAxis: this.option.yAxis,
                series: [
                {
                    name: '下行速率',
                    type: 'line',
                    showSymbol: false,
                    hoverAnimation: false,
                    data: data
                }
                ]
            }
        }

        myChart2.setOption(option2)

        let linecharts2 = setInterval(function () {
            data.shift()
            data.push(newData2(para.down))
            console.log('data')
            console.log(data)
            myChart2.setOption(option2)
            console.log('option2')
            console.log(option2)
        }, 2000)

        this.$store.state.app.timers.linecharts2 = linecharts2
        window.addEventListener('resize', function () {
            myChart2.resize()
        })
    },

    translate () {
        if (this.$store.state.app.language === 'en') {

            option1.title.text = 'Up Rate'
            option2.title.text = 'Down Rate'
        }
        if (this.$store.state.app.language === 'zh') {
         
            option1.title.text = '上行速率'
            option2.title.text = '下行速率'
        }

            myChart1.setOption(option1)
            myChart2.setOption(option2)
        }
    },

    watch: {
        '$store.state.app.language': function () {
          console.log(this.$store.state.app.language)
          this.translate()
        }
    },

    mounted () {
        this.drawLineChart1()
        this.drawLineChart2()
        this.translate()
    }
}
</script>
