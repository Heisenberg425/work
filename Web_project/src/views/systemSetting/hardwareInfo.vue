<template>
  <section>
    <div class="line_02">
      <span>{{ $t('systemSetting.hardwareInfo') }}</span>
    </div>
    <el-table
      :data="hardwareInfo"
      style="width: 100%"
      header-cell-style="padding: 0">
      <el-table-column
        prop="name"
        width="180"/>
      <el-table-column
        prop="hardware"
        min-width="360"/>
    </el-table>
  </section>
</template>

<script>
import { getHardware, getPorts } from '../../api/api'
export default {
  name: 'HardwareInfo',

  data () {
    return {
      hardwareInfo: [
        {
          name: 'CPU',
          hardware: ''
        },
        {
          name: '存储',
          hardware: ''
        },
        {
          name: '内存',
          hardware: ''
        },
        {
          name: '网卡',
          hardware: ''
        }
      ],
      ports: [],
      deviceName: []
    }
  },
  methods: {
    getInfo () {
      getHardware()
        .then(res => {
          if (res.data.code === 200) {
            this.hardwareInfo[0].hardware = res.data.cpu
            this.hardwareInfo[1].hardware = res.data.sto
            this.hardwareInfo[2].hardware = res.data.mem
          }
        })
    },
    getPortsInfo () {
      this.deviceName = []
      getPorts()
        .then(res => {
          if (res.data.code === 200) {
            this.ports = res.data.interfaces
            this.ports.forEach(item => {
              this.deviceName.push(item.devname)
              console.log(this.deviceName)
            })
          }
          this.hardwareInfo[3].hardware = this.deviceName.join(', ')
        })
    },
    // wyk1130中英文切换
    changeLan() {
      if (this.$store.state.app.language === 'zh') {
        this.hardwareInfo = [
          {
            name: 'CPU',
            hardware: ''
          },
          {
            name: '存储',
            hardware: ''
          },
          {
            name: '内存',
            hardware: ''
          },
          {
            name: '网卡',
            hardware: ''
          }
        ]
      } else {
        this.hardwareInfo = [
          {
            name: 'CPU',
            hardware: ''
          },
          {
            name: 'Storage',
            hardware: ''
          },
          {
            name: 'Memory',
            hardware: ''
          },
          {
            name: 'NetCard',
            hardware: ''
          }
        ]
      }
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.changeLan()
      this.getInfo()
      this.getPortsInfo()
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
    this.changeLan()
    this.getInfo()
    this.getPortsInfo()
  }
}
</script>

<style scoped>
</style>
