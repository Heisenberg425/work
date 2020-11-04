<template>
  <section>
    <client class="client"/>

    <div class="line_02">
      <span>{{ $t('kidVPN.server.title1') }}</span>
    </div>

    <server class="server"/>

    <connection class="connection"/>

    <visualization class="visualization"/>

  </section>
</template>

<script>
import { client, server, connection, visualization } from './components'

export default {
  name: 'Index',
  components: {
    client,
    server,
    connection,
    visualization
  },
  data () {
    return {}
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

<style scoped>
.client {
  height: 40%;
  width: 100%;
}
.visualization {
  display: flex;
  height: 30%;
  width: 48%;
}
</style>
