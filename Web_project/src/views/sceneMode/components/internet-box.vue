<template>
  <div class="internet_container">
    <div class="header">
      <div class="tip">{{ $t('sceneRouter.cpu') + ': ' }}{{ $store.state.app.systemData.cpu | percent }}</div>
      <div class="tip">{{ $t('sceneRouter.ram') + ': ' }}{{ $store.state.app.systemData.memory | percent }}</div>
    </div>

    <div class="content">
      <div class="square">
        <div class="square_title">{{ $t('sceneRouter.extralNet') }}</div>
        <div class="square_body">
          <div
            v-for="(item, index) in extralNetList"
            :key="index"
            class="entry">
            <div class="entry-title">{{ item.name }}</div>
            <div class="entry_content">{{ item.value }}</div>
          </div>
        </div>
      </div>

      <div class="square">
        <div class="square_title">{{ $t('sceneRouter.bandWidth') }}</div>
        <div class="square_body">
          <iframe
            src="http://plugin.speedtest.cn/#1d2133"
            frameborder="no"
            scrolling="auto" 
             style='overflow:scroll;overflow-x:hidden'/>
        </div>
      </div>

    </div>

  </div>

</template>

<script>
export default {
  filters: {
    percent: function (val) {
      if (!val) {
        return ''
      }
      val = Number(val).toFixed(1) + '%'
      return val
    }
  },
  data () {
    return {
      CPU: this.$store.state.app.systemData.cpu,
      RAM: this.$store.state.app.systemData.memory,
      extralNetList: [
        {
          name: '连接类型',
          value: this.internetInfo.wan_link_type
        },
        {
          name: 'IP地址',
          value: this.internetInfo.wan_ip
        },
        {
          name: 'DNS',
          value: this.internetInfo.wan_dns
        },
        {
          name: '网关地址',
          value: this.internetInfo.wan_gateway
        }
      ]
    }
  },
  props: {
    internetInfo: {
      type: Object,
      default: function () {
        return {}
      }
    }
  },

  methods: {
  },
  watch: {
    '$store.state.app.language': function () {
      if (this.$store.state.app.language === 'zh') {
        this.extralNetList[0].name = '连接类型'
        this.extralNetList[1].name = 'IP地址'
        this.extralNetList[3].name = '网关地址'
      } else {
        this.extralNetList[0].name = 'Connection type'
        this.extralNetList[1].name = 'IP'
        this.extralNetList[3].name = 'Gateway'
      }
    }},
  mounted () {
    if (this.$store.state.app.language === 'zh') {
      this.extralNetList[0].name = '连接类型'
      this.extralNetList[1].name = 'IP地址'
      this.extralNetList[3].name = '网关地址'
    } else {
      this.extralNetList[0].name = 'Connection type'
      this.extralNetList[1].name = 'IP'
      this.extralNetList[3].name = 'Gateway'
    }
    this.$emit('getinternetinfo')
    console.log(this.internetInfo)
  }
}
</script>

<style scoped lang="scss">
.internet_container {
  height: 100%;
  border: 1px solid ghostwhite;
  border-radius: 1rem;
  background: #fff;
  color: rgb(70, 114, 196);
  border: 1px solid grey;

  .header {
    height: 1rem;
    display: flex;
    justify-content: flex-end;
    margin-top: 1rem;
    .tip {
      width: 10rem;
      height: 1rem;
      line-height: 1rem;
    }
  }
  .content {
    display: flex;
    justify-content: space-around;
    flex-flow: row wrap;
    padding-bottom: 1rem;

    .square {
      width: 45%;
      height: 35rem;
      margin: 1rem auto;
      border-radius: 1rem;
      background: #fff;
      border: 1px solid grey;
      .square_title {
        margin-left: 2rem;
        margin-top: 1rem;
        line-height: 2rem;
        font-weight: bold;
      }
      .square_body {
        width: 100%;
        height: 30rem;
        display: flex;
        flex-flow: row wrap;
        justify-content: space-around;
        iframe {
          width: 100%;
          height: 100%;
          body{
           overflow-x: hidden; 
          }
          
          // .speedtest-container{
          //   .logInformation{
          //     .logInformation[data-v-0ef51150] {
          //       margin-top: .5rem;
          //     } 
          //   }
          //   .speedButton2{
          //     .speedButton2[data-v-0ef51150] {
          //       display: -ms-flexbox;
          //       display: flex;
          //       -ms-flex-pack: center;
          //       justify-content: center;
          //       margin: -4.5rem auto;
          //     }
          //   }
          //   }
        }
        .entry {
          width: 50%;
          height: 50%;
          text-align: center;
          .entry-title {
            height: 30%;
            line-height: 1rem;
            font-weight: bold;
          }
          .entry_content {
            height: 70%;
          }
        }
      }
    }
  }
}
</style>
