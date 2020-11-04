<template>
  <div id="echarts"/>
</template>

<script>
import { getMonitorInfo } from '@/api/api.js'
import flowMonitor from 'echarts'
require('echarts/theme/walden')

export default {
    name: 'PieCharts',
    data () {
        return {
            legend: {
                orient: 'vertical',
                x: 'right',
                data: ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他']
            },
            data: [
                {value: 0, name: 'HTTP协议'},
                {value: 0, name: '邮件服务'},
                {value: 0, name: '网络聊天'},
                {value: 0, name: '文件传输'},
                {value: 0, name: '其他'}
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
         *  绘制饼状图
         *
         *  @param  {none}
         *  @return {none}
         */
        drawCharts () {
            let myChart = flowMonitor.init(document.getElementById('echarts'), 'walden');
            let option  = {};
            if (this.$store.state.app.language === 'zh') {
                option = {
                    title: {
                        text: '流量信息',
                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: {
                        trigger: 'item',

                        formatter: function(p) {
                            console.log('formatter: ')
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
                        radius: ['50%', '60%'],
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
                        data: this.data
                    }
                    ]
                }
            } else {
                option = {
                    title: {
                        text: 'FlowInfo',

                        textStyle: {
                            fontSize: 14,
                            fontWeight: 'normal'
                        }
                    },
                    tooltip: {
                        trigger: 'item',
                            formatter: function(p) {
                            console.log('formatter: ')
                            console.log(p)
                            let str = p.seriesName + '<br/>' + p.name + ': ' + formatNum(p.value) + 'Bit' + '(' + p.percent + '%' + ')'
                            return  str
                        }
                    },
                    legend: this.legend,
                    series: [
                    {
                        name: 'Protocol Traffic',
                        type: 'pie',
                        radius: ['50%', '60%'],
                        avoidLabelOverlap: false,
                        label: {
                            normal: {
                                show: false,
                                position: 'center',
                                formatter: function(data) {
                                    return formatNum(data.value)
                                }
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
                        data: this.data
                    }
                    ] 
                }
            }

            /**
             *  对数据格式化
             *
             *  @param  {strNum}  需要格式化的数据
             *  @return {none}
             */
            function formatNum(strNum) {
                console.log('strNum：');
                console.log(strNum);

                if (strNum.length <= 3) {
                    return strNum;
                }
            
                if (!/^(\+|-)?(\d+)(\.\d+)?$/.test(strNum)) {
                    return strNum;
                }
            
                var a = RegExp.$1;
                var b = RegExp.$2;
                var c = RegExp.$3;
                var re = new RegExp();
                re.compile('(\\d)(\\d{3})(,|$)');
            
                while (re.test(b)) {
                    b = b.replace(re, '$1,$2$3');
                }

                return a + '' + b + '' + c;
            }

            myChart.setOption(option);

            window.addEventListener('resize', function () {
                myChart.resize()
            })
        },

        /**
         *  获取流量信息，用于绘制饼状图
         *
         *  @param  {none}
         *  @return {none}
         */
        getFlowInfo () {
            let para  = Object.assign({}, this.form);
            para.type = 4;
            para.page = 1;

            getMonitorInfo(para).then((res) => {
                if (res.data.code === 200) {
                    this.original = res.data.data;
                }
                puthToData(this.original, this.data);
            }).then(() => {
                this.drawCharts();
            }).catch(err => {
                console.log(err);
            })

            /**
             * 当TMT_LINE_APP时，返回Protocol值取:
             * app_type_http = 1,
             * app_type_mail = 2,
             * app_type_chat = 3,
             * app_type_transfer = 4,
             * app_type_others = 5,
             */
            function puthToData (val, data) {
                for (let i = 0; i < val.length; i++) {                 
                    let index = val[i].protocol - 1;
                    data[index].value = val[i].recv_total_length;
                }
            }
        },

        translate() {
            if (this.$store.state.app.language === 'en') {
                console.log('pie en')
                this.legend.data = ['HTTP', 'Email', 'Chat', 'Transmission', 'Other']
                this.data = [
                    {value: 0, name: 'HTTP'},
                    {value: 0, name: 'Email'},
                    {value: 0, name: 'Chat'},
                    {value: 0, name: 'Transmission'},
                    {value: 0, name: 'Other'}
                ]
            } else {
                console.log('pie zh')
                this.legend.data = ['HTTP协议', '邮件服务', '网络聊天', '文件传输', '其他']
                this.data = [
                  {value: 0, name: 'HTTP协议'},
                  {value: 0, name: '邮件服务'},
                  {value: 0, name: '网络聊天'},
                  {value: 0, name: '文件传输'},
                  {value: 0, name: '其他'}
                ]
            }
        }
    },

    watch: {
        '$store.state.app.language': function () {
            console.log(this.$store.state.app.language);
            this.translate();
            this.getFlowInfo();
            this.drawCharts();
        }
    },

    mounted () {
        this.translate();
        this.getFlowInfo();
        this.drawCharts();
    }
}
</script>

<style scoped lang="scss">
</style>
