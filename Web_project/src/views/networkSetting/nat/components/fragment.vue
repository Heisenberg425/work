<template>
  <div class="tips">
    <el-checkbox
      class="checkbox"
      :label="$t('NAT.selectFunction.TCP')"
      v-model="TCP"
      @change="handleTCPChange"
      name="type"/><br>
    <el-checkbox
      class="checkbox"
      :label="$t('NAT.selectFunction.UDP')"
      v-model="UDP"
      @change="handleUDPChange"
      name="type"/><br>
    <el-checkbox
      class="checkbox"
      :label="$t('NAT.selectFunction.ICMP')"
      v-model="ICMP"
      @change="handleICMPChange"
      name="type"/>
  </div>
</template>

<script>
import { getFragmentInfo, sendFragment } from '../../../../api/api'

export default {
  name: 'Fragment',
  data () {
    return {
      TCP: false,
      UDP: false,
      ICMP: false
    }
  },
  methods: {
    // 分片包功能信息获取
    getFragmentPackageInfo: function () {
      getFragmentInfo()
        .then((res) => {
          this.TCP = !!res.data.tcp
          this.UDP = !!res.data.udp
          this.ICMP = !!res.data.icmp
          // console.log("the value of fragment are "+this.TCP+" "+this.UDP+" "+this.ICMP);
        })
        .catch(error => {
          console.log(error)
        })
    },
    // 处理分片包选择
    handleTCPChange: function (val) {
      let para = {}
      if (val) {
        para.handle = 1
      } else {
        para.handle = 0
      }
      para.fragtype = 'tcp'
      if (this.$store.state.app.language === 'zh') {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: '提交成功',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      } else {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: 'SubmitSuccess',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      }
      this.getFragmentPackageInfo()
    },
    handleUDPChange: function (val) {
      let para = {}
      if (val) {
        para.handle = 1
      } else {
        para.handle = 0
      }
      para.fragtype = 'udp'
      if (this.$store.state.app.language === 'zh') {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: '提交成功',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      } else {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: 'SubmitSuccess',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      }
      this.getFragmentPackageInfo()
    },
    handleICMPChange: function (val) {
      let para = {}
      if (val) {
        para.handle = 1
      } else {
        para.handle = 0
      }
      para.fragtype = 'icmp'
      if (this.$store.state.app.language === 'zh') {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: '提交成功',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      } else {
        sendFragment(para)
          .then(() => {
            this.$message({
              message: 'SubmitSuccess',
              type: 'success'
            })
          })
          .catch(error => {
            console.log(error)
          })
      }
      this.getFragmentPackageInfo()
    }
  },
  mounted () {
    this.getFragmentPackageInfo()
  }
}
</script>

<style scoped>
  .checkbox {
    margin: 10px;
  }
</style>
