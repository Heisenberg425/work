<template>
  <div id="map"/>
</template>

<script>
import kidVPN from 'echarts'
import axios from 'axios'
import { getCityInfo } from '@/api/api.js'
// import chinaJson from '../../static/china.json';

export default {
  name: 'KidVPN',
  timer:0,

  data() {
    return {
      testData: {}
    }
  },

  methods: {
    getTestData: async function() {
      let getPostionInfo

      // let reg = new RegExp('"', 'g')

      /**
       * 此处需要的数据为VPN地址的列表，
       * IP解析是否由后台进行
       */
      await getCityInfo()
        .then(res => {
          if (res.data.code === 200) {
            getPostionInfo = res.data
          }
        })
        .catch(err => {
          console.log(err)
        })

      this.drawMap(getPostionInfo)
    },

    drawMap: function(getPostionInfo) {
      console.log('in drawmap')
      let IPMap = kidVPN.init(document.getElementById('map'))

      let chinaJson

      let className = []

      axios.get('../../static/china.json').then(res => {
        chinaJson = res.data
        // console.log(chinaJson);
        setMap(chinaJson)
      })

      function setMap() {
        kidVPN.registerMap('china', chinaJson) // 注册地图

        // 参数实例 https://gallery.echartsjs.com/editor.html?c=xBkrP1lbqG
        let geoCoordMap = {
          上海: [121.4648, 31.2891],
          东莞: [113.8953, 22.901],
          东营: [118.7073, 37.5513],
          中山: [113.4229, 22.478],
          临汾: [111.4783, 36.1615],
          临沂: [118.3118, 35.2936],
          丹东: [124.541, 40.4242],
          丽水: [119.5642, 28.1854],
          乌鲁木齐: [87.9236, 43.5883],
          佛山: [112.8955, 23.1097],
          保定: [115.0488, 39.0948],
          兰州: [103.5901, 36.3043],
          包头: [110.3467, 41.4899],
          北京: [116.4551, 40.2539],
          北海: [109.314, 21.6211],
          南京: [118.8062, 31.9208],
          南宁: [108.479, 23.1152],
          南昌: [116.0046, 28.6633],
          南通: [121.1023, 32.1625],
          厦门: [118.1689, 24.6478],
          台州: [121.1353, 28.6688],
          合肥: [117.29, 32.0581],
          呼和浩特: [111.4124, 40.4901],
          咸阳: [108.4131, 34.8706],
          哈尔滨: [127.9688, 45.368],
          唐山: [118.4766, 39.6826],
          嘉兴: [120.9155, 30.6354],
          大同: [113.7854, 39.8035],
          大连: [122.2229, 39.4409],
          天津: [117.4219, 39.4189],
          太原: [112.3352, 37.9413],
          威海: [121.9482, 37.1393],
          宁波: [121.5967, 29.6466],
          宝鸡: [107.1826, 34.3433],
          宿迁: [118.5535, 33.7775],
          常州: [119.4543, 31.5582],
          广州: [113.5107, 23.2196],
          廊坊: [116.521, 39.0509],
          延安: [109.1052, 36.4252],
          张家口: [115.1477, 40.8527],
          徐州: [117.5208, 34.3268],
          德州: [116.6858, 37.2107],
          惠州: [114.6204, 23.1647],
          成都: [103.9526, 30.7617],
          扬州: [119.4653, 32.8162],
          承德: [117.5757, 41.4075],
          拉萨: [91.1865, 30.1465],
          无锡: [120.3442, 31.5527],
          日照: [119.2786, 35.5023],
          昆明: [102.9199, 25.4663],
          杭州: [119.5313, 29.8773],
          枣庄: [117.323, 34.8926],
          柳州: [109.3799, 24.9774],
          株洲: [113.5327, 27.0319],
          武汉: [114.3896, 30.6628],
          汕头: [117.1692, 23.3405],
          江门: [112.6318, 22.1484],
          沈阳: [123.1238, 42.1216],
          沧州: [116.8286, 38.2104],
          河源: [114.917, 23.9722],
          泉州: [118.3228, 25.1147],
          泰安: [117.0264, 36.0516],
          泰州: [120.0586, 32.5525],
          济南: [117.1582, 36.8701],
          济宁: [116.8286, 35.3375],
          海口: [110.3893, 19.8516],
          淄博: [118.0371, 36.6064],
          淮安: [118.927, 33.4039],
          深圳: [114.5435, 22.5439],
          清远: [112.9175, 24.3292],
          温州: [120.498, 27.8119],
          渭南: [109.7864, 35.0299],
          湖州: [119.8608, 30.7782],
          湘潭: [112.5439, 27.7075],
          滨州: [117.8174, 37.4963],
          潍坊: [119.0918, 36.524],
          烟台: [120.7397, 37.5128],
          玉溪: [101.9312, 23.8898],
          珠海: [113.7305, 22.1155],
          盐城: [120.2234, 33.5577],
          盘锦: [121.9482, 41.0449],
          石家庄: [114.4995, 38.1006],
          福州: [119.4543, 25.9222],
          秦皇岛: [119.2126, 40.0232],
          绍兴: [120.564, 29.7565],
          聊城: [115.9167, 36.4032],
          肇庆: [112.1265, 23.5822],
          舟山: [122.2559, 30.2234],
          苏州: [120.6519, 31.3989],
          莱芜: [117.6526, 36.2714],
          菏泽: [115.6201, 35.2057],
          营口: [122.4316, 40.4297],
          葫芦岛: [120.1575, 40.578],
          衡水: [115.8838, 37.7161],
          衢州: [118.6853, 28.8666],
          西宁: [101.4038, 36.8207],
          西安: [109.1162, 34.2004],
          贵阳: [106.6992, 26.7682],
          连云港: [119.1248, 34.552],
          邢台: [114.8071, 37.2821],
          邯郸: [114.4775, 36.535],
          郑州: [113.4668, 34.6234],
          鄂尔多斯: [108.9734, 39.2487],
          重庆: [107.7539, 30.1904],
          金华: [120.0037, 29.1028],
          铜川: [109.0393, 35.1947],
          银川: [106.3586, 38.1775],
          镇江: [119.4763, 31.9702],
          长春: [125.8154, 44.2584],
          长沙: [113.0823, 28.2568],
          长治: [112.8625, 36.4746],
          阳泉: [113.4778, 38.0951],
          青岛: [120.4651, 36.3373],
          韶关: [113.7964, 24.7028],
          局域网: [140, 31]
        }

        // 测试数据 （后端传过来的数据）
        // var testpostion = {
        //   data: [
        //     {
        //       cli_ip: '10.9.0.254',
        //       cli_pos: '南京',
        //       ser_ip: '192.168.11.2',
        //       ser_pos: '北京'
        //     },
        //     {
        //       cli_ip: '10.9.0.254',
        //       cli_pos: '南京',
        //       ser_ip: '192.168.11.2',
        //       ser_pos: '广州'
        //     },
        //     {
        //       cli_ip: '10.9.0.254',
        //       cli_pos: '局域网',
        //       ser_ip: '192.168.11.2',
        //       ser_pos: '南京'
        //     },
        //     {
        //       cli_ip: '10.9.0.254',
        //       cli_pos: '青岛',
        //       ser_ip: '192.168.11.2',
        //       ser_pos: '南京'
        //     }
        //   ],
        //   total: 3
        // }

        // 测试数据 经过 step1 后的数据格式
        // var TestData = [
        //   [{ name: '北京' }, { name: '南京', value: 90 }],
        //   [{ name: '长沙' }, { name: '南京', value: 90 }],
        //   [{ name: '局域网' }, { name: '南京', value: 90 }],
        //   [{ name: '南京' }, { name: '上海', value: 95 }],
        //   [{ name: '南京' }, { name: '广州', value: 90 }],
        //   [{ name: '南京' }, { name: '北京', value: 90 }]
        // ]

        // 测试数据 经过 step2 后的数据格式
        // var lastdata = [
        //   [
        //     [{ name: '南京' }, { name: '上海', value: 95 }],
        //     [{ name: '南京' }, { name: '广州', value: 90 }],
        //     [{ name: '南京' }, { name: '北京', value: 90 }]
        //   ],
        //   [[{ name: '北京' }, { name: '南京', value: 90 }]],
        //   [[{ name: '长沙' }, { name: '局域网', value: 90 }]],
        //   [[{ name: '局域网' }, { name: '银川', value: 90 }]]
        // ]

        // step1 ：用于将后端传过来的数据转为 echart 一开始需要的格式
        var convertPostion = function(positiondata) {
          var postionVal = []

          for (var i = 0; i < positiondata.total; i++) {
            var tmpVal = []
            var positiondataItem = positiondata.data[i]

            tmpVal.push({
              name: positiondataItem.ser_pos
            })

            tmpVal.push({
              name: positiondataItem.cli_pos,
              value:
                'S: ' +
                positiondataItem.ser_ip +
                ' C: ' +
                positiondataItem.cli_ip
            })

            postionVal.push(tmpVal)
          }
          return postionVal
        }

        // step 2:根据服务器地址进行分类

        var PostionClass = function(positiondata) {
          var AllPositionVal = []
          var lastname = 'none'
          var lasttmp = []
          var first = 1

          for (var j = 0, len = positiondata.length; j < len; j++) {
            if (positiondata[j][0].name !== lastname) {
              if (!first) {
                className.push(lastname)
                AllPositionVal.push(lasttmp)
              }
              var newtmp = []
              newtmp.push(positiondata[j])
              lasttmp = newtmp
              lastname = positiondata[j][0].name
              first = 0
            } else {
              lasttmp.push(positiondata[j])
            }
          }

          className.push(lastname)

          AllPositionVal.push(lasttmp)

          return AllPositionVal
        }

        // step3 ：用于 echart 一开始需要的格式再次进行一次转换，转为组件需要的格式
        var convertData = function(data) {
          console.log(data)
          var res = []
          for (var i = 0; i < data.length; i++) {
            var dataItem = data[i]
            var fromCoord = geoCoordMap[dataItem[0].name]
            var toCoord = geoCoordMap[dataItem[1].name]
            if (fromCoord && toCoord) {
              res.push({
                fromName: dataItem[0].name,
                toName: dataItem[1].name,
                coords: [fromCoord, toCoord],
                value: dataItem[1].value
              })
            }
          }
          return res
        }

        let color = [
          '#a6c84c',
          '#ffa022',
          '#12bee9',
          '#13c84c',
          '#14a022',
          '#15bee9',
          '#16c84c',
          '#17a022',
          '#18bee9',
          '#19c84c',
          '#1aa022',
          '#1bbee9'
        ]
        let series = []
        let PostionSeries = []

        var posval = convertPostion(getPostionInfo)
        var PositionToShowVal = PostionClass(posval)

        for (var i = 0, len = PositionToShowVal.length; i < len; i++) {
          console.log('className[i] = ' + className[i])
          console.log('iterm[i] = ' + PositionToShowVal[i])
          PostionSeries.push(
            {
              name: className[i],
              type: 'lines',
              zlevel: 1,
              effect: {
                show: true,
                period: 6,
                trailLength: 0.7,
                color: '#fff',
                symbolSize: 3
              },
              lineStyle: {
                normal: {
                  color: color[(i + 12) % 12],
                  width: 0,
                  curveness: 0.2
                }
              },
              data: convertData(PositionToShowVal[i])
            },
            {
              name: className[i],
              type: 'lines',
              zlevel: 2,
              symbol: ['none', 'arrow'],
              symbolSize: 10,
              effect: {
                show: true,
                period: 6,
                trailLength: 0,
                // symbol: planePath,
                symbol: 'arrow', // 箭头图标
                symbolSize: 5 // 图标大小
              },
              lineStyle: {
                normal: {
                  color: color[(i + 12) % 12],
                  width: 1,
                  opacity: 0.6,
                  curveness: 0.2
                }
              },
              data: convertData(PositionToShowVal[i])
            },
            {
              name: className[i],
              type: 'effectScatter',
              coordinateSystem: 'geo',
              zlevel: 2,
              rippleEffect: {
                // 涟漪特效
                period: 4, // 动画时间，值越小速度越快
                brushType: 'stroke', // 波纹绘制方式 stroke, fill
                scale: 4 // 波纹圆环最大限制，值越大波纹越大
              },
              label: {
                normal: {
                  show: true,
                  position: 'right',
                  symbol: 'circle',
                  symbolSize: function(val) {
                    return 5 + val[2] * 5 // 圆环大小
                  },
                  formatter: '{b}'
                }
              },
              symbol: 'circle',
              symbolSize: function(val) {
                return val[2] / 8
              },
              itemStyle: {
                normal: {
                  color: color[(i + 12) % 12]
                }
              },
              data: PositionToShowVal[i].map(function(dataItem) {
                return {
                  name: dataItem[1].name,
                  value: geoCoordMap[dataItem[1].name].concat([
                    dataItem[1].value
                  ])
                }
              })
            }
          )
        }

        let option = {
          backgroundColor: '#404a59',
          title: {
            text: 'VPN 连接地信息',
            subtext: '中国地区(箭头由服务器指向客户端)',
            left: 'center',
            textStyle: {
              color: '#fff'
            }
          },
          // 用于鼠标点击时显示的信息
          tooltip: {
            trigger: 'item',
            formatter: function(params, ticket, callback) {
              console.log(params)
              if (params.seriesType === 'effectScatter') {
                return '节点：' + params.data.name
              } else if (params.seriesType === 'lines') {
                return (
                  'S: ' +
                  params.data.fromName +
                  ' C: ' +
                  params.data.toName +
                  '<br />' +
                  params.data.value
                )
              } else {
                return params.name
              }
            }
          },
          legend: {
            orient: 'vertical',
            top: 'bottom',
            left: 'right',
            data: className,
            textStyle: {
              color: '#fff'
            },
            selectedMode: 'multiple'
          },
          geo: {
            map: 'china',
            label: {
              emphasis: {
                show: true,
                color: '#fff'
              }
            },
            roam: true,
            itemStyle: {
              normal: {
                areaColor: '#323c48',
                borderColor: '#404a59'
              },
              emphasis: {
                areaColor: '#2a333d'
              }
            }
          },
          series: PostionSeries
        }

        console.log('series', series)
        IPMap.setOption(option)
      }
    }
  },

  updateCityInfo () {
      this.getCityInfo();
      this.timer = setInterval(() => {
          this.getCityInfo();
      }, 5000)
  },

  mounted() {
    // this.drawMap()
    this.getTestData()
    this.updateCityInfo();
  },

  destroyed () {
      clearInterval(this.timer)
  },
}
</script>

<style scoped>
#map {
  height: 500px;
  width: 100%;
  margin-top: 20px;
}
</style>
