<template>
  <div>
    <div class="line_02"><span>{{ $t('systemSetting.versionStatus') }}</span></div>

    <div>
      <!-- 10.29 pky 要求删除版本检测功能 -->
      <!-- <div style="display: flex;">
        <p style="margin-left: 50px; width:180px;">{{ $t('systemSetting.VersionDetection') }}</p>
        <el-form>
          <el-button
            type="primary"
            size="small"
            @click="checkVersion">{{ $t('systemSetting.DetectNewVersion') }}</el-button>
        </el-form>
      </div> -->
      <div style="display: flex">
        <p style="margin-left: 50px; width:180px;">{{ $t('systemSetting.RoutingSystemVersion') }}</p>
        <p style="margin-left: 100px;">{{ version }}</p>
      </div>
      <!-- <p style="display: flex">
        <span style="margin-left: 50px; width:180px;">{{$t('systemSetting.SylixOSVersion')}}</span>
        <span style="margin-left: 100px;">2.0.23</span>
      </p> -->
    </div>
    <div class="line_02"><span>{{ $t('systemSetting.update') }}</span></div>
    <!--
      <div style="background-color: #EEF6F6">
          <p style="padding: 5px 10px; font-weight: bold">手动升级</p>
      </div>
      -->
    <div style="display: flex; vertical-align: middle">
      <p style="margin-left: 50px; width:180px;">{{ $t('systemSetting.localUpdate') }}</p>
      <el-upload
        class="upload-demo"
        ref="upload"
        :on-preview="checkFileType"
        :file-list="fileList"
        action="action/updateFirmwareAction">
        <el-button
          slot="trigger"
          size="small">{{ $t('systemSetting.fileSelection') }}</el-button>
        <el-button
          style="margin-left: 10px; margin-top:10px;"
          size="small"
          @click="submitUpload">{{ $t('systemSetting.submitUpload') }}</el-button>
      </el-upload>
    </div>

    <!--检测页面-->
    <el-dialog
    :close-on-click-modal="false"
      :title="$t('systemSetting.check')"
      :visible.sync="versionVisual">
      <p>{{ $t('systemSetting.latestVersion') }}</p>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button type="primary">{{ $t('systemSetting.updateNow') }}</el-button>
      </div>
    </el-dialog>

    <el-dialog
    :close-on-click-modal="false"
      :title="$t('systemSetting.updateFinished')"
      :visible.sync="uploadVisual"
      @close="handleClose">
      <p>{{ $t('systemSetting.refreshAfter') }}{{ timeLeft }}{{ $t('systemSetting.sDoNotTurnOff') }}</p>

    </el-dialog>

  </div>
</template>

<script>
import { getVersion } from '@/api/api.js'
let stopSingal
export default {
  name: 'Update',
  data () {
    return {
      fileList: [],
      versionVisual: false,
      uploadVisual: false,
      timeLeft: '', // 倒计时
      version: ''
    }
  },

  methods: {
    cancel () {
      this.versionVisual = false
      this.$refs.upload.abort(this.fileList)
    },
    checkFileType (file) {
      console.log(file.type)
    },
    submitUpload () {
      this.$refs.upload.submit()
      this.uploadVisual = true

      setTimeout(() => {
      }, 500)

      let duration = 60
      stopSingal = setInterval(() => {
        if (duration > 0) {
          duration--
          this.timeLeft = duration
        } else {
          clearInterval(stopSingal)
          location.reload()
        }
      }, 1000)
    },
    checkVersion () {
      this.versionVisual = true
    },
    getVersionInfo () {
      getVersion()
        .then(res => {
          if (res.data.code === 200) {
            this.version = res.data.version
          }
        })
    },

    handleClose () {
      clearInterval(stopSingal)
      this.timeLeft = 60
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
    this.getVersionInfo()
  }
}
</script>

<style scoped>
.el-button{
  margin-left: 100px;
  margin-top:10px;
}
</style>
