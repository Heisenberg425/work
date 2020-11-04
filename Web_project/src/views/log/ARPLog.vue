<template>
  <section>

    <div class="line_02">
      <span>{{ $t('ARPLog.ARPLog') }}</span>
    </div>
    <textarea
      :value="content"
      readonly />

  </section>
</template>

<script>
export default {
  name: 'ARPLog',
  data () {
    return {
      content: '获取中'
    }
  },
  methods: {
    generateTip () {
      if (this.$store.state.app.language === 'zh') {
        this.tip = '读取中'
      } else {
        this.tip = 'Reading'
      }
    },
    getLog () {
      // this.$axios.get('/action/dhcpdlog')
      this.$axios
        .get('/ArpSpoodDefenseLog')
        .then(res => {
          this.content = res.data
        })
        .catch(err => {
          console.log(err)
          if (this.$store.state.app.language === 'zh') {
            this.content = '未能获取文件'
          } else {
            this.content = 'Failed to get file'
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
