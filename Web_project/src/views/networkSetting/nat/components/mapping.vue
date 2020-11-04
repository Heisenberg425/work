<template>
  <section>
    <!--端口映射工具栏和表格-->
    <div class="table">
      <el-col
        :span="24"
        class="toolbar"
        style="padding-bottom: 0;">
        <el-form :inline="true">
          <el-form-item>
            <el-button
              type="primary"
              @click="addMappingHandle">{{ $t('operation.add') }}</el-button>
          </el-form-item>
          <el-form-item>
            <el-button
              type="danger"
              @click="delMappingHandle"
              :disabled="this.mappingSelections.length===0">{{ $t('operation.delete') }}</el-button>
          </el-form-item>
         
        </el-form>
      </el-col>

      <!--端口映射表格-->
      <el-table
        :data="mapping"
        v-focus
        @selection-change="mappingSelChange"
        style="width: 100%"
        :header-cell-style="headerStyle">
        <el-table-column
          type="selection"
          min-width="30"/>
        <el-table-column
          type="index"
          width="60"/>
        <el-table-column
          prop="assport"
          :label="$t('NAT.mapping.assPort')"
          min-width="120"/>
        <el-table-column
          prop="localport"
          :label="$t('NAT.mapping.localPort')"
          min-width="120"/>
        <el-table-column
          prop="ip"
          :label="$t('NAT.mapping.IP')"
          min-width="120"/>
        <el-table-column
          prop="ipcnt"
          :label="$t('NAT.mapping.IPCnt')"
          min-width="120"/>
        <el-table-column
          prop="proto"
          :label="$t('NAT.mapping.proto')"
          min-width="120"/>
        <el-table-column
          :label="$t('operation.operation')"
          min-width="120">
          <template slot-scope="scope">
            <!--<el-button  size="small" @click="handleMappingEdit(scope.$index, scope.row)">编辑</el-button>-->
            <el-button
              size="small"
              @click="signalDelMappingHandle(scope.$index, scope.row)">{{ $t('operation.delete') }}</el-button>
          </template>
        </el-table-column>
      </el-table>
       <el-pagination
            layout="total, prev, pager, next, jumper"
            @current-change="mappingHandleCurrentChange"
            :page-size="5"
            :total=mappingTotal
            style="float:right;"/>
    </div>

    <!--新增端口映射-->
    <!--wyk1120修改labwidth-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('NAT.mapping.add')"
      :visible.sync="mappingFormVisible"
      :close-on-click-modal="false"
      width="30%">
      <el-form
        :model="mappingForm"
        :rules="rules"
        label-position="left"
        ref="mappingForm"
        label-width="7rem"
        size="small">
        <el-form-item
          :label="$t('NAT.mapping.assPort')"
          
          prop="assport"
          >
          <el-input v-model="mappingForm.assport"/>
        </el-form-item>
        <el-form-item
          :label="$t('NAT.mapping.localPort')"
          
          prop="localport"
          >
          <el-input v-model="mappingForm.localport"/>
        </el-form-item>
        <el-form-item
          :label="$t('NAT.mapping.IP')"
         
          prop="ip"
          >
          <el-input v-model="mappingForm.ip"/>
        </el-form-item>
        <el-form-item
          :label="$t('NAT.mapping.IPCnt')"
          
          prop="ipcnt"
          >
          <el-input v-model="mappingForm.ipcnt"/>
        </el-form-item>
        <el-form-item
          :label="$t('NAT.mapping.proto')"
          
          prop="proto"
          >
          <el-select
            v-model="mappingForm.proto"
            :placeholder="$t('NAT.mapping.placeholder')"
            value=""
            style="width: 100%;">
            <el-option
              v-for="item in protoOption"
              :key="item.label"
              :label="item.label"
              :value="item.label"/>
          </el-select>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          type="danger"
          @click="mappingCancel">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="addMappingSubmit">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!--编辑端口映射-->
    <!-- <el-dialog title="编辑端口映射" :visible.sync="mappingEditFormVisible" width="50%">
    <el-form :model="mappingForm" status-icon :rules="mappingRule" label-position="left" ref="mappingForm" size="small">
      <el-form-item :label="$t('NAT.mapping.assPort')" prop="assport" :label-width="tooltipLabelWidth">
        <el-input v-model="mappingForm.assport"></el-input>
      </el-form-item>
      <el-form-item :label="$t('NAT.mapping.localPort')" prop="localport" :label-width="tooltipLabelWidth">
        <el-input v-model="mappingForm.localport"></el-input>
      </el-form-item>
      <el-form-item :label="$t('NAT.mapping.IP')" prop="ip" :label-width="tooltipLabelWidth">
        <el-input v-model="mappingForm.ip"></el-input>
      </el-form-item>
      <el-form-item :label="$t('NAT.mapping.IPCnt')" prop="ipcnt" :label-width="tooltipLabelWidth">
        <el-input v-model="mappingForm.ipcnt"></el-input>
      </el-form-item>
      <el-form-item :label="$t('NAT.mapping.proto')" prop="proto" :label-width="tooltipLabelWidth">
        <el-select v-model="mappingForm.proto" placeholder="请选择" value="" style="width: 100%;">
          <el-option
            v-for="item in protoOption"
            :key="item.label"
            :label="item.label"
            :value="item.label">
          </el-option>
        </el-select>
      </el-form-item>
    </el-form>
    <div slot="footer" class="dialog-footer">
      <el-button type="danger" @click="mappingCancel">取 消</el-button>
      <el-button type="primary" @click="editMappingSubmit">确 定</el-button>
    </div>
  </el-dialog> -->
  </section>
