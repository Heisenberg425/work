<template>
  <section>
    <span>{{ $t('kidVPN.status.title') }}:</span>
    <el-table :data="VPNList" :header-cell-style="headerStyle" style="width: 100%">
      <el-table-column type="selection" min-width="30"/>
      <el-table-column prop="vndid" :label="$t('kidVPN.status.vndid')" min-width="120"/>
      <el-table-column prop="cliip" :label="$t('kidVPN.status.ip')" min-width="120"/>
      <el-table-column prop="climac" :label="$t('kidVPN.status.mac')" min-width="120"/>
      <el-table-column prop="position" :label="$t('kidVPN.status.clientLoc')" min-width="120"/>
      <el-table-column :label="$t('operation.operation')" min-width="60">
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="deleteClient(scope.$index, scope.row)"
          >{{ $t('operation.delete') }}</el-button>
        </template>
      </el-table-column>
    </el-table>
    <el-pagination
      @current-change="handleCurrentChange"
      :current-page="currentPage"
      :page-size="5"
      layout="total, prev, pager, next, jumper"
      :total="total"
      style="float:right"
    />
  </section>
</template>

<script>
import { getLinkStatus } from '@/api/api.js'
let timer
export default {
  name: 'Status',
  data() {
    return {
      currentPage: 1,
      total: 0,
      VPNList: [],

      form: {
        vndid: Number,
        cliip: '',
        climac: ''
      }
    }
  },
  methods: {
    headerStyle() {
      return this.header()
    },
    handleCurrentChange(val) {
      this.currentPage = val
      this.getInfo()
    },
    getInfo() {
      let para = {
        page: this.currentPage,
        pagecount: 10
      }
      getLinkStatus(para).then(res => {
        if (res.data.code === 200) {
          this.VPNList = res.data.data
          this.total = res.data.total
        }
      })
    }
  },
  mounted() {
    this.getInfo()
    timer = setInterval(this.getLinkStatus, 2000)
  },
  destroyed() {
    clearInterval(timer)
  }
}
</script>

<style scoped>
p {
  height: 28px;
  margin: 1rem 0 0 0;
}

.el-table {
  margin: 15px 0 0 0;
}
</style>
