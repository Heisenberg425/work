<template>
  <div>
    <div class="line_02">
      <span>{{ $t('ARP.title') }}</span>
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
            @click="batchDelete"
            type="danger">{{ $t('operation.delete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <el-table
      :data="form"
      @selection-change="change"
      :header-cell-style="headerStyle">
      <el-table-column
        type="selection"
        min-width="60"/>
      <el-table-column
        prop="ipaddr"
        :label="$t('ARP.IP')"
        min-width="120"/>
      <el-table-column
        prop="hwaddr"
        :label="$t('ARP.MAC')"
        min-width="120"/>
      <el-table-column
        prop="status"
        :label="$t('ARP.status')"
        min-width="120">
        <template slot-scope="scope">
          <span :class="{ warning: scope.row.status === 'invalid' }">{{ scope.row.status }}</span>
        </template>
      </el-table-column>
      <el-table-column
        :label="$t('operation.operation')"
        min-width="60">
        <template slot-scope="scope">
          <el-button
            v-if="scope.row.status==='configured'"
            type="text"
            @click="edit(scope.row)"
            size="small">
            {{ $t('operation.edit') }}
          </el-button>
          <el-button
            type="text"
            @click="deleteForm(scope.row)"
            size="small">
            {{ $t('operation.delete') }}
          </el-button>
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

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('ARP.title2')"
      :visible.sync="isFormVisible"
      :close-on-click-modal="false"
      @selection-change="change()">
      <el-form
        :model="ARPForm"
        label-position="left"
        ref="ARPForm"
        :rules="rules"
        size="mini">
        <el-form-item
          prop="ipaddr"
          :label="$t('ARP.IP')"
          
          :label-width="labelWidth">
          <el-input style="width:85%" v-model="ARPForm.ipaddr"/>
        </el-form-item>
        <el-form-item
          prop="hwaddr"
          :label="$t('ARP.MAC')"
          
          :label-width="labelWidth">
          <el-input style="width:85%" v-model="ARPForm.hwaddr"/>
        </el-form-item>
      </el-form>
      <div slot="footer">
        <el-button
          type="danger"
          @click="formCancel()">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="formSubmit('set')">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
</div>

<div class="thisdialogstyle">
    <el-dialog
      :title="$t('ARP.title3')"
      :close-on-click-modal="false"
      :visible.sync="isEditFormVisible">
      <el-form
        :model="editForm"
        label-position="left"
        :rules="rules"
        ref="editForm"
        size="mini">
        <el-form-item
          prop="ipaddr"
          :label="$t('ARP.IP')"
          :label-width="labelWidth">
          <el-input
            v-model="editForm.ipaddr"
            style="width:85%"
            disabled="true"/>
        </el-form-item>
        <el-form-item
          prop="hwaddr"
          :label="$t('ARP.MAC')"
          :label-width="labelWidth">
          <el-input style="width:85%" v-model="editForm.hwaddr"/>
        </el-form-item>
      </el-form>
      <div slot="footer">
        <el-button
          type="danger"
          @click="editCancel()">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="formSubmit('update')">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
</div>

    <!-- 动态ARP表 -->

    <el-collapse>
      <el-collapse-item
        :title="$t('ARP.dynamicARP')"
        name="1">
        <div>
          <el-table
            :data="formDynamic"
            :header-cell-style="headerStyle">
            <el-table-column
              prop="ipaddr"
              :label="$t('ARP.IP')"
              min-width="120"/>
            <el-table-column
              prop="hwaddr"
              :label="$t('ARP.MAC')"
              min-width="120"/>
          </el-table>
        </div>
        <el-pagination
          @current-change="handleCurrentChangeDynamic"
          :current-page="currentPageDynamic"
          :page-size="5"
          layout="total, prev, pager, next, jumper"
          :total="this.totalDynamic"
          style="float:right"/>
      </el-collapse-item>
    </el-collapse>
  </div>
</template>

<script>
import { getARP, getDynamicARP, handleARP } from '@/api/api.js'
import validate from '@/utils/rules.js'
export default {
  name: 'ARP',
  data () {
    return {
      isFormVisible: false,
      isEditFormVisible: false,

      total: 0,
      totalDynamic: 0,
      currentPage: 1,
      currentPageDynamic: 1,

      form: [],
      formDynamic: [],

      ARPForm: {
        ipaddr: '',
        hwaddr: '',
        status: '',
        oper_type: ''
      },
      editForm: {
        ipaddr: '',
        hwaddr: '',
        oper_type: ''
      },
      sels: [],

      labelWidth: '5rem',

      rules: {
        ipaddr: [
          { required: true, message: 'IP地址不能为空!/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        hwaddr: [
          { required: true, message: 'MAC地址不能为空!/Please input MAC' },
          {
            validator: validate('mac', '请输入正确MAC！/Please input correct MAC', 'MAC地址不能为空！/Please input MAC'), trigger: 'blur'
          }
        ]
      }
    }
  },
  methods: {
    headerStyle () {
      return this.header()
    },
    change (sels) {
      this.sels = sels
    },
    handleCurrentChange (val) {
      let para = {}
      para.ipaddr = ''
      para.hwaddr = ''
      para.oper_type = 'arp'
      para.page = val
      getARP(para)
        .then(res => {
          if (res.data.code === 200) {
            this.form = res.data.data
          }
          this.currentPage = val
        })
        .catch(error => {
          console.log(error)
        })
    },
    handleCurrentChangeDynamic (val) {
      let para = {}
      para.ipaddr = ''
      para.hwaddr = ''
      para.oper_type = 'arp'
      para.page = val
      getDynamicARP(para)
        .then(res => {
          if (res.data.code === 200) {
            this.formDynamic = res.data.data
          }
          this.currentPageDynamic = val
        })
        .catch(error => {
          console.log(error)
        })
    },

    addHandle: function () {
      this.isFormVisible = true
    },
    // 编辑
    edit (row) {
      this.editForm = row
      this.isEditFormVisible = true
    },
    editCancel () {
      this.isEditFormVisible = false
      this.$refs['editForm'].resetFields()
    },
    // 添加
    formCancel: function () {
      this.isFormVisible = false
      this.$refs['ARPForm'].resetFields()
    },
    formSubmit: function (type) {
      let isValid = false
      this.isFormVisible = false
      this.isEditFormVisible = false

      let para
      if (type === 'set') {
        para = Object.assign({}, this.ARPForm)
        this.$refs.ARPForm.validate(valid => {
          isValid = valid
        })
      } else if (type === 'update') {
        para = Object.assign({}, this.editForm)
        this.$refs.editForm.validate(valid => {
          isValid = valid
        })
      }
      if (isValid) {
        para.oper_type = type
        handleARP(para)
          .then(res => {
            if (res.data.code === 200) {
              this.getARPInfo()
              this.getDynamicARPInfo()
            }
          })
          .catch(error => {
            console.log(error)
          })
      }
      this.$refs['ARPForm'].resetFields()
      this.$refs['editForm'].resetFields()
    },
    // 删除
    deleteForm (row) {
      let para = Object.assign({}, row)
      para.oper_type = 'clear'
      console.log('para')
      console.log(para)

      handleARP(para)
        .then(res => {
          if (res.data.code === 200) {
            this.getARPInfo()
            this.getDynamicARPInfo()
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    batchDelete () {
      console.log('sels')
      console.log(this.sels)
      for (let i = 0; i < this.sels.length; i++) {
        console.log('del')
        del(this.sels[i])
      }
      this.getARPInfo()
      this.getDynamicARPInfo()

      function del (row) {
        let para = Object.assign({}, row)
        para.oper_type = 'clear'

        handleARP(para)
          .then(res => {
            if (res.data.code === 200) {
              return true
            } else if (res.data.code === 500) {
              return false
            }
          })
          .catch(error => {
            console.log(error)
          })
      }
    },

    getARPInfo () {
      let para = {}
      para.ipaddr = ''
      para.hwaddr = ''
      para.oper_type = 'arp'
      para.page = this.currentPage
      getARP(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.length !== 0) {
              this.form = res.data.data
              this.total = res.data.total
            } else {
              if (this.total === 0) {
                this.total = res.data.total
                this.form = res.data.data
              } else {
                this.total = res.data.total
                this.currentPage -= 1
                this.getARPInfo()
              }
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },
    getDynamicARPInfo () {
      let para = {}
      para.ipaddr = ''
      para.hwaddr = ''
      para.page = this.currentPageDynamic
      getDynamicARP(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.length !== 0) {
              this.formDynamic = res.data.data
              this.totalDynamic = res.data.total
            } else {
              if (this.totalDynamic === 0) {
                this.totalDynamic = res.data.total
                this.formDynamic = res.data.data
              } else {
                this.totalDynamic = res.data.total
                this.currentPageDynamic -= 1
                this.getDynamicARPInfo()
              }
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
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
    this.getARPInfo()
    this.getDynamicARPInfo()
  }
}
</script>

<style lang="scss" scoped>
.warning {
  color: red;
}
</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}

</style>

