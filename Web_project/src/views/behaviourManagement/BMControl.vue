<template>
  <div>
    <div class="line_02"><span>{{ $t('BMControl.record') }}</span></div>
    <div>
      <div style="display: flex;">
        <p style="margin-left: 50px; width:150px;">{{ $t('BMControl.DeviceRecord') }}</p>
        <el-checkbox
          v-model="checked"
          style="margin-top: 14px; margin-left: 100px;">{{ $t('BMControl.on') }}</el-checkbox>
      </div>
      <div style="display: flex">
        <p style="margin-left: 50px; width:150px;">{{ $t('BMControl.WebRecord') }}</p>
        <el-checkbox
          v-model="checked"
          style="margin-top: 14px; margin-left: 100px;">{{ $t('BMControl.on') }}</el-checkbox>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'BMControl',
  data () {
    return {
      checked: ''
    }
  }
}
</script>

<style scoped>

</style>
