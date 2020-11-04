<template>
  <section>
    <div class="line_02">
      <span>{{ $t('currentRouter.monitor') }}</span>
    </div>
    <el-table
      :data="users"
      @selection-change="selsChange"
      style="width: 100%;">
      <el-table-column
        prop="line"
        :label="$t('currentRouter.line')"
        min-width="120"/>
      <el-table-column
        prop="targetAddress"
        :label="$t('currentRouter.targetAddr')"
        min-width="180"/>
      <el-table-column
        prop="SubnetMask"
        :label="$t('currentRouter.subnetmask')"
        min-width="120"/>
      <el-table-column
        prop="Gateway"
        :label="$t('currentRouter.gateway')"
        min-width="120"/>
      <el-table-column
        prop="metric"
        label="Metric"
        min-width="120"/>
      <el-table-column
        prop="tip"
        :label="$t('currentRouter.tip')"
        min-width="180"/>
    </el-table>
  </section>
</template>

<script>
export default {
  name: 'CurrentRouter',
  mounted () {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
  }
}
</script>

<style scoped>
.line_02 {
  margin-bottom: 20px;
  margin-top: 30px;
  height: 1px;
  border-top: 1px solid #ddd;
  text-align: left;
  color: #909399;
}
.line_02 span {
  position: relative;
  top: -10px;
  background: #fff;
  padding: 0 30px 0 0;
  font-weight: bold;
}
</style>