</template>

<script>
import {
  getMapping,
  addMapping,
  delMapping,
  editMapping
} from '@/api/api'
import validate from '@/utils/rules'

export default {
  name: 'Mapping',
  directives: {
    focus: {
      inserted: function (el) {
        el.focus()
      }
    }
  },
  data () {
    let proto = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择协议！/Please select protocol'))
      }
      callback()
    }
    return {
      mappingFormVisible: false, // 端口映射添加表格显示控制
      mappingEditFormVisible: false, // 端口映射编辑表格显示控制

      mappingPage: 1,
      mappingTotal: 0,

      mappingForm: {
        assport: '', // 外网端口
        localport: '', // 内网端口
        ip: '', // 内网起始IP
        ipcnt: '', // 内网个数
        proto: '', // 协议
        handle: ''
      },

      protoOption: [
        {
          label: 'tcp'
        },
        {
          label: 'udp'
        }
      ],

      mapping: [],
      mappingSelections: [], // 端口映射批量删除列表选中列

      tooltipLabelWidth: '100px',
      formLabelWidth: '130px',

      rules: {
        assport: [
          { required: true, message: '端口不能为空！/Please input number' },
          {
            validator: validate('number', '请输入数字！/Please input correct number', '端口不能为空！/Please input number', '0', 63487),
            trigger: 'blur'
          }
        ],
        localport: [
          { required: true, message: '端口不能为空！/Please input number' },
          {
            validator: validate('number', '请输入数字！/Please input correct number', '端口不能为空！/Please input number', '0', 65535),
            trigger: 'blur'
          }
        ],
        ip: [
          { required: true, message: 'IP不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP不能为空！/Please input IP'),
            trigger: 'blur'
          }
        ],
        ipcnt: [
          { required: true, message: '请输入内网个数！/Please input number' },
          {
            validator: validate('number', '请输入正确数字！/Please input correct number', '请输入内网个数！/Please input number', 1),
            trigger: 'blur'
          }
        ],
        proto: [
          { required: true, message: '请选择协议！/Please select protocol' },
          {
            validator: proto,
            trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },

    // 获得mapping信息
    getMappingInfo: function () {
      let para = {
        page: this.mappingPage,
        count: 5
      }
      getMapping(para)
        .then(res => {
          if (this.mappingPage === res.data.page) {
            if (res.data.total !== 0) {
              if (res.data.data.length !== 0) {
                this.mappingTotal = res.data.total
                this.mapping = res.data.data
              } else if (res.data.total > 5) {
                this.mappingPage -= 1
                this.mappingTotal = res.data.total
                this.getMappingInfo()
              }
            } else if (res.data.total === 0) {
              this.mapping = []
              this.mappingTotal = res.data.total
            }
          } else {
            this.getMappingInfo()
            this.mappingTotal = res.data.total
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    // 映射表格编辑
    handleMappingEdit: function (index, row) {
      this.mappingEditFormVisible = true
      this.mappingForm = Object.assign({}, row)
    },
    // 映射表格编辑提交
    editMappingSubmit: function () {
      this.mappingForm.handle = 0
      let para = Object.assign({}, this.mappingForm)
      // console.log("mappingForm is"+this.mappingForm);
      if (this.$store.state.app.language === 'zh') {
        editMapping(para)
          .then(() => {
            this.$message({
              message: '提交成功',
              type: 'success'
            })
            this.$refs['mappingForm'].resetFields()
            this.mappingEditFormVisible = false
            this.getMappingInfo()
          })
          .catch(error => {
            console.log(error)
          })
      } else {
        editMapping(para)
          .then(() => {
            this.$message({
              message: 'SubmitSuccess',
              type: 'success'
            })
            this.$refs['mappingForm'].resetFields()
            this.mappingEditFormVisible = false
            this.getMappingInfo()
          })
          .catch(error => {
            console.log(error)
          })
      }
    },
    // 映射表格编辑取消
    mappingCancel () {
      this.mappingFormVisible = false
      this.mappingEditFormVisible = false
    },
    // 映射表格添加处理 显示端口dialogue
    addMappingHandle: function () {
      this.mappingFormVisible = true
    },
    // 映射表格添加提交
    addMappingSubmit: function () {
      this.$refs['mappingForm'].validate(valid => {
        console.log(valid)
        if (valid) {
          let para = Object.assign({}, this.mappingForm)
          para.handle = 0
          if (this.$store.state.app.language === 'zh') {
            addMapping(para)
              .then(res => {
                if (res.data.code === 200) {
                  this.$message({
                    message: '提交成功',
                    type: 'success'
                  })
                }
                this.$refs['mappingForm'].resetFields()
                this.getMappingInfo()
              })
              .then(() => {
                this.mappingFormVisible = false
                this.mappingForm = {}
              })
              .catch(error => {
                console.log(error)
              })
          } else {
            addMapping(para)
              .then(() => {
                this.$message({
                  message: 'SubmitSuccess',
                  type: 'success'
                })
                this.$refs['mappingForm'].resetFields()
                this.getMappingInfo()
              })
              .then(() => {
                this.mappingFormVisible = false
                this.mappingForm = {}
              })
              .catch(error => {
                console.log(error)
              })
          }
        }
      })
    },
    // 映射表格删除
    delMappingHandle: function () {
      if (this.$store.state.app.language === 'zh') {
        this.$confirm('确认删除记录吗？', '提示', {
          type: 'warning'
        })
          .then(() => {
            for (let i = 0; i < this.mappingSelections.length; i++) {
              del(this.mappingSelections[i])
            }
          })
          .then(() => {
            this.getMappingInfo()
          })
      } else {
        this.$confirm('Are you sure to delete record？', 'Warning', {
          type: 'warning'
        })
          .then(() => {
            for (let i = 0; i < this.mappingSelections.length; i++) {
              del(this.mappingSelections[i])
            }
          })
          .then(() => {
            this.getMappingInfo()
          })
      }

      function del (item) {
        let para = Object.assign({}, item)
        para.handle = 1
        para.assport = String(para.assport)
        para.ipcnt = String(para.ipcnt)
        para.localport = String(para.localport)
        console.log(para.assport)
        delMapping(para)
          .then(() => {})
          .catch(error => {
            console.log(error)
          })
      }
    },
    // 单独删除一个映射表格
    signalDelMappingHandle: function (index, row) {
      // row.handle = 1
      // row.assport = String(row.assport)
      // row.ipcnt = String(row.ipcnt)
      // row.localport = String(row.localport)
      let para = Object.assign({}, row)
      para.handle = 1
      para.assport = String(para.assport)
      para.ipcnt = String(para.ipcnt)
      para.localport = String(para.localport)
      // console.log("the row is "+row);
      if (this.$store.state.app.language === 'zh') {
        this.$confirm('确认删除选中记录吗？', '提示', {
          type: 'warning'
        }).then(() => {
          delMapping(para).then(() => {
            this.$message({
              message: '删除成功',
              type: 'success'
            })
            this.getMappingInfo()
          })
        })
          .catch(error => {
            console.log(error)
          })
      } else {
        this.$confirm('Are you sure to delete the selected record？', 'Warning', {
          type: 'warning'
        }).then(() => {
          delMapping(para).then(() => {
            this.$message({
              message: 'DeleteSuccess',
              type: 'success'
            })
            this.getMappingInfo()
          })
        })
          .catch(error => {
            console.log(error)
          })
      }
    },
    // 映射表格 分页选择 跳到选中页面
    mappingHandleCurrentChange (val) {
      this.mappingPage = val
      this.getMappingInfo()
    },
    // 映射表格 多选 确定选中选项
    mappingSelChange: function (sels) {
      this.mappingSelections = sels
    }
  },
  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
    }
  },
  mounted () {
    this.getMappingInfo()
  }
}
</script>

<style scoped>
.table {
  width: 100%;
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
