<template>
  <section>
    <el-col
      :span="24"
      class="title"
      >
      <el-form inline>
        <el-form-item>
          <el-button
            type="primary"
            @click="addServer"
            v-if="isEmpty"
            >{{ $t('kidVPN.server.button5') }}</el-button>
        <!-- <el-button type="danger" size="mini" @click="delServer">{{$t('kidVPN.server.button1')}}</el-button> -->
        </el-form-item>
      </el-form>
    </el-col>

    <el-table
      :data="addedClientList"
      :header-cell-style="headerStyle"
      >
      <el-table-column
        prop="vndid"
        :label="$t('kidVPN.server.vndid')"
        min-width="120"
        />
      <el-table-column
        prop="serip"
        :label="$t('kidVPN.server.serip')"
        min-width="150"
        />
      <el-table-column
        prop="locip"
        :label="$t('kidVPN.server.locip')"
        min-width="120"
        />
      <el-table-column
        prop="netmask"
        :label="$t('kidVPN.server.netmask')"
        min-width="120"
        />
      <el-table-column
        prop="gateway"
        :label="$t('kidVPN.server.gateway')"
        min-width="120"
        />
      <el-table-column
        prop="mac"
        :label="$t('kidVPN.server.mac')"
        min-width="120"
        />
      <el-table-column
        prop="mtu"
        type="number"
        :label="$t('kidVPN.server.mtu')"
        min-width="100"
        />
      <el-table-column
        :label="$t('operation.operation')"
        min-width="140"
        >
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="delServer(scope.index, scope.row)"
            >{{ $t('operation.delete') }}</el-button>
          <el-button
            size="small"
            @click="getAESKeyInfo(scope.row)"
            >{{ $t('kidVPN.server.showAESkey') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      :total="total"
      :current-page="currentPage"
      :page-size="pageSize"
      layout="total, prev, pager, next, jumper"
      @current-change="handleChange"
      style="float:right"
      />

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('kidVPN.server.title2')"
      :visible.sync="isAddingServer"
      :close-on-click-modal="false"
      >
      <el-form
        :model="form"
        :rules="rules"
        ref="form"
        label-width="120px"
        label-position="left"
        >
        <el-form-item
          prop="ipaddr"
          :label="$t('kidVPN.server.ipaddr')"
          >
          <el-input v-model="form.ipaddr" />
        </el-form-item>
        <el-form-item
          prop="netmask"
          :label="$t('kidVPN.server.netmask')"
          >
          <el-input v-model="form.netmask" />
        </el-form-item>
        <el-form-item
          prop="gateway"
          :label="$t('kidVPN.server.gateway')"
          >
          <el-input v-model="form.gateway" />
        </el-form-item>
        <el-form-item
          prop="mac"
          :label="$t('kidVPN.server.mac')"
          >
          <el-input v-model="form.mac" />
        </el-form-item>
        <el-form-item
          prop="mtu"
          type="number"
          :label="$t('kidVPN.server.mtu')"
          >
          <el-input v-model="form.mtu" />
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer"
        >
        <el-button
          type="danger"
          @click="triggerAddingServer"
          >{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="KidVPNServerNextStop"
          >{{ $t('kidVPN.server.button3') }}</el-button>
      </div>
    </el-dialog>
</div>

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('kidVPN.server.title3')"
      :visible.sync="isInitingKey"
      class="key"
      :close-on-click-modal="false"
      >
      <el-form
       ref="initAESKeyForm"
        :model="initAESKeyForm"
        :rules="rules"
        label-width="150px"
        label-position="left"
        >
        <el-form-item
          prop="ipaddr"
          :label="$t('kidVPN.server.ipaddr1')"
          >
          <el-input v-model="initAESKeyForm.ipaddr" />
        </el-form-item>
        <el-form-item
          prop="port"
          :label="$t('kidVPN.client.port')"
          >
          <el-input
            type="number"
            v-model="initAESKeyForm.port"
            />
        </el-form-item>
        <el-form-item
          prop="bit"
          :label="$t('kidVPN.server.bit')"
          >
          <el-select v-model="initAESKeyForm.bit">
            <el-option
              v-for="(item, index) in bitList"
              :key="index"
              :label="item"
              :value="item"
              />
          </el-select>
        </el-form-item>
        <el-form-item
          prop="passwd"
          :label="$t('kidVPN.client.pw')"
          >
          <el-input v-model="initAESKeyForm.passwd" show-password/>
        </el-form-item>
        <el-form-item label='自行生成Aeskey'>
          <el-checkbox v-model="isGeneratedAeskey">
          </el-checkbox>
        </el-form-item>
        <el-form-item
        v-if="!isGeneratedAeskey"
          prop="aeskey"
          :label="$t('kidVPN.server.aeskey')"
          >
          <textarea v-model="initAESKeyForm.aeskey" />
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <!-- <el-button
          type="danger"
          @click="initAESKey">{{ $t('kidVPN.server.button4') }}</el-button> -->
        <el-button
          type="primary"
          :disabled="!canCreateServer"
          @click="saveKey">{{ $t('kidVPN.server.button5') }}</el-button>
      </div>
    </el-dialog>
</div>

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('kidVPN.server.title3')"
      :visible.sync="isGettingKey"
      :close-on-click-modal="false"
      class="key">
      <el-form
        :model="getAESKeyForm"
        label-width="8rem">
        <el-form-item
          prop="aeskey"
          :label="$t('kidVPN.server.aeskey')">
          <textarea v-model="getAESKeyForm.aeskey"/>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          type="danger"
          @click="closeGettingKey">{{ $t('kidVPN.server.button6') }}</el-button>
      </div>
    </el-dialog>
</div>

  </section>
</template>

<script>
import {
  addVND,
  addKidVPN,
  generateAESKey,
  getAESKey,
  getKidVPNInfo,
  delKidVPN
} from '@/api/api.js'
import validate from '@/utils/rules.js'

export default {
  name: 'Server',
  data () {
    let mtu = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('MTU不能为空！/Please input MTU'))
      } else if (value > 1472 || value < 1280) {
        callback(new Error('请输入1280~1472之间的数字！/Please enter number between 1280 and 1472'))
      }
      callback()
    }

    let bit = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择AES Key bit位数！/Please select AES Key Width'))
      }
      callback()
    }
    let passwd = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入密码！/Please input password'))
      }
      callback()
    }
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
    return {
      isAddingServer: false,
      isInitingKey: false,
      isGettingKey: false,
      isEmpty: true,
      canCreateServer: true, //  创建VPN服务器 按钮是否可用
      addedForm: [],
      addedClientList: [],
      form: {
        serip: '',
        netmask: '',
        gateway: '',
        mtu: '',
        mac: '',
        vndid: ''
      },
      initAESKeyForm: {
        vndid: '',
        ipaddr: '',
        port: '',
        bit: '',
        aeskey: '',
        passwd: ''
      },
      getAESKeyForm: {
        ipaddr: '',
        aeskey: ''
      },
      bitList: ['128', '192', '256'],
      AESKey: '',
      vndid: '',

      isGeneratedAeskey: true,

      total: 0,
      currentPage: 1,
      pageSize: 5,

      rules: {
        ipaddr: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP不能为空！/Please input IP'),
            trigger: 'blur'
          }
        ],
        netmask: [
          { required: true, message: '子网掩码不能为空！Please input netmask' },
          {
            validator: validate(
              'netmask',
              '请输入正确子网掩码！/Please input correct netmask',
              '子网掩码不能为空！Please input netmask'
            ),
            trigger: 'blur'
          }
        ],
        gateway: [
          { required: true, message: '网关地址不能为空！/Please input gateway' },
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
            validator: mtu, trigger: 'blur'
          }
        ],
        port: [
          { required: true, message: '请输入服务器端口！/Please input server port' },
          {
            // validator: validate('number', '请输入数字！/Please input correct number', '请输入服务器端口！/Please input server port'),
            validator: portRules,
            trigger: 'blur'
          }
        ],
        bit: [
          { required: true, message: '请选择AES Key bit位数！/Please select AES Key Width' },
          {
            validator: bit, trigger: 'blur'
          }
        ],
        passwd: [
          { required: true, message: '请输入密码！/Please input password' },
          {
            validator: passwd, trigger: 'blur'
          }
        ]
      }
    }
  },
  watch: {
    'initAESKeyForm.bit': function() {
      if (this.initAESKeyForm.bit !== '' && this.isGettingKey === true) {
        this.canCreateServer = true
      } else if (this.initAESKeyForm.aeskey !== '') {
        this.canCreateServer = true
      }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    addServer () {
      this.triggerAddingServer()
    },
    triggerAddingServer () {
      this.isAddingServer = !this.isAddingServer
    },
    triggerInitingKey () {
      this.isInitingKey = !this.isInitingKey
    },
    triggerGettingKey () {
      this.isGettingKey = !this.isGettingKey
    },
    triggerCanCreateServer () {
      this.canCreateServer = !this.canCreateServer
    },
    closeGettingKey () {
      this.triggerGettingKey()
    },
    handleChange (val) {
      this.currentPage = val
      this.getInfo()
    },
    KidVPNServerNextStop () {
      console.log('hello')
      this.$refs['form'].validate(valid => {
        console.log(valid)
        if (valid) {
          let para = Object.assign({}, this.form)
          // para.handle = 'addServer'
          addVND(para).then(res => {
            if (res.data.code === 200) {
              this.vndid = res.data.vndid
            }
          })
          this.triggerAddingServer()
          this.triggerInitingKey()
          this.getInfo()
          // this.$refs["forms"].resetFields()
        }
      })
    },
    initAESKey () {
      if (this.initAESKeyForm.bit === '') {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '请设置AES Key bit位数',
            type: 'error'
          })
        } else {
          this.$message({
            message: 'Please set AES Key Width',
            type: 'error'
          })
        }
      } else {
        let para = {}
        para.bit = this.initAESKeyForm.bit
        generateAESKey(para)
          .then(res => {
            if (res.data.code === 200) {
              this.initAESKeyForm.aeskey = res.data.aeskey
              // this.triggerGettingKey()
              // this.canCreateServer = true
              this.saveKeyNext()
            }
          })
          .catch(err => {
            console.log(err)
          })
      }
    },
    saveKey () {
      this.$refs['initAESKeyForm'].validate(async valid => {
        console.log(valid)
        if (valid) {
          if (this.isGeneratedAeskey) {
            await this.initAESKey()
          }
        }
      })
    },

    saveKeyNext () {
      let para = Object.assign({}, this.initAESKeyForm)
      para.port = Number(para.port)
      para.handle = 'addServer'
      para.vndid = this.vndid
      para.mtu = this.form.mtu
      addKidVPN(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '发送成功',
                type: 'success'
              })
            } else {
              this.$message({
                message: 'SendSuccess',
                type: 'success'
              })
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.triggerInitingKey()
      // this.addedClientList = Object.assign( {}, this.form )
      this.getServerInfo()
    },

    delServer (index, val) {
      let para = {}
      para.vndid = val.vndid
      delKidVPN(para).then(res => {
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
          this.getServerInfo()
          // this.$refs['addedClientList'].resetFields()
          this.bus.$emit('update')
        }
      })
    },
    getAESKeyInfo (val) {
      console.log('val is ' + val)
      let para = {
        vndid: val.vndid
      }
      getAESKey(para)
        .then(res => {
          this.getAESKeyForm.aeskey = res.data.aeskey
          this.triggerGettingKey()
        })
        .catch(err => {
          console.log(err)
        })
    },
    iconToGetAESKeyInfo () {
      let para = {}
      para.vndid = this.addedClientList.vndid
      getAESKey(para)
        .then(res => {
          this.getAESKeyForm.aeskey = res.data.aeskey
        })
      // this.triggerGettingKey()
    },
    getInfo () {
      let para = {}
      para.page = this.currentPage
      this.addedClientList = []
      getKidVPNInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.currentPage === res.data.page) {
              // if (res.data.total !== 0) {
              //   for (let i = 0; i < res.data.data.length; i++) {
              //     if (res.data.data[i].type === 'servervpn') {
              //       // this.addedClientList = res.data.data[i]
              //       this.addedClientList.push(res.data.data[i])
              //       this.total += 1
              //     }
              //   }
              // }
              if (res.data.data.length !== 0) {
                console.log('infor ' + res.data.data.length)

                let param = res.data.data
                this.total = res.data.total

                for (let i = 0; i < param.length; i++) {
                  if (param[i].type === 'clientvpn') {
                    console.log('in sever getInfo' + JSON.stringify(param[i]))
                    param.splice(i, 1)
                    this.total -= 1
                  }
                }
              } else if (res.data.page > 1) {
                this.currentPage -= 1
                this.getInfo()
                this.total = res.data.total
              }
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    getServerInfo () {
      let para = {}
      para.page = this.currentPage
      // this.addedClientList = []
      getKidVPNInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.currentPage === res.data.page) {
              if (res.data.data.length !== 0) {
                console.log('infor ' + res.data.data.length)

                let param = res.data.data
                this.total = res.data.total

                for (let i = 0; i < param.length; i++) {
                  if (param[i].type === 'clientvpn') {
                    console.log('in clinet getInfo' + JSON.stringify(param[i]))
                    param.splice(i, 1)
                    i -= 1 // splice会改变原数组
                    this.total -= 1
                  }
                }
                this.addedClientList = param
                console.log(this.addedClientList)
              } else if (res.data.page > 1) {
                this.currentPage -= 1
                this.getInfo()
                this.total = res.data.total
              } else {
                this.addedClientList = res.data.data
                this.total = res.data.total
              }
            }
          }
        })
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
    }
  },
  mounted () {
    // this.getInfo()
    this.getServerInfo()
  }
}
</script>

<style scoped>
.title {
  text-align: left;
  vertical-align: baseline;
}
.form {
  overflow: hidden;
  margin-top: 1rem;
  background: #bbbbbb;
}
.form > .el-form {
  margin: 15px;
}
img {
  width: 20px;
  height: 20px;
}
textarea {
  height: 90px;
  line-height: 30px;
  resize: none;
  
}
.key .el-input,
.el-select,
textarea {
  width: 80%;
}
</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}

</style>
