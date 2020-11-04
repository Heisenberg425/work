<template>
  <section>
    <div class="line_02">
      <span>{{ $t('systemSetting.AccountSetting') }}</span>
    </div>

    <!--添加账户的按钮-->
    <el-col :span="24">
      <el-form :inline="true">
        <el-form-item>
          <el-button
            type="danger"
            disabled="false"
            @click="addAccount" >{{ $t('systemSetting.addAccount') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <!--表格头-->
    <el-table
      :data="userShow"
      @selection-change="selsChange"
      style="width: 100%;"
      :header-cell-style="headerStyle">
      <!-- <el-table-column type="selection" width="55">
      </el-table-column> -->
      <el-table-column
        prop="username"
        :label="$t('systemSetting.username')"
        min-width="120"/>
      <el-table-column
        prop="passwdShow"
        :label="$t('systemSetting.passwd')"
        min-width="120"/>
      <el-table-column
        prop="role"
        :label="$t('systemSetting.role')"
        min-width="120"/>
      <el-table-column
        prop="status"
        :label="$t('systemSetting.status')"
        min-width="120"/>
      <el-table-column
        prop="behaviour"
        :label="$t('systemSetting.behaviour')"
        min-width="240">
        <template slot-scope="scope">
          <el-button
            size="small"
            @click="changePasswd(scope.row)">{{ $t('systemSetting.changePasswd') }}</el-button>
          <el-button
            size="small"
            :disabled="scope.row.username == 'admin'"
            @click="deleteUser(scope.row)">{{ $t('systemSetting.deleteUser') }}</el-button>  <!--wyk0930修改管理员账号不可删除与禁用-->
          <el-button
            size="small"
            :disabled="scope.row.username == 'admin'"
            @click="changeStatus(scope.row)">{{ $t('systemSetting.changeStatus') }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <!--修改密码的功能实现-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('systemSetting.changePasswd')"
      :close-on-click-modal="false"
      :visible.sync="isInChanging">
      <el-form
        :model="checkForm"
        ref="addForm"
        :rules="rules">
        <el-form-item
          prop="old"
          
          :label="$t('systemSetting.old')">
          <el-input
            type="password"
            maxlength=63
            v-model="checkForm.old"
            :placeholder="$t('placeholder.pw1')"/    >
        </el-form-item>
        <el-form-item
          prop="one"
          :label="$t('systemSetting.one')">
          <el-input
            type="password"
            maxlength=63
            v-model="checkForm.one"
            :placeholder="$t('placeholder.pw2')"/>
        </el-form-item>
        <el-form-item
          prop="two"
          :label="$t('systemSetting.two')">
          <el-input
            type="password"
            maxlength=63
            v-model="checkForm.two"
            :placeholder="$t('placeholder.pw3')"/>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          type="danger"
          @click="addCancel">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="addSubmit">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!--更改权限的功能实现-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('systemSetting.changeStatus')"
      :close-on-click-modal="false"
      :visible.sync="isInChangingStatus">
      <el-form
        ref="form"
        :model="userRec"
        label-width="8rem">
        <el-form-item
          prop="status"
          :label="$t('systemSetting.selectStatus')">
          <el-radio
            v-model="status1"
            label = "0">{{ $t('systemSetting.On') }} </el-radio>
          <el-radio
            v-model="status1"
            label = "1">{{ $t('systemSetting.Off') }}</el-radio>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          type="danger"
          @click="changeStatusCancel">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="subChangeStatus">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!--删除用户的功能实现-->
    <div class="thisdialogstyle">
    <el-dialog
      :title="$t('systemSetting.deleteUser')"
      :close-on-click-modal="false"
      :visible.sync="isInDelete">
      <el-form
        ref="form"
        :model="userRec"
        label-width="16rem">
        <el-form-item
          prop="delete"
          :label="$t('systemSetting.ifDelete')"/>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          type="danger"
          @click="deleteCancel">{{ $t('operation.cancel') }}</el-button>
        <el-button
          type="primary"
          @click="deldeteSubmit">{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

    <!--添加用户弹出界面的功能实现-->
    <div class="thisdialogstyle">
    <el-dialog
    :close-on-click-modal="false"
      :title="$t('systemSetting.addAccount')"
      :visible.sync="isAdd">
      <el-form
        ref="form"
        :model="userRec"
        :rules="rules"
        label-width="8rem">
        <el-form-item
          prop="username"
          :label="$t('systemSetting.username')">
          <el-input
            v-model="userRec.username"
            placeholder="请输入用户名"/>
        </el-form-item>
        <el-form-item
          prop="passwd"
          :label="$t('systemSetting.passwd1')">
          <el-input
            type="password"
            v-model="passwd1"
            placeholder="请输入密码"/>
        </el-form-item>
        <el-form-item
          prop="passwd"
          :label="$t('systemSetting.passwd2')">
          <el-input
            type="password"
            v-model="passwd2"
            placeholder="请再次输入密码"/>
        </el-form-item>
        <el-form-item
          prop="role"
          :label="$t('systemSetting.selectPower')">
          <el-select
            v-model="userRec.role"
            placeholder="请选择">
            <el-option
              v-for="item in options"
              :key="item.value"
              :label="item.label"
              :value="item.value"/>
          </el-select>
        </el-form-item>
      </el-form>
      <div
        slot="footer"
        class="dialog-footer">
        <el-button
          size="small"
          @click="addNewAccountCancel" >{{ $t('operation.cancel') }}</el-button>
        <el-button
          size="small"
          @click="addNewAccount" >{{ $t('operation.submit') }}</el-button>
      </div>
    </el-dialog>
    </div>

  </section>
</template>

<script>
import { changePasswordAction, queryUsersAction, addUserAction, delUserAction, disableUserAction } from '@/api/api.js'
import md5 from 'js-md5'
export default {
  name: 'AccountSetting',
  data () {
    let passwd = (rules, value, callback) => {
      if (this.passwd2 !== this.passwd1) {
        callback(new Error('两次密码不一致！/The two passwords differ'))
      }
    }
    return {
      new_passwd: '',
      userNowRole: '',
      userNowStatus: '',
      userNowName: '',
      userNowPasswd: '',
      delete: 0,
      isInDelete: false,
      status1: '',
      passwd1: '',
      passwd2: '',
      isInChanging: false,
      isInChangingStatus: false,
      sels: [],
      checkForm: {},
      list: [{status: ''}],
      form: [],
      isAdd: false,
      currentPage: 1,
      total: 0,
      selectPower: '',
      user: { /* 发送给后端的账号信息                       */

        cUsername: '', /*  用户名                                  */
        cPasswd: '', /*  密码                                    */
        iRole: Number, /*  角色权限 1:admin 2: operator 3: visitor */
        iStatus: Number /*  状态 0：启用、1：禁用                    */

      },
      userShow: [], /* 显示出来的账号信息                       */
      userRec: { /* 从输入框得到的账号信息                       */
        username: '',
        passwd: '',
        role: '',
        status: Number

      },
      options: [
        {
          value: 2,
          label: 'operator'
        },
        {
          value: 3,
          label: 'visitor'
        }
      ],
      rules: {
        old: [
          {
            min: 1, max: 63, message: '最大支持63个字符', trigger: 'change'
          }
        ],
        one: [
          {
            min: 1, max: 63, message: '最大支持63个字符', trigger: 'change'
          }
        ],
        two: [
          {
            min: 1, max: 63, message: '最大支持63个字符', trigger: 'change'
          },
          {
            validator: passwd, trigger: 'blur'
          }
        ],
        passwd: [
          {
            min: 1, max: 63, message: '最大支持63个字符', trigger: 'change'
          }
        ]
      }

    }
  },

  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      this.getUserInfo()
    }
  },

  methods: {
    headerStyle () {
      return this.header()
    },

    deleteCancel () {
      this.isInDelete = false
      this.$refs['form'].resetFields()
    },

    deleteUser (val) {
      console.log(val)
      this.userNowName = val.username
      this.isInDelete = true
    },

    addAccount () {
      this.isAdd = true
    },

    selsChange: function (sels) {
      this.sels = sels
    },

    changePasswd (val) {
      console.log(val)
      this.userNowName = val.username
      this.userNowPasswd = val.passwd
      this.isInChanging = true
    },

    changeStatus (val) {
      console.log(val)
      this.userNowName = val.username
      this.isInChangingStatus = true
    },

    deldeteSubmit () {
      let para = {}
      para.username = this.userNowName
      delUserAction(para).then(res => {
        if (res.data.code === 200) {
          if (this.$store.state.app.language === 'zh') {
            this.list[0].status = '修改成功'
          } else {
            this.list[0].status = 'Modified success'
          }
        } else {
          if (this.$store.state.app.language === 'zh') {
            this.list[0].status = '修改失败'
          } else {
            this.list[0].status = 'Modified failed'
          }
        }
        this.getUserInfo()
        this.isInDelete = false
      })
    },

    subChangeStatus () {
      let para = {}
      para.username = this.userNowName
      if (this.status1 === '0') {
        para.status = 0
        disableUserAction(para).then(res => {
          if (res.data.code === 200) {
            console.log('ccc')
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改成功'
            } else {
              this.list[0].status = 'Modified success'
            }
          } else {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改失败'
            } else {
              this.list[0].status = 'Modified failed'
            }
          }
          this.getUserInfo()
          this.isInChangingStatus = false
        })
      } else if (this.status1 === '1') {
        para.status = 1
        disableUserAction(para).then(res => {
          if (res.data.code === 200) {
            console.log('ddd')
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改成功'
            } else {
              this.list[0].status = 'Modified success'
            }
          } else {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改失败'
            } else {
              this.list[0].status = 'Modified failed'
            }
          }
          this.getUserInfo()
          this.isInChangingStatus = false
        })
      }
    },

    changeStatusCancel () {
      this.isInChangingStatus = false
      this.$refs['form'].resetFields()
    },

    addCancel () {
      this.isInChanging = false
      this.$refs['addForm'].resetFields()
    },

    addSubmit () {
      if (this.checkForm.one === this.checkForm.two) {
        let para = {}
        para.username = this.userNowName
        para.passwd = md5(this.checkForm.old)
        para.new_passwd = md5(this.checkForm.one)
        changePasswordAction(para).then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改成功'
              this.$message({
                message: '修改成功',
                type: 'success'
              })
            } else {
              this.list[0].status = 'Modified success'
              this.$message({
                message: 'Modified success',
                type: 'success'
              })
            }
            this.isInChanging = false
          } else {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改失败'
              this.$message({
                message: '修改失败',
                type: 'error'
              })
            } else {
              this.list[0].status = 'Modified failed'
              this.$message({
                message: 'Modified failed',
                type: 'error'
              })
            }
          }
        })
        this.getUserInfo()
      } else {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            type: 'error',
            message: '两次输入密码不一致！'
          })
        } else {
          this.$message({
            type: 'error',
            message: 'Two password inconsistencies！'
          })
        }
        this.getUserInfo()
        this.$refs['addForm'].resetFields()
      }
    },

    addNewAccountCancel () {
      this.isAdd = false
    },

    addNewAccount () {
      if (this.passwd1 === this.passwd2) {
        this.user.cUsername = this.userRec.username
        this.user.iRole = this.userRec.role
        this.user.iStatus = 0
        let para = {}
        para.username = this.user.cUsername
        para.passwd = md5(this.passwd1)
        para.role = this.user.iRole
        para.status = this.user.iStatus
        addUserAction(para).then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改成功'
            } else {
              this.list[0].status = 'Modified success'
            }
            this.getUserInfo()
          } else {
            if (this.$store.state.app.language === 'zh') {
              this.list[0].status = '修改失败'
            } else {
              this.list[0].status = 'Modified failed'
            }
          }
          this.isAdd = false
        })
      } else {
        if (this.$store.state.app.language === 'zh') {
          this.$message({
            type: 'error',
            message: '两次输入密码不一致！'
          })
        } else {
          this.$message({
            type: 'error',
            message: 'Two password inconsistencies！'
          })
        }
        this.$refs['form'].resetFields()
      }
    },

    getUserInfo () {
      let para = {}
      para.username = ''
      para.passwd = ''
      para.role = 1
      para.status = 1
      queryUsersAction(para)
        .then(res => {
          this.userShow = res.data.data
          for (let i = 0; i < res.data.data.length; i++) {
            this.userShow[i].passwdShow = '******'
            if (this.userShow[i].role === 1) {
              this.userShow[i].role = 'admin'
            } else if (this.userShow[i].role === 2) {
              this.userShow[i].role = 'operator'
            } else {
              this.userShow[i].role = 'visitor'
            }
            if (res.data.data[i].status === 0) {
              if (this.$store.state.app.language === 'zh') {
                this.userShow[i].status = '启用'
              } else {
                this.userShow[i].status = 'On'
              }
            } else {
              if (this.$store.state.app.language === 'zh') {
                this.userShow[i].status = '禁用'
              } else {
                this.userShow[i].status = 'Off'
              }
            }
          }
          console.log(this.userShow)
        })
    },

    getInfo () {
      let data = JSON.parse(sessionStorage.getItem('user'))
      console.log('user is ' + sessionStorage.getItem('user'))

      let para = {}
      para.username = data.userName
      para.passwd = '******'
      if (this.$store.state.app.language === 'zh') {
        para.status = '使用中'
      } else {
        para.status = 'InUse'
      }
      this.list.push(para)
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
    // this.getInfo()
    this.getUserInfo()
  }
}
</script>

<style scoped>
</style>
</style>
<style lang="scss">
.thisdialogstyle .el-dialog {
   
   width: 30%;
 
}

</style>