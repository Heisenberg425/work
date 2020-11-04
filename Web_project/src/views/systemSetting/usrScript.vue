<template>
  <div>
    <div class="line_02">
      <span>{{$t('usrScriptNameDef.head')}}</span>
    </div>
    <el-input type="textarea" :rows="20" placeholder v-model="usrScriptVal"></el-input>

    <el-button
      type="primary"
      @click="handle"
      style="margin-left: 0px;margin-top: 10px;"
    >{{$t('usrScriptNameDef.button')}}</el-button>
  </div>
</template>

<script>
import { getusrScript, setusrScript } from '@/api/api.js'
import { Loading } from 'element-ui'
export default {
  name: 'usrScript',
  data() {
    return {
      usrScriptGetType: {
        type: 'run'
      },

      usrScriptVal: '',
      repValCode: 100,

      screenCanClose: 0, // 定时器是否可以进行关闭全屏加载的操作
      resCanClose: 0 // 返回处理是否可以进行关闭全屏加载的操作
    }
  },
  methods: {
    // 弹出框显示当前操作信息内容
    messageshow(messageZh, messageEn, typeval) {
      if (this.$store.state.app.language === 'zh') {
        this.$message({
          message: messageZh,
          type: typeval
        })
      } else {
        this.$message({
          message: messageEn,
          type: typeval
        })
      }
    },

    // 打开全屏等待
    openFullScreen() {
      const loading = Loading.service({
        lock: true,
        text: '正在提交并保存',
        spinner: 'el-icon-loading',
        background: 'rgba(0, 0, 0, 0.7)'
      })

      setTimeout(() => {
        if (this.runTimrCanClose) {
          // 返定时器进行全屏加载关闭
          console.log('time 700 close')
          this.closeFullScreen(loading)
          this.runTimrCanClose = 0 // 复位
        } else {
          console.log('set 3000 time')
          this.resCanClose = 1 // 告诉返回处理可以进行全屏加载关闭了
          setTimeout(() => {
            console.log('time 3000 finish')
            // 再次设置超时时间，如果还收不到返回，那么就超时关闭
            if (this.outTimrCanClose) {
              this.closeFullScreen(loading)
              this.screenCanClose = 0
              this.messageshow('操作超时', 'timeout', 'warning')
            }
            this.resCanClose = 0
          }, 3000)
        }
      }, 700)

      return loading
    },
    // 关闭全屏等待
    closeFullScreen(loadfd) {
      loadfd.close()
      if (this.repValCode === 200) {
        this.messageshow('操作成功', 'success', 'success')
      } else if (this.repValCode === 100) {
        this.messageshow('操作失败', 'failed', 'failed')
      } else if (this.repValCode === 500) {
        this.messageshow('输入信息异常', 'input error', 'warning')
      }
    },

    handle() {
      var usrScriptSetType = {
        script: this.usrScriptVal,
        type: 'run',
        len: this.usrScriptVal.length
      }
      var loadfd = this.openFullScreen()
      setusrScript(usrScriptSetType).then(res => {
        this.repValCode = res.data.code
        if (this.resCanClose) {
          this.closeFullScreen(loadfd)
          this.outTimrCanClose = 0 // 告诉超时定时器，全屏加载已经关闭了
        } else {
          // 在一定的时间内，全屏加载是不允许关闭的，需要由定时器去关闭
          this.runTimrCanClose = 1 // 告诉运行定时器，全屏加载可以关闭了
        }
        this.getusrScriptInfo()
      })
    },
    getusrScriptInfo() {
      getusrScript(this.usrScriptGetType)
        .then(res => {
          if (res.data.code === 200) {
            this.usrScriptVal = res.data.script
            console.log('get usr script info')
          } else {
            console.log('can not get usr script info')
          }
        })
        .catch(err => {
          console.log(err)
        })
    }
  },

  mounted() {
    console.log('usr script info')
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
    this.getusrScriptInfo()
  }
}
</script>

<style scoped>
.line_02 {
  margin-bottom: 20px;
  margin-top: 30px;
  height: 1px;
  border-top: 1px solid #ddd;
  text-align: left;
  color: #909399;
}
.line_02 span {
  position: relative;
  top: -10px;
  background: #fff;
  padding: 0 30px 0 0;
  font-weight: bold;
}
.el-form {
  width: 80%;
  min-width: 20rem;
}
.el-button {
  margin-left: 5rem;
}
</style>
