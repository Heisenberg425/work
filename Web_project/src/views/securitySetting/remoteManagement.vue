<template>
  <div>
    <div class="line_02"><span>{{ $t('securitySetting.whitelist') }}</span></div>
    <div>
      <el-col :span="24">
        <el-form :inline="true">
          <el-form-item>
            <el-button type="primary" >{{ $t('securitySetting.add') }}</el-button>
          </el-form-item>
          <el-form-item>
            <el-button
              type="danger"
              right>{{ $t('securitySetting.delete') }}</el-button>
          </el-form-item>
        </el-form>
      </el-col>
      <el-table
        style="width: 100%"
        :header-cell-style="headerStyle">
        <el-table-column
          prop="IP"
          :label="$t('securitySetting.IP')"
          min-width="180"/>
        <el-table-column
          prop="time"
          :label="$t('securitySetting.time')"
          min-width="180"/>
        <el-table-column
          :label="$t('securitySetting.enable')"
          min-width="150">
          <template slot-scope="scope">
            <el-button size="small">{{ $t('securitySetting.edit') }}</el-button>
            <el-button type="danger">{{ $t('securitySetting.delete') }}</el-button>
          </template>
        </el-table-column>
      </el-table>
    </div>
    <div class="line_02"><span>{{ $t('securitySetting.blacklist') }}</span></div>
    <div>
      <el-col :span="24">
        <el-form :inline="true">
          <el-form-item>
            <el-button type="primary" >{{ $t('securitySetting.add') }}</el-button>
          </el-form-item>
          <el-form-item>
            <el-button
              type="danger"
              right>{{ $t('securitySetting.delete') }}</el-button>
          </el-form-item>
        </el-form>
      </el-col>
      <el-table
        style="width: 100%"
        :header-cell-style="headerStyle">
        <el-table-column
          prop="IP"
          :label="$t('securitySetting.IP')"
          min-width="180"/>
        <el-table-column
          prop="time"
          :label="$t('securitySetting.time')"
          min-width="180"/>
        <el-table-column
          :label="$t('securitySetting.enable')"
          min-width="150">
          <template slot-scope="scope">
            <el-button size="small">{{ $t('securitySetting.edit') }}</el-button>
            <el-button type="danger">{{ $t('securitySetting.delete') }}</el-button>
          </template>
        </el-table-column>
      </el-table>
    </div>
  </div>
</template>s

<script>
export default {
  name: 'RemoteManage',
  methods: {
    headerStyle () {
      return this.header()
    }
  }
}
</script>

<style scoped>

</style>
