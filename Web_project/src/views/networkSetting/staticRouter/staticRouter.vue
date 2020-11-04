<template>
  <section>
    <div class="line_02">
      <span>{{ $t('staticRouter.title') }}</span>
    </div>

    <el-col  :span="24"  class="toolbar"  style="padding-bottom: 0px;">
      <el-form :inline="true">
        <el-form-item>
          <el-button  type="primary"  @click="showAddRouter">{{ $t('operation.add') }}</el-button>
        </el-form-item>
        <el-form-item>
          <el-button  type="danger"   @click="batchDeletion">{{ $t('operation.batchdelete') }}</el-button>
        </el-form-item>
      </el-form>
    </el-col>

    <el-table  :data="info"  @selection-change="selChange"  style="width: 100%;"  :header-cell-style="headerStyle">
      <el-table-column  type="selection"   :selectable="selectable"  min-width="30"/>
      <el-table-column  prop="destination" :label="$t('staticRouter.destination')"  min-width="120"/>
      <el-table-column  prop="genmask"     :label="$t('staticRouter.genmask')"      min-width="120"/>
      <el-table-column  prop="gateway"     :label="$t('staticRouter.gateway')"      min-width="120"/>
      <el-table-column  prop="flags"       :label="$t('staticRouter.flags')"        min-width="120"/>
      <el-table-column  prop="metric"      :label="$t('staticRouter.metric')"       min-width="120"/>
      <el-table-column  prop="iface"       :label="$t('staticRouter.iface')"        min-width="120"/>
      <el-table-column  :label="$t('operation.operation')"  min-width="60">
        <template slot-scope="scope">
          <el-button    size="small"  @click="deleteRouter(scope.$index, scope.row)"  v-if="(scope.row.route_type==='-host')||(scope.row.route_type==='-net')">
            {{ $t('operation.delete') }}
          </el-button>
          <el-button    size="small"  @click="editRouter(scope.$index, scope.row)"  v-else>
            {{ $t('staticRouter.button3') }}
          </el-button>
            <!-- <el-button size="small" @click="editRouter(scope.$index, scope.row)">{{$t('operation.edit')}}</el-button> -->
        </template>
      </el-table-column>
    </el-table>
    <el-pagination  @current-change="handleCurrentChange"  :current-page="currentPage"  :page-size="5"  layout="total, prev, pager, next, jumper"
      :total="total"  style="float:right"/>

    <systemRouter      :update="updateSignal"  class="systemRouter"/>
    <systemRouterIpv6  :update="updateSignal"  class="systemRouterIpv6"/>

    <!--新增路由类型选择界面-->
    <div class="thisdialogstyle">
      <el-dialog  :title="$t('staticRouter.button3')"  :visible.sync="isAddingRouter"  :close-on-click-modal="false">
        <div class="shortselect">
          <span>{{ $t('staticRouter.subtitle') }}:</span>
          <el-select  v-model="addedFunction"  :placeholder="$t('placeholder.select')">
            <el-option  v-for="item in options"  :key="item.value"  :label="item.label"  :value="item.value"/>
          </el-select>
        </div>
        <div  slot="footer"  class="dialog-footer">
          <el-button type="danger" @click="cancelAddForm">{{ $t('operation.cancel') }}</el-button>
          <el-button type="primary" @click="addRouteNextStop">{{ $t('operation.next') }}</el-button>
        </div>
      </el-dialog>
    </div>

    <!-- 添加主机路由页面 -->
    <div class="thisdialogstyle">
      <el-dialog  :title="$t('staticRouter.title3')"  :visible.sync="isHostRouter"  :close-on-click-modal="false">
        <el-form  :model="addRouterForm"  :rules="rules"  ref="addRouterForm"  label-position="left"  size="small">
          <el-form-item  prop="destination"  :label="$t('staticRouter.destination')"  :label-width="formLabelWidth">
            <el-input v-model="addRouterForm.destination"/>
          </el-form-item>
          <el-form-item  prop="genmask"      :label="$t('staticRouter.genmask')"      :label-width="formLabelWidth">
            <el-input v-model="addRouterForm.genmask"/>
          </el-form-item>
          <el-form-item  prop="gateway"      :label="$t('staticRouter.gateway')"      :label-width="formLabelWidth">
            <el-input v-model="addRouterForm.gateway"/>
          </el-form-item>
          <el-form-item  prop="iface"        :label="$t('staticRouter.iface')"        :label-width="formLabelWidth">
            <el-select  v-model="addRouterForm.iface"  :placeholder="$t('placeholder.select')">
              <el-option  v-for="(item, index) in ports"  :key="index"  :label="item.webname"  :value="item.webname"/>
            </el-select>
          </el-form-item>
          <el-form-item  :label="$t('staticRouter.metric')"  :label-width="formLabelWidth">
            <el-input  v-model="addRouterForm.metric"/>
          </el-form-item>
        </el-form>

        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="cancelRouter">{{ $t('operation.cancel') }}</el-button>
          <el-button  type="primary" @click="addHostRouter">{{ $t('operation.determine') }}</el-button>
        </div>
      </el-dialog>
    </div>

    <!-- 添加网络路由页面 -->
    <div class="thisdialogstyle">
      <el-dialog  :title="$t('staticRouter.title5')"  :visible.sync="isNetRouter"  :close-on-click-modal="false">
        <el-form  :model="addnetRouterForm"  :rules="rules"  ref="addnetRouterForm"  label-position="left"  size="small">
          <el-form-item  prop="destination"  :label="$t('staticRouter.destination')"  :label-width="formLabelWidth">
            <el-input v-model="addnetRouterForm.destination"/>
          </el-form-item>
          <el-form-item  prop="genmask"      :label="$t('staticRouter.genmask')"      :label-width="formLabelWidth">
            <el-input v-model="addnetRouterForm.genmask"/>
          </el-form-item>
          <el-form-item  prop="gateway"      :label="$t('staticRouter.gateway')"      :label-width="formLabelWidth">
            <el-input v-model="addnetRouterForm.gateway"/>
          </el-form-item>
          <el-form-item  prop="iface"        :label="$t('staticRouter.iface')"        :label-width="formLabelWidth">
            <el-select  v-model="addnetRouterForm.iface"  :placeholder="$t('placeholder.select')">
              <el-option  v-for="(item, index) in ports"  :key="index"  :label="item.webname"  :value="item.webname"/>
            </el-select>
          </el-form-item>
          <el-form-item  :label="$t('staticRouter.metric')"  :label-width="formLabelWidth">
            <el-input v-model="addnetRouterForm.metric"/>
          </el-form-item>
        </el-form>

        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="cancelRouter">{{ $t('operation.cancel') }}</el-button>
          <el-button  type="primary" @click="addNetRouter">{{ $t('operation.determine') }}</el-button>
        </div>
      </el-dialog>
    </div>

    <!--添加默认出口页面-->
    <div class="thisdialogstyle">
      <el-dialog  :title="$t('staticRouter.title6')"  :visible.sync="isIface"  :close-on-click-modal="false">
        <el-form  :model="addRouterForm"  ref="addRouterForm"  label-position="left"  :rules="rules"  size="small">
          <!-- <el-form-item prop="gateway" label="网关地址:" :label-width="formLabelWidth">
            <el-input v-model="addRouterForm.gateway" ></el-input>
          </el-form-item>        -->
          <el-form-item  prop="iface"  :label="$t('staticRouter.iface')"  :label-width="formLabelWidth">
            <el-select  v-model="addRouterForm.iface"  :placeholder="$t('placeholder.select')">
              <el-option  v-for="(item, index) in ports"  :key="index"    :label="item.webname"  :value="item.webname"/>
            </el-select>
          </el-form-item>
        </el-form>
      
        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="cancelRouter">{{ $t('operation.cancel') }}</el-button>
          <el-button  type="primary" @click="addIface">{{ $t('operation.determine') }}</el-button>
        </div>
      </el-dialog>
    </div>

    <!-- 添加默认网关页面 -->
    <div class="thisdialogstyle">
      <el-dialog  :title="$t('staticRouter.title7')"  :visible.sync="isGateway"  :close-on-click-modal="false">
        <el-form  :model="addRouterForm"  ref="addRouterForm"  label-position="left"  size="small"  :rules="rules">
          <el-form-item  prop="gateway"  :label="$t('staticRouter.gateway')"  :label-width="formLabelWidth">
            <el-input v-model="addRouterForm.gateway"/>
          </el-form-item>
          <el-form-item  prop="iface"    :label="$t('staticRouter.iface')"    :label-width="formLabelWidth">
            <el-select  v-model="addRouterForm.iface"  :placeholder="$t('placeholder.select')">
              <el-option  v-for="(item, index) in ports"  :key="index"  :label="item.webname"  :value="item.webname"/>
            </el-select>
          </el-form-item>
        </el-form>
      
        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="cancelRouter">{{ $t('operation.cancel') }}</el-button>
          <el-button  type="primary" @click="addGateway">{{ $t('operation.determine') }}</el-button>
        </div>
      </el-dialog>
    </div>

  </section>
