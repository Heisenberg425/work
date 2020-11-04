<template>
  <section>

    <div class="line_02">
      <span>{{ $t('DHCPLog.DHCPLog') }}</span>
    </div>

    <textarea
      :value="log"
      readonly />

  </section>
</template>

<script>
export default {
  name: 'DHCPLog',
  data () {
    return {
      tip: '',
      log: ''
    }
  },
  methods: {
    generateTip () {
      if (this.$store.state.app.language === 'zh') {
        this.tip = '读取中'
      } else {
        this.tip = 'Reading'
      }
      this.log = this.tip
    },
    getLog () {
      // this.$axios.get('/action/dhcpdlog')
      this.$axios
        .get('/dhcpdlog')
        .then(res => {
          this.log = res.data
        })
        .catch(err => {
          console.log(err)
          if (this.$store.state.app.language === 'zh') {
            this.log = '未能获取文件'
          } else {
            this.log = 'Failed to get file'
          }
        })
    }
  },
  watch: {
    '$store.state.app.language': function () {
      this.generateTip()
      this.getLog()
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
    this.generateTip()
    this.getLog()
  }
}
</script>

<style scoped>
textarea {
  width: 100%;
  height: 70vh;
  resize: none;
}
</style>
