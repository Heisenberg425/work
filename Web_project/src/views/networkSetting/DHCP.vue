<template>
  <section>
    <div class="line_02">
      <span>DHCP</span>
    </div>

    <!--工具条-->

    <el-col :span="24">
      <el-form :inline="true">
        <el-form-item>
          <el-button
            type="primary"
            @click="addDHCP" >{{ $t('DHCP.add') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button
            type="danger"
            @click="batchDeletion">{{ $t('operation.batchdelete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <!--表格-->
    <el-table
      :data="form"
      @selection-change="selsChange"
      style="width: 100%;"
      :header-cell-style="headerStyle">
      <el-table-column
        type="selection"
        :selectable="selectable"
        min-width="30"/>
      <el-table-column
        prop="interface"
        :label="$t('DHCP.interface')"
        min-width="60"/>
      <el-table-column
        prop="start"
        :label="$t('DHCP.start')"
        min-width="60"/>
      <el-table-column
        prop="end"
        :label="$t('DHCP.end')"
        min-width="60"/>
      <el-table-column
        prop="mask"
        :label="$t('DHCP.mask')"
        min-width="60"/>
      <el-table-column
        prop="gateway"
        :label="$t('DHCP.gateway')"
        min-width="60"/>
      <el-table-column
        prop="dns1"
        :label="$t('DHCP.dns1')"
        min-width="60"/>
      <el-table-column
        prop="dns2"
        :label="$t('DHCP.dns2')"
        min-width="60"/>
      <el-table-column
        prop="lease"
        :label="$t('DHCP.lease')"
        min-width="60"/>
      <el-table-column
        prop="enable"
        :label="$t('DHCP.status')"
        min-width="60"/>

      <el-table-column
        prop="behaviour"
        :label="$t('DHCP.behaviour')"
        min-width="100">
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="deleteDHCP(scope.row)">{{ $t('DHCP.deleteDHCP') }}</el-button>
          <el-button
            size="small"
            @click="editDHCP(scope.row)">{{ $t('DHCP.editDHCP') }}</el-button>

        </template>
      </el-table-column>
    </el-table>

    <!--删除
<el-dialog :title="$t('DHCP.deleteDHCP')" :visible.sync="isInDelete">
      <el-form ref="form" :model="form" label-width="8rem">
        <el-form-item prop="delete" :label="$t('DHCP.ifDelete')"></el-form-item>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button type="danger" @click="deleteCancel">{{$t('operation.cancel')}}</el-button>
        <el-button type="primary" @click="deldeteSubmit">{{$t('operation.submit')}}</el-button>
      </div>
    </el-dialog>-->
    <!--修改-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('DHCP.edit')"
      :visible.sync="isInEdit" :close-on-click-modal="false" >
      <el-form
        :model="edit"
        label-width="100px"
        label-position="left"
        ref="edit"
        :rules="rules">
        <el-form-item
          prop="start"
          :label="$t('DHCP.start')">
          <el-input
            v-model="edit.start"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="end"
          :label="$t('DHCP.end')">
          <el-input
            v-model="edit.end"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="mask"
          :label="$t('DHCP.mask')">
          <el-input
            v-model="edit.mask"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="interface"
          :label="$t('DHCP.interface')">

          <el-select
            v-model="edit.interface"
            :placeholder="$t('placeholder.select')"
            @change="change1"
            style="width: 100%;">
            <el-option
              v-for="item in portsListNew"
              :key="item.interface"
              :label="item.interface"
              :value="item.interface"
              :disabled="item.disabled"/>
          </el-select>
        </el-form-item>
        <el-form-item
          prop="gateway"
          :label="$t('DHCP.gateway')">
          <el-input
            v-model="edit.gateway"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="dns1"
          :label="$t('DHCP.dns1')">
          <el-input
            v-model="edit.dns1"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="dns2"
          :label="$t('DHCP.dns2')">
          <el-input
            v-model="edit.dns2"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="lease"
          :label="$t('DHCP.lease')">
          <el-input
            type="number"
            v-model="edit.lease"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="enable"
          :label="$t('DHCP.status')">
          <el-select
            v-model="edit.enable"
            :placeholder="$t('placeholder.select')"
            @change="change2"
            style="width: 100%;">
            <el-option
              v-for="item in editOptions"
              :key="item.value"
              :label="item.label"
              :value="item.value"/>
          </el-select>
        </el-form-item >
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
        type="danger"
          size="small"
          @click="editCancel" >{{ $t('operation.cancel') }}</el-button>
        <el-button
        type="primary"
          size="small"
          @click="editSubmit" >{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>
    <!--新增界面-->
    <div class="thisdialogstyle"> 
    <el-dialog
      :title="$t('DHCP.addNew')"
      :visible.sync="isAddFormVisible" :close-on-click-modal="false">
      <el-form
        :model="addForm"
        label-width="100px"
        label-position="left"
        ref="addForm"
        :rules="rules">
        <el-form-item
          prop="start"
          :label="$t('DHCP.start')">
          <el-input
            v-model="addForm.start"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="end"
          :label="$t('DHCP.end')">
          <el-input
            v-model="addForm.end"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="mask"
          :label="$t('DHCP.mask')">
          <el-input
            v-model="addForm.mask"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="interface"
          :label="$t('DHCP.interface')">

          <el-select
            v-model="addForm.interface"
            :placeholder="$t('placeholder.select')"
            style="width: 100%;">
            <el-option
              v-for="item in portsListNew"
              :key="item.interface"
              :label="item.interface"
              :value="item.interface"
              :disabled="item.disabled"/>
          </el-select>
        </el-form-item>
        <el-form-item
          prop="gateway"
          :label="$t('DHCP.gateway')">
          <el-input
            v-model="addForm.gateway"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="dns1"
          :label="$t('DHCP.dns1')">
          <el-input
            v-model="addForm.dns1"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="dns2"
          :label="$t('DHCP.dns2')">
          <el-input
            v-model="addForm.dns2"
            auto-complete="on"/>
        </el-form-item>
        <el-form-item
          prop="lease"
          :label="$t('DHCP.lease')">
          <el-input
            type="number"
            v-model="addForm.lease"
            auto-complete="on"/>
        </el-form-item >
        <el-form-item
          prop="status"
          :label="$t('DHCP.status')">
          <el-select
            v-model="addForm.status"
            :placeholder="$t('placeholder.select')"
            style="width: 100%;">
            <el-option
              v-for="item in options"
              :key="item.value"
              :label="item.label"
              :value="item.value"/>
          </el-select>
        </el-form-item >
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
        type="danger"
          size="small"
          @click="addCancel" >{{ $t('operation.cancel') }}</el-button>
        <el-button
        type="primary"
          size="small"
          @click="addSubmit" >{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

  </section>
</template>

<script>
import { dhcpServerConfigAction, dhcpServerShowAction, dhcpServerDelAction, getPorts } from '../../api/api.js'
import validate from '@/utils/rules.js'
export default {
  name: 'DHCP',
  data () {
    let enable = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择状态！/Please select status'))
      }
      callback()
    }
    let port = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择网卡接口！/Please select interface'))
      }
      callback()
    }
    return {
      choosenItem: [],
      saveInterface: '',
      deleteInterface: '',
      isInEdit: false,
      isInDelete: false,
      isInchangeStatus: false,
      addLoading: false,
      isInuse: false,
      isAddFormVisible: false,
      options: [{
        value: '1',
        label: '启用'
      }, {
        value: '0',
        label: '禁用'
      }],
      editOptions: [{
        value: '1',
        label: '启用'
      }, {
        value: '0',
        label: '禁用'
      }],
      // 添加页面的数据
      addForm: {
        enable: '',
        start: '', // 开始地址
        end: '', // 结束地址
        interface: '', // 网卡接口
        dns1: '',
        dns2: '',
        mask: '', // 掩码
        gateway: '', // 网关
        lease: '' // 租约时间
      },
      edit: {
        enable: '',
        start: '', // 开始地址
        end: '', // 结束地址
        interface: '', // 网卡接口
        dns1: '',
        dns2: '',
        mask: '', // 掩码
        gateway: '', // 网关
        lease: '' // 租约时间
      }, // 修改的数据
      form: [], // 显示的数据
      portsList: [],
      portsListNew: [],

      rules: {
        gateway: [
          { required: true, message: '网关地址不能为空！/Please input gateway' },
          {
            validator: validate('ip', '请输入正确网关地址！/Please input correct gateway', '网关地址不能为空！/Please input gateway'), trigger: 'blur'
          }
        ],

        start: [
          { required: true, message: 'IP地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP地址！/Please input correct IP', 'IP地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        end: [
          { required: true, message: 'IP地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP地址！/Please input correct IP', 'IP地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        mask: [
          { required: true, message: '掩码地址不能为空！/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确掩码地址！/Please input correct netmask', '掩码地址不能为空！/Please input netmask'), trigger: 'blur'
          }
        ],
        dns1: [
          { required: true, message: 'DNS地址不能为空！/Please input DNS' },
          {
            validator: validate('ip', '请输入正确DNS地址！/Please input correct DNS', 'DNS地址不能为空！/Please input DNS'), trigger: 'blur'
          }
        ],
        dns2: [
          {
            validator: validate('ip', '请输入正确DNS地址！/Please input correct DNS', ''), trigger: 'blur'
          }
        ],
        lease: [
          { required: true, message: '时间不能为空！/Please input time' },
          {
            validator: validate('time', '时间超过设定！/Please input correct time', '时间不能为空！/Please input time'), trigger: 'blur'
          }
        ],
        enable: [
          { required: true, message: '请选择状态！/Please select status' },
          {
            validator: enable, trigger: 'blur'
          }
        ],
        status: [
          { required: true, message: '请选择状态！/Please select status' },
          {
            validator: enable, trigger: 'blur'
          }
        ],
        interface: [
          { required: true, message: '请选择网卡接口！/Please select interface' },
          {
            validator: port, trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    selsChange (sels) {
      this.choosenItem = sels
      console.log(this.choosenItem)
    },
    // 批量删除
    async batchDeletion() {
      let i
      for (i = 0; i < this.choosenItem.length; i++) {
        del(this.choosenItem[i])
      }

      async function del (item) {
        let para = Object.assign({}, item)

        para.interface = item.interface
        dhcpServerDelAction(para).then(res => {

        }
        )
      }
      this.getPortsInfo()
      this.getInfo()
      this.isInDelete = false
    },
    // 删除接口
    deleteDHCP (val) {
      this.deleteInterface = val.interface
      if (this.$store.state.app.language === 'zh') {
        this.$confirm('此操作将删除该接口, 是否继续?', '提示', {
          confirmButtonText: '确定',
          cancelButtonText: '取消',
          type: 'warning'
        }).then(() => {
          this.deldeteSubmit()
          this.$message({
            type: 'success',
            message: '删除成功!'
          })
        }).catch(() => {
          this.$message({
            type: 'info',
            message: '已取消删除'
          })
        })
      } else {
        this.$confirm('Are you sure to delte this interface?', 'Warning', {
          confirmButtonText: 'Delete',
          cancelButtonText: 'Cancle',
          type: 'warning'
        }).then(() => {
          this.deldeteSubmit()
          this.$message({
            type: 'success',
            message: 'Deleted!'
          })
        }).catch(() => {
          this.$message({
            type: 'info',
            message: 'Cancled'
          })
        })
      }
    },
    editCancel () {
      this.getPortsInfo()
      this.getInfo()
      this.isInEdit = false
    },
    // 提交修改
    editSubmit () {
      // this.$refs[this.edit.lease].clearValidate()

      this.$refs['edit'].validate((valid) => {
        if (!valid) {
          this.$message.error('请正确输入相关信息/Please input correct information')
        } else {
          let para = {}
          para.lease = this.edit.lease
          para.lease = Number(para.lease)
          if (this.edit.enable === 'On' || this.edit.enable === '启用' || this.edit.enable === '1') {
            para.enable = 1
            console.log('555')
          } else {
            para.enable = 0
          }
          para.start = this.edit.start
          para.end = this.edit.end
          para.dns1 = this.edit.dns1
          para.dns2 = this.edit.dns2
          para.mask = this.edit.mask
          para.gateway = this.edit.gateway
          para.interface = this.edit.interface
          console.log('para+' + this.edit.enable)
          this.deleteInterface = this.saveInterface
          this.deldeteSubmit()
          dhcpServerConfigAction(para)
            .then(res => {
              if (res.data.code === 200) {
                this.isInEdit = false
                this.getPortsInfo()
                this.getInfo()
              } else {
                if (this.$store.state.app.language === 'zh') {
                  this.$message.error('修改失败，请检查')
                } else {
                  this.$message.error('Edit failed, please check')
                }
              }

              console.log('777')
            })

            .catch(error => {
              console.log(error)
            })
        }
      })
    },
    /* deleteDHCP(val) {
      console.log(val)
      this.deleteInterface = val.interface
      this.isInDelete = true
    }, */
    deleteCancel () {
      this.isInDelete = false
    },
    // 提交删除
    deldeteSubmit () {
      let para = {}
      para.interface = this.deleteInterface
      dhcpServerDelAction(para).then(res => {
        this.getPortsInfo()
        this.getInfo()
        this.isInDelete = false
      }
      )
    },
    editDHCP (val) {
      if (this.$store.state.app.language === 'zh') {
        this.editOptions = [{
          value: '1',
          label: '启用'
        }, {
          value: '0',
          label: '禁用'
        }]
      } else {
        this.editOptions = [{
          value: '1',
          label: 'Enable'
        }, {
          value: '0',
          label: 'Disable'
        }]
      }
      this.edit.dns1 = val.dns1
      this.edit.dns2 = val.dns2
      this.edit.enable = val.enable
      this.edit.end = val.end
      this.edit.gateway = val.gateway
      this.edit.interface = val.interface
      this.edit.lease = val.lease
      this.edit.mask = val.mask
      this.edit.start = val.start
      this.saveInterface = val.interface
      this.isInEdit = true
      this.getPortsInfo()
      this.getInfo()
      console.log(this.edit)
    },
    change1 (val) {
      this.edit.interface = val
      console.log(val)
    },
    change2 (val) {
      this.edit.enable = val
      console.log(val)
    },

    addDHCP () {
      // this.resetvalidate('addForm')
      this.isAddFormVisible = true
      if (this.$store.state.app.language === 'zh') {
        // this.rules.start[0].validator = validate('ip', '请输入正确IP地址', 'IP地址不能为空')
        this.options = [{
          value: '1',
          label: '启用'
        }, {
          value: '0',
          label: '禁用'
        }]
      } else {
        // this.rules.start[0].validator = validate('ip', 'Please input correct IP', 'Please input IP')
        this.options = [{
          value: '1',
          label: 'Enable'
        }, {
          value: '0',
          label: 'Disable'
        }]
      }
    },
    headerStyle () {
      return this.header()
    },
    // 添加提交
    addSubmit () {
      this.$refs['addForm'].validate((valid) => {
        if (!valid) {
          this.$message.error('请正确输入相关信息/Please input correct information')
        } else {
          let para = {}
          para.lease = this.addForm.lease
          para.lease = Number(para.lease)
          para.enable = this.addForm.status
          para.enable = Number(para.enable)
          para.start = this.addForm.start
          para.end = this.addForm.end
          para.dns1 = this.addForm.dns1
          para.dns2 = this.addForm.dns2
          para.mask = this.addForm.mask
          para.gateway = this.addForm.gateway
          para.interface = this.addForm.interface
          console.log('666')

          console.log(para)
          dhcpServerConfigAction(para)
            .then(res => {
              if (res.data.code === 200) {
                this.getPortsInfo()
                this.getInfo()
                this.isAddFormVisible = false
              } else {
                if (this.$store.state.app.language === 'zh') {
                  this.$message.error('添加失败，请检查')
                } else {
                  this.$message.error('Add failed, please check')
                }
              }
            })
            .catch(error => {
              console.log(error)
            })
        }
      })
    },
    addCancel () {
      this.isAddFormVisible = false
      // this.refs['addForm'].resetFields()
    },

    getInfo () {
      let para = {}
      para.page = 1
      dhcpServerShowAction(para)
        .then((res) => {
          if (res.data.code === 200) {
            this.form = res.data.data
            for (let i = 0; i < res.data.data.length; i++) {
              console.log(this.portsList.length)
              for (let j = 0; j < this.portsList.length; j++) {
                if (this.form[i].interface === this.portsList[j]) {
                  this.portsListNew[j].disabled = true
                  console.log(this.portsListNew[j].interface + '222' + this.portsListNew[j].disabled)
                }
              }

              if (this.form[i].enable === 1) {
                if (this.$store.state.app.language === 'zh') {
                  this.form[i].enable = '启用'
                } else {
                  console.log('333')
                  this.form[i].enable = 'On'
                }
              } else {
                if (this.$store.state.app.language === 'zh') {
                  this.form[i].enable = '禁用'
                } else {
                  console.log('444')
                  this.form[i].enable = 'Off'
                }
              }
            }
            console.log(this.form)
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    getPortsInfo () {
      getPorts()
        .then(res => {
          if (res.data.code === 200) {
            console.log('reslength' + res.data.interfaces.length)
            for (let i = 0; i < res.data.interfaces.length; i++) {
              this.portsList[i] = res.data.interfaces[i].webname
            }
            this.portsListNew = res.data.interfaces
            for (let j = 0; j < this.portsList.length; j++) {
              console.log('portinterface' + this.portsList[j])
              this.portsListNew[j].interface = this.portsList[j]
              this.portsListNew[j].disabled = false
            }
            console.log('interface' + this.portsListNew)
          }
        })
    },
    async getReady () {
      await this.getPortsInfo()
      await this.getInfo()
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.getInfo()
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
    this.getReady()
  }
}
</script>

<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}
</style>
