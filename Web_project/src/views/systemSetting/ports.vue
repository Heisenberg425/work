<template>
  <div>

    <div class="line_02">
      <span>{{ $t('systemSetting.portadapt') }}</span>
    </div>

    <div class="top">
      <span>{{ $t('systemSetting.portstatus') }}{{ status }}</span>
      <el-button
        type="primary"
        @click.native="readapt">{{ $t('systemSetting.readapt') }}</el-button>
    </div>

    <adaptive
      ref="router"
      class="whole-router"/>

  </div>
</template>

<script>
import adaptive from '@/utils/adaptive.vue'
export default {
  name: 'Ports',
  data () {
    return {
      status: '已完成定位'
    }
  },
  components: {
    adaptive
  },
  watch: {
    code: function (code) {
      if (code === 200) {
        this.status = '已完成定位'
      } else if (code === 0) {
        this.status = '请完成网口定位'
      } else {
        this.status = '网口定位中，请稍等'
      }
    },
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      if (this.$store.state.app.language === 'zh') {
        if (this.status === 'Adapted') {
          this.status = '已完成定位'
        } else if (this.status === 'NotAdapted') {
          this.status = '请完成网口定位'
        } else if (this.status === 'Adapting') {
          this.status = '网口定位中，请稍等'
        }
      } else {
        if (this.status === '已完成定位') {
          this.status = 'Adapted'
        } else if (this.status === '请完成网口定位') {
          this.status = 'NotAdapted'
        } else if (this.status === '网口定位中，请稍等') {
          this.status = 'Adapting'
        }
      }
    }
  },
  methods: {
    readapt () {
      this.$refs.router.modifyChecked()
    }
  },
  // wyk1130中英文切换
  mounted () {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
    if (this.$store.state.app.language === 'zh') {
      if (this.status === 'Adapted') {
        this.status = '已完成定位'
      } else if (this.status === 'NotAdapted') {
        this.status = '请完成网口定位'
      } else if (this.status === 'Adapting') {
        this.status = '网口定位中，请稍等'
      }
    } else {
      if (this.status === '已完成定位') {
        this.status = 'Adapted'
      } else if (this.status === '请完成网口定位') {
        this.status = 'NotAdapted'
      } else if (this.status === '网口定位中，请稍等') {
        this.status = 'Adapting'
      }
    }
  }
}
</script>

<style scoped>
.top {
  width: 100%;
}
.top > span {
    width: 10rem;
  }
p {
  margin: 3rem auto 1rem auto;
}
.whole-router {
  width: 80%;
  margin: 2rem auto;
}
</style>
