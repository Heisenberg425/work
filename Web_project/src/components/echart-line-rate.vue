<template>
  <ve-line
    :data="chartData"
    :settings="chartSettings"
    :grid="grid"
    :y-axis="yAxis"
    :legend="legend"
    :events="chartEvents"
    height="90%"
    width="100%;" />
</template>

<script>
import { getMonitorInfo } from '@/api/api.js'
export default {
  name: 'UpRate',
  data () {
    this.grid = {
      left: '1%',
      right: '5%',
      bottom: '10%'
    }

    this.yAxis = {
      type: 'value',
      boundaryGap: ['0%', '0%'],
      splitNumber: 3, // 分割 端数
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

    if (this.$store.state.app.language === 'zh') {
      this.chartSettings = {
        xAxisType: 'time',

        // 填入数据的别名对照表
        labelMap: {
          time: '时间',
          upRate: '上行速率',
          downRate: '下行速率'
        }
      }
      this.legend = {
        data: ['上行速率', '下行速率'],
        selected: {
          '上行速率': true,
          '下行速率': true
        }
      }
    } else {
      this.chartSettings = {
        xAxisType: 'time',

        // 填入数据的别名对照表
        labelMap: {
          time: 'Time',
          upRate: 'Up Rate',
          downRate: 'Down Rate'
        }
      }
      this.legend = {
        data: ['Up Rate', 'Down Rate'],
        selected: {
          'UpRate': true,
          'DownRate': true
        }
      }
    }

    this.chartEvents = {
      legendselectchanged: function (e) {
        changeSelected(e)
      }
    }

    let legend = this.legend
    function changeSelected (val) {
      legend.selected = val.selected
      // console.log(legend)
    }

    return {
      uprate: '',
      downrate: '',
      rateData: [],
      timer: '', // 定时器

      chartData: {
        columns: ['time', 'upRate', 'downRate'],
        rows: this.rateData
        // rows: [
        //   {time: '2018-05-06 12:12:02', value: 10},
        //   {time: '2018-05-06 12:12:03', value: 20},
        //   {time: '2018-05-06 12:12:04', value: 13}
        // ]
      }
    }
  },

  methods: {
    fillXaxisData () {
      let transformDateFormat = this.transformDateFormat

      // 填充5分钟的数据，默认都为0
      for (let i = 0; i < 30; i++) {
        this.rateData.push(newData(0, i))
      }

      this.chartData.rows = this.rateData

      // 该函数获得下一秒的时间，传入参数为该时间点的数据和该时间点
      function newData (val, i) {
        let now = new Date()
        let newNow = new Date(+now - (30 - i) * 1000)

        let time = transformDateFormat(newNow)
        return {
          time: time,
          upRate: val,
          downRate: val
        }
      }
    },

    // 填充实时数据，循环调用，每次只填充一个值
    fillRealTimeData () {
      let now = new Date()

      this.rateData.shift()

      let time = this.transformDateFormat(now)
      // let val = Math.random() * 10000
      // let upRate = ''// 获取上行速率
      // let downRate = '' // 获取下行速率
      // console.log('this.$store.state.app.systemData.up')
      // console.log('this.$store.state.app.systemData.down')
      // console.log(this.$store.state.app.systemData.up)
      // console.log(this.$store.state.app.systemData.down)
      // if (this.$store.state.app.systemData.up === '' || this.$store.state.app.systemData.down === '') {
      console.log('novalue')
      let para = {}
      para.page = 1
      para.type = 5
      getMonitorInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.uprate = res.data.data[0].send_rate
            this.downrate = res.data.data[0].recv_rate
          }
        })
        .catch(error => {
          console.log(error)
        })
      // }
      console.log('upRate')
      console.log(this.uprate)
      console.log('downRate')
      console.log(this.downrate)
      this.rateData.push(
        {
          time: time,
          upRate: this.uprate,
          downRate: this.downrate
        })
      this.chartData.rows = this.rateData
    },

    // 修改日期的格式 为 yyyy-MM-dd hh:mm:ss
    transformDateFormat (val) {
      let dformat = [
        val.getFullYear(),
        addZero(val.getMonth() + 1),
        addZero(val.getDate())
      ].join('-') +
        ' ' +
      [
        addZero(val.getHours()),
        addZero(val.getMinutes()),
        addZero(val.getSeconds())
      ].join(':')
      return dformat

      function addZero (val) {
        if (val.toString().length === 1) {
          val = '0' + val
        }
        return val
      }
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log('this.$store.state.app.language')
      console.log(this.$store.state.app.language)
      window.location.reload()
    }
  },
  mounted () {
    let para = {}
    para.page = 1
    para.type = 5
    getMonitorInfo(para)
      .then(res => {
        if (res.data.code === 200) {
          this.uprate = res.data.data[0].send_rate
          this.downrate = res.data.data[0].recv_rate
        }
      })
      .catch(error => {
        console.log(error)
      })
    // if (this.$store.state.app.language === 'zh') {
    //   this.legend = {
    //     data: ['上行速率', '下行速率'],
    //     selected: {
    //       '上行速率': true,
    //       '下行速率': true
    //     }
    //   }
    // } else {
    //   this.legend = {
    //     data: ['UpRate', 'DownRate'],
    //     selected: {
    //       'UpRate': true,
    //       'DownRate': true
    //     }
    //   }
    // }

    this.fillXaxisData()
    this.timer = setInterval(this.fillRealTimeData, 2000)
  },

  destroyed () {
    clearInterval(this.timer)
  }

}
</script>

<style scoped>
</style>
