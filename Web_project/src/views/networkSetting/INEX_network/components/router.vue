<template>
  <section>
    <div class="wholeRouter">

      <!-- power button -->
      <div class="power">
        <div class="powerButton">
          <img style="width: 60px; height: 50px; border-radius: 5px;"  :src="powerButton" >
          <div class="text-area">
            <p class="text">{{ $t('router.debug') }}</p>
          </div>
        </div>
      </div>

      <!-- router info -->
      <div class="router">
        <div v-for="(item, index) in ports"  class="port" :key="index">
          <div>
            <el-tooltip  class="item"  effect="light">
              <img  style="width: 60px; height: 50px; border-radius: 5px;"  :src=selectUrl(item.linkstatus)  @click="dialog(index, item)" >
              <div  slot="content"  class="tooltip-content">
                <h3>{{ ports[index].webname }}</h3>
                <el-form  label-position="left"  size="mini">
                  <el-form-item  :label="$t('systemStatus.middle.connectStatus')"  :label-width="tooltipLabelWidth"  style="margin: 0;">{{ ports[index].linkstatus }}
                  </el-form-item>
                  <el-form-item   label="LAN IP:" :label-width="tooltipLabelWidth"  style="margin: 0;">{{ ports[index].ip }}
                  </el-form-item>
                  <el-form-item  :label="$t('systemStatus.middle.netmask')" :label-width="tooltipLabelWidth">{{ ports[index].netmask }}
                  </el-form-item>
                </el-form>
                <el-form  label-position="left"  size="small">
                  <el-form-item  :label="$t('systemStatus.middle.device')"  :label-width="tooltipLabelWidth"  style="margin: 0;">{{ ports[index].devname }}
                  </el-form-item>
                  <el-form-item  :label="$t('systemStatus.middle.mac')"  :label-width="tooltipLabelWidth"  style="margin: 0;">{{ ports[index].mac }}
                  </el-form-item>
                  <el-form-item  :label="$t('systemStatus.middle.rate')"  :label-width="tooltipLabelWidth"  style="margin: 0;">{{ ports[index].speed }}
                  </el-form-item>
                </el-form>
              </div>
            </el-tooltip>
            <div class="text-area">
              <span v-show="item.function !== 'normal'">
                <svg class="icon">
                  <use :xlink:href="selectIcon(item.function)"/>
                </svg>
              </span>
              <span>{{ ports[index].webname }}</span>
              <span v-show="item.type !== 'normal'">
                <svg class="icon">
                  <use :xlink:href="selectIcon2(item.type)"/>
                </svg>
              </span>
            </div>
          </div>
        </div>
        <div  v-if='false' class="mask">
          <el-button  class="button"  type="primary"  @click="sortingHandle">{{ $t('NAT.adaptive.maskTip') }}
          </el-button>
        </div>
      </div>
    </div>

    <!-- config dialog -->
    <el-dialog  :title="$t('INEXNetwork.cardConfig.cardConfig')"  :visible.sync="isSettingNetcardVisible"  width="30%" :close-on-click-modal="false">

      <!-- inner dialog LAN -->
      <el-dialog  width="30%"  height="100%"  :title="$t('INEXNetwork.cardConfig.innerLANTitle')"  :visible.sync="isLANInnerVisible"
        :close-on-click-modal="false"  append-to-body :before-close='LANDialogClose'>
        <el-form  :model="lanForm"  label-position="left"  size="small"  ref="lanForm"  :rules="rules">
          <el-form-item  prop="ip"       :label="$t('INEXNetwork.cardConfig.IPAddress')"  :label-width="formLabelWidth">
            <el-input  v-model="lanForm.ip"       auto-complete="off"/>
          </el-form-item>
          <el-form-item  prop="netmask"  :label="$t('INEXNetwork.cardConfig.mask')"       :label-width="formLabelWidth">
            <el-input  v-model="lanForm.netmask"  auto-complete="off"  placeholder="255.255.255.0"/>
          </el-form-item>
          <el-form-item  prop="gateway"  :label="$t('INEXNetwork.cardConfig.gateway')"    :label-width="formLabelWidth">
            <el-input  v-model="lanForm.gateway"  auto-complete="off"  placeholder=""/>
          </el-form-item>
        </el-form>
        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="LANFormCancel">{{ $t('operation.cancel') }}
          </el-button>
          <el-button  type="danger"  @click="LANFormPrevious">{{ $t('operation.previous') }}
          </el-button>
          <el-button  type="primary" @click="LANFormSubmit">{{ $t('operation.determine') }}
          </el-button>
        </div>
      </el-dialog>

      <!-- inner dialog WAN -->
      <el-dialog  width="30%"  height="100%" :title="$t('INEXNetwork.cardConfig.innerWANTitle')"  :visible.sync="isWANInnerVisible"
        :close-on-click-modal="false"  append-to-body  :show-close="isWANCloseButtonVisible" :before-close='WANDialogClose'>
        <el-form  :rules="rules" :model="wanForm"  label-position="left"  size="small"  ref="wanForm">
          <el-form-item  :label="$t('INEXNetwork.cardConfig.linkType')"  prop="accessMode"  :label-width="formLabelWidth">
            <el-select  v-model="wanForm.accessMode"  :placeholder="$t('placeholder.select')"  style="width: 100%;"  @change="WANOnSelect"  :disabled="isADSLDialing||isDHCPConnecting">
              <el-option  v-for="(item, index) in options"
                :key="item.value"  :label="item.label"  :value="item.value"
                :disabled="(currentPort.type !== 'normal') && (currentPort.type !== options[index].message)"/>
            </el-select>
          </el-form-item>

          <!--  ADSL 连接所用的 form, 嵌入在 inner dialog WAN 中  -->
          <el-form :model="formDialUp" ref="formDialUp" v-if="isADSLView">
            <el-form-item :label="$t('INEXNetwork.cardConfig.account')" prop="account" :label-width="formLabelWidth">
              <el-input v-model="formDialUp.account" auto-complete="off" maxlength="19" :disabled="isGottonStatus||isADSLDialing"/>
            </el-form-item>
            <el-form-item :label="$t('INEXNetwork.cardConfig.passwd')"  prop="passwd"  :label-width="formLabelWidth">
              <el-input v-model="formDialUp.passwd"  auto-complete="off" maxlength="19" :disabled="isGottonStatus||isADSLDialing" type="password"/>
            </el-form-item>
            <el-form-item :label="$t('INEXNetwork.cardConfig.status')"  prop="status"
               v-model="formDialUp.status" :label-width="formLabelWidth"  v-loading="isGottonStatus">{{ formDialUp.status }}
              <el-button  type="primary"  @click="WANAdslLink"   style="margin-left: 20px" v-loading="isADSLDialing"
                :disabled="isInCancel||isADSLDialing" v-if="isADSLConnButtonVisible">{{ $t('operation.connect') }}  <!-- :disabled="isInCancel||checktime" -->
              </el-button>
              <el-button  type="danger"   @click="WANAdslCancel" style="margin-left: 20px" v-loading="isInCancel"
                v-if="!isADSLConnButtonVisible && !isADSLConnected" :disabled="checktime||isDead||isInCancel">{{ $t('operation.dialcancel') }}
              </el-button>
              <el-button  type="danger"   @click="WANAdslUnlink" style="margin-left: 20px" v-if="isADSLConnected" :disabled="isUnlinking">{{ $t('operation.disconnect') }}
              </el-button>
            </el-form-item>
          </el-form>

          <!-- DHCP 所用的 form, 嵌入在 inner dialog WAN 中 -->
          <el-form v-if="isDHCPView">
            <el-form-item  :label="$t('INEXNetwork.cardConfig.status')"  :label-width="formLabelWidth">{{ DHCPStatus }}           
            </el-form-item>
          </el-form>
          
          <!-- ADSL、DHCP、STATIC 共用的 form, 嵌入在 inner dialog WAN 中 -->
          <el-form-item   :label="$t('INEXNetwork.cardConfig.IPAddress')"  prop="ip"  :label-width="formLabelWidth">
            <el-input  v-model="wanForm.ip"  :disabled="!isStaticView||isDHCPView||isADSLView"  auto-complete="off"/>
          </el-form-item>
          <el-form-item   :label="$t('INEXNetwork.cardConfig.mask')"       prop="netmask"  :label-width="formLabelWidth">
            <el-input  v-model="wanForm.netmask"  :disabled="!isStaticView||isDHCPView||isADSLView"  auto-complete="off"/>
          </el-form-item>
          <el-form-item   :label="$t('INEXNetwork.cardConfig.gateway')"    prop="gateway"  :label-width="formLabelWidth">
            <el-input  v-model="wanForm.gateway"  :disabled="!isStaticView||isDHCPView||isADSLView"  auto-complete="off"/>
          </el-form-item>
        </el-form>

        <div  slot="footer"  class="dialog-footer">
          <el-button  type="danger"  @click="WANFormCancel"  :disabled="isCancelButtonDisable">{{ $t('operation.cancel') }}
          </el-button>
          <el-button  type="danger"  @click="WANFormPrevious"  v-show="isPrevButtonVisible">{{ $t('operation.previous') }}
          </el-button>
          <el-button  type="primary" @click="WANFormSubmit"  :disabled="isSubmitButtonDisable">{{ $t('operation.determine') }}
          </el-button>
        </div>
      </el-dialog>

      <!-- outer dialog -->
      <el-form  :model="optionForm"  label-position="left"  size="small"  ref="optionForm">
        <el-form-item  :label="$t('INEXNetwork.cardConfig.chooseFunc')"  prop="id"  :label-width="formLabelWidth">
          <el-select  v-model="optionForm.id"  :placeholder="$t('placeholder.selectNet')"  value="">
            <el-option  :label="$t('INEXNetwork.cardConfig.LAN')"  value="lan"  :disabled="isWANPort"/>
            <el-option  :label="$t('INEXNetwork.cardConfig.WAN')"  value="wan"  :disabled="isLANPort"/>
          </el-select>
        </el-form-item>
      </el-form>
      <div  slot="footer"  class="dialog-footer">
        <el-button  type="danger"  @click="unbindGeneral"  :disabled="isUnbindButtonDisable">{{ $t('operation.unbind') }}
        </el-button>
        <el-button  type="primary" @click="nextStep">{{ $t('operation.next') }}
        </el-button>
      </div>
      
    </el-dialog>
  </section>
