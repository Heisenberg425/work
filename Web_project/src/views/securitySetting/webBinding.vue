<template>
  <div>

    <div class="line_02">
      <span>{{ $t('securitySetting.webbind') }}</span>
    </div>

    <el-form
      ref="form"
      :model="list"
      label-position="left"
      label-width="7rem">
      <el-form-item  label-position="left" :label="$t('securitySetting.bindstatus')">

        <div style="display: inline-block; width: 20rem">
          <span>{{ bindedPort }}</span>
        </div>

        <el-button
          type="danger"
          @click="unbindWeb"
          :disabled="!isBinded">
          {{ $t('securitySetting.unbind') }}
        </el-button>

      </el-form-item>

      <el-form-item  label-position="left" :label="$t('securitySetting.bindport')" >
        <el-select
          v-model="choosenPort"
          :placeholder="$t('placeholder.bindchoose')"
          style="width: 20rem">
          <el-option
            v-for="(item, index) in availablePortList"
            :key="index"
            :label="item"
            :value="item">
          </el-option>
        </el-select>

        <el-button
          type="primary"
          @click="bindWeb">
          {{ $t('securitySetting.bind') }}
        </el-button>
      </el-form-item>
    </el-form>

  </div>
</template>

<script>
import { getPorts, webBind, webUnbind, getBind } from '@/api/api.js'
export default {
  name: 'webBinding',
  data () {
    return {
      list: {},
      isBinded: false,
      bindedPort: '未绑定',
      choosenPort: '',
      availablePortList: []
    }
  },
  watch: {
    bindedPort: function () {
      if (this.bindedPort === '') {
        if (this.$store.state.app.language === 'zh') {
          this.bindedPort = '未绑定'
        } else {
          this.bindedPort = 'NotBound'
        }
      } else {
        if (this.$store.state.app.language === 'zh') {
          this.bindPort = '已绑定' + this.bindedPort
        } else {
          this.bindPort = 'Bound' + this.bindedPort
        }
      }
    },
    '$store.state.app.language': function() {
      console.log(this.$store.state.app.language)
      if (this.$store.state.app.language === 'zh') {
        if (this.bindedPort === 'NotBound') {
          this.bindedPort = '未绑定'
        }
        if (this.bindPort === 'Bound' + this.bindedPort) {
          this.bindPort = '已绑定' + this.bindedPort
        }
      } else {
        if (this.bindedPort === '未绑定') {
          this.bindedPort = 'NotBound'
        }
        if (this.bindPort === '已绑定' + this.bindedPort) {
          this.bindPort = 'Bound' + this.bindedPort
        }
      }
    }
  },

  methods: {
    bindWeb () {
      let para = {
        webnetif: ''
      }
      para.webnetif = this.choosenPort
      webBind(para)
        .then(res => {
          if (res.data.code === 100) {
            if (this.$store.state.app.language === 'zh') {
              alert('参数错误')
            } else {
              alert('Parameter error')
            }
          } else if (res.data.code === 500) {
            if (this.$store.state.app.language === 'zh') {
              alert('操作失败')
            } else {
              alert('Operation failed')
            }
          } else {
            this.getBindInfo()
          }
        })
    },

    unbindWeb () {
      webUnbind()
        .then(res => {
          if (res.data.code === 100) {
            if (this.$store.state.app.language === 'zh') {
              alert('参数错误')
            } else {
              alert('Parameter error')
            }
          } else if (res.data.code === 500) {
            if (this.$store.state.app.language === 'zh') {
              alert('操作失败')
            } else {
              alert('Operation failed')
            }
          } else {
            this.getBindInfo()
          }
        })
    },

    getListInfo () {
      let result = []
      getPorts()
        .then(res => {
          result = res.data.interfaces

          result.forEach(element => {
            if (element.function === 'lan') {
              if (element.ip !== '') {
                this.availablePortList.push(element.webname)
                console.log(this.availablePortList)
              }
            }
          })
        })
    },

    getBindInfo () {
      getBind()
        .then(res => {
          if (res.data.code === 200) {
            this.isBinded = res.data.status
            this.bindedPort = res.data.webnetif
          } else {
            if (this.$store.state.app.language === 'zh') {
              alert('操作失败')
            } else {
              alert('Operation failed')
            }
          }
        })
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
    this.getListInfo()
    this.getBindInfo()
    if (this.$store.state.app.language === 'zh') {
      if (this.bindedPort === 'NotBound') {
        this.bindedPort = '未绑定'
      }
      if (this.bindPort === 'Bound' + this.bindedPort) {
        this.bindPort = '已绑定' + this.bindedPort
      }
    } else {
      if (this.bindedPort === '未绑定') {
        this.bindedPort = 'NotBound'
      }
      if (this.bindPort === '已绑定' + this.bindedPort) {
        this.bindPort = 'Bound' + this.bindedPort
      }
    }
  }
}
</script>

