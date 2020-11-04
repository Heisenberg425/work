<template>
  <div>
    <el-table
      :data="formDynamic"
      :header-cell-style="headerStyle">
      <el-table-column
        prop="ip"
        :label="$t('ARP.IP')"
        min-width="120"/>
      <el-table-column
        prop="oldmac"
        :label="$t('ARP.MAC')"
        min-width="120"/>
      <el-table-column
        prop="newmac"
        :label="$t('ARP.newmac')"
        min-width="120">
        
      </el-table-column>
    </el-table>

    <el-pagination
      @current-change="handleCurrentChangeDynamic"
      :current-page="currentPageDynamic"
      :page-size="5"
      layout="total, prev, pager, next, jumper"
      :total="this.totalDynamic"
      style="float:right"/>
  </div>
</template>

<script>
import { getARPInfo } from '@/api/api.js'
// import validate from '@/utils/rules.js'
export default {
  name: 'ARP',
  data () {
    return {

      totalDynamic: 0,
      currentPageDynamic: 1,

      formDynamic: [],

      labelWidth: '5rem'

      // rules: {
      //   ipaddr: [
      //     {
      //       validator: validate('ip', '请输入正确IP', 'IP不能为空')
      //     }
      //   ],
      //   hwaddr: [
      //     {
      //       validator: validate('mac', '请输入正确MAC', 'MAC不能为空')
      //     }
      //   ]
      // }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    // wyk1210更换接口，改为只显示ip，原mac，冲突mac
    // handleCurrentChangeDynamic (val) {
    //   let para = {}
    //   para.ipaddr = ''
    //   para.hwaddr = ''
    //   para.oper_type = 'arp'
    //   para.page = val
    //   getDynamicARP(para)
    //     .then(res => {
    //       if (res.data.code === 200) {
    //         this.formDynamic = res.data.data
    //       }
    //       this.currentPageDynamic = val
    //     })
    //     .catch(error => {
    //       console.log(error)
    //     })
    // },

    getDynamicARPInfo () {
      let para = {}
      // para.ipaddr = ''
      // para.hwaddr = ''
      // para.page = this.currentPageDynamic
      // getDynamicARP(para)
      //   .then(res => {
      //     if (res.data.code === 200) {
      //       if (res.data.length !== 0) {
      //         this.formDynamic = res.data.data
      //       } else {
      //         if (this.totalDynamic === 0) {
      //           this.totalDynamic = res.data.total
      //           this.formDynamic = res.data.data
      //         } else {
      //           this.currentPageDynamic -= 1
      //           this.getDynamicARPInfo()
      //         }
      //       }
      //     }
      //   })
      //   .catch(error => {
      //     console.log(error)
      //   })
      para.page = 1
      para.pagecount = 5
      para.type = 1
      getARPInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.formDynamic = res.data.data
            this.totalDynamic = res.data.total
          }
        })
        .catch(err => {
          console.log(err)
        })
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      // if (this.$store.state.app.language === 'zh') {
      //   this.rules.ipaddr[0].validator = validate('ip', '请输入正确IP', 'IP不能为空')
      //   this.rules.hwaddr[0].validator = validate('mac', '请输入正确MAC', 'MAC不能为空')
      // } else {
      //   this.rules.ipaddr[0].validator = validate('ip', 'Please input correct IP', 'Please input IP')
      //   this.rules.hwaddr[0].validator = validate('mac', 'Please input correct MAC', 'Please input MAC')
      // }
    }
  },
  mounted () {
    // if (this.$store.state.app.language === 'zh') {
    //   this.rules.ipaddr[0].validator = validate('ip', '请输入正确IP', 'IP不能为空')
    //   this.rules.hwaddr[0].validator = validate('mac', '请输入正确MAC', 'MAC不能为空')
    // } else {
    //   this.rules.ipaddr[0].validator = validate('ip', 'Please input correct IP', 'Please input IP')
    //   this.rules.hwaddr[0].validator = validate('mac', 'Please input correct MAC', 'Please input MAC')
    // }
    this.getDynamicARPInfo()
    console.log('执行了arp信息查询')
  }
}
</script>

<style lang="scss" scoped>
.warning {
  color: red;
}
.el-pagination {
  float: right;
}
</style>
