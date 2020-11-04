<template>
  <section>
    <el-row
      :gutter="0"
      class="setting">

      <el-col
        :md="9"
        :lg="9"
        :xl="9">
        <div class="container">
          <div class="icon-div">
            <svg class="icon-big">
              <use xlink:href="#icon-wan"/>
            </svg>
          </div>
          <div class="tip1">
            <h2>{{ WAN }}</h2>
            <p class="bottomLine">{{ $t('INEXNetwork.terminal.WANLinked') }}</p>
          </div>
          <!-- <div class="tip1">
            <h2>{{ totalBandWidth }}</h2>
            <p class="bottomLine">{{ $t('INEXNetwork.terminal.bandWidth') }}</p>
          </div> -->
        </div>
      </el-col>

      <el-col
        :md="15"
        :lg="15"
        :xl="15">
        <div class="container">
          <div class="icon-div">
            <svg class="icon-big">
              <use xlink:href="#icon-pc"/>
            </svg>
          </div>
          <div class="tip">
            <h2>{{ LAN }}</h2>
            <p class="bottomLine">{{ $t('INEXNetwork.terminal.LANLinked') }}</p>
          </div>
          <div class="tip">
            <h2>{{ terminal }}</h2>
            <p class="bottomLine">{{ $t('INEXNetwork.terminal.terminalLinked') }}</p>
          </div>
          <!-- <div class="tip">
            <h2>{{ DHCP }}</h2>
            <p class="bottomLine">{{ $t('INEXNetwork.terminal.DHCP') }}</p>
          </div> -->
        </div>
      </el-col>

    </el-row>
  </section>
</template>

<script>
export default {
  name: 'DataDisplayer',
  data () {
    return {
      WAN: 0,
      LAN: 0,
      DHCP: 0,
      terminal: this.$store.state.app.systemData.userNum,
      totalBandWidth: 0
    }
  },
  props: {
    sendPortsInfo: {type: String, default: ''}
  },
  watch: {
    sendPortsInfo: function () {
      this.DHCP = this.LAN = this.WAN = 0
      if (!this.terminal) {
        this.terminal = 0
      }
      this.sendPortsInfo.forEach(element => {
        if (element.type === 'dhcp') {
          this.DHCP++
        }
        if (element.function === 'lan') {
          this.LAN++
        }
        if (element.function === 'wan') {
          this.WAN++
        }
      })
    }
  }
}
</script>

<style scoped>
.container {
  display: flex;
  margin: 10px;
  height: 100px;
  padding: 0 20px;
  border: 1px solid lightgrey;
  align-items: center;
}
.icon-div {
  height: 20px;
  width: 20px;
  margin-right: 5px;
}
.icon-big {
  height: 100%;
  width: 100%;
}
.tip1 {
  display: flex;
  width: 45%;
}
.bottomLine {
  line-height: 50px;
}
.tip {
  display: flex;
  width: 30%;
}
.bottomLine {
  line-height: 50px;
}
</style>
