<template>
  <div>
    <shell :code="code" :type="type"></shell>
  </div>
</template>

<script>
import shell from '@/utils/newshell.vue'
export default {
  name: 'bgpd',
  data() {
    return {
      type: 'bgp',
      code: 2605
    }
  },
  components: {
    shell
  },
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

