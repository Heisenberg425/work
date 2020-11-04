<template>
  <section>

    <div class="line_02">
      <span>{{$t('powerSetting.powerSetting')}}</span>
    </div>

    

    <!-- 表格-->
    <el-table :data="powerShow" :span-method="objectSpanMethod" border style="width: 100%; margin-top: 20px" :header-cell-style="headerStyle">
      <el-table-column prop="all" :label="$t('powerSetting.all')" width="180">
      </el-table-column>
      <el-table-column prop="detail" :label="$t('powerSetting.detail')">
      </el-table-column>

      <el-table-column property="operator" :label="$t('powerSetting.operator')" width="200">
        <template slot-scope="scope1">
          <el-switch  :on-text="$t('placeholder.on')" :off-text="$t('placeholder.off')" on-color="#5B7BFA" off-color="#dadde5" v-model="scope1.row.operator" @change = change1(scope1.$index,scope1.row)>
          </el-switch>
        </template>
      </el-table-column>

      <el-table-column property="visitor" :label="$t('powerSetting.visitor')" width="200">
        <template slot-scope="scope2">
          <el-switch :on-text="$t('placeholder.enable')" :off-text="$t('placeholder.disable')" on-color="#5B7BFA" off-color="#dadde5" v-model="scope2.row.visitor" @change = change2(scope2.$index,scope2.row)>
          </el-switch>
        </template>
      </el-table-column>
    </el-table>

   

  </section>
</template>


