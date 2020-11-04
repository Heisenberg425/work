<template>
  <div>
    <div class="line_02">
      <span>{{ $t('vpnLog.log') }}</span>
    </div>

    <!--工具条-->
    <el-col :span="24">
      <el-form
        :model="request"
        :inline="true"
        ref="request">
        <el-form-item prop="time">
          <el-date-picker
            v-model="request.time"
            type="datetimerange"
            :range-separator="$t('placeholder.rangeSeparator')"
            :start-placeholder="$t('placeholder.startDate')"
            :end-placeholder="$t('placeholder.endDate')"
            @change="getTime"/>
        </el-form-item>
        <el-form-item prop="keyword">
          <el-input
            :placeholder="$t('vpnLog.search')"
            v-model="request.vndid">
            <i
              slot="prefix"
              class="el-input__icon el-icon-search"/>>
          </el-input>
        </el-form-item>
        <el-form-item>
          <el-button
            type="primary"
            @click="sendTime">{{ $t('vpnLog.confirm') }}</el-button> 
        </el-form-item>
        <el-form-item class="clear">
          <el-button @click="empty" type="danger">{{ $t('vpnLog.clear') }}</el-button>
        </el-form-item>
        <el-form-item>
          <span
            class="tip"
            v-if="request.time === ''">{{ $t('vpnLog.provide') }}</span>
        </el-form-item>
      </el-form>
    </el-col>

    <el-table
      :data="form"
      :header-cell-style="headerStyle">
      <el-table-column
        type="index"
        width="60"/>
      <el-table-column
        prop="time"
        :label="$t('log.time')"/>
      <el-table-column
        prop="vndid"
        label='VND_ID'/>
      <el-table-column
        prop="event"
        :label="$t('log.detail')"/>
    </el-table>

    <el-pagination
      @current-change="handleCurrentChange"
      :current-page="currentPage"
      :page-size="10"
      layout="total, prev, pager, next, jumper"
      :total="this.total"
      style="float:right"/>

  </div>
</template>

<script>
import { getVPNLog, vpnLogDelAction } from '../../api/api.js'
export default {
  name: 'Log',
  data () {
    return {
      currentPage: 1,
      total: 0,
      request: {
        time: '',
        vndid: ''
      },
      form: [],
      example: {
        time: '',
        vndid: '',
        event: ''
      },
      dialForm: {
        starttime: '',
        endtime: '',
        page: '',
        vndid: -1,
        pagecount: ''
      }
    }
  },
  watch: {
    'request.vndid': function () {
      if (this.request.vndid === '') {
        this.dialForm.vndid = -1
      } else {
        this.dialForm.vndid = Number(this.request.vndid)
      }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    getTime () {
      this.dialForm.starttime = this.transform(this.request.time[0])
      this.dialForm.endtime = this.transform(this.request.time[1])
    },
    sendTime () {
      if (this.dialForm.starttime && this.dialForm.endtime) {
        this.getLogInfo()
      } else {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '输入查询时间',
            type: 'warning'
          })
        } else {
          this.$message({
            message: 'Please input query time',
            type: 'warning'
          })
        }
      }
    },
    empty () {
      this.$refs['request'].resetFields()
      vpnLogDelAction()
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '清空成功',
                type: 'success'
              })
            } else {
              this.$message({
                message: 'Clear successful',
                type: 'success'
              })
            }
          } else {
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '清空失败',
                type: 'error'
              })
            } else {
              this.$message({
                message: 'Clear failed',
                type: 'error'
              })
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.getLogInfo()
    },
    handleCurrentChange (val) {
      this.currentPage = val
      let para = Object.assign({}, this.dialForm)
      if (this.request.vndid !== '') {
        para.vndid = this.request.vndid
      }
      para.page = this.currentPage
      para.pagecount = 10
      getVPNLog(para)
        .then(res => {
          if (res.data.code === 200) {
            this.form = res.data.data
            this.total = res.data.total
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    getLogInfo () {
      let para = Object.assign({}, this.dialForm)
      if (this.request.time === '') {
        let now = new Date()
        this.dialForm.starttime = this.transform(
          new Date(now - 24 * 60 * 60 * 1000)
        )
        para.starttime = this.dialForm.starttime
        this.dialForm.endtime = this.transform(now)
        para.endtime = this.dialForm.endtime
      }
      this.currentPage = 1
      para.page = this.currentPage
      para.pagecount = 10
      console.log(para)
      getVPNLog(para).then(res => {
        if (res.data.code === 200) {
          if (res.data.data.length !== 0) {
            this.form = res.data.data
            this.total = res.data.total
          } else {
            if (res.data.total === 0) {
              this.form = []
              this.total = res.data.total
            } else {
              this.currentPage -= 1
              this.getLogInfo()
              this.total = res.data.total
            }
          }
        } else {
          this.form = []
          this.total = 0
          this.total = res.data.total
        }
      })
    },
    transform (val) {
      let time
      time =
        val.toDateString() +
        ' ' +
        val.getHours() +
        ':' +
        val.getMinutes() +
        ':' +
        val.getSeconds()
      return time
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
    this.getLogInfo()
  }
}
</script>

<style lang="scss" scoped>
.clear {
  float: right;
}
.tip {
  color: #909399;
}
</style>
