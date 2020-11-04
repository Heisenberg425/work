<template>
  <section>
    <div class="line_02">
      <span>{{ $t('kidVPN.client.title') }}</span>
    </div>

    <el-col :span="24" style="padding: 0">
      <el-form inline>
        <el-form-item>
          <el-button type="primary" @click="addClient">{{ $t('kidVPN.client.button1') }}</el-button>
        </el-form-item>
        <!-- 10.29 pky 要求去掉批量删除功能 -->
        <!-- <el-form-item>
          <el-button
            type="danger"
            @click="batchDeletion">{{ $t('kidVPN.client.button2') }}</el-button>
        </el-form-item>-->
      </el-form>
    </el-col>

    <el-table
      :data="addedClientList"
      style="width: 100%"
      @selection-change="selChange"
      :header-cell-style="headerStyle"
    >
      <el-table-column prop="vndid" :label="$t('kidVPN.client.vndid')" min-width="120"/>
      <el-table-column prop="serip" :label="$t('kidVPN.client.serip')" min-width="120"/>
      <el-table-column prop="locip" :label="$t('kidVPN.client.locip')" min-width="120"/>
      <el-table-column prop="netmask" :label="$t('kidVPN.client.netmask')" min-width="120"/>
      <el-table-column prop="gateway" :label="$t('kidVPN.client.gateway')" min-width="120"/>
      <el-table-column prop="mtu" :label="$t('kidVPN.client.mtu')" min-width="60"/>
      <el-table-column prop="position" :label="$t('kidVPN.client.serverLoc')" min-width="120"/>
      <el-table-column prop="status" :label="$t('kidVPN.client.status')" min-width="120"/>
      <el-table-column :label="$t('operation.operation')" min-width="60">
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="deleteClient(scope.$index, scope.row)"
          >{{ $t('operation.delete') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      @current-change="handleCurrentChange"
      :current-page="currentPage"
      :page-size="5"
      layout="total, prev, pager, next, jumper"
      :total="total"
      style="float:right"
    />
    <div class="thisdialogstyle">
      <el-dialog
        :title="$t('kidVPN.client.title1')"
        :visible.sync="isAddingClient"
        :close-on-click-modal="false"
      >
        <el-form
          :model="VNDForm"
          :rules="rules"
          ref="VNDForm"
          :label-width="labelWidth"
          label-position="left"
        >
          <el-form-item prop="ipaddr" :label="$t('kidVPN.client.ipaddr')">
            <el-input v-model="VNDForm.ipaddr"/>
          </el-form-item>
          <el-form-item prop="netmask" :label="$t('kidVPN.client.netmask')">
            <el-input v-model="VNDForm.netmask"/>
          </el-form-item>
          <el-form-item prop="gateway" :label="$t('kidVPN.client.gateway')">
            <el-input v-model="VNDForm.gateway"/>
          </el-form-item>
          <el-form-item prop="mac" :label="$t('kidVPN.client.mac')">
            <el-input v-model="VNDForm.mac"/>
          </el-form-item>
          <el-form-item prop="mtu" :label="$t('kidVPN.client.mtu')">
            <el-input v-model="VNDForm.mtu"/>
          </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
          <el-button type="danger" @click="triggerAddKidVPNClient">{{ $t('operation.cancel') }}</el-button>
          <el-button type="primary" @click="kidVPNNextStep">{{ $t('kidVPN.client.button3') }}</el-button>
        </div>
      </el-dialog>
    </div>
    <div class="thisdialogstyle">
      <el-dialog
        :title="$t('kidVPN.client.title2')"
        :visible.sync="isLinkingClient"
        class="key"
        :close-on-click-modal="false"
      >
        <el-form
          ref="kidVPNForm"
          :model="kidVPNForm"
          :label-width="labelWidth"
          label-position="left"
          :rules="rules"
        >
          <el-form-item prop="ipaddr" :label="$t('kidVPN.client.ipaddr1')">
            <el-input v-model="kidVPNForm.ipaddr"/>
          </el-form-item>
          <el-form-item prop="port" :label="$t('kidVPN.client.port')">
            <el-input type="number" v-model="kidVPNForm.port"/>
          </el-form-item>
          <el-form-item prop="passwd" :label="$t('kidVPN.client.pw')">
            <el-input v-model="kidVPNForm.passwd" show-password/>
          </el-form-item>
          <el-form-item prop="aeskey" :label="$t('kidVPN.client.aeskey')">
            <el-input v-model="kidVPNForm.aeskey" class="aeskey"/>
          </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
          <el-button type="danger" @click="triggerLinkKidVPNClient">{{ $t('operation.cancel') }}</el-button>
          <el-button type="primary" @click="linkKidVPN">{{ $t('operation.determine') }}</el-button>
        </div>
      </el-dialog>
    </div>
  </section>
</template>

/**
 * 该页面代码已合
 */
<script>
import { addVND, addKidVPN, delKidVPN, getKidVPNInfo } from '@/api/api.js'
import validate from '@/utils/rules.js'
let timer
export default {
  name: 'Client',
  data() {
    let portRules = (rule, value, callback) => {
      var reg = /.*\..*/

      if (value === '') {
        callback(new Error('请输入服务器端口！/Please input server port'))
        console.log('端口号')
      } else if (reg.test(value) === true) {
        callback(new Error('请输入整数！/Please input integer'))
        console.log('shuzi')
      }
      callback()
    }
    let mtu = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('MTU不能为空！/Please input MTU'))
      } else if (value > 1472 || value < 1280) {
        callback(
          new Error(
            '请输入1280~1472之间的数字！/Please enter number between 1280 and 1472'
          )
        )
      }
      callback()
    }
    let passwd = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入密码！/Please input password'))
      }
      callback()
    }
    let aeskey = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入AES Key！/Please input AES Key'))
      }
      callback()
    }
    return {
      isAddingClient: false,
      isLinkingClient: false,
      currentPage: 1,
      total: 0,
      addedClientList: [],
      VNDForm: {
        ipaddr: '',
        netmask: '',
        gateway: '',
        mtu: '',
        mac: ''
      },
      kidVPNForm: {
        vndid: Number, // 虚拟网卡IP
        ipaddr: '', // IP地址
        port: '',
        aeskey: '', // AES KEY
        mtu: '', // MTU
        passwd: '',
        handle: ''
      },
      vndid: '',
      chosenItems: [],
      labelWidth: '8rem',

      rules: {
        ipaddr: [
          { required: true, message: 'IP地址不能为空!/Please input IP' },
          {
            validator: validate(
              'ip',
              '请输入正确IP！/Please input correct IP',
              'IP不能为空！/Please input IP'
            ),
            trigger: 'blur'
          }
        ],
        netmask: [
          { required: true, message: '子网掩码不能为空!/Please input netmask' },
          {
            validator: validate(
              'netmask',
              '请输入正确子网掩码！/Please input correct netmask',
              '子网掩码不能为空！/Please input netmask'
            ),
            trigger: 'blur'
          }
        ],
        gateway: [
          { required: true, message: '网关地址不能为空!/Please input gateway' },
          {
            validator: validate(
              'ip',
              '请输入正确网关地址！/Please input correct gateway',
              '网关地址不能为空！/Please input gateway'
            ),
            trigger: 'blur'
          }
        ],
        mac: [
          { required: true, message: 'MAC地址不能为空！/Please input MAC' },
          {
            validator: validate(
              'mac',
              '请输入正确MAC地址！/Please input correct MAC',
              'MAC地址不能为空！/Please input MAC'
            ),
            trigger: 'blur'
          }
        ],
        mtu: [
          { required: true, message: 'MTU不能为空！/Please input MTU' },
          {
            validator: mtu,
            trigger: 'blur'
          }
        ],
        port: [
          {
            required: true,
            message: '请输入服务器端口！/Please input server port'
          },
          {
            // validator: validate('number', '请输入数字！/Please input correct number', '请输入服务器端口！/Please input server port'),
            validator: portRules,
            trigger: 'blur'
          }
        ],
        passwd: [
          { required: true, message: '请输入密码！/Please input password' },
          {
            validator: passwd,
            trigger: 'blur'
          }
        ],
        aeskey: [
          { required: true, message: '请输入AES Key！/Please input AES Key' },
          {
            validator: aeskey,
            trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    headerStyle() {
      return this.header()
    },
    handleCurrentChange(val) {
      this.currentPage = val
      this.getInfo()
    },
    triggerAddKidVPNClient() {
      this.isAddingClient = !this.isAddingClient
    },
    triggerLinkKidVPNClient() {
      this.isLinkingClient = !this.isLinkingClient
    },
    addClient() {
      this.triggerAddKidVPNClient()
    },
    kidVPNNextStep() {
      this.$refs['VNDForm'].validate(valid => {
        console.log(valid)
        if (valid) {
          let para = Object.assign({}, this.VNDForm)

          addVND(para)
            .then(res => {
              if (res.data.code === 200) {
                this.vndid = res.data.vndid
              }
              this.triggerAddKidVPNClient()
              this.triggerLinkKidVPNClient()
            })
            .catch(error => {
              console.log(error)
            })
        }
      })
    },

    linkKidVPN() {
      this.$refs['kidVPNForm'].validate(valid => {
        console.log(valid)
        if (valid) {
          let para = Object.assign({}, this.kidVPNForm)
          para.port = Number(para.port)
          para.vndid = this.vndid
          para.mtu = this.VNDForm.mtu
          para.handle = 'addClient'

          addKidVPN(para)
            .then(res => {
              if (res.data.code === 200) {
                this.getInfo()
              } else if (res.data.code === 100) {
                if (this.$store.state.app.language === 'zh') {
                  this.$message({
                    message: '参数错误',
                    type: 'error'
                  })
                } else {
                  this.$message({
                    message: 'ParameterError',
                    type: 'error'
                  })
                }
              } else if (res.data.code === 500) {
                if (this.$store.state.app.language === 'zh') {
                  this.$message({
                    message: '操作失败',
                    type: 'error'
                  })
                } else {
                  this.$message({
                    message: 'OperateFailed',
                    type: 'error'
                  })
                }
              }
              this.triggerLinkKidVPNClient()
            })
            .catch(error => {
              console.log(error)
            })
        }
      })
    },

    deleteClient(index, val) {
      let para = {}
      para.vndid = val.vndid

      delKidVPN(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '删除成功',
                type: 'success'
              })
            } else {
              this.$message({
                message: 'DeleteSuccess',
                type: 'success'
              })
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.getInfo()
    },
    selChange(sels) {
      this.chosenItems = sels
    },
    async batchDeletion() {
      let i
      for (i = 0; i < this.chosenItems.length; i++) {
        await this.deleteClient(i, this.chosenItems[i])
      }
    },
    Utf16ToUtf8(unicode) {
      var uchar
      var utf8str = ''
      var i

      for (i = 0; i < unicode.length; i += 2) {
        uchar = (unicode[i] << 8) | unicode[i + 1] // UNICODE为2字节编码，一次读入2个字节
        utf8str = utf8str + String.fromCharCode(uchar) // 使用String.fromCharCode强制转换
      }
      return utf8str
    },
    getInfo() {
      // let result
      let para = {}
      para.page = this.currentPage
      getKidVPNInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.currentPage === res.data.page) {
              if (res.data.data.length !== 0) {
                console.log('infor ' + res.data.data.length)

                let param = res.data.data
                this.total = res.data.total

                for (let i = 0; i < param.length; i++) {
                  if (param[i].type === 'servervpn') {
                    console.log('in clinet getInfo' + JSON.stringify(param[i]))
                    param.splice(i, 1)
                    i -= 1
                    this.total -= 1
                    console.log('param.position1')
                    console.log(param.position)
                  }
                }

                if (param.status === 'OFF') {
                  if (this.$store.state.app.language === 'zh') {
                    param.status = '连接成功'
                  } else {
                    param.status = 'ConnectSuccess'
                  }
                } else {
                  if (this.$store.state.app.language === 'zh') {
                    param.status = '连接失败'
                  } else {
                    param.status = 'ConnectFailed'
                  }
                }
                // for (let i = 0; i < param.length; i++) {
                //   param[i].position = this.Utf16ToUtf8(param[i].position)
                // }
                this.addedClientList = param
                console.log('param.position2')

                console.log(param.length)
                console.log('this.addedClientList')
                console.log(this.addedClientList)
              } else if (res.data.page > 1) {
                this.currentPage -= 1
                this.getInfo()
                this.total = res.data.total
                console.log('6666')
              } else {
                this.addedClientList = res.data.data
                this.total = res.data.total
                console.log('7777')
              }
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    // 处理兄弟组件的bus事件
    liseterUpdate() {
      this.bus.$on('update', this.getInfo())
    }
  },
  watch: {
    '$store.state.app.language': function() {
      console.log(this.$store.state.app.language)
    }
  },
  mounted() {
    this.getInfo()
    timer = setInterval(this.getInfo, 2000)
  },
  destroyed() {
    clearInterval(timer)
  }
}
</script>

<style scoped>
.el-input {
  width: 80%;
}
.el-select {
  width: 80%;
}
textarea {
  resize: none;
  width: 80%;
  height: 100%;
  padding: 0;
  margin-top: 0.5rem;
}
</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
  width: 30%;
}
</style>