</template>

<script>
import { handleRouter, vndinterfaceGetAction, getRouterInfo } from '@/api/api'
import { systemRouter, systemRouterIpv6 } from './components'
import validate from '@/utils/rules.js'

export default {
  name: 'StaticRouter',
  components: {
    systemRouter,
    systemRouterIpv6
  },

  data () {
    let metricRule = (rule, value, callback) => {
      value = Number(value)
      if (Number.isNaN(value)) {
        callback(new Error('请输入数字！/Please input number'))
      }
      if (value < 1 || value > 32767) {
        callback(new Error('请输入1~32767范围内的值！/Please input number range from 1 to 32767'))
      }
      callback()
    }

    let ifaceRule = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请选择端口！/Please select port'))
      }

      callback()
    }

    return {
      isAddingRouter: false,
      isHostRouter: false,
      isNetRouter: false,
      isIface: false,
      isGateway: false,
      addedFunction: '',
      choosenItem: [],
      ifaceTitle: '',
      ifaceTitle1: '添加默认出口',
      ifaceTitle2: '修改默认出口',
      gatewayTitle: '',
      gatewayTitle1: '添加默认网关',
      gatewayTitle2: '修改默认网关',
      // routerInfo: [],                  // 用来存放获取的routerInfo
      total: 0, // 存放总页数
      currentPage: 1, // 当前页数
      devName: [],
      info: [],
      form: {
        destination: '', // 目的地址
        gateway: '', // 网关
        genmask: '', // 掩码
        flags: '', // 路由flags
        prefix: '', // ipv6 前缀
        metric: '', // 路由度量值
        ref: '', // 引用值
        iface: '', // 使用
        ip_family: '' // ip地址族
      },
      addRouterForm: {
        destination: '', // 目的地址
        gateway: '', // 网关
        genmask: '', // 掩码
        flags: '', // 路由flags
        prefix: '', // ipv6 前缀
        metric: '', // 路由度量值
        ref: '', // 引用值
        iface: '', // 接口名称 出口
        ip_family: '', // ip地址族
        oper_type: '', // 增删改查
        route_type: '', // 路由类型
        page: '', // 当前页数
        total: '', // 总页数
        data: '' // 内网端口
      },
      addnetRouterForm: {
        destination: '', // 目的地址
        gateway: '', // 网关
        genmask: '', // 掩码
        flags: '', // 路由flags
        prefix: '', // ipv6 前缀
        metric: '', // 路由度量值
        ref: '', // 引用值
        iface: '', // 接口名称 出口
        ip_family: '', // ip地址族
        oper_type: '', // 增删改查
        route_type: '', // 路由类型
        page: '', // 当前页数
        total: '', // 总页数
        data: '' // 内网端口
      },
      options: [
        {
          value: '1',
          label: '添加主机路由'
        },
        {
          value: '2',
          label: '添加网络路由'
        },
        {
          value: '3',
          label: '添加默认网口'
        },
        {
          value: '4',
          label: '添加默认网关'
        }
      ],
      ports: [],
      updateSignal: false, // 提醒组件更新
      formLabelWidth: '100px',

      rules: {
        destination: [

          { required: true, message: 'IP地址不能为空!/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', 'IP地址不能为空!/Please input IP'), trigger: 'blur'
          }
        ],
        gateway: [
          { required: true, message: '网关地址不能为空!/Please input gateway' },
          {
            validator: validate('ip', '请输入正确网关地址！/Please input correct gateway', '网关地址不能为空!/Please input gateway'), trigger: 'blur'
          }
        ],
        genmask: [
          { required: true, message: '子网掩码不能为空!/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确子网掩码！/Please input correct netmask', '子网掩码不能为空!/Please input netmask'), trigger: 'blur'
          }
        ],
        metric: [
          { required: true, message: '请输入数字！/Please input number' },
          {
            validator: metricRule, trigger: 'blur'
          }
        ]//,
        // iface: [
        //   { required: true, message: '请选择端口！/Please select port' },
        //   {
        //     validator: ifaceRule, trigger: 'blur'
        //   }
        // ]
      }
    }
  },

  methods: {
    selectable(row, index) {
      if ((row.route_type === '-host') || (row.route_type === '-net')) {
        return true
      } else {
        return false
      }
    },

    headerStyle () {
      return this.header()
    },

    // 显示新增界面
    showAddRouter: function () {
      this.isAddingRouter = true
      if (this.$store.state.app.language === 'zh') {
        this.options = [
          {
            value: '1',
            label: '添加主机路由'
          },
          {
            value: '2',
            label: '添加网络路由'
          },
          {
            value: '3',
            label: '添加默认网口'
          },
          {
            value: '4',
            label: '添加默认网关'
          }
        ]
      } else {
        this.options = [
          {
            value: '1',
            label: 'Add HostRouter'
          },
          {
            value: '2',
            label: 'Add NetRouter'
          },
          {
            value: '3',
            label: 'Add DefaultNetPort'
          },
          {
            value: '4',
            label: 'Add DefaultGateway'
          }
        ]
      }
    },

    closeAddRouter: function () {
      this.isAddingRouter = false
    },

    // 选择添加路由类型
    addRouteNextStop () {
      if (this.addedFunction === '1') {
        this.triggerAddHostRouter()
      } else if (this.addedFunction === '2') {
        this.triggerAddNetRouter()
      } else if (this.addedFunction === '3') {
        this.ifaceTitle = this.ifaceTitle1
        this.triggerAddIface()
      } else if (this.addedFunction === '4') {
        this.gatewayTitle = this.gatewayTitle1
        this.triggerAddGateway()
      }
    },

    cancelAddForm () {
      this.closeAddRouter()
      this.addedFunction = ''
    },

    triggerAddHostRouter () {
      this.isHostRouter = !this.isHostRouter
    },

    triggerAddNetRouter () {
      this.isNetRouter = !this.isNetRouter
    },

    triggerAddIface () {
      this.isIface = !this.isIface
    },

    triggerAddGateway () {
      this.isGateway = !this.isGateway
    },

    editRouter (index, row) {
      if (row.route_type === '-gateway') {
        this.editGateway()
      } else if (row.route_type === 'default') {
        this.editIface()
      }
    },

    editIface () {
      this.ifaceTitle = this.ifaceTitle2
      this.triggerAddIface()
    },

    editGateway () {
      this.gatewayTitle = this.gatewayTitle2
      this.triggerAddGateway()
    },

    addHostRouter () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'add'
      para.route_type = '-host'
      para.ip_family = 'IPv4'
      para.metric = Number(para.metric)

      this.$refs.addRouterForm.validate(valid => {
        if (valid) {
          handleRouter(para)
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

          this.getRouterInfomation()
          this.update()

          this.triggerAddHostRouter()
          this.closeAddRouter()
          this.$refs['addRouterForm'].resetFields()
        }
      })
    },

    addNetRouter () {
      let para = Object.assign({}, this.addnetRouterForm)
      para.oper_type = 'add'
      para.route_type = '-net'
      para.metric = Number(para.metric)

      this.$refs.addnetRouterForm.validate(valid => {
        if (valid) {
          handleRouter(para)
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

          this.getRouterInfomation()
          this.update()

          this.triggerAddNetRouter()
          this.closeAddRouter()
          this.$refs['addnetRouterForm'].resetFields()
        }
      })
    },

    addIface () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'add'
      para.route_type = 'default'

      this.$refs.addRouterForm.validate(valid => {
        if (valid) {
          handleRouter(para)
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

          this.getRouterInfomation()
          this.update()

          this.triggerAddIface()
          this.closeAddRouter()
          this.$refs['addRouterForm'].resetFields()
        }
      })
    },

    addGateway () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'add'
      para.route_type = 'default'

      this.$refs.addRouterForm.validate(valid => {
        if (valid) {
          handleRouter(para)
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

          this.getRouterInfomation()
          this.update()

          this.triggerAddGateway()
          this.closeAddRouter()
          this.$refs['addRouterForm'].resetFields()
        }
      })
    },

    cancelRouter () {
      if (this.addedFunction === '1') {
        this.triggerAddHostRouter()
      } else if (this.addedFunction === '2') {
        this.triggerAddNetRouter()
        this.$refs['addnetRouterForm'].resetFields()
      } else if (this.addedFunction === '3') {
        this.triggerAddIface()
        console.log(this.addRouterForm.iface)
        console.log(this.ports)
      } else if (this.addedFunction === '4') {
        this.triggerAddGateway()
      }
      this.$refs['addRouterForm'].resetFields()
      // this.$refs.addRouterForm.resetFields()
      // this.addRouterForm = ''
    },

    deleteRouter (index, row) {
      let para = Object.assign({}, row)
      para.oper_type = 'del'
      para.ip_family = 'IPv4'
      console.log(row)

      handleRouter(para)
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
      this.getRouterInfomation()
      this.update()
      this.$forceUpdate()
    },

    selChange (sels) {
      this.choosenItem = sels
      console.log(this.choosenItem)
    },

    async batchDeletion () {
      let i
      for (i = 0; i < this.choosenItem.length; i++) {
        del(this.choosenItem[i])
      }

      async function del (item) {
        let para = Object.assign({}, item)
        para.oper_type = 'del'
        para.ip_family = 'IPv4'
        // console.log('in delection ' + Object.assign(item))

        handleRouter(para).then(res => {
          if (res.data.code === 200) {

          }
        })
      }
      this.getRouterInfomation()
      this.update()
    },

    getRouterInfomation () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'query'
      para.route_type = 'static'
      para.ip_family = 'IPv4'
      para.page = this.currentPage

      getRouterInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.data.length !== 0) {
              this.total = res.data.total
              this.info = JSON.parse(JSON.stringify(res.data.data))
            // console.log(JSON.stringify(this.info))
            } else {
              if (this.currentPage > 1) {
                this.currentPage -= 1
                this.getRouterInfomation()
                this.total = res.data.total
              } else {
                this.total = res.data.total
                this.info = JSON.parse(JSON.stringify(res.data.data))
              }
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    handleCurrentChange (val) {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'query'
      para.route_type = 'static'
      para.ip_family = 'IPv4'
      para.page = val
      console.log('val is: ' + val)
      getRouterInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            if (res.data.data.length !== 0) {
              this.total = res.data.total
              this.info = JSON.parse(JSON.stringify(res.data.data))
            // console.log(JSON.stringify(this.info))
            } else {
              this.total = res.data.total
              this.currentPage -= 1
              this.getRouterInfomation()
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
      this.currentPage = val
    },

    getInfo () {
      let para = Object.assign({}, this.addRouterForm)
      para.oper_type = 'query'
      para.route_type = 'static'
      para.ip_family = 'IPv4'
      para.page = 1

      getRouterInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.total = res.data.total
            this.info = JSON.parse(JSON.stringify(res.data.data))
            console.log(this.info)
          }
        })
        .catch(err => {
          console.log(err)
        })
    },

    update () {
      this.updateSignal = !this.updateSignal
    },

    getPortsInfo () {
      vndinterfaceGetAction()
        .then(res => {
          if (res.data.code === 200) {
            this.ports = res.data.interfaces
            console.log(this.ports)
          }
        })
        .catch(err => {
          console.log(err)
        })
    },
    setCustomValidity() {
      return ''
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
    this.getInfo()
    this.getPortsInfo()
  }
}
</script>

<style scoped>
.el-form {
  width: 80%;
  margin-left: 10px;
}
.el-pagination {
  text-align: right;
}
.systemRouter {
  margin-top: 10px;
}
.systemRouterIpv6 {
  margin-top: 10px;
}
.el-input {
  width: 100%;
}
.el-select {
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