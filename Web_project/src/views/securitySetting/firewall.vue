<template>
  <div>

    <div class="line_02">
      <span>{{ $t('securitySetting.firewall') }}</span>
    </div>

    <el-col :span="24">
      <el-form :inline="true">
        <el-form-item>
          <el-button
            type="primary"
            @click="handleAdd">{{ $t('securitySetting.add') }}</el-button>
        </el-form-item>
        <!-- <el-form-item>
          <el-button
            type="primary"
            @click="handleEnable">{{ $t('securitySetting.start') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button
            type="danger"
            @click="handleDisable">{{ $t('securitySetting.stop') }}</el-button>
        </el-form-item> -->
        <!-- <el-form-item>
          <el-input
            v-model="portChoosen"
            placeholder="根据网口搜索"
            style="width: 100%;"/>
        </el-form-item> -->
        <el-form-item>
          <div class="shortselect">
          <el-select
            v-model="portChoosen"
            :placeholder="$t('placeholder.portChoose')">
            <el-option
              v-for="item in ports"
              :key="item.webname"
              :label="item.webname"
              :value="item.webname" />
          </el-select>
          </div>
        </el-form-item>
        <el-form-item>
          <el-button
            @click.native="search"
            type="primary">{{ $t('operation.search') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button
            type="danger"
            @click="batchDeletion">{{ $t('operation.batchdelete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <!-- <el-table :data="rulesList" :header-cell-style="headerStyle">
      <el-table-column prop="port" label="网口" align="center"></el-table-column>
      <el-table-column label="IP策略" align="center">
        <el-table-column prop="ips" label="开始地址" align="center"></el-table-column>
        <el-table-column prop="ipe" label="结束地址" align="center"></el-table-column>
      </el-table-column>
      <el-table-column label="MAC策略" align="center">
        <el-table-column prop="mac" label="MAC地址" align="center"></el-table-column>
      </el-table-column>
      <el-table-column label="TCP策略" align="center">
        <el-table-column prop="tcps" label="开始端口" align="center"></el-table-column>
        <el-table-column prop="tcpe" label="结束端口" align="center"></el-table-column>
      </el-table-column>
      <el-table-column label="UDP策略" align="center">
        <el-table-column prop="udps" label="开始端口" align="center"></el-table-column>
        <el-table-column prop="udpe" label="结束端口" align="center"></el-table-column>
      </el-table-column>
      <el-table-column prop="enable" align="center" label="操作">
        <template slot-scope="scope">
          <el-button type="text" @click="deleteRule(scope.$index, scope.row)">{{$t('operation.delete')}}</el-button>
        </template>
      </el-table-column>
    </el-table> -->

    <el-table
      @selection-change="selChange"
      :data="rulesList"
      :header-cell-style="headerStyle">
      <el-table-column
        type="selection"
        :selectable="selectable"
        min-width="30"/>
      <el-table-column
        prop="port"
        :label="$t('securitySetting.port')"
        align="center"/>
      <el-table-column
        prop="type"
        :label="$t('securitySetting.type')"
        align="center"/>
      <el-table-column
        prop="ipSE"
        :label="$t('securitySetting.ipSE')"
        align="center"/>
      <el-table-column
        prop="mac"
        :label="$t('securitySetting.mac')"
        align="center"/>
      <el-table-column
        prop="portSE"
        :label="$t('securitySetting.portSE')"
        align="center"/>
      <!-- <el-table-column
        prop="status"
        :label="$t('securitySetting.status')"
        align="center"/>
      <el-table-column
        prop="enable"
        align="center"
        :label="$t('securitySetting.enable')"/> -->
      <el-table-column
        prop="direct"
        align="center"
        :label="$t('securitySetting.direct')"/>
      <el-table-column
        prop="mangle"
        align="center"
        :label="$t('securitySetting.enable')">
        <template slot-scope="scope">
          <el-button
            type="text"
            @click="deleteRule(scope.$index, scope.row)">{{ $t('operation.delete') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      
      layout="total, prev, pager, next, jumper"
      @current-change="handleCurrentChange"
      :page-size="10"
      :total="total"
      :current-page.sync="currentPage"
      style="float:right;"/>

    <!-- 禁用网卡页面 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.disablePort')"
      :close-on-click-modal="false"
      :visible.sync="isInDisabling">
      
      <el-select
        v-model="disablePort"
        :placeholder="$t('placeholder.selectDisablePort')">
        <el-option
          v-for="item in disablePortList"
          :label="item"
          :value="item"
          :key="item"/>
      </el-select>
      
      <el-button
        @click="disableSubmit"
        type="primary">{{ $t('securitySetting.disableSubmit') }}</el-button>
      <el-button @click="enableCancle" type="danger">{{ $t('securitySetting.Cancle') }}</el-button>
    </el-dialog>
    </div>

    <!-- 启用网卡页面 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.isInEnabling')"
      :close-on-click-modal="false"
      :visible.sync="isInEnabling">
      
      <el-select
        v-model="enablePort"
        :placeholder="$t('placeholder.selectEnablePort')">
        <el-option
          v-for="item in enablePortList"
          :label="item"
          :value="item"
          :key="item"/>
      </el-select>
      
      <el-button
        @click="enableSubmit"
        type="primary">{{ $t('securitySetting.enableSubmit') }}</el-button>
      <el-button @click="enableCancle" type="danger">{{ $t('securitySetting.Cancle') }}</el-button>
    </el-dialog>
    </div>

    <!-- 添加页面 策略/端口选择 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.selectStrategy')"
      :close-on-click-modal="false"
      :visible.sync="isAdding">
      
      <el-select
        v-model="ifaceChoosen"
        :placeholder="$t('placeholder.selectPort')">
        <el-option
          v-for="item in nameList"
          :key="item.name"
          :label="item.name"
          :value="item.name"/>
      </el-select>
      
      <el-select
        v-model="rulesChoosen"
        :placeholder="$t('placeholder.chooseRule')">
        <el-option
          v-for="item in list"
          :key="item.value"
          :label="item.name"
          :value="item.value"/>
      </el-select>
      
      <div slot="footer">
        <el-button @click="addPortCancel" type="danger">{{ $t('operation.cancel') }}</el-button>
        <el-button @click="addNext" type="primary">{{ $t('operation.next') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!-- TCP的策略页面 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.isTcpRule')"
      :close-on-click-modal="false"
      :visible.sync="isTcpRule">
      <el-form
        :model="addForm"
        ref="addForm"
        label-width="7rem"
        label-position="left"
        :rules="validateRules">
        <el-form-item
          prop="ips"
          :label="$t('securitySetting.ips')">
          <el-input style="width:85%" v-model="addForm.ips"/>
        </el-form-item>
        <el-form-item
          prop="ipe"
          :label="$t('securitySetting.ipe')">
          <el-input style="width:85%" v-model="addForm.ipe"/>
        </el-form-item>
        <el-form-item
          prop="ports"
          :label="$t('securitySetting.tcps')">
          <el-input style="width:85%" v-model="addForm.ports"/>
        </el-form-item>
        <el-form-item
          prop="porte"
          :label="$t('securitySetting.tcpe')">
          <el-input style="width:85%" v-model="addForm.porte"/>
        </el-form-item>
        <el-form-item
        prop="direct"
          :label="$t('securitySetting.direct')">
          <div class="shortselect">
          <el-select
            v-model="addForm.direct"
            :placeholder="$t('placeholder.chooseDirect')">
            <el-option
              v-for="item in directionList"
              :key="item.name"
              :label="item.name"
              :value="item.value"/>
          </el-select>
          </div>
        </el-form-item>
        <el-form-item
        prop="mangle"
          :label="$t('securitySetting.mangle')">
          <div class="shortselect">
          <el-select
            v-model="addForm.mangle"
            :placeholder="$t('placeholder.chooseMangle')">
            <el-option
              v-for="item in mangleList"
              :key="item.value"
              :label="item.name"
              :value="item.value"/>
          </el-select>
          </div>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button @click.native="addCancel" type="danger">{{ $t('operation.cancel') }}</el-button>
        <el-button
          @click.native="addSubmit"
          type="primary">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!-- UDP策略 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.isUdpRule')"
      :close-on-click-modal="false"
      :visible.sync="isUdpRule">
      <el-form
        :model="addForm"
        ref="addForm"
        label-width="8rem"
        label-position="left"
        :rules="validateRules">
        <el-form-item
          prop="ips"
          :label="$t('securitySetting.ips')">
          <el-input style="width:85%" v-model="addForm.ips"/>
        </el-form-item>
        <el-form-item
          prop="ipe"
          :label="$t('securitySetting.ipe')">
          <el-input style="width:85%" v-model="addForm.ipe"/>
        </el-form-item>
        <el-form-item
          prop="ports"
          :label="$t('securitySetting.udps')">
          <el-input style="width:85%" v-model="addForm.ports"/>
        </el-form-item>
        <el-form-item
          prop="porte"
          :label="$t('securitySetting.udpe')">
          <el-input style="width:85%" v-model="addForm.porte"/>
        </el-form-item>
        <el-form-item
        prop="direct"
        :label="$t('securitySetting.direct')">
        <div class="shortselect">
          <el-select
            v-model="addForm.direct"
            :placeholder="$t('placeholder.chooseDirect')">
            <el-option
              v-for="item in directionList"
              :key="item.name"
              :label="item.name"
              :value="item.value"/>
          </el-select>
        </div>
        </el-form-item>
        <el-form-item
        prop="mangle"
        :label="$t('securitySetting.mangle')">
        <div class="shortselect">
          <el-select
            v-model="addForm.mangle"
            :placeholder="$t('placeholder.chooseMangle')">
            <el-option
              v-for="item in mangleList"
              :key="item.value"
              :label="item.name"
              :value="item.value"/>
          </el-select>
        </div>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button @click.native="addCancel" type="danger">{{ $t('operation.cancel') }}</el-button>
        <el-button
          @click.native="addSubmit"
          type="primary">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!-- IP策略 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.isIpRule')"
      :close-on-click-modal="false"
      :visible.sync="isIpRule">
      <el-form
        :model="addForm"
        ref="addForm"
        label-width="6rem"
        label-position="left"
        :rules="validateRules">
        <el-form-item
          prop="ips"
          :label="$t('securitySetting.ips')">
          <el-input style="width:85%" v-model="addForm.ips"/>
        </el-form-item>
        <el-form-item
          prop="ipe"
          :label="$t('securitySetting.ipe')">
          <el-input style="width:85%" v-model="addForm.ipe"/>
        </el-form-item>
        <el-form-item
        prop="direct"
       :label="$t('securitySetting.direct')" >
       <div class="shortselect">
          <el-select
            v-model="addForm.direct"
            :placeholder="$t('placeholder.chooseDirect')">
            <el-option
              v-for="item in directionList"
              :key="item.name"
              :label="item.name"
              :value="item.value"/>
          </el-select>
       </div>
        </el-form-item>
        <el-form-item
        prop="mangle"
        :label="$t('securitySetting.mangle')">
        <div class="shortselect">
          <el-select
            v-model="addForm.mangle"
            :placeholder="$t('placeholder.chooseMangle')">
            <el-option
              v-for="item in mangleList"
              :key="item.value"
              :label="item.name"
              :value="item.value"/>
          </el-select>
        </div>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button @click.native="addCancel" type="danger">{{ $t('operation.cancel') }}</el-button>
        <el-button
          @click.native="addSubmit"
          type="primary">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!-- mac策略 -->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('securitySetting.isMacRule')"
      :close-on-click-modal="false"
      :visible.sync="isMacRule">
      <el-form
        :model="addForm"
        ref="addForm"
        label-width="6rem"
        label-position="left"
        :rules="validateRules">
        <el-form-item
          prop="mac"
          :label="$t('securitySetting.mac')">
          <el-input style="width:85%" v-model="addForm.mac"/>
        </el-form-item>
        <el-form-item
        prop="direct"
        :label="$t('securitySetting.direct')">
        <div class="shortselect">
          <el-select
            v-model="addForm.direct"
            :placeholder="$t('placeholder.chooseDirect')">
            <el-option
              v-for="item in directionList"
              :key="item.name"
              :label="item.name"
              :value="item.value"/>
          </el-select>
        </div>
        </el-form-item>
        <el-form-item
        prop="mangle"
        :label="$t('securitySetting.mangle')">
        <div class="shortselect">
          <el-select
            v-model="addForm.mangle"
            :placeholder="$t('placeholder.chooseMangle')">
            <el-option
              v-for="item in mangleList"
              :key="item.value"
              :label="item.name"
              :value="item.value"/>
          </el-select>
        </div>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button @click.native="addCancel" type="danger">{{ $t('operation.cancel') }}</el-button>
        <el-button
          @click.native="addSubmit"
          type="primary">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

  </div>
</template>

<script>
import { handle, showInfo, enable, getPorts } from '@/api/api.js'
import validate from '@/utils/rules.js'
export default {
  name: 'Firewall',
  data () {
    let portRules = (rule, value, callback) => {
      var reg = /.*\..*/

      if (value === '') {
        callback(new Error('请输入端口号！/Please input port number'))
        console.log('端口号')
      } else if (reg.test(value) === true) {
        callback(new Error('请输入整数！/Please input integer'))
        console.log('shuzi')
      } else if (value < 0 || value > 65535) {
        callback(new Error('端口范围为0~65535/port number range from 0 to 65535'))
        console.log('范围')
      }
      callback()
    }
    let direct = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择报文方向！/Please select direction'))
        console.log('端口号')
      }
      callback()
    }
    let mangle = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择策略类型！/Please select strategy'))
        console.log('端口号')
      }
      callback()
    }
    return {
      choosenItem: [],
      rulesList: [],
      rulesTable: {},
      list: [
        {
          value: 'ip',
          name: 'IP策略'
        },
        {
          value: 'mac',
          name: 'MAC策略'
        },
        {
          value: 'tcp',
          name: 'TCP策略'
        },
        {
          value: 'udp',
          name: 'UDP策略'
        }
      ],
      isAdding: false,
      isInDisabling: false,
      isInEnabling: false,
      isTcpRule: false,
      isUdpRule: false,
      isIpRule: false,
      isMacRule: false,
      currentFunction: '',
      nameList: [], // 当前ports的名称
      disablePort: '',
      enablePort: '',
      disablePortList: '',
      enablePortList: '',

      rulesChoosen: '',
      ifaceChoosen: '',

      portChoosen: '',
      portChoosenInuse: '',
      addForm: {
        ips: '',
        ipe: '',
        ports: '',
        porte: '',
        direct: '',
        mangle: '',
        optype: '',
        rules: ''
      },
      enlist: [],

      portStorage: '',
      typeStorage: '',
      enableStorage: '',

      total: 0,
      currentPage: 1,
      pagesize: 10,
      inLoading: true,

      ports: [],
      rules: [],

      directionList: [
        {
          name: '报文输出',
          value: 'output'
        },
        {
          name: '报文输入',
          value: 'input'
        }
      ],
      mangleList: [
        {
          name: '允许通过',
          value: 'allow'
        },
        {
          name: '禁止通过',
          value: 'deny'
        }
      ],

      validateRules: {
        ips: [
          { required: true, message: 'IP地址不能为空!/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP地址不能为空！/Please input IP'),
            trigger: 'blur'
          }
        ],
        ipe: [
          { required: true, message: 'IP地址不能为空!/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP地址不能为空！/Please input IP'),
            trigger: 'blur'
          }
        ],
        ports: [
          { required: true, message: '请输入端口号！/Please input port number' },
          {
            validator: portRules,
            trigger: 'blur'
          }
        ],
        porte: [
          { required: true, message: '请输入端口号！/Please input port number' },
          {
            validator: portRules,
            trigger: 'blur'
          }
        ],
        mac: [
          { required: true, message: 'MAC地址不能为空！/Please input MAC' },
          {
            validator: validate('mac', '请输入正确MAC！/Please input correct MAC', 'MAC地址不能为空！/Please input MAC'),
            trigger: 'blur'
          }
        ],
        direct: [
          { required: true, message: '请选择报文方向！/Please select direction' },
          {
            validator: direct, trigger: 'blur'
          }
        ],
        mangle: [
          { required: true, message: '请选择策略类型！/Please select strategy' },
          {
            validator: mangle, trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    selChange (sels) {
      this.choosenItem = sels
      console.log(this.choosenItem)
    },
    batchDeletion() {
      let i
      for (i = 0; i < this.choosenItem.length; i++) {
        del(this.choosenItem[i])
      }

      async function del (item) {
        let para = {
          data: []
        }
        let object = {}
        object.optype = 'del'
        object.id = item.id
        para.data.push(object)
        handle(para)
          .then(res => {
            if (res.data.code === 200) {

            }
          })
          .catch(err => {
            console.log(err)
          })
      }
      this.getInformation()
      this.getPorts()
      this.$forceUpdate()
    },
    headerStyle () {
      return this.header()
    },
    handleCurrentChange (val) {
      console.log('page ' + val)
      this.currentPage = val
      this.getInformation()
    },
    handleDisable () {
      this.isInDisabling = true
    },
    handleEnable () {
      this.isInEnabling = true
    },
    handleAdd () {
      this.isAdding = true
      if (this.$store.state.app.language === 'zh') {
        this.list = [
          {
            value: 'ip',
            name: 'IP策略'
          },
          {
            value: 'mac',
            name: 'MAC策略'
          },
          {
            value: 'tcp',
            name: 'TCP策略'
          },
          {
            value: 'udp',
            name: 'UDP策略'
          }
        ]
        this.directionList = [
          {
            name: '报文输出',
            value: 'output'
          },
          {
            name: '报文输入',
            value: 'input'
          }
        ]
        this.mangleList = [
          {
            name: '允许通过',
            value: 'allow'
          },
          {
            name: '禁止通过',
            value: 'deny'
          }
        ]
      } else {
        this.list = [
          {
            value: 'ip',
            name: 'IP'
          },
          {
            value: 'mac',
            name: 'MAC'
          },
          {
            value: 'tcp',
            name: 'TCP'
          },
          {
            value: 'udp',
            name: 'UDP'
          }
        ]
        this.directionList = [
          {
            name: 'Output',
            value: 'output'
          },
          {
            name: 'Input',
            value: 'input'
          }
        ]
        this.mangleList = [
          {
            name: 'Allow',
            value: 'allow'
          },
          {
            name: 'Deny',
            value: 'deny'
          }
        ]
      }
    },
    addPortCancel () {
      this.isAdding = false
      this.$refs['addForm'].resetFields()
      this.rulesChoosen = ''
      this.ifaceChoosen = ''
    },
    addNext () {
      if (this.ifaceChoosen !== '' && this.rulesChoosen !== '') {
        this.isAdding = false
        switch (this.rulesChoosen) {
          case 'tcp':
            this.isTcpRule = true
            break
          case 'udp':
            this.isUdpRule = true
            break
          case 'ip':
            this.isIpRule = true
            break
          case 'mac':
            this.isMacRule = true
            break
        }
      } else if (this.ifaceChoosen === '' && this.rulesChoosen === '') {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '请选择网口与策略',
            type: 'warning'
          })
        } else {
          this.$message({
            message: 'Please select port and strategy',
            type: 'warning'
          })
        }
      } else if (this.ifaceChoosen === '' && this.rulesChoosen !== '') {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '请选择网口',
            type: 'warning'
          })
        } else {
          this.$message({
            message: 'Please select port',
            type: 'warning'
          })
        }
      } else if (this.ifaceChoosen !== '' && this.rulesChoosen === '') {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            message: '请选择策略',
            type: 'warning'
          })
        } else {
          this.$message({
            message: 'Please select strategy',
            type: 'warning'
          })
        }
      }
    },
    addCancel () {
      this.isTcpRule = false
      this.isUdpRule = false
      this.isIpRule = false
      this.isMacRule = false
      this.$refs['addForm'].resetFields()
    },
    addSubmit () {
      let para = {
        data: []
      }
      let object = {}
      object = Object.assign({}, this.addForm)
      // 端口地址要求是int类型
      console.log(object)
      if (object.mac === '') {
        if (compareIP(object.ips, object.ipe) === 1) {
          if (this.$store.state.app.language === 'zh') {
            this.$message({
              type: 'error',
              message: '请正确输入IP起始、结束地址'
            })
          } else {
            this.$message({
              type: 'error',
              message: 'Please input correct start IP and end IP'
            })
          }
          console.log('大于')
        } else  if (compareIP(object.ips, object.ipe) === -1) {
          for (let i in object) {
            if (i === 'ports' || i === 'porte') {
              object[i] = Number(object[i])
            }
          }
          console.log('小于')
          object.optype = 'add'
          object.rules = setRules(this.rulesChoosen)
          object.iface = this.ifaceChoosen
          // object.webname = this.ifaceChoosen
          // console.log('submit rules is ' + object.rules)
          para.data.push(object)
          // console.log('编号是 ' + this.rulesChoosen)

          this.$refs['addForm'].validate((valid) => {
            console.log('val is ' + valid)

            if (valid) {
              handle(para)
                .then(res => {
                  if (res.data.code === 200) {
                    this.getInformation()
                    this.addCancel()
                  } else {
                  }
                })
                .catch(error => {
                  console.log(error)
                })

              this.rulesChoosen = ''
              this.ifaceChoosen = ''
            }
          })
        }
      } else {
        for (let i in object) {
          if (i === 'ports' || i === 'porte') {
            object[i] = Number(object[i])
          }
        }
        console.log('小于')
        object.optype = 'add'
        object.rules = setRules(this.rulesChoosen)
        object.iface = this.ifaceChoosen
        // object.webname = this.ifaceChoosen
        // console.log('submit rules is ' + object.rules)
        para.data.push(object)
        // console.log('编号是 ' + this.rulesChoosen)

        this.$refs['addForm'].validate((valid) => {
          console.log('val is ' + valid)

          if (valid) {
            handle(para)
              .then(res => {
                if (res.data.code === 200) {
                  this.getInformation()
                  this.addCancel()
                } else {
                }
              })
              .catch(error => {
                console.log(error)
              })

            this.rulesChoosen = ''
            this.ifaceChoosen = ''
          }
        })
      }

      function setRules (val) {
        let i
        if (val === 'mac') {
          i = 0
        } else if (val === 'ip') {
          i = 1
        } else if (val === 'udp') {
          i = 2
        } else if (val === 'tcp') {
          i = 3
        }
        return i
      }
      function compareIP(ipBegin, ipEnd)       {
        var temp1
        var temp2
        console.log('ipBegin')
        console.log(ipBegin)
        console.log(ipEnd)
        temp1 = ipBegin.split('.')
        temp2 = ipEnd.split('.')
        for (var i = 0; i < 4; i++) {
          if (temp1[i] > temp2[i]) {
            return 1
          } else if (temp1[i] < temp2[i]) {
            return -1
          }
        }
        // return 0
      }
    },
    disableSubmit () {
      let para = {
        data: []
      }
      let object = {}
      object.iface = this.disablePort
      object.enable = 0
      para.data.push(object)
      enable(para)
        .then(res => {
          if (res.data.code === 200) {
            this.getInformation()
            this.isInDisabling = false
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    enableSubmit () {
      let para = {}
      para.iface = this.enablePort
      para.enable = 1
      let val = {
        data: []
      }
      val.data.push(para)
      enable(val)
        .then(res => {
          if (res.data.code === 200) {
            this.getInformation()
            this.isInEnabling = false
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    deleteRule (index, val) {
      console.log(val)
      let para = {
        data: []
      }
      let object = {}
      object.optype = 'del'
      object.id = val.id
      para.data.push(object)
      handle(para)
        .then(res => {
          if (res.data.code === 200) {

          }
        })
        .catch(err => {
          console.log(err)
        })
      this.getInformation()
      this.getPorts()
      this.$forceUpdate()
    },
    enableCancle () {
      this.isInEnabling = false
      this.isInDisabling = false
    },

    // 开始处理拿到的数组
    // getRules() {
    //   this.rules.forEach(this.getPortRules)
    // },
    // getPortRules(element, index, array) {
    //   this.portStorage = element.iface
    //   this.enableStorage = element.enable
    //   if (element.enable === 1) {
    //     this.disablePortList.push(element.iface)
    //   } else if (element.enable === 0) {
    //     this.enablePortList.push(element.iface)
    //   }
    //   element.rules.forEach(this.getTypeRules)
    // },
    // getTypeRules(item, index, array) {
    //   for (let i in item) {
    //     console.log('i ' + i)
    //     if (item[i].length !== 0) {
    //       if (i === 'macs') {
    //         this.typeStorage = 'MAC'
    //       } else if (i === 'ip') {
    //         this.typeStorage = 'IP'
    //       } else if (i === 'udp') {
    //         this.typeStorage = 'UDP'
    //       } else if (i === 'tcp') {
    //         this.typeStorage = 'TCP'
    //       }
    //     }
    //     // console.log('item[Object.keys(item)] ' + item[Object.keys(item)])
    //     // console.log('Object.keys(item) ' + Object.keys(item))
    //   }
    //   // console.log('in 3rd step ' + item)
    //   // 获取策略类型
    //   item[Object.keys(item)].forEach(this.getValue)
    // },
    // getValue(val, index, array) {
    //   let table = {}
    //   // console.log('last step ' + JSON.stringify(val))
    //   table.id = val.id
    //   table.port = this.portStorage
    //   table.type = this.typeStorage
    //   table.status = result(table.status, this.enableStorage)

    //   function result(status, val) {
    //     if (val === 0) {
    //       status = '未启用'
    //     } else if (val === 1) {
    //       status = '启用中'
    //     }
    //     return status
    //   }
    //   console.log('aaaaaaaaaaaaaaaaa' + table.status)
    //   for (let i in val) {
    //     if (i === 'ips') {
    //       table.ipSE = val.ips + '-' + val.ipe
    //     }
    //     if (i === 'ports') {
    //       table.portSE = val.ports + '-' + val.porte
    //     }
    //     if (i === 'mac') {
    //       table[i] = val[i]
    //     }
    //   }
    //   this.rulesList.push(table)
    //   // console.log('last step ruleList ' + JSON.stringify(this.rulesList))
    // },

    getRules () {
      console.log('getRules')
      this.rules.forEach(this.getValues)
    },
    getValues (item, index, array) {
      console.log('getvalues')
      console.log('item : ')
      console.log(item)
      let para = {}
      para.id = item.id
      para.port = item.iface
      console.log('1')
      para.type = getType()
      console.log('2')
      para.status = getStatus(item.enable)
      console.log('3')
      para.direct = item.direct
      console.log('4')
      para.mangle = item.mangle
      console.log('5')
      for (let i in item) {
        if (i === 'ips') {
          para.ipSE = item.ips + '-' + item.ipe
        }
        if (i === 'ports') {
          para.portSE = item.ports + '-' + item.porte
        }
        if (i === 'mac') {
          para[i] = item[i]
        }

        // 生成已启用/未启用端口的列表
        if (i === 'enable') {
          if (item[i] === 0) {
            this.enablePortList.push(item.iface)
          } else {
            this.disablePortList.push(item.iface)
          }
        }
      }

      this.rulesList.push(para)

      function getType () {
        let para = ''
        if (item.ruletype === 0) {
          para = 'MAC'
        }
        if (item.ruletype === 1) {
          para = 'IP'
        }
        if (item.ruletype === 2) {
          para = 'UDP'
        }
        if (item.ruletype === 3) {
          para = 'TCP'
        }
        return para
      }

      function getStatus (val) {
        let status = ''
        if (val === 0) {
          // if (this.$store.state.app.language === 'zh') {
          status = '未启用'
          // } else {
          //   status = 'Disable'
          // }
        } else if (val === 1) {
          // if (this.$store.state.app.language === 'zh') {
          status = '启用中'
          // } else {
          //   status = 'Enable'
          // }
        }
        return status
      }
    },

    search () {
      this.currentPage = 1
      let para = {}
      para.page = this.currentPage
      para.pagesize = this.pagesize
      // para.iface = []
      if (this.portChoosen !== '') {
        // para.iface.push(this.portChoosen)
        // para.webname = '(' + '\'' + this.portChoosen + '\'' + ')'
        para.webname = this.portChoosen
        this.portChoosenInuse = this.portChoosen
      }
      // para.page = this.currentPage

      showInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.inLoading = false
            if (res.data.total !== 0) {
              if (res.data.data.rules.length > 0) {
                this.rules = res.data.data.rules
                this.currentPage = res.data.page
                setTimeout(() => {
                  this.inLoading = true
                }, 0)
              } else {
                this.currentPage -= 1
                this.getInformation()
                return
              }
            } else {
              this.rules = []
              this.rulesList = []
              return
            }
            this.total = res.data.total
            this.rulesList = []
            this.enablePortList = []
            this.disablePortList = []
            this.getRules()
          } else {
            this.rulesList = []
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    async getInformation () {
      let val = ''
      if (this.portChoosenInuse) {
        val = this.portChoosenInuse
      }
      let para = {}
      // para.iface = this.enlist
      // this.enlist.forEach(spliceArray)
      // para.ifacestr = val
      para.webname = val
      para.pagesize = this.pagesize
      para.page = this.currentPage
      // console.log('ifacestr is ' + para.ifacestr)

      this.enablePortList = []
      this.disablePortList = []

      showInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.total !== 0) {
              if (res.data.data.rules.length > 0) {
                this.rules = res.data.data.rules
              } else {
                this.currentPage -= 1
                this.getInformation()
                return
              }
            } else if (res.data.total === 0) {
              this.rules = []
            }
            this.total = res.data.total
            this.rulesList = []
            this.enablePortList = []
            this.disablePortList = []
            this.getRules()
          }
        })
        .catch(error => {
          console.log(error)
        })

      // function spliceArray (item, index, array) {
      //   // console.log('in foreach ' + item + '   ' + index)
      //   if (index === 0) {
      //     val = '(' + '\'' + item + '\'' + ', '
      //   } else if (index === array.length - 1) {
      //     val = val + '\'' + item + '\'' + ')'
      //   } else {
      //     val = val + '\'' + item + '\'' + ', '
      //   }
      // }
    },

    async getPorts () {
      await getPorts()
        .then(res => {
          if (res.data.code === 200) {
            this.ports = res.data.interfaces
            this.ports.forEach(this.getPortsName)
          }
        })
        .catch(err => {
          console.log(err)
        })

      this.getInformation()
    },
    getPortsName (item) {
      // console.log('item ' + item)
      let para = {}
      para.name = item.webname
      this.enlist.push(item.webname)
      para.enable = getEnable(item)

      this.nameList.push(para)
      // console.log(para)
      // console.log(this.nameList)

      function getEnable (val) {
        if (val.function !== 'normal') {
          return 1
        } else {
          return 0
        }
      }
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
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
    this.getPorts()
    // this.getInformation()
  }
}
</script>

<style lang="scss" scoped>
.el-input {
  display: inline-block;
  
}

</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}
.shortselect .el-select {
  width: 85%;
}
</style>
