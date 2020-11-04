<template>
  <section>
    <div class="line_02"><span>{{ $t('staticRouter.title4') }}</span></div>

    <el-collapse v-model="routerIpv4">
      <el-collapse-item  :title="$t('staticRouter.subtitle4')"  name="1">
        <div>
          <el-table  :data="form"  style="width: 100%;"  :header-cell-style="headerStyle"  v-model="form">
            <el-table-column  prop="destination"  :label="$t('staticRouter.destination')"  min-width="120"/>
            <el-table-column  prop="genmask"      :label="$t('staticRouter.genmask')"      min-width="120"/>
            <el-table-column  prop="gateway"      :label="$t('staticRouter.gateway')"      min-width="120"/>
            <el-table-column  prop="flags"        :label="$t('staticRouter.flags')"        min-width="120"/>
            <el-table-column  prop="metric"       :label="$t('staticRouter.metric')"       min-width="120"/>
            <el-table-column  prop="iface"        :label="$t('staticRouter.iface')"        min-width="120"/>
          </el-table>
        </div>
        <el-pagination  @current-change="handleCurrentChange"  :current-page="currentPage"  :page-size="5"
          layout="total, prev, pager, next, jumper"  :total="this.total"  style="float:right"/>
      </el-collapse-item>
    </el-collapse>

  </section>
</template>

<script>
import { getRouterInfo } from '@/api/api'

export default {
  name: 'SystemRouter',
  props: {
    update: {type: String, default: ''}
  },

  data () {
    return {
      routerIpv4: [],
      routerIpn6: [],
      form: [],
      total: Number,
      currentPage: 1,
      addRouterForm: {
        destination: '',                                                // 目的地址
        gateway: '',                                                    // 网关
        genmask: '',                                                    // 掩码
        flags: '',                                                      // 路由 flags
        prefix: '',                                                     // ipv6 前缀
        metric: '',                                                     // 路由度量值
        ref: '',                                                        // 引用值
        iface: '',                                                      // 接口名称 出口
        ip_family: '',                                                  // ip 地址族
        oper_type: '',                                                  // 增删改查
        route_type: '',                                                 // 路由类型
        page: '',                                                       // 当前页数
        total: '',                                                      // 总页数
        data: ''                                                        // 内网端口
      }
    }
  },

  watch: {
    update: function () {
      setTimeout(() => {}, 500)
      this.getInfo()
    }
  },

  methods: {
    headerStyle () {
      return this.header()
    },

    handleCurrentChange (val) {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'query'
      para.ip_family = 'IPv4'
      para.page = val
      console.log('val is:' + val)
      getRouterInfo(para)
        .then((res) => {
          if (res.data.code === 200) {
            this.form = res.data.data
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.currentPage = val
    },

    getInfo () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'query'
      para.ip_family = 'IPv4'
      para.page = 1
      getRouterInfo(para)
        .then((res) => {
          if (res.data.code === 200) {
            this.total = res.data.total
            this.form = res.data.data
            console.log(JSON.stringify(this.form))
          }
        })
        .catch(error => {
          console.log(error)
        })
    }
  },

  mounted () {
    this.getInfo()
  }
}
</script>

<style scoped>
.el-pagination {
  text-align: right;
}
</style>