</template>

<script>
import validate from '@/utils/rules.js'
import { getPorts, dialUp, DHCP, staticIP, sendWANLAN } from '@/api/api'
import { constants } from 'crypto';

let stopSignal1, stopSignal2, stopSignal

export default {
  name: 'INEXNetwork',
  data () {
    return {
      isSettingNetcardVisible: false, // 是否显示配置网卡界面（点击 port 图标触发）
      isLANInnerVisible: false,       // 是否显示 LAN 的子级 dialog
      isWANInnerVisible: false,       // 是否显示 WAN 的子级 dialog 
      isPrevButtonVisible: true,      // 是否显示 "上一步" 按钮  
      isADSLConnButtonVisible: true,  // 是否显示 ADSL 的 "连接" 按钮
      isWANCloseButtonVisible: true,  // 是否显示 ADSL 的 "关闭" 按钮
      isCancelButtonDisable: false,   // 是否禁能 "取消"   按钮
      isSubmitButtonDisable: false,   // 是否禁能 "确定"   按钮
      isUnbindButtonDisable: false,   // 是否禁能 "解绑"   按钮
    
      isADSLView: false,              // 是否是 ADSL 拨号界面
      isDHCPView: false,              // 是否是 DHCP 界面
      isStaticView: false,            // 是否是 Static IP 界面
            
      isADSLDialing: false,           // 是否显示 ADSL 连接时的加载动画
      isInCancel: false,              // 是否取消 ADSL 请求中
      isUnlinking: false,             // 是否正在断开 ADSL 过程中
      isGottonStatus: false,          // 是否获取到 ADSL 拨号状态
      isADSLConnected: false,         // 是否 ADSL 连接完成
      isDHCPConnecting: false,        // 是否 DHCP 正在连接
      isDHCPConnected: false,         // 是否 DHCP 已经连接
      
      DHCPStatus: '未连接',      
      isWANPort: false,               // 是否是WAN口
      isLANPort: false,               // 是否是LAN口
      currentPort: {},                // 当前选中的端口

      options: [
        {
          message: 'static',
          value: '1',
          label: '静态IP(固定IP)'
        },
        {
          message: 'dhcp',
          value: '2',
          label: 'DHCP(动态获取)'
        },
        {
          message: 'adsl',
          value: '3',
          label: 'ADSL/PPPoE拨号'
        }
      ],
      modeOptions: [
        {
          value: '1',
          label: '自动协商(默认)'
        },
        {
          value: '2',
          label: '全双工'
        },
        {
          value: '3',
          label: '半双工'
        }
      ],
      rateOptions: [
        {
          value: '1',
          label: '自动协商(默认)'
        },
        {
          value: '2',
          label: '10M'
        },
        {
          value: '3',
          label: '100M'
        },
        {
          value: '4',
          label: '1000M'
        }
      ],

      ports: [                                                          // 记录所有端口信息
        {
          ename: '',
          devname: '',
          linkstatus: '',
          function: '',                                                 // 端口的功能类型 LAN / WAN
          type:'',                                                      // 端口的 WAN 工作模式
          ip: '',
          netmask: '',
          mac: '',
          speed: ''
        }
      ],

      lanForm: {
        ip: '',
        gateway: '',
        netmask: ''
      },
      advancedLANForm: {
        mode: '',
        rate: '',
        control: ''
      },

      wanForm: {
        accessMode: '',
        ip: '',
        netmask: '',
        gateway: '',
        primaryDNS: '',
        secondaryDNS: ''
      },
      advancedWANForm: {
        mode: '',
        rate: ''
      },

      formDialUp: {
        webindex: '',
        account: '',
        passwd: '',
        status: '',
        handle: 0 // 后台用,勿删
      },
      optionForm: {
        id: ''
      },
     
      formLabelWidth: '8rem',
      tooltipLabelWidth: '5rem',
      powerButton: 'static/port3.png',

      checktime: false,          
      isDead: false,

      rules: {
        ip: [
          { required: true, message: '请输入IP！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', '请输入IP！/Please input IP'), trigger: 'blur'
          }
        ],
        netmask: [
          { required: true, message: '请输入子网掩码！/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确子网掩码！/Please input correct netmask', '请输入子网掩码！/Please input netmask'), trigger: 'blur'
          }
        ],
        gateway: [
          { required: true, message: '请输入网关！/Please input gateway' },
          {
            validator: validate('ip', '请输入正确网关！/Please input correct gateway', '请输入网关！/Please input gateway'), trigger: 'blur'
          }
        ]
      }
    }
  },

  watch: {
    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language);
    },

    /* 
     *  Outer Dialog 是否可见
     */
    isSettingNetcardVisible: {
      handler: function () {
        if (this.isSettingNetcardVisible === false) {
          clearInterval(stopSignal1);
          clearInterval(stopSignal2);

          this.isADSLView = false;
          this.isADSLDialing = false;
          this.isInCancel = false;
          this.isPrevButtonVisible = true;
          this.isCancelButtonDisable = false;
          this.isSubmitButtonDisable = false;
        }
      }
    },

    /* 
     *  WAN Inner Dialog 是否可见
     */
    isWANInnerVisible: {
      handler: function () {      
      }
    }
  },

  methods: {
    /*  
     * 选择链接状态对应图标
     */
    selectUrl (para) {
      if (para === 'off') {
        return 'static/port2.png';           // 未连接图标
      } else {
        return 'static/port3.png';           // 已连接图标
      }
    },

    /* 
     * 选择 router 页面里的 LAN、WAN 标志
     */
    selectIcon (para) {
      if (para === 'wan') {
        return '#icon-wan';                  // WAN 口图标
      } else if (para === 'lan') {
        return '#icon-pc';                   // LAN 口图标
      } else {
        return false;
      }
    },

    /* 
     * 选择 router 页面里的 功能 标志
     */
    selectIcon2 (para) {
      if (para === 'adsl') {
        return '#icon-phone';                // ADSL 拨号接入
      } else if (para === 'dhcp') {
        return '#icon-dynamic';              // 动态 DHCP 接入
      } else if (para === 'static') {
        return '#icon-ip';                   // 静态 IP 接入
      }
    },

    /* 
     *  @func  : 弹出外部 dialog
     *  @param : para  端口序号
     *           obj   选定的端口
     */
    dialog: function (para, obj) {
      this.currentPort = obj;                                           // 获得当前选中的端口

      console.warn("dialog");

      this.formDialUp.webindex = para;                                  // 获得需要处理的端口的编号
      this.formDialUp.enname = obj.enname;                              // 获得对应端口的 enname，DHCP 要用

      this.currentPort.function = obj.function;                         // 得到当前端口的 WAN / LAN
      this.currentPort.type = obj.type;                                 // 得到当前端口的 WAN 模式

      console.warn("dialog obj.function:" + obj.function);
      console.warn("dialog obj.type:" + obj.type);

      if (this.currentPort.type === '') {
        this.currentPort.type = 'normal';
      }

      this.isLANPort = false;                
      this.isWANPort = false;
      this.isUnbindButtonDisable = true;
      this.isADSLConnButtonVisible = true;
      this.isADSLConnected = false;

      if (this.$store.state.app.language === 'zh') {
        this.formDialUp.status = '未连接';
        this.DHCPStatus = '未连接';
      } else {
        this.formDialUp.status = 'Not Connected';
        this.DHCPStatus = 'Not Connected';
      }

      this.isStaticView = false;
      this.isDHCPView = false;      
      this.isADSLView = false;

      this.wanForm.accessMode = undefined;

      if (this.currentPort.function === 'lan') {                        // 获取该端口是 LAN 还是 WAN
        this.isLANPort = true;
        this.isUnbindButtonDisable = false;
        this.optionForm.id = 'lan';
      } else if (this.currentPort.function === 'wan') {
        this.isWANPort = true;
        this.isUnbindButtonDisable = false;
        this.optionForm.id = 'wan';
        if (this.currentPort.type === 'normal') {                       // WAN 口 normal 状态即为 STATIC
          this.currentPort.type = 'static';
        }
      } else {
        this.optionForm.id = undefined;
      }

      if (this.currentPort.type === 'static') {
        this.wanForm.accessMode = '1';

        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );

      } else if (this.currentPort.type === 'dhcp') {                    // WAN 口 DHCP 时处理
        this.isWANPort = true;
        this.isUnbindButtonDisable = false;

        this.isDHCPView = true;
        this.isStaticView = false;
        this.isADSLView = false;

        this.wanForm.accessMode = '2';

        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );

        if (this.$store.state.app.language === 'zh') {

          console.log("dhcp set connected");

          this.DHCPStatus = '已连接';
        } else {
          this.DHCPStatus = 'Connected';
        }

      } else if (this.currentPort.type === 'adsl') {                    // WAN 口 ADSL 时处理
        this.isWANPort = true;
        this.isUnbindButtonDisable = false;
        this.isADSLConnButtonVisible = false;

        this.isDHCPView = false;
        this.isStaticView = false;
        this.isADSLView = true;

        this.wanForm.accessMode = '3';
        this.currentPort.function = 'wan';
          
        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );

        this.isADSLConnected = true;

        if (this.$store.state.app.language === 'zh') {
          this.formDialUp.status = '已连接';
        } else {
          this.formDialUp.status = 'Connected';
        }
      }

      console.warn("dialog this.wanForm.accessMode:" + this.wanForm.accessMode);
      console.warn("dialog this.currentPort.function:" + this.currentPort.function);
      console.warn("dialog this.currentPort.type:" + this.currentPort.type);

      this.isSettingNetcardVisible = true;                              // 显示网络配置主 dialog
    },
    
    /* 
     *  @func  : 点击外部 dialog "下一步" 触发的函数
     *  @param : none
     */
    nextStep: function () {

      console.warn("nextStep " + this.wanForm.accessMode);

      if (this.optionForm.id === 'lan') {                               // 如果是 LAN 口        
        this.isLANInnerVisible = true;                                  // 显示 LAN 口配置 dialog
        this.lanForm = Object.assign({}, this.ports[this.formDialUp.webindex]);
      } else if (this.optionForm.id === 'wan') {                        // 如果是 WAN 口
        this.isWANInnerVisible = true;                                  // 显示 WAN 口配置 dialog        
      }

      if (this.currentPort.type === 'static' &&                         // 如果是 STATIC IP，在这个阶段不显示 IP
          this.wanForm.accessMode != 1) {                               // 并且当前未选择 静态 IP
        this.isStaticView = false; 
      }

      if (this.$store.state.app.language === 'zh') {
        this.options = [                                                // WAN 口接入方式选项
          { 
            message: 'static',
            value: '1',
            label: '静态IP (固定IP)'
          }, {
            message: 'dhcp',
            value: '2',
            label: 'DHCP (动态获取)'
          }, {
            message: 'adsl',
            value: '3',
            label: 'ADSL/PPPoE 拨号'
          }
        ];
      } else {
        this.options = [                                                // WAN 口接入方式选项
          {
            message: 'static',
            value: '1',
            label: 'StaticIP'
          }, {
            message: 'dhcp',
            value: '2',
            label: 'DHCP'
          }, {
            message: 'adsl',
            value: '3',
            label: 'ADSL/PPPoE'
          }
        ];
      }
    },

    /* 
     *  @ func:  设置为 WAN / LAN 口 函数
     *  @ param: func  WAN / LAN 口
     */
    setPort (func) {

      console.warn("setPort func:" + func);

      let para;
      this.wanForm.handle = 1;
      if (func === 'wan') {                                             // 如果设置为 WAN 口
        para = Object.assign({}, this.wanForm);
      } else if (func === 'lan') {                                      // 如果设置为 LAN 口
        para = Object.assign({}, this.lanForm);
      }

      console.warn("setPort para.webname:" + para.webname);

      para.webnetif = String(para.webname);
      para.use = func;
      para.handle = 1;

      console.warn("setPort para.webname:" + para.webname);

      console.warn(para);

      sendWANLAN(para)                                                  // 实际设置 WAN 口 / LAN 口操作
        .then(res => {

          console.log("res.data.code " + res.data.code);

          if (res.data.code === 200) {                                  // 如果设置成功
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '设置成功',
                type: 'success'
              });
            } else {
              this.$message({
                message: 'Config Success',
                type: 'success'
              });
            }

            console.warn("setPort this.formDialUp.webindex:" + this.formDialUp.webindex);
            this.ports[this.formDialUp.webindex].function = func;       // 记录为 WAN / LAN 口

          } else {                                                      // 如果设置失败
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '设置失败',
                type: 'warning'
              })
            } else {
              this.$message({
                message: 'Config Failed',
                type: 'warning'
              })
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    /* 
     *  @func:  获取端口最新状态
     *  @parma: none
     */
    getPortsInfo: function () {
      getPorts()
        .then(res => {       
          function sortNumber (a, b) {                                   // 根据webindex排序，确保端口顺序正确
            return a.webindex - b.webindex
          }
          res.data.interfaces.sort(sortNumber)

          this.ports = res.data.interfaces
          this.$emit('getPortsInfo', this.ports)
        })
        .catch(error => {
          console.log(error)
        })
    },

    /* 
     * WAN / LAN 口 解绑函数
     */
    unbindGeneral: function (val) {
      let para = Object.assign({}, this.formDialUp);
      let enname = this.formDialUp.enname;
      this.ports.forEach(element => {
        if (element.enname === enname) {
          para.use = element.function
          para.index = element.webindex
          para.webnetif = String(element.webname)
        }
      });
      para.handle = 0;

      sendWANLAN(para)                                                  // 实际解绑 WAN 口 / LAN 口操作
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.$message({
                message: '发送成功',
                type: 'success'
              });
            } else {
              this.$message({
                message: 'SendSuccess',
                type: 'success'
              })
            }
          }
        })
        .then(() => {
          console.log(this.ports[para.index])
          if (this.ports[para.index].type === 'dhcp') {                 // 如果原来是 DHCP 
            console.log('dhcpunbind');
            console.log(this.ports[para.index]);
            this.WANDhcpDisconnect();                                   // 断开 DHCP 
          } else if (this.ports[para.index].type === 'adsl') {          // 如果原来是 ADSL
            console.log('adslunbind');
            console.log(this.ports[para.index]);
            this.WANAdslUnlink();                                       // 断开 ADSL
          }
          this.isSettingNetcardVisible = false
        })
        .catch(error => {
          console.log(error)
        })
    },

    /* 
     *  
     *  @func  : WAN 口根据接入方式，触发不同的配置界面
     *  @param : none
     */
    WANOnSelect: function () {

      console.warn("WANOnSelect " + this.wanForm.accessMode);
      
      if (this.wanForm.accessMode === '3') {                            // 如果是 ADSL 拨号
        this.isADSLView = true;                                         // 显示 ADSL 界面
        this.isDHCPView = false;
        this.isStaticView = false;
        
        this.WANFormInfoReset();
        
        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );
 
        let para = Object.assign({}, this.formDialUp);
        para.handle = 2;                                                // handle 等于 2 时 代表获取状态
        
        this.isGottonStatus = true;                                     // 显示获取拨号状态界面

        dialUp(para)                                                    // 进行拨号状态获取操作
          .then(res => {
            if (res.data.code === 200) {                                // 如果状态获取成功
              this.translate(res.data.status);                          // 解析状态值
              this.formDialUp.account = res.data.account;
              this.formDialUp.passwd  = res.data.passwd;
              this.isGottonStatus = false;                              // 关闭获取拨号状态动画
            } else {                                                    // 如果状态获取失败
              if (this.$store.state.app.language === 'zh') {
                this.formDialUp.status = '获取失败！';
              } else {
                this.formDialUp.status = 'GetFailed！';
              }
              this.isGottonStatus = false;                              // 关闭获取拨号状态动画
            }
          })
          .catch(error => {
            console.log(error);
          })
      }

      if (this.wanForm.accessMode === '2') {                            // 如果是 DHCP 获取
        this.isDHCPView = true;
        this.isADSLView = false;
        this.isStaticView = false;

        this.WANFormInfoReset();

        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );
      }

      if (this.wanForm.accessMode === '1') {                            // 如果是 静态 IP 设置
        this.isStaticView = true;                                       // 显示 Static 界面
        this.isADSLView = false;
        this.isDHCPView = false;
        
        this.wanForm = Object.assign(
          this.wanForm,
          this.ports[this.formDialUp.webindex]
        );

        console.warn("WANOnSelect this.ports.ip:" + this.ports[this.formDialUp.webindex].ip);
      }
    },

    sortingHandle: function () {
      this.$router.push('nat')
    },

    /* 
     *  
     */
    WANFormInfoReset: function () {

      console.warn("WANFormInfoReset");

      this.$refs['wanForm'].fields.forEach(function (e) {
        if (e.prop == 'ip' || e.prop == 'netmask' || e.prop == 'gateway') {
            e.resetField();
        }
      });
    },

    /* 
     *  @func:  WAN 口拨号状态解析
     *  @param: val 状态值
     */
    translate: function (val) {
      this.isDead = false;

      if (this.$store.state.app.language === 'zh') {
        if (val === 'run') {

          clearInterval(stopSignal2);
          this.isInCancel = false;

          console.log("dial success show");

          this.formDialUp.status = '拨号成功';
          this.isADSLDialing = false;                                   // 关闭拨号动画
          this.isADSLConnected = true;                                  // 拨号成功标志
          this.currentPort.type = 'adsl';                               // 标记为 ADSL 拨号
          this.isCancelButtonDisable = false;                           // 使能 "取消" 按钮
          this.isSubmitButtonDisable = false;                           // 使能 "确定" 按钮
          this.isWANCloseButtonVisible = true;                          // 显示 "关闭" 按钮
          this.isADSLConnButtonVisible = false;                         // 不显示 "连接" 按钮
          
          this.checktime = false; 

          clearInterval(stopSignal1);

        } else if (val === 'init') {

          if (!this.isInCancel) {
            this.formDialUp.status = '拨号初始化...';
          }
        } else if (val === 'auth') {
          if (!this.isInCancel) {
            this.formDialUp.status = '用户验证...';
          }
        } else if (val === 'disconnect') {

          console.log("translate disconnect");

          this.formDialUp.status = '连接断开';
          this.isADSLConnected = false;                                 // 拨号失败标志
        } else if (val === 'dead') {

          if (this.isADSLDialing) {
            console.log("translate failed");

            this.formDialUp.status = '连接失败';
            this.isADSLConnButtonVisible = true;

          } else {

            console.log("translate unconnected");
            this.formDialUp.status = '未连接';
          }

          this.isDead = true;
          this.isADSLDialing = false;
          
          this.isADSLConnected = false;                                 // 拨号失败标志
          
          this.isPrevButtonVisible = true;                              // 显示 "上一步" 按钮
          this.isCancelButtonDisable = false;                           // 使能 "取消" 按钮
          this.isSubmitButtonDisable = false;                           // 使能 "确定" 按钮
          this.isWANCloseButtonVisible = true;                          // 显示 "关闭" 按钮

          clearInterval(stopSignal1)
        }
      } else {
        if (val === 'run') {

          clearInterval(stopSignal2);
          this.isInCancel = false;

          this.formDialUp.status = 'Dial Success';
          this.isADSLDialing = false;
          this.isADSLConnected = true;                                  // 拨号成功标志
          this.currentPort.type = 'adsl';                               // 标记为 ADSL 拨号
          this.isCancelButtonDisable = false;                           // 使能 "取消" 按钮
          this.isSubmitButtonDisable = false;                           // 使能 "确定" 按钮
          this.isWANCloseButtonVisible = true;                          // 显示 "关闭" 按钮
          this.isADSLConnButtonVisible = false;                         // 不显示 "连接" 按钮

          this.checktime = false; 

          clearInterval(stopSignal1);
        } else if (val === 'init') {
          if (!this.isInCancel) {
            this.formDialUp.status = 'Initiating...';
          }
        } else if (val === 'auth') {
          if (!this.isInCancel) {
            this.formDialUp.status = 'Verificating...';
          }
        } else if (val === 'disconnect') {

          console.log("translate disconnect");

          this.formDialUp.status = 'Disconnect';
          this.isADSLConnected = false;                                 // 拨号失败标志

        } else if (val === 'dead') {

          if (this.isADSLDialing) {
            this.formDialUp.status = 'Connect Failed';
            this.isADSLConnButtonVisible = true;

          } else {
            this.formDialUp.status = 'Not Connected';
          }

          this.isADSLDialing = false;
          this.isADSLConnected = false;                                 // 拨号失败标志

          this.isPrevButtonVisible = true;                              // 显示 "上一步" 按钮
          this.isCancelButtonDisable = false;                           // 使能 "取消" 按钮
          this.isSubmitButtonDisable = false;                           // 使能 "确定" 按钮
          this.isWANCloseButtonVisible = true;                          // 显示 "关闭" 按钮

          clearInterval(stopSignal1);
        }
      }
    },

    /* 
     *  @func:  WAN 口 ADSL 拨号连接函数
     *  @param: none
     */
    WANAdslLink: function () {
      this.checktime = true
      
      if (this.formDialUp.account.length > 0 && this.formDialUp.account.length < 20 && 
          this.formDialUp.passwd.length  > 0 && this.formDialUp.passwd.length  < 20) {   // 尝试拨号
        
        console.log(this.formDialUp.account.length);
        console.log(this.formDialUp.passwd.length);

        this.isPrevButtonVisible = false;                               // 不显示 "上一步" 按钮
        this.isCancelButtonDisable = true;                              // 禁能 "取消" 按钮
        this.isSubmitButtonDisable = true;                              // 禁能 "确定" 按钮
        this.isADSLConnButtonVisible = false;                           // 不显示 "连接"   按钮
        this.isWANCloseButtonVisible = false;                           // 不显示 "关闭" 按钮

        this.isADSLDialing = true;                                      // 显示 "正在拨号" 动画

        this.formDialUp.handle = 1;                                     // handle = 1 的意义是拨号连接

        let para = Object.assign({}, this.formDialUp);

        this.checktime = true;        
        setTimeout(() => {
          this.checktime = false;          
        }, 8000)

        dialUp(para)                                                    // 进行拨号操作
          .then(res => {
            if (res.data.code === 200) {                                // 拨号指令调用成功 
              this.translate(res.data.status);              

              this.ports[this.formDialUp.webindex].function = 'wan';    // 只修改图标

            } else {
              if (this.$store.state.app.language === 'zh') {            // 拨号指令调用失败
                this.formDialUp.status = '拨号失败!'
              } else {
                this.formDialUp.status = 'Dial Failed!'
              }
            }
          })
          .catch(error => {
            console.log(error)
          })

        stopSignal1 = setInterval(() => {                               // 周期性的获取状态
          para.handle = 2;                                              // handle = 2 的意义是获取状态
          dialUp(para)
            .then(res => {
              if (res.data.code === 200) {                              // 如果获取状态成功
                this.translate(res.data.status)
              } else  if (res.data.code === 300) {                      // 获取状态失败
                if (this.$store.state.app.language === 'zh') {
                  this.formDialUp.status = '拨号次数已达上限';
                } else {
                  this.formDialUp.status = 'Dials reach the upper limit';
                }
                this.isADSLDialing = false;                             // 关闭 “正在拨号” 动画
                clearInterval(stopSignal1)
              }
            })
            .catch(error => {
              console.log(error)
            })
        }, 1000);                                                       // 周期间隔 1000ms

        this.isWANInnerVisible = true;                                  // 保持 WAN 窗口显示
        this.isSettingNetcardVisible = true;                            // 保持 网络配置 窗口显示        
        this.getPortsInfo();                                            // 获取端口信息

      } else {                                                          // 输入的账号或密码长度有误
        if (this.$store.state.app.language === 'zh') {
          this.$message.error('账号或密码无效，请重试')
        } else {
          this.$message.error('The account or password is invalid. Please try again')
        }
      }
    },

    /* 
     *  @func: WAN 口 ADSL 取消拨号函数
     *  @func：none
     */
    WANAdslCancel () {
      clearInterval(stopSignal2);

      let para = Object.assign({}, this.formDialUp)
      console.log(this.formDialUp)

      console.log("WANAdslCancel");

      this.isInCancel = true;

      if (this.$store.state.app.language === 'zh') {
        this.formDialUp.status = '正在取消...';
      } else {
        this.formDialUp.status = 'Cancel Dialing...';
     }

      para.handle = 0
      stopSignal2 = setInterval(() => {
        dialUp(para)
          .then(res => {
            if (res.data.code === 200) {

              console.log("set this.isInCancel false");

              this.isInCancel = false;
              this.isADSLConnButtonVisible = true;
              clearInterval(stopSignal2);
            }
            if (res.data.code === 500) {

              console.log("set this.isInCancel false");

              this.isInCancel = false;

              if (this.$store.state.app.language === 'zh') {
                this.$message({
                  message: '取消失败!',
                  type: 'error'
                })
              } else {
                this.$message({
                  message: 'Cancel Failed!',
                  type: 'error'
                })
              }

              if (this.$store.state.app.language === 'zh') {
                this.formDialUp.status = '取消失败!';
              } else {
                this.formDialUp.status = 'Cancel Failed!';
              }

              clearInterval(stopSignal2);

              this.isADSLConnButtonVisible = true;
            }
          })
          .catch(error => {
            console.log(error)
          })
      }, 2000)
    },

   
    /* 
     *  @func:  点击 WAN 口 ADSL 断开连接 按钮
     *  @parma: none
     */
    WANAdslUnlink: function () {
      clearInterval(stopSignal1);
      
      this.isPrevButtonVisible = false;                                 // 不显示 "上一步" 按钮
      this.isWANCloseButtonVisible = false;                             // 不显示 "关闭" 按钮
      this.isCancelButtonDisable = true;                                // 禁能 "取消" 按钮
      this.isSubmitButtonDisable = true;                                // 禁能 "确定" 按钮
      this.isUnlinking = true;

      if (this.$store.state.app.language === 'zh') {
        this.formDialUp.status = '正在取消拨号...';
      } else {
        this.formDialUp.status = 'Cancel Dialing ...';
      }

      setTimeout(() => {
        this.isADSLConnected = false;          
        this.isADSLConnButtonVisible = true;
        this.isUnlinking = false;

        this.isPrevButtonVisible = true;                                // 显示 "上一步" 按钮
        this.isWANCloseButtonVisible = true;                            // 显示 "关闭" 按钮
        this.isCancelButtonDisable = false;                             // 使能 "取消" 按钮
        this.isSubmitButtonDisable = false;                             // 使能 "确定" 按钮
      }, 5000)

      let para = Object.assign({}, this.formDialUp);
      para.handle = 0;
      dialUp(para)
        .then(() => {
          if (this.$store.state.app.language === 'zh') {
            this.$message({
              message: '断开拨号成功',
              type: 'success'
            });
            this.formDialUp.status = '未连接';
          } else {
            this.$message({
              message: 'Unlink Success',
              type: 'success'
            })
            this.formDialUp.status = 'Not Connected';
          }

          let index = this.formDialUp.webindex;

          console.log("unbind set normal");
          this.ports[index].type = 'adsl';                              // change icon status
        })
        .catch(error => {
          console.log(error)
        })
      
      this.getPortsInfo();
    },

    /* 
     *  @func:  WAN 静态 IP 设置
     *  @parma: none
     */
    WANStaticSet: function () {

        console.warn("this.isStaticView");

        let para = {};        
        para.enname = this.formDialUp.enname;
        para.ipaddr = this.wanForm.ip;
        para.netmask = this.wanForm.netmask;
        para.gateway = this.wanForm.gateway;

        console.warn(para);

        staticIP(para)
          .then(res => {
            if (res.data.code === 200) {                                // 如果设置成功
              this.isSettingNetcardVisible = false;                     // 关闭 dialog
              this.isWANInnerVisible = false;             
              this.setPort('wan');                                      // 端口设置为 WAN 口

              console.warn("WANFormSubmit this.formDialUp.webindex:" + this.formDialUp.webindex);
              console.warn("WANFormSubmit set type:" + 'static');

            } else if (res.data.code === 300) {
              if (this.$store.state.app.language === 'en') {
                this.$message('This IP is in the same network segment as other network ports, configure failed');
              } else {
                this.$message('该 IP 与其他网口处于同一网段，设置失败');
              }
            }
          })
          .then(() => {
          })
          .catch(error => {
            console.log(error)
          })

          this.getPortsInfo();                                          // 更新界面显示
    },

    /* 
     *  @func:  WAN DHCP 连接
     *  @parma: none
     */
    WANDhcpLink : function () {
      let para = {};
      para.netifname = this.formDialUp.enname;
      para.handle = 'enable';

      DHCP(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.DHCPStatus = '已连接'
            } else {
              this.DHCPStatus = 'Connected'
            }

            this.setPort('wan');                                        // 端口设置为 WAN 口

            console.warn("dhcp success");
          }

          if (res.data.code === 500) {
            if (this.$store.state.app.language === 'zh') {
              this.DHCPStatus = '连接失败'
            } else {
              this.DHCPStatus = 'ConnectFailed'
            }

            console.warn("dhcp fail");
          }          
        })
        .catch(error => {
          console.log(error)
        })
       
        this.getPortsInfo();                                            // 更新 IP、子网掩码、网关信息
    },

    /* 
     *  @func:  WAN DHCP 断开连接
     *  @parma: none
     */
    WANDhcpDisconnect : function () {
      let para = {}
      para.netifname = this.formDialUp.enname
      para.handle = 'disable'

      DHCP(para)
        .then(res => {
          if (res.data.code === 200) {
            if (this.$store.state.app.language === 'zh') {
              this.DHCPStatus = '未连接'
            } else {
              this.DHCPStatus = 'NotConnect'
            }
          }
          if (res.data.code === 500) {
            if (this.$store.state.app.language === 'zh') {
              this.DHCPStatus = '断开连接失败'
            } else {
              this.DHCPStatus = 'DisconnectFailed'
            }
          }
        })
        .catch(error => {
          console.log(error)
        })
    },

    /* 
     *  @func:  LAN 信息设置
     *  @parma: none
     */
    LANFormSubmit () {
      this.$refs.lanForm.validate(valid => {
        if (valid) {
          this.isSettingNetcardVisible = false;
          this.isLANInnerVisible = false;
      
          this.setPort('lan');

          let para = {};
          para.enname = this.formDialUp.enname;
          para.ipaddr = this.lanForm.ip;
          para.netmask = this.lanForm.netmask;
          para.gateway = this.lanForm.gateway;

          staticIP(para)
            .then(res => {
              if (res.data.code === 200) {
                this.isSettingNetcardVisible = false
              } else if (res.data.code === 300) {
                if (this.$store.state.app.language === 'en') {
                  this.$message('This IP is in the same network segment as other network ports, configure failed')
                } else {
                  this.$message('该IP与其他网口处于同一网段，设置失败')
                }
              }
            }).then(() => {              
              this.setPort('lan')
            })
            .catch(error => {
              console.log(error)
            })

          this.getPortsInfo();
        }
      })
    },

    /* 
     *  @func:  LAN Dialog 取消
     *  @parma: none
     */
    LANFormCancel: function () {      
      this.isSettingNetcardVisible = false;
      this.isLANInnerVisible = false;
    },

    /* 
     *  @func:  点击 LAN 口 "关闭" 按钮
     *  @parma: none
     */
    LANDialogClose: function() {

      console.log("LANDialogClose");

      this.isSettingNetcardVisible = false;
      this.isLANInnerVisible = false;
    },

    /* 
     *  @func:  LAN Dialog 上一步
     *  @parma: none
     */
    LANFormPrevious: function () {      
      this.isLANInnerVisible = false;
    },

    /* 
     *  @func:  WAN 确定按钮
     *  @parma: none
     */
    WANFormSubmit: function () {

      if (this.isStaticView) {                                          // 如果是设置 static IP 界面
          this.WANStaticSet();

      } else if (this.isDHCPView) {                                     // 如果是设置 DHCP IP 界面
          this.WANDhcpLink();                                           // 进行 DHCP 连接

          console.log("WANFormSubmit isDHCPConnected");
          this.isSettingNetcardVisible = false;                         // 关闭 dialog
          this.isWANInnerVisible = false;

      } else if (this.isADSLView) {                                     // 如果是 ADSL 拨号
          this.isSettingNetcardVisible = false;                         // 关闭 dialog
          this.isWANInnerVisible = false;
      }
    },

    /* 
     *  @func:  点击 WAN 口 "上一步" 按钮
     *  @parma: none
     */
    WANFormPrevious: function () {      
      console.warn("WANFormPrevious");

      this.isWANInnerVisible = false;                                   // 不显示 WAN 口配置，直接退回 "配置网卡" 界面
    },

    /* 
     *  @func:  点击 WAN 口 "关闭" 按钮
     *  @parma: none
     */
    WANDialogClose: function() {

      console.log("WANDialogClose");

      this.isSettingNetcardVisible = false;
      this.isWANInnerVisible = false;
    },

    /* 
     *  @func:  点击 WAN 口 "取消" 按钮
     *  @parma: none
     */
    WANFormCancel: function () {
      
      console.warn("WANFormCancel");

      this.isSettingNetcardVisible = false;
      this.isWANInnerVisible = false;
    },

    pushDNS: function () {
      this.$store.commit('changeRoutePush', true)
      this.$router.push({ path: '../DNS' })
    }
  },

  mounted () {
    this.getPortsInfo()
    stopSignal = setInterval(this.getPortsInfo, 5000)
  },

  beforeDestroy () {
    clearInterval(stopSignal)
  }
}
</script>

