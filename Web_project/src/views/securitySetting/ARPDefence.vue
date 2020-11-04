<template>
  <div>
    <div class="line_02">
      <span>{{ $t('securitySetting.ARPDefence') }}</span>
    </div>

    <div class="status-line">
      <span>{{ $t('securitySetting.status') }}</span>

      <span v-if="this.isInuse">{{ $t('securitySetting.inUse') }}</span>
      <span v-else>{{ $t('securitySetting.stopped') }}</span>

      <el-button
        v-if="this.isInuse"
        @click="disable"
        type="danger"
        class="button">{{ $t('securitySetting.stop') }}</el-button>
      <el-button
        v-else
        @click="enable"
        type="primary"
        class="button">{{ $t('securitySetting.start') }}</el-button>
    </div>

    <el-radio
      v-model="infoType"
      label='0'>{{ $t('securitySetting.showInfo') }}</el-radio>
    <el-radio
      v-model="infoType"
      label='1'>{{ $t('securitySetting.showConflict') }}</el-radio>

    <el-table
      :data="list"
      :header-cell-style="headerStyle">
      <el-table-column
        prop="ip"
        :label="$t('securitySetting.IP')"/>
      <el-table-column
        prop="oldmac"
        :label="$t('securitySetting.MACInfo')"/>
      <el-table-column
        prop="newmac"
        :label="$t('securitySetting.conflictMACAddr')"/>
      <el-table-column
        prop="flag"
        :label="$t('securitySetting.status')"/>
    </el-table>

    <el-pagination
      layout="total, prev, pager, next, jumper"
      @current-change="handleCurrentChange"
      :page-size="5"
      :total="total"
      style="float:right;"/>

  </div>
</template>

<script>
import { setARP, getARPInfo } from '../../api/api.js'
let stopSignal = ''
export default {
  name: 'ARPDefence',
  data () {
    return {
      isInuse: false,
      infoType: '0',
      list: [],
      currentPage: 1,
      pagecount: 5,
      total: 0
    }
  },
  watch: {
    infoType: function () {
      this.getARPInfomation()
    },
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.getARPInfomation()
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    handleCurrentChange (val) {
      this.currentPage = val
      this.getARPInfomation()
    },
    enable () {
      let para = {}
      para.en = 1
      setARP(para)
        .then(res => {
          if (res.data.code === 200) {
            this.isInuse = true
          }
        })
        .catch(err => {
          console.log(err)
        })
    },
    disable () {
      let para = {}
      para.en = 0
      setARP(para)
        .then(res => {
          if (res.data.code === 200) {
            this.isInuse = false
          }
        })
        .catch(err => {
          console.log(err)
        })
    },
    getARPInfomation () {
      let para = {}
      para.page = this.currentPage
      para.pagecount = this.pagecount
      para.type = Number(this.infoType)
      getARPInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.status === 'open') {
              this.isInuse = true

              this.list = res.data.data
              this.list.forEach(element => {
                if (element.newmac === '00:00:00:00:00:00') {
                  if (this.$store.state.app.language === 'zh') {
                    element.newmac = '无'
                  } else {
                    element.newmac = 'No'
                  }
                }
              })
            } else {
              this.isInuse = false
            }
            this.total = res.data.total
          }
        })
        .catch(err => {
          console.log(err)
        })
      console.log(this.isInuse)
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
    this.getARPInfomation()
    stopSignal = setInterval(this.getARPInfomation, 2000)
  },
  beforeDestroy () {
    clearInterval(stopSignal)
  }
}
</script>

<style lang="scss" scoped>
  .status-line {
    // height: 5rem;
    line-height: 5rem;
    .button {
      margin-left: 5rem;
    }
  }
  .el-radio {
    margin-top: 1rem;
    margin-bottom: 1rem;
  }
</style>
