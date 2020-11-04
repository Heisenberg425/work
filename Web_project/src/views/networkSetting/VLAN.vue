<template>
  <section>

    <div class="line_02">
      <span>VLAN</span>
    </div>

    <el-col
      :span="24"
      class="toolbar"
      style="padding-bottom: 0px;">
      <el-form :inline="true">
        <el-form-item>
          <el-button
            type="primary"
            @click="showAddForm">{{ $t('VLAN.add') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button
            type="danger"
            @click="batchDelete">{{ $t('VLAN.delete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <el-table
      :data="form"
      @selection-change="selsChange"
      class="table"
      :header-cell-style="headerStyle">
      <el-table-column
        type="selection"
        min-width="30"/>
      <el-table-column
        prop="iface"
        :label="$t('VLAN.interface')"
        min-width="120"/>
      <el-table-column
        prop="vlan_id"
        :label="$t('VLAN.id')"
        min-width="120"/>
      <el-table-column
        prop="tag"
        :label="$t('VLAN.tag')"
        min-width="120"/>
      <el-table-column
        prop="priority"
        :label="$t('VLAN.priority')"
        min-width="120"/>
      <el-table-column
        :label="$t('operation.operation')"
        min-width="60">
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="deleteInfo(scope.$index, scope.row)">{{ $t('operation.delete') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      @current-change="handleCurrentChange"
      :current-page="currentPage"
      :page-size="5"
      layout="total, prev, pager, next, jumper"
      :total="this.total"
      style="float:right"/>

    <!--新增界面-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('VLAN.add')"
      :visible.sync="addFormVisible"
      class="dialog"
      :close-on-click-modal="false">
      <el-form
        :model="addForm"
        :rules="rules"
        label-width="6rem"
        :rule="rules"
        label-position="left"
        ref="addForm">
        <el-form-item
          :label="$t('VLAN.id')"
          prop="vlan_id" 
          >
          <el-input v-model="addForm.vlan_id"/>
        </el-form-item>
        <el-form-item
          :label="$t('VLAN.interface')"
          prop="iface"
          required>
          <el-select
            v-model="addForm.iface"
            :placeholder="$t('placeholder.select')">
            <el-option
              v-for="(item, index) in ports"
              :key="index"
              :label="item.webname"
              :value="item.webname"/>
          </el-select>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer"
        >
        <el-button @click="cancelForm" type="danger">{{ $t('VLAN.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="addVLAN">{{ $t('VLAN.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>
  </section>
</template>

<script>
import { getVLAN, handleVLAN, getPorts } from '@/api/api.js'
export default {
  name: 'VLAN',
  data () {
    let vlanRules = (rule, value, callback) => {
      var reg = /.*\..*/
      if (value === '') {
        callback(new Error('请输入端口号！/Please input port number'))
      } else if (reg.test(value) === true) {
        callback(new Error('请输入整数！/Please input integer'))
        console.log('shuzi')
      } else if (value < 1 || value > 4094) {
        callback(new Error('端口范围为1~4094/port number range from 1 to 4094'))
      }
      callback()
    }
    return {
      addFormVisible: false,
      currentPage: 1,
      total: 0,

      form: [],
      addForm: {
        vlan_id: '', //
        iface: '', // 接口名称
        tag: '', // tag
        priority: '', // 优先级
        oper_type: ''
      },

      sels: [],
      ports: [],
      options: [],

      rules: {

        vlan_id: [
          { required: true, message: '请输入端口号！/Please input port number' },
          {
            validator: vlanRules, trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    triggerAdd () {
      this.addFormVisible = !this.addFormVisible
    },
    showAddForm: function () {
      this.triggerAdd()
    },
    handleCurrentChange (val) {
      this.currentPage = val
      this.getVLANInfo()
    },
    selsChange: function (sels) {
      this.sels = sels
    },
    addVLAN () {
      this.$refs['addForm'].validate(valid => {
        if (valid) {
          let para = Object.assign({}, this.addForm)
          para.oper_type = 'set'
          para.vlan_id = Number(para.vlan_id)
          para.page = this.currentPage
          handleVLAN(para)
            .then(res => {
              if (res.data.code === 200) {
                this.getVLANInfo()
                this.triggerAdd()
                this.addFormVisible = false
                this.$refs['addForm'].resetFields()
              }
            })
            .catch(error => {
              console.log(error)
            })
        }
      })
    },
    cancelForm () {
      this.triggerAdd()
      this.$refs['addForm'].resetFields()
    },
    deleteInfo (index, row) {
      let para = Object.assign({}, row)
      para.oper_type = 'clear'
      para.vlan_id = Number(para.vlan_id)
      console.log(para.vlan_id)
      handleVLAN(para)
        .then(res => {
          if (res.data.code === 200) {
            this.getVLANInfo()
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    async batchDelete () {
      for (let i = 0; i < this.sels.length; i++) {
        del(i, this.sels[i])
      }
      this.getVLANInfo()

      function del (index, row) {
        let para = Object.assign({}, row)
        para.oper_type = 'clear'
        para.vlan_id = Number(para.vlan_id)
        handleVLAN(para)
          .then(res => {
            if (res.sata.code !== 200) {
              handleVLAN(para)
            }
          })
          .catch(error => {
            console.log(error)
          })
      }
    },
    getVLANInfo () {
      let para = Object.assign({}, this.addForm)
      para.oper_type = 'vlan'
      para.page = this.currentPage
      getVLAN(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.total !== 0) {
              if (res.data.data.length !== 0) {
                this.total = res.data.total
                this.form = res.data.data
              } else if (this.currentPage > 1) {
                this.currentPage -= 1
                this.getVLANInfo()
                this.total = res.data.total
              }
            } else {
              this.form = []
              this.total = res.data.total
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    getPortsInfo: function () {
      getPorts()
        .then(res => {
          if (res.data.code === 200) {
            this.ports = res.data.interfaces
            console.log(this.ports)
          }
        })
        .catch(error => {
          console.log(error)
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
    this.getPortsInfo()
    this.getVLANInfo()
  }
}
</script>

<style scoped>
.table {
  width: 100%;
}
.table .el-table-column {
  width: 80%;
}
.dialog .el-input {
  width: 80%;
}
.dialog .el-select {
  width: 80%;
}
.el-pagination {
  float: right;
}
</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}

</style>