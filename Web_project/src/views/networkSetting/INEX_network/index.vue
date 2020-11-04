<template>
  <section>
    <!-- 终端设置部分 -->
    <div class="line_02"><span>{{ $t('INEXNetwork.dividingLine.title1') }}</span></div>

    <dataDisplayer
      :send-ports-info="interfaces"
      class="setting"/>

    <!-- 状态接口部分 -->
    <div class="line_02"><span>{{ $t('INEXNetwork.dividingLine.title2') }}</span></div>

    <div class="router">
      <router
        class="wholeRouter-out"
        @getPortsInfo="handlePortsInfo"/>
      <div class="tips">
        <div
          v-for="(item, index) in list"
          :key="index"
          class="row">
          <div class="container">
            <svg class="icon"><use :xlink:href="item.url"/></svg>
          </div>
          
          <span>{{ item.tip }}</span>
        </div>
      </div>
    </div>

  </section>
</template>

<script >
import { dataDisplayer, router } from './components'
export default {
  name: 'Index',
  components: {
    dataDisplayer,
    router
  },
  data () {
    return {
      list: [
        {
          name: 'asdl',
          tip: 'ADSL拨号接入',
          tipEn: 'ASDL',
          url: '#icon-phone'
        },
        {
          name: 'dhcp',
          tip: '动态DHCP接入',
          tipEn: 'DHCP',
          url: '#icon-dynamic'
        },
        {
          name: 'static',
          tip: '静态IP接入',
          tipEn: 'Static IP',
          url: '#icon-ip'
        }, {
          name: 'wan',
          tip: 'WAN口',
          tipEn: 'WAN',
          url: '#icon-wan'
        },
        {
          name: 'lan',
          tip: 'LAN口',
          tipEn: 'LAN',
          url: '#icon-pc'
        }
      ],
      interfaces: []

    }
  },
  methods: {
    handlePortsInfo (val) {
      this.interfaces = val
    },
    // wyk1130改中英文
    changeList() {
      if (this.$store.state.app.language === 'zh') {
        this.list = [
          {
            name: 'asdl',
            tip: 'ADSL拨号接入',
            tipEn: 'ASDL',
            url: '#icon-phone'
          },
          {
            name: 'dhcp',
            tip: '动态DHCP接入',
            tipEn: 'DHCP',
            url: '#icon-dynamic'
          },
          {
            name: 'static',
            tip: '静态IP接入',
            tipEn: 'Static IP',
            url: '#icon-ip'
          }, {
            name: 'wan',
            tip: 'WAN口',
            tipEn: 'WAN',
            url: '#icon-wan'
          },
          {
            name: 'lan',
            tip: 'LAN口',
            tipEn: 'LAN',
            url: '#icon-pc'
          }
        ]
      } else {
        this.list = [
          {
            name: 'asdl',
            tip: 'ADSL',
            tipEn: 'ASDL',
            url: '#icon-phone'
          },
          {
            name: 'dhcp',
            tip: 'DHCP',
            tipEn: 'DHCP',
            url: '#icon-dynamic'
          },
          {
            name: 'static',
            tip: 'StaticIP',
            tipEn: 'Static IP',
            url: '#icon-ip'
          }, {
            name: 'wan',
            tip: 'WAN',
            tipEn: 'WAN',
            url: '#icon-wan'
          },
          {
            name: 'lan',
            tip: 'LAN',
            tipEn: 'LAN',
            url: '#icon-pc'
          }
        ]
      }
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.changeList()
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
    this.changeList()
  }
}
</script>

<style lang="scss" scoped>
  .setting {
    margin: 10px;
    color: #909399;
  }
  .router {
    display: flex;
    width: 100%;
    .wholeRouter-out {
      width: 65%;
      margin-left: 20px;
      border: 1px solid lightgrey;
      background-color: ghostwhite;
      color: #909399;
    }
    .tips {
      // display: flex;
      // align-content: space-between;
      width: 30%;
      margin-left: 20px;
      margin-right: 20px;
      border: 1px solid lightgrey;
      .row {
        display: flex;
        height: 20%;
        align-items: center;
        margin-left: 10px;
        .container {
          height: 1.2rem;
          .icon {
            width: 1.2rem;
            height: 1.2rem;
          }
        }
        span {
          margin-left: 10px;
          line-height: 1.2rem;
        }
      }
    }
  }
</style>