<style scoped>
.wholeRouter {
  display: flex;
  position: relative;
  width: 100%;
}
.power {
  border: 1px solid lightgrey;
  border-radius: .6rem;
  text-align: center;
  margin: auto;
  background-color: dodgerblue;
}
.powerButton {
  width: 4rem;
  height: 3rem;
  margin: 1.5rem 1.2rem;
}
.router {
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: .6rem;
  margin: .6rem 1.2rem;
  background-color: lightgoldenrodyellow;
  display: flex;
  justify-content: space-around;
  flex-wrap: wrap;
}
.port {
  width: 3.6rem;
  height: 3rem;
  text-align: center;
  margin: 1.5rem auto;
  flex-grow: 1;
  border-radius: .4rem;
}
.port img:hover {
  cursor: pointer;
}
.advancedSetting {
  margin-bottom: 20px;
}
.tooltip-content {
  min-width: 150px;
  border: 0;
}
.tip {
  display: flex;
  width: 30%;
}
.tip1 {
  display: flex;
  width: 45%;
}
.icon-div {
  height: 20px;
  width: 20px;
  margin-right: 5px;
}
.text-area {
  font-weight: bold;
}
.text-area .text {
  margin: 0;
}
.icon {
  width: 1em;
  height: 1em;
  vertical-align: -0.15em;
  fill: currentColor;
  overflow: hidden;
}
.mask {
  position: absolute;
  top: 0;
  left: 0;
  background-color: rgba(70, 70, 70, 0.5);
  width: 100%;
  height: 100%;
}
.mask .button {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
}

h2 {
  font-family: Roboto-Thin, serif;
}
@font-face {
  font-family: Roboto-Thin;
  src: url("../../../../assets/font/Roboto-Thin.ttf");
}
</style>
