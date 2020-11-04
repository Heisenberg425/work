<template>
  <div>
    <div class="line_02">
      <span>{{ $t('Qos.QosSetting') }}</span>
    </div>

    <el-col :span="24">
      <el-form :inline="true">
        <el-form-item>
          <el-button
            type="primary"
            @click="addHandle">{{ $t('operation.add') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button
            type="danger"
            @click="batchDelete">{{ $t('VLAN.delete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <el-table
      :data="data"
      @selection-change="selsChange"
      :header-cell-style="headerStyle">
       <el-table-column
        type="selection"
        min-width="30"/>
      <el-table-column
        prop="netif"
        :label="$t('Qos.netif')"/>
      <el-table-column
        prop="rule"
        :label="$t('Qos.rule')"/>
      <el-table-column
        prop="ipStart"
        :label="$t('Qos.ipStart')"/>
      <el-table-column
        prop="ipEnd"
        :label="$t('Qos.ipEnd')"/>
      <el-table-column
        prop="cmpMethod"
        :label="$t('Qos.cmpMethod')"/>
      <el-table-column
        prop="priority"
        :label="$t('Qos.priority')"/>
      <el-table-column
        prop="dontDrop"
        :label="$t('Qos.dontDrop')"/>
      <el-table-column>
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="deleteItem(scope.index, scope.row)">{{ $t('operation.delete') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      layout="total, prev, pager, next, jumper"
      @current-change="handleChange"
      :total="total"
      :current-page="currentPage"
      :page-size="pageSize"
      style="float:right"/>

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('Qos.addNewQos')"
      :visible.sync="isShow"
      :close-on-click-modal="false">
      <el-form
        :model="addForm"
        ref="addForm"
        label-position="left"
        label-width="5rem"
        :rules="ruleChosen === 'ip'?ruleIp:rules"
        validate-on-rule-change>
        <el-form-item
          prop="netif"
          :label="$t('Qos.netif')">
          <div class="shortselect">
          <el-select
            v-model="addForm.netif"
            :placeholder="$t('placeholder.selectNetcard')">
            <el-option
              v-for="(item, index) in netList"
              :key="index"
              :label="item"
              :value="item"/>
          </el-select>
          </div>
        </el-form-item>
        <el-form-item
          prop="rule"
          :label="$t('Qos.rule')">
          <div class="shortselect">
          <el-select
            v-model="addForm.rule"
            :placeholder="$t('placeholder.selectRule')"
            @change="changeHandle">
            <el-option
              v-for="(item, index) in ruleList"
              :key="index"
              :label="item.label"
              :value="item.label"/>
          </el-select>
          </div>
        </el-form-item>
        <el-form-item
          prop="ipStart"
          :label="$t('Qos.ipStart')">
          <el-input
            v-model="addForm.ipStart"
            style="width:85%"
            :placeholder="$t('placeholder.startIP')"/>
        </el-form-item>
        <el-form-item
          prop="ipEnd"
          :label="$t('Qos.ipEnd')">
          <el-input
            v-model="addForm.ipEnd"
            style="width:85%"
            :placeholder="$t('placeholder.endIP')"/>
        </el-form-item>
        <el-form-item
          prop="portStart"
          :label="$t('Qos.portStart')">
          <el-input
            v-model="addForm.portStart"
            style="width:85%"
            :disabled="isIP"
            :placeholder="$t('placeholder.startPort')"/>
        </el-form-item>
        <el-form-item
          prop="portEnd"
          :label="$t('Qos.portEnd')">
          <el-input
            v-model="addForm.portEnd"
            :disabled="isIP"
            style="width:85%"
            :placeholder="$t('placeholder.endPort')"/>
        </el-form-item>
        <el-form-item
          prop="cmpMethod"
          :label="$t('Qos.cmpMethod')">
          <div class="shortselect">
          <el-select
            v-model="addForm.cmpMethod"
            :placeholder="$t('placeholder.compareMethod')">
            <el-option
              v-for="(item, index) in methodList"
              :key="index"
              :label="item.label"
              :value="item.label"/>
          </el-select>
          </div>
        </el-form-item>
        <el-form-item
          prop="priority"
          :label="$t('Qos.priority')">
          <div class="shortselect">
          <el-select
            v-model="addForm.priority"
            :placeholder="$t('placeholder.selectPriority')">
            <el-option
              v-for="item in priorityNum"
              :key="item.num"
              :label="item.num"
              :value="item.num"/>
          </el-select>
          </div>
        </el-form-item>
        <el-form-item
          prop="dontDrop"
          :label="$t('Qos.dontDrop')">
          <el-radio
            v-model="addForm.dontDrop"
            label="0">No</el-radio>
          <el-radio
            v-model="addForm.dontDrop"
            label="1">Yes</el-radio>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
        type="danger"
          size="small"
          @click="addCancel">{{ $t('operation.cancel') }}</el-button>
        <el-button
        type="primary"
          size="small"
          @click="addSubmit">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
</div>

  </div>
</template>

<script>
import { setQos, delQos, getQos, getPorts } from '@/api/api.js'
import validate from '@/utils/rules.js'
export default {
  name: 'Qos',
  data () {
    let portRules = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入端口号！/Please input port number'))
      } else if (value < 1 || value > 65535) {
        callback(new Error('端口范围为0~65535/port number range from 0 to 65535'))
      }
      callback()
    }
    let netif = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择网卡接口！/Please select net port'))
      }
      callback()
    }
    let rule = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择规则！/Please select rule'))
      }
      callback()
    }
    let cmpMethod = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择比较方法！/Please select compare method'))
      }
      callback()
    }
    let priority = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择优先级！/Please select priority'))
      }
      callback()
    }
    let dontDrop = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择丢包选项！/Please choose the setting of drop'))
      }
      callback()
    }
    return {
      sels: [],
      isShow: false,
      isIP: false,

      ruleChosen: '',

      addForm: {
        netif: '',
        rule: '',
        ipStart: '',
        ipEnd: '',
        portStart: '',
        portEnd: '',
        cmpMethod: '',
        priority: '',
        dontDrop: ''
      },
      ruleList: [
        {
          label: 'ip'
        },
        {
          label: 'udp'
        },
        {
          label: 'tcp'
        }
      ],
      methodList: [
        {
          label: 'SRC'
        },
        {
          label: 'DEST'
        },
        {
          label: 'BOTH'
        }
      ],
      netList: [],
      data: [],

      total: 0,
      currentPage: 1,
      pageSize: 5,
      priorityNum: [{num: 0}, {num: 1}, {num: 2}, {num: 3}, {num: 4}, {num: 5}, {num: 6}, {num: 7}],
      rules: {
        ipStart: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确地址！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        ipEnd: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确地址！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        portStart: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: portRules, trigger: 'blur'
          }
        ],
        portEnd: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: portRules, trigger: 'blur'
          }
        ],
        netif: [
          { required: true, message: '请选择网卡接口！/Please select net port' },
          {
            validator: netif, trigger: 'blur'
          }
        ],
        rule: [
          { required: true, message: '请选择规则！/Please select rule' },
          {
            validator: rule, trigger: 'blur'
          }
        ],
        cmpMethod: [
          { required: true, message: '请选择比较方法！/Please select compare method' },
          {
            validator: cmpMethod, trigger: 'blur'
          }
        ],
        priority: [
          { required: true, message: '请选择优先级！/Please select priority' },
          {
            validator: priority, trigger: 'blur'
          }
        ],
        dontDrop: [
          { required: true, message: '请选择丢包选项！/Please choose the setting of drop' },
          {
            validator: dontDrop, trigger: 'blur'
          }
        ]
      },
      ruleIp: {
        ipStart: [
          {
            validator: validate('ip', '请输入正确地址/Please input correct IP', '地址不能为空/Please input IP'), trigger: 'blur'
          }
        ],
        ipEnd: [
          {
            validator: validate('ip', '请输入正确地址/Please input correct IP', '地址不能为空/Please input IP'), trigger: 'blur'
          }
        ]
      }
    }
  },
  watch: {
    ruleChosen: function () {
      if (this.ruleChosen === 'ip') {
        this.isIP = true
      } else {
        this.isIP = false
      }
    },
    isIP: function () {
      if (this.isIP) {
        this.addForm.portStart = 0
        this.addForm.portEnd = 0
      } else {
        this.addForm.portStart = ''
        this.addForm.portEnd = ''
      }
    },
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      if (this.$store.state.app.language === 'zh') {
        this.$route.matched[0].name = this.$route.matched[0].meta.zh
        this.$route.matched[1].name = this.$route.matched[1].meta.zh
      } else {
        this.$route.matched[0].name = this.$route.matched[0].meta.en
        this.$route.matched[1].name = this.$route.matched[1].meta.en
      }
    }
  },
  methods: {
    batchDelete () {
      for (let i = 0; i < this.sels.length; i++) {
        // del(i, this.sels[i])
        this.deleteItem(i, this.sels[i])
      }

      // function del (index, row) {
      //   let para = Object.assign({}, row)
      //   console.log('this.currentPage')
      //   console.log(this.currentPage)
      //   para.page = this.currentPage
      //   delQos(para)
      //     .then(res => {
      //       if (res.data.code === 200) {

      //       }
      //     })
      //     .catch(error => {
      //       console.log(error)
      //     })
      //   this.getQosInfo()
      // }
    },
    selsChange: function (sels) {
      this.sels = sels
    },
    headerStyle () {
      return this.header()
    },
    addHandle () {
      this.isShow = true
    },
    changeHandle (val) {
      this.ruleChosen = val
    },
    handleChange (val) {
      this.currentPage = val
      this.getQosInfo()
    },
    deleteItem (index, val) {
      let para = Object.assign({}, val)
      para.page = this.currentPage
      delQos(para)
        .then(res => {
          if (res.data.code === 200) {

          }
        })
      this.getQosInfo()
    },
    addCancel () {
      this.isShow = false
      this.$refs.addForm.resetFields()
    },
    addSubmit () {
      this.$refs['addForm'].validate(valid => {
        if (valid) {
          let para = Object.assign({}, this.addForm)
          para.portEnd = Number(para.portEnd)
          para.portStart = Number(para.portStart)
          para.dontDrop = Number(para.dontDrop)
          setQos(para)
            .then(res => {
              if (res.data.code === 200) {
                this.getQosInfo()
                this.isShow = false
                this.$refs.addForm.resetFields()
              } else  { // wyk930返回500报配置错误
                if (this.$store.state.app.language === 'zh') {
                  this.$message({
                    message: '配置数据有误，请重新配置',
                    type: 'warning'
                  })
                } else {
                  this.$message({
                    message: 'The configuration information is incorrect. Please reconfigure it.',
                    type: 'warning'
                  })
                }
              }
            })
            .catch(error => {
              console.log(error)
            })
        }
      })
    },
    getQosInfo () {
      let para = {}
      para.page = this.currentPage
      getQos(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.total !== 0) {
              if (res.data.data.length > 0) {
                this.total = res.data.total
                this.data = res.data.data
              } else {
                this.currentPage -= 1
                this.total = res.data.total
                this.getQosInfo()
              }
            } else {
              this.total = 0
              this.data = []
            }
          }
        })
    },
    getInfo () {
      getPorts()
        .then(res => {
          if (res.data.code === 200) {
            res.data.interfaces.forEach(element => {
              this.netList.push(element.webname)
            })
          }
          console.log(this.netList)
        })
        .then(this.getQosInfo)
        .catch(error => {
          console.log(error)
        })
    }
  },
  beforeCreate: function() {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
  },
  created: function() {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
  },
  beforeMount: function() {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
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
    this.getInfo()
  }
}
</script>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}
.shortselect .el-select {
  width: 85%;
}
</style>