<script>
// import md5 from 'js-md5'
import { getpower, setpower } from '@/api/api.js'
export default {
  name: 'powerSetting',
  data() {
    return {
      rowspan1: 1,
      spanArr: [],
      sels: [],
      checkForm: {},
      list: [{ status: '' }],
      form: [],
      isAdd: false,
      powerNow: [],
      powerShow: [
        {
          type: 'MONITOR',
          all: '状态监控',
          detail: '状态监控',
          operator: false,
          visitor: false
        },
        {
          type: 'UDIAL',
          all: '网络设置',
          detail: '拨号设置',
          operator: false,
          visitor: false
        },
        {
          type: 'INTERFACE',
          all: '网络设置',
          detail: '网口配置',
          operator: false,
          visitor: false
        },
        {
          type: 'DHCPSERVER',
          all: '网络设置',
          detail: 'DHCP服务器设置',
          operator: false,
          visitor: false
        },
        {
          type: 'DNS',
          all: '网络设置',
          detail: 'DNS设置',
          operator: false,
          visitor: false
        },
        {
          type: 'ROUTE',
          all: '网络设置',
          detail: '静态路由配置',
          operator: false,
          visitor: false
        },
        {
          type: 'VLAN',
          all: '网络设置',
          detail: 'VLAN配置',
          operator: false,
          visitor: false
        },
        {
          type: 'VPN',
          all: '网络设置',
          detail: 'VPN配置',
          operator: false,
          visitor: false
        },
        {
          type: 'VND',
          all: '网络设置',
          detail: '虚拟网卡配置',
          operator: false,
          visitor: false
        },
        {
          type: 'NAT',
          all: '网络设置',
          detail: 'NAT配置',
          operator: false,
          visitor: false
        },
        {
          type: 'QoS',
          all: '网络设置',
          detail: 'QoS配置',
          operator: false,
          visitor: false
        },
        {
          type: 'DEVNAME',
          all: '系统设置',
          detail: '设备名称配置',
          operator: false,
          visitor: false
        },
        {
          type: 'TIMESET',
          all: '系统设置',
          detail: '时间配置',
          operator: false,
          visitor: false
        },
        {
          type: 'ACCOUNT',
          all: '系统设置',
          detail: '账户配置',
          operator: false,
          visitor: false
        },
        {
          type: 'AUTH',
          all: '系统设置',
          detail: '权限配置',
          operator: false,
          visitor: false
        },
        {
          type: 'UPDATEFIRMWARE',
          all: '系统设置',
          detail: '版本升级配置',
          operator: false,
          visitor: false
        },
        {
          type: 'HWINFO',
          all: '系统设置',
          detail: '硬件信息获取',
          operator: false,
          visitor: false
        },
        {
          type: 'RESET',
          all: '系统设置',
          detail: '复位配置',
          operator: false,
          visitor: false
        },
        {
          type: 'INTADA',
          all: '系统设置',
          detail: '网卡定位配置',
          operator: false,
          visitor: false
        },
        {
          type: 'FLOWCTL',
          all: '系统设置',
          detail: '流控配置',
          operator: false,
          visitor: false
        },
        {
          type: 'BEHAVIOUR',
          all: '系统设置',
          detail: '行为管理配置',
          operator: false,
          visitor: false
        },
        {
          type: 'ARP',
          all: '安全设置',
          detail: 'ARP绑定配置',
          operator: false,
          visitor: false
        },
        {
          type: 'ARP_DEFENSE',
          all: '安全设置',
          detail: 'ARP防御配置',
          operator: false,
          visitor: false
        },
        {
          type: 'NPF',
          all: '安全设置',
          detail: '规则防火墙配置',
          operator: false,
          visitor: false
        },
        {
          type: 'PING_TEST',
          all: '系统工具',
          detail: 'ping测试',
          operator: false,
          visitor: false
        },
        {
          type: 'SPEEDTEST',
          all: '系统工具',
          detail: '网速测试',
          operator: false,
          visitor: false
        },
        {
          type: 'UDIAL_LOG',
          all: '日志系统',
          detail: '拨号日志',
          operator: false,
          visitor: false
        },
        {
          type: 'OPERA_LOG',
          all: '日志系统',
          detail: '操作日志',
          operator: false,
          visitor: false
        },
        {
          type: 'VPN_LOG',
          all: '日志系统',
          detail: 'VPN日志',
          operator: false,
          visitor: false
        },
        {
          type: 'ASDEFENSE_LOG',
          all: '日志系统',
          detail: 'ARP防御日志',
          operator: false,
          visitor: false
        },
        {
          type: 'DHCPSERVER_LOG',
          all: '日志系统',
          detail: 'DHCP服务器日志',
          operator: false,
          visitor: false
        },
        {
          type: 'QUAGGA',
          all: '动态路由',
          detail: '动态路由配置',
          operator: false,
          visitor: false
        }
      ]
    }
  },

  methods: {
    headerStyle() {
      return this.header()
    },

    change1: function(index, row) {
      let para = {}
      para.type = row.type
      para.role = 'operaauth'
      if (row.operator === false) {
        para.handle = 'off'
      } else if (row.operator === true) {
        para.handle = 'on'
      } else { console.log('error') }
      setpower(para).then(res => {
        if (res.data.code === 200) {
          console.log('修改成功')
        } else {
          console.log('修改失败')
        }
      })
      this.getPowerInfo()
    },
    change2: function(index, row) {
      let para = {}
      para.type = row.type
      para.role = 'viewauth'
      if (row.visitor === false) {
        para.handle = 'off'
      } else if (row.visitor === true) {
        para.handle = 'on'
      } else { console.log('error') }
      setpower(para).then(res => {
        if (res.data.code === 200) {
          console.log('修改成功')
        } else {
          console.log('修改失败')
        }
      })
      this.getPowerInfo()
    },
    objectSpanMethod({ row, column, rowIndex, columnIndex }) {
      if (columnIndex === 0) {
        if (rowIndex === 0) {
          return {
            rowspan: 1,
            colspan: 1
          }
        } else if (rowIndex === 1) {
          return {
            rowspan: 10,
            colspan: 1
          }
        } else if (rowIndex === 11) {
          return {
            rowspan: 10,
            colspan: 1
          }
        } else if (rowIndex === 21) {
          return {
            rowspan: 3,
            colspan: 1
          }
        } else if (rowIndex === 24) {
          return {
            rowspan: 2,
            colspan: 1
          }
        } else if (rowIndex === 26) {
          return {
            rowspan: 5,
            colspan: 1
          }
        } else if (rowIndex === 31) {
          return {
            rowspan: 1,
            colspan: 1
          }
        } else {
          return {
            rowspan: 0,
            colspan: 1
          }
        }
      }
    },

    getPowerInfo() {
      getpower().then(res => {
        if (res.data.code === 200) {
          console.log('7777')
          console.log(res.data.operadata.MONITOR)
          if (res.data.viewdata.MONITOR === 'off') {
            this.powerShow[0].visitor = false
          } else {
            this.powerShow[0].visitor = true
          }
          if (res.data.viewdata.UDIAL === 'off') {
            this.powerShow[1].visitor = false
          } else {
            this.powerShow[1].visitor = true
          }
          if (res.data.viewdata.INTERFACE === 'off') {
            this.powerShow[2].visitor = false
          } else {
            this.powerShow[2].visitor = true
          }
          if (res.data.viewdata.DHCPSERVER === 'off') {
            this.powerShow[3].visitor = false
          } else {
            this.powerShow[3].visitor = true
          }
          if (res.data.viewdata.DNS === 'off') {
            this.powerShow[4].visitor = false
          } else {
            this.powerShow[4].visitor = true
          }
          if (res.data.viewdata.ROUTE === 'off') {
            this.powerShow[5].visitor = false
          } else {
            this.powerShow[5].visitor = true
          }
          if (res.data.viewdata.VLAN === 'off') {
            this.powerShow[6].visitor = false
          } else {
            this.powerShow[6].visitor = true
          }
          if (res.data.viewdata.VPN === 'off') {
            this.powerShow[7].visitor = false
          } else {
            this.powerShow[7].visitor = true
          }
          if (res.data.viewdata.VND === 'off') {
            this.powerShow[8].visitor = false
          } else {
            this.powerShow[8].visitor = true
          }
          if (res.data.viewdata.NAT === 'off') {
            this.powerShow[9].visitor = false
          } else {
            this.powerShow[9].visitor = true
          }
          if (res.data.viewdata.QoS === 'off') {
            this.powerShow[10].visitor = false
          } else {
            this.powerShow[10].visitor = true
          }
          if (res.data.viewdata.DEVNAME === 'off') {
            this.powerShow[11].visitor = false
          } else {
            this.powerShow[11].visitor = true
          }
          if (res.data.viewdata.TIMESET === 'off') {
            this.powerShow[12].visitor = false
          } else {
            this.powerShow[12].visitor = true
          }
          if (res.data.viewdata.ACCOUNT === 'off') {
            this.powerShow[13].visitor = false
          } else {
            this.powerShow[13].visitor = true
          }
          if (res.data.viewdata.AUTH === 'off') {
            this.powerShow[14].visitor = false
          } else {
            this.powerShow[14].visitor = true
          }
          if (res.data.viewdata.UPDATEFIRMWARE === 'off') {
            this.powerShow[15].visitor = false
          } else {
            this.powerShow[15].visitor = true
          }
          if (res.data.viewdata.HWINFO === 'off') {
            this.powerShow[16].visitor = false
          } else {
            this.powerShow[16].visitor = true
          }
          if (res.data.viewdata.RESET === 'off') {
            this.powerShow[17].visitor = false
          } else {
            this.powerShow[17].visitor = true
          }
          if (res.data.viewdata.INTADA === 'off') {
            this.powerShow[18].visitor = false
          } else {
            this.powerShow[18].visitor = true
          }
          if (res.data.viewdata.FLOWCTL === 'off') {
            this.powerShow[19].visitor = false
          } else {
            this.powerShow[19].visitor = true
          }
          if (res.data.viewdata.BEHAVIOUR === 'off') {
            this.powerShow[20].visitor = false
          } else {
            this.powerShow[20].visitor = true
          }
          if (res.data.viewdata.ARP === 'off') {
            this.powerShow[21].visitor = false
          } else {
            this.powerShow[21].visitor = true
          }
          if (res.data.viewdata.ARP_DEFENSE === 'off') {
            this.powerShow[22].visitor = false
          } else {
            this.powerShow[22].visitor = true
          }
          if (res.data.viewdata.NPF === 'off') {
            this.powerShow[23].visitor = false
          } else {
            this.powerShow[23].visitor = true
          }
          if (res.data.viewdata.PING_TEST === 'off') {
            this.powerShow[24].visitor = false
          } else {
            this.powerShow[24].visitor = true
          }
          if (res.data.viewdata.SPEEDTEST === 'off') {
            this.powerShow[25].visitor = false
          } else {
            this.powerShow[25].visitor = true
          }
          if (res.data.viewdata.UDIAL_LOG === 'off') {
            this.powerShow[26].visitor = false
          } else {
            this.powerShow[26].visitor = true
          }
          if (res.data.viewdata.OPERA_LOG === 'off') {
            this.powerShow[27].visitor = false
          } else {
            this.powerShow[27].visitor = true
          }
          if (res.data.viewdata.VPN_LOG === 'off') {
            this.powerShow[28].visitor = false
          } else {
            this.powerShow[28].visitor = true
          }
          if (res.data.viewdata.ASDEFENSE_LOG === 'off') {
            this.powerShow[29].visitor = false
          } else {
            this.powerShow[29].visitor = true
          }
          if (res.data.viewdata.DHCPSERVER_LOG === 'off') {
            this.powerShow[30].visitor = false
          } else {
            this.powerShow[30].visitor = true
          }
          if (res.data.viewdata.QUAGGA === 'off') {
            this.powerShow[31].visitor = false
          } else {
            this.powerShow[31].visitor = true
          }
          if (res.data.operadata.MONITOR === 'off') {
            this.powerShow[0].operator = false
          } else {
            this.powerShow[0].operator = true
          }
          if (res.data.operadata.UDIAL === 'off') {
            this.powerShow[1].operator = false
          } else {
            this.powerShow[1].operator = true
          }
          if (res.data.operadata.INTERFACE === 'off') {
            this.powerShow[2].operator = false
          } else {
            this.powerShow[2].operator = true
          }
          if (res.data.operadata.DHCPSERVER === 'off') {
            this.powerShow[3].operator = false
          } else {
            this.powerShow[3].operator = true
          }
          if (res.data.operadata.DNS === 'off') {
            this.powerShow[4].operator = false
          } else {
            this.powerShow[4].operator = true
          }
          if (res.data.operadata.ROUTE === 'off') {
            this.powerShow[5].operator = false
          } else {
            this.powerShow[5].operator = true
          }
          if (res.data.operadata.VLAN === 'off') {
            this.powerShow[6].operator = false
          } else {
            this.powerShow[6].operator = true
          }
          if (res.data.operadata.VPN === 'off') {
            this.powerShow[7].operator = false
          } else {
            this.powerShow[7].operator = true
          }
          if (res.data.operadata.VND === 'off') {
            this.powerShow[8].operator = false
          } else {
            this.powerShow[8].operator = true
          }
          if (res.data.operadata.NAT === 'off') {
            this.powerShow[9].operator = false
          } else {
            this.powerShow[9].operator = true
          }
          if (res.data.operadata.QoS === 'off') {
            this.powerShow[10].operator = false
          } else {
            this.powerShow[10].operator = true
          }
          if (res.data.operadata.DEVNAME === 'off') {
            this.powerShow[11].operator = false
          } else {
            this.powerShow[11].operator = true
          }
          if (res.data.operadata.TIMESET === 'off') {
            this.powerShow[12].operator = false
          } else {
            this.powerShow[12].operator = true
          }
          if (res.data.operadata.ACCOUNT === 'off') {
            this.powerShow[13].operator = false
          } else {
            this.powerShow[13].operator = true
          }
          if (res.data.operadata.AUTH === 'off') {
            this.powerShow[14].operator = false
          } else {
            this.powerShow[14].operator = true
          }
          if (res.data.operadata.UPDATEFIRMWARE === 'off') {
            this.powerShow[15].operator = false
          } else {
            this.powerShow[15].operator = true
          }
          if (res.data.operadata.HWINFO === 'off') {
            this.powerShow[16].operator = false
          } else {
            this.powerShow[16].operator = true
          }
          if (res.data.operadata.RESET === 'off') {
            this.powerShow[17].operator = false
          } else {
            this.powerShow[17].operator = true
          }
          if (res.data.operadata.INTADA === 'off') {
            this.powerShow[18].operator = false
          } else {
            this.powerShow[18].operator = true
          }
          if (res.data.operadata.FLOWCTL === 'off') {
            this.powerShow[19].operator = false
          } else {
            this.powerShow[19].operator = true
          }
          if (res.data.operadata.BEHAVIOUR === 'off') {
            this.powerShow[20].operator = false
          } else {
            this.powerShow[20].operator = true
          }
          if (res.data.operadata.ARP === 'off') {
            this.powerShow[21].operator = false
          } else {
            this.powerShow[21].operator = true
          }
          if (res.data.operadata.ARP_DEFENSE === 'off') {
            this.powerShow[22].operator = false
          } else {
            this.powerShow[22].operator = true
          }
          if (res.data.operadata.NPF === 'off') {
            this.powerShow[23].operator = false
          } else {
            this.powerShow[23].operator = true
          }
          if (res.data.operadata.PING_TEST === 'off') {
            this.powerShow[24].operator = false
          } else {
            this.powerShow[24].operator = true
          }
          if (res.data.operadata.SPEEDTEST === 'off') {
            this.powerShow[25].operator = false
          } else {
            this.powerShow[25].operator = true
          }
          if (res.data.operadata.UDIAL_LOG === 'off') {
            this.powerShow[26].operator = false
          } else {
            this.powerShow[26].operator = true
          }
          if (res.data.operadata.OPERA_LOG === 'off') {
            this.powerShow[27].operator = false
          } else {
            this.powerShow[27].operator = true
          }
          if (res.data.operadata.VPN_LOG === 'off') {
            this.powerShow[28].operator = false
          } else {
            this.powerShow[28].operator = true
          }
          if (res.data.operadata.ASDEFENSE_LOG === 'off') {
            this.powerShow[29].operator = false
          } else {
            this.powerShow[29].operator = true
          }
          if (res.data.operadata.DHCPSERVER_LOG === 'off') {
            this.powerShow[30].operator = false
          } else {
            this.powerShow[30].operator = true
          }
          if (res.data.operadata.QUAGGA === 'off') {
            this.powerShow[31].operator = false
          } else {
            this.powerShow[31].operator = true
          }
        } else { console.log('error') }
      })
    }
  },
  mounted() {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
    // this.getInfo()
    this.getPowerInfo()
  }
}
</script>

<style scoped>
.menusStateTrue {
  background: #5b7bfa;
  color: white;
}
.menusStateTrue:hover {
  background: #5b7bfa;
  color: white;
}
</style>


