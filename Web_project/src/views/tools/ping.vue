<template>
  <div>
    <el-form :model="form" ref="form">
      <el-form-item prop="address" :label="$t('ping.ping')" label-width="6rem">
        <el-input v-model="form.address">{{$t('ping.addr')}}</el-input>
      </el-form-item>
      <!-- <el-form-item prop="length">
        <el-input v-model="form.length">长度</el-input>
      </el-form-item>
      <el-form-item prop="amount">
        <el-input v-model="form.amount">次数</el-input>
      </el-form-item> -->
      <el-form-item porp="address" :label="$t('ping.result')" label-width="6rem">
        <el-input v-model="text" type="textarea" :rows="12" :placeholder="$t('ping.result')" id="container"></el-input>
      </el-form-item>
    </el-form>
    <div slot="footer" class="dialog-footer">
      <el-button type="primary" :disabled="isInTest" @click.native="start">{{$t('ping.start')}}</el-button>
      <el-button type="danger" :disabled="!isInTest" @click.native="stop">{{$t('ping.stop')}}</el-button>
    </div>
  </div>
</template>

<script>
import { ping } from '@/api/api.js'

// let stopSignal

export default {
  name: 'ping',
  data() {
    return {
      isInTest: false,
      form: {},
      text: [],
      height: 0
    }
  },
  watch: {
    height: function() {
      let container = this.$el.querySelector('#container')
      console.log(container)
      container.scrollTop = container.scrollHeight
    }
  },
  methods: {
    start() {
      console.log(this.form.address)

      if (this.form.address === undefined || this.form.address === undefined === '') {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '请输入地址',
            type: 'warning'
          })
        } else {
          this.$message({
            message: 'Please input address',
            type: 'warning'
          })
        }
        return false
      }

      this.isInTest = true
      this.text = ''
      this.text = this.text + 'Pinging ' + this.form.address + '\n'

      let para = {}
      para['ip/domain'] = this.form.address
      para.interface = 'en1'
      para.data_size = 64

      let test = () => {
        ping(para)
          .then(res => {
            if (res.data.code === 200) {
              if (this.isInTest) {
                let content = res.data.result.split('\n')
                console.log(content)
                if (content[1] !== '') {
                  this.text = this.text + content[1] + '\n'
                  this.height++
                } else {
                  this.text = this.text + content[0] + '\n'
                  this.height++
                }
                setTimeout(test, 1000)
              }
            }
          })
      }

      test()
    },
    stop() {
      this.isInTest = false
      // clearInterval(stopSignal)
    }
  },
  beforeDestroy() {
    this.stop()
    this.$refs.form.resetFields()
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
.el-form {
  margin: 2rem .5rem;
}
.el-input, .el-textarea {
  width: 80%;
}
.el-button {
  margin-left: 6.5rem;
}
</style>

