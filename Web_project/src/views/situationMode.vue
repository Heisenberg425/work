<template>
<div class="container">
  
      
   

<!--可选功能-->
  <div class="thisdialogstyle">           <!--本页面dialog的特定样式，在css内已注释该样式-->
    <el-dialog  width="40%"  :visible="showselectFunc" @close='closeDialog'  :title="$t('situationMode.selectFuncTitle')">
      <div class="defaultFunc" style="display: inline-block;margin-top: 0px;">
      <div style="margin-top: 20px;text-align:center">
        <img style="width: 250px; height:80px; border-radius: 5px;" :src="this.wanImage"  />
        <!--<el-checkbox-button v-model="func1"  @change = "changeClient">KidVPN客户端</el-checkbox-button>-->
      </div >
      <div style="margin-top: 20px;text-align:center;margin-bottom:20px">
        <img style="width: 250px; height: 80px; border-radius: 5px;" :src="this.lanImage"   />
        <!--<el-checkbox-button v-model="func2" @change = "changeSever">KidVPN服务器</el-checkbox-button>-->
      </div>
     
      <div style="margin-top: 10px;text-align:center">
        <img style="width: 250px; height:80px; border-radius: 5px;" :src="this.clientImage"  @click="changeClient()" />
        <!--<el-checkbox-button v-model="func1"  @change = "changeClient">KidVPN客户端</el-checkbox-button>-->
      </div >
      <div style="margin-top: 10px;text-align:center">
        <img style="width: 250px; height: 80px; border-radius: 5px;" :src="this.severImage"  @click="changeSever()" />
        <!--<el-checkbox-button v-model="func2" @change = "changeSever">KidVPN服务器</el-checkbox-button>-->
      </div>
      <div style="margin-top: 10px;text-align:center">
        <img style="width: 250px; height: 80px; border-radius: 5px;" :src="this.defendImage"  @click="changeDefend()" />
        <!--<el-checkbox-button v-model="func3" @change = "changeDefend">局域网欺骗防御</el-checkbox-button>-->
      </div>
      </div>
      <div slot="footer" class="dialog-footer">
        <el-button  size="medium" class = "button" @click="funcSubmit" round>{{$t('situationMode.funcSubmit')}}</el-button>
      </div>
    </el-dialog>
  </div>

<!--选择外网端口-->
  <div class="thisdialogstyle">
    <el-dialog  width="40%"  :visible="selectWanPort" @close='closeDialog'>
      <el-steps :active="active" finish-status="success" process-status= "success"  simple>
        <el-step :title= "item" v-for="item in step" class="el-step__title" :key="item" style="width: 80px; height: 30px; "></el-step>
      </el-steps>
      <div style="margin-top: 20px;">{{$t('situationMode.wanSelectTitle')}}</div>
        <div class="middleBottom">
          <div class="router">
            <div  v-for="(item, index1) in ports" class="port"  @click="pushToWanINEX(item)" :key="index1">                
              <div class="item" effect="light" >
                <img style="width: 50px; height: 50px; border-radius: 5px;" :src="item.imgsrc" />
              </div>
              <div class="text-area">
                <p class="text">{{ item.webname }}</p>
              </div>
            </div>
          </div>
        </div>
        <div slot="footer" class="dialog-footer">
          <el-button class = "button" size="medium" @click="backSelcetFunc"  round>{{$t('situationMode.back')}}</el-button>
          <el-button class = "button" size="medium" @click="selectWanPortFinish"  round>{{$t('situationMode.next')}}</el-button>
        </div>
    </el-dialog>
  </div>
<!--选择wan方式-->
  <div class="thisdialogstyle">
    <el-dialog  width="40%"  :visible="selectWanMethod" @close='closeDialog'>
      <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
        <el-step :title= "item" v-for="item in step" :key="item"></el-step>
      </el-steps>
      <div style="margin-top: 20px;">{{$t('situationMode.selectWanMethod')}}</div>
      <div  class="wanInfo" >
        <el-collapse class="test"  v-model="wanMethod" @change="changeLabel" accordion>
        <div>
          <div style="display:inline-block; vertical-align:text-top;margin-top: 15px;">
            <el-radio   v-model="wanMethod" label="1"  style=" margin-top: 20px;margin-left: 30px;">{{" "}}
            </el-radio>
          </div>
          <div style="display:inline-block; vertical-align:text-top;">
            <el-collapse-item class="green" :title="$t('situationMode.adsl')" name="1" style="width:100%;display: inline-block;margin-top: 19px;margin-left: 0px;">
              <el-form ref="form" :model="adslInfo" :rules="rules" label-position="left" label-width="8rem" style="margin-top: 10px;margin-left: 30px;">
                <el-form-item prop="adslAccount" :label="$t('situationMode.account')" style="text-align:left" >
                  <el-input v-model="adslInfo.adslAccount" :placeholder="$t('placeholder.account')" ></el-input>
                </el-form-item>
                <el-form-item prop="adslpasswd1" :label="$t('situationMode.pw')" style="text-align:left" >
                  <el-input type="password" v-model="adslInfo.adslpasswd1" :placeholder="$t('placeholder.password1')" ></el-input>
                </el-form-item>
                <el-form-item prop="adslpasswd2" :label="$t('situationMode.confirmPw')" style="text-align:left" >
                  <el-input type="password" v-model="adslInfo.adslpasswd2" :placeholder="$t('placeholder.password2')" ></el-input>
                </el-form-item>
              </el-form>
              </el-collapse-item>
          </div>
        </div>       
        <div>
          <div style="display:inline-block; vertical-align:text-top;margin-top: 15px;">
            <el-radio   v-model="wanMethod" label="2"  style=" margin-top: 0px;margin-left: 30px;">{{" "}}{{$t('situationMode.lanDHCP')}}
            </el-radio>
          </div>
        </div>
        <div>
          <div style="display:inline-block; vertical-align:text-top;margin-top: 15px;">
            <el-radio   v-model="wanMethod" label="3"  style=" margin-top: 20px;margin-left: 30px;">{{" "}}
            </el-radio>
          </div>
          <div style="display:inline-block; vertical-align:text-top;" >
            <div style="margin-top: 17px;margin-left: 0px;" class="green">
              <el-collapse-item :title="$t('situationMode.static')" name="3" style="width:100%;display: inline-block;">
                <el-form ref="form" :model="staticIpInfo" label-position="left" :rules="rules" label-width="7rem" style="margin-top: 10px;margin-left:30px;">
                  <el-form-item prop="staticIp" :label="$t('situationMode.staticIp')" style="text-align:left" >
                    <el-input v-model="staticIpInfo.staticIp" :placeholder="$t('placeholder.ip')" ></el-input>
                  </el-form-item>
                  <el-form-item prop="staticNetmask" :label="$t('situationMode.netmask')" style="text-align:left" >
                    <el-input v-model="staticIpInfo.staticNetmask" :placeholder="$t('placeholder.netmask')" ></el-input>
                  </el-form-item>
                  <el-form-item prop="staticGateway" :label="$t('situationMode.gateway')" style="text-align:left" >
                    <el-input v-model="staticIpInfo.staticGateway" :placeholder="$t('placeholder.gateway')" ></el-input>
                  </el-form-item>
                </el-form>
              </el-collapse-item>
              </div>
            </div>
          </div>
        </el-collapse>
      </div>
      <div slot="footer" class="dialog-footer">
        <el-button class = "button" size="medium" @click="backSelcetPort"  round>{{$t('situationMode.back')}}</el-button>
        <el-button class = "button" size="medium" @click="selectWanMethodFinish"  round>{{$t('situationMode.next')}}</el-button>
      </div>
    </el-dialog>
  </div>
<!--外网接入测试-->
  <div class="thisdialogstyle">
    <el-dialog  width="40%"  :visible="wanTest" @close='closeDialog'>
      <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
        <el-step :title= "item" v-for="item in step" :key="item"></el-step>
        </el-steps>
        <div style="margin-top: 20px;text-align: center;">{{$t('situationMode.wantest')}}</div>
        <div slot="footer" class="dialog-footer">
          <el-button class = "button" size="medium" v-if="showTestButton" @click="backSelectWanMethod"  round>{{$t('situationMode.back')}}</el-button>
          <!--el-button class = "button" size="medium" @click="wanTestFinish"  round>下一步</el-button-->
        </div>
    </el-dialog>
  </div>
      <!--外网接入完成-->
  <div class="thisdialogstyle">
    <el-dialog  width="40%" :visible="wanConnected" @close='closeDialog'>
      <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
        <el-step :title= "item" v-for="item in step" :key="item"></el-step>
      </el-steps>
      <div style="margin-bottom: 30px;text-align: center;margin-top: 20px" >
        <img src="static/netConnect.png"  height="100" width="100"  />
      </div>
      <div style="margin-top: 20px;text-align: center;">{{$t('situationMode.wanconnected')}}</div>
        <div slot="footer" class="dialog-footer">
          <!--el-button class = "button" size="medium" @click="backWanTest"  round>上一步</el-button-->
          <el-button class = "button" size="medium" @click="wanFinish"  round>{{$t('situationMode.startIntranetSet')}}</el-button>
        </div>
      </el-dialog>
  </div>

<!--选择内网端口-->
  <div class="thisdialogstyle">
    <el-dialog  width="40%"  :visible="selectLanPort" @close='closeDialog'>
      <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
        <el-step :title= "item" v-for="item in step" :key="item"></el-step>
      </el-steps>
      <div style="margin-top: 20px;">{{$t('situationMode.lanSelectTitle')}}</div>
        <div class="middleBottom" >
          <div class="router">
            <div  v-for="(item, index2) in ports" class="port"  @click="pushToLanINEX(item)" :key="index2">                
              <div class="item" effect="light" >
                <img style="width: 50px; height: 50px; border-radius: 5px;" :src="item.imgsrc" />
              </div>
              <div class="text-areaLan">
                <p class="textLan">{{ item.webname }}</p>
                <div :class="{ 'triangular': ports[index2].showtri === true }"></div>
              </div>
            </div>
          </div>
        </div>
        <div class="lanSelectinfo">
          <!-- <div >
            <el-checkbox style="margin-top: 20px;margin-left: 30px" v-model="setLan" @change="connectLan">{{$t('situationMode.setlan')}}</el-checkbox>
          </div> -->
          <div style="margin-top: 15px;margin-left: 23px;margin-right: 23px;margin-bottom: 30px; text-align: left;" >
            <!-- <div :class="setLan==true?'green':'grey'" style="width:100%;">     -->
              <div class="green" style="width:100%;"> 
              <el-collapse v-model="lanMethod" @change="selectLanMethod" accordion>
                <div style="display:inline-block; vertical-align:text-top;margin-top: 15px;">
                  <el-radio  v-bind:disabled= "isDHCP" @change="selectLanMethod" v-model="lanMethod" label="2"  style=" margin-top: 20px;margin-left: 30px;"> {{" "}}
                  </el-radio>
                </div>
                <!-- <div :class=" setLan==true? 'green':'grey' " style="display:inline-block; vertical-align:text-top;"> -->
                  <div class="green" style="display:inline-block; vertical-align:text-top;">
                  <el-collapse-item :title="$t('situationMode.dhcp')" name="2" style="display: inline-block;margin-top: 20px;">
                    <div style="width: 200px;margin-top: 20px;margin-left: 45px">
                      <p >{{$t('situationMode.dhcpStartIp')}}{{DHCPIP}}</p>
                    </div>
                    <div style="width: 200px;margin-top: 20px;margin-left: 45px">
                      <p >{{$t('situationMode.dhcpEndIp')}}{{DHCPStoreIP}}</p>
                    </div>
                  </el-collapse-item>
                </div>
                <!--给两个单选框添加了已有函数wyk-->
                <div style="width:100%;margin-top: 5px;margin-left: 30px;">
                  <div style="display:inline-block; vertical-align:text-top;margin-top: 12px;">
                    <el-radio  v-bind:disabled= "isStatic" @change="selectLanMethod"  v-model="lanMethod"  label="3"  style="display: inline-block;"> {{" "}}
                      </el-radio>
                      </div>
                    <!-- <div :class="setLan==true?'green':'grey'" style="display:inline-block; vertical-align:text-top;"> -->
                      <div class="green" style="display:inline-block; vertical-align:text-top;">
                      <el-collapse-item :title="$t('situationMode.lanstatic')" name="3" style="display: inline-block;">
                        <div  style="width: 200px;margin-top: 20px;margin-left: 45px;">
                          <p >{{$t('situationMode.lanStaticIP')}}{{thisStaticIP}}</p>
                        </div>
                        <div style="width: 200px;margin-top: 20px;margin-left: 45px;">
                          <p>{{$t('situationMode.lanNetmask')}}{{thisStaticNetmask}}</p>
                        </div>
                        <div style="width: 200px;margin-top: 20px;margin-left: 45px;">
                          <p>{{$t('situationMode.lanGateway')}}{{thisStaticGateway}}</p>
                        </div>
                      </el-collapse-item>
                      </div>
                </div>
              </el-collapse>
            </div>
          </div>
        </div>
        <div slot="footer" class="dialog-footer">
          <el-button class = "button" size="medium" @click="selectLanPortFinish"  round>{{$t('situationMode.next')}}</el-button>
        </div>
      </el-dialog>
    </div>


 <!--内网接入完成-->
    <div class="thisdialogstyle">
      <el-dialog  width="40%"  :visible="lanConnected" @close='closeDialog'>
        <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
          <el-step :title= "item" v-for="item in step" :key="item"></el-step>
        </el-steps>
        <div style="margin-bottom: 30px;text-align: center;margin-top: 20px" >
          <img src="static/netConnect.png"  height="100" width="100"  />
        </div>
        <div style="margin-top: 20px;text-align: center;">{{$t('situationMode.lanConnected')}}</div>
          <div slot="footer" class="dialog-footer">
            <el-button class = "button" size="medium" @click="seniorSet"  round>{{$t('situationMode.next')}}</el-button>
          </div>
      </el-dialog>
      </div>


 <!-- KidVPN客户端配置label-width="80px"-->
    <div class="thisdialogstyle">
      <el-dialog  width="40%"  :visible="clientSet" @close='closeDialog'>
        <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
          <el-step :title= "item" v-for="item in step" :key="item"></el-step>
        </el-steps>
        <div height="100" width="600"  style="margin-top: 20px;margin-left: 75px;" >  
          <div  height="90" width="90" style="display: inline-block;">
            <img src="static/client.png" height="70" width="70" />
          </div>
          <div  height="50" width="150" style="display: inline-block;margin-left: 10px;">
            <div>
              <img src="static/connect.png" height="30" width="30" style="margin-left: 55px;"/>
            </div>
            <div>
              <img src="static/line.png"  height="2" width="150" style="margin-bottom: 30px;" />
            </div>
          </div>
          <div  height="90" width="90" style="display: inline-block;margin-left: 10px;">
            <img src="static/internet.png" height="70" width="70" />
          </div>
          <div  height="50" width="150" style="display: inline-block;margin-left: 10px;">
            <div>
              <img src="static/disconnect.png" height="30" width="30" style="margin-left: 55px;" />
            </div>
            <div>
              <img src="static/line.png"  height="2" width="150" style="margin-bottom: 30px;"/>
            </div>
          </div>
          <div  height="90" width="90" style="display: inline-block;margin-left: 10px;">
            <img src="static/server.png" height="70" width="70" />
          </div>
        </div>
        <div class="info" style="margin-top: 20px;">
          <div style="margin-left: 25px;margin-top: 20px;">{{$t('situationMode.clientInfo')}}</div>
            <el-form ref="form" :model="clientRec"  :rules="rules" label-position="left" label-width="12rem" style="margin-left: 45px;margin-right: 25px;">
              <el-form-item prop="severWanIP"  :label="$t('situationMode.severInterIP')" style="width:100%;margin-top: 20px;text-align:left" >
                <el-input  v-model="clientRec.severWanIP"  ></el-input>
              </el-form-item>
               <el-form-item prop="clientLanIP" :label="$t('situationMode.clientIntranetIP')" style="width:100%;margin-top: 20px;text-align:left" >
                <el-input  v-model="clientRec.clientLanIP"  ></el-input>
              </el-form-item>
              <el-form-item prop="clientNetmask" :label="$t('situationMode.clientIntranetNetmask')" style="width:100%;margin-top: 20px;text-align:left" >
                <el-input   v-model="clientRec.clientNetmask" ></el-input>
              </el-form-item>
              <el-form-item prop="vpnKey" :label="$t('situationMode.vpnKey')" style="width:100%;margin-top: 20px;text-align:left" >
                <el-input   v-model="clientRec.vpnKey"  ></el-input>
              </el-form-item> 
              <el-form-item prop="vpnPasswd" :label="$t('situationMode.vpnPw')" style="width:100%;margin-top: 20px;text-align:left" >
                <el-input  v-model="clientRec.vpnPasswd"  ></el-input>
              </el-form-item>
            </el-form>
        </div>
        <div slot="footer" class="dialog-footer">
          <el-button class = "button" size="medium" @click="skipclient" round>{{$t('situationMode.skip')}}</el-button>
          <el-button class = "button" size="medium" @click="connectSever" round>{{$t('situationMode.connectSever')}}</el-button>
        </div>
      </el-dialog>
    </div>

 <!-- KidVPN客户端配完成-->
    <div class="thisdialogstyle">
      <el-dialog  width="40%"  :visible="clientSetted" @close='closeDialog'>
        <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
          <el-step :title= "item" v-for="item in step" :key="item"></el-step>
        </el-steps>
        <div height="100" width="600"  style="margin-top: 30px;margin-left: 75px"> 
          <div  height="90" width="90" style="display: inline-block;">
            <img src="static/client.png" height="70" width="70"/>
          </div>
          <div  height="50" width="150" style="display: inline-block;margin-left: 10px;">
            <div>
              <img src="static/connect.png" height="30" width="30" style="margin-left: 55px;"/>
            </div>
            <div>
              <img src="static/line.png"  height="2" width="150" style="margin-bottom: 30px;" />
            </div>
          </div>
          <div  height="90" width="90" style="display: inline-block;margin-left: 10px;">
            <img src="static/internet.png" height="70" width="70" />
          </div>
          <div  height="50" width="150" style="display: inline-block;margin-left: 10px;">
            <div>
              <img :src="this.connectStatusImg" height="30" width="30" style="margin-left: 55px;" />
            </div>
            <div>
              <img src="static/line.png"  height="2" width="150" style="margin-bottom: 30px;"/>
            </div>
          </div>
          <div  height="90" width="90" style="display: inline-block;margin-left: 10px;">
            <img src="static/server.png" height="70" width="70" />
          </div>
        </div>

        <div  style="margin-top: 20px;text-align: center;">{{connectSeverStatus}}</div>
          <div slot="footer" class="dialog-footer">
             <el-button class = "button" size="medium" @click="backClientSet"  round>{{$t('situationMode.back')}}</el-button>
          <el-button class = "button" size="medium" @click="clientNext"  round>{{$t('situationMode.next')}}</el-button>
        </div>
      </el-dialog>
    </div>
      <!-- KidVPN服务器配置-->
      <div class="thisdialogstyle">
        <el-dialog  width="40%"  :visible="severSet" @close='closeDialog'>
          <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
            <el-step :title= "item" v-for="item in step" :key="item"></el-step>
          </el-steps>
          <div class="info" style="margin-top: 25px;">
            <div style="margin-left: 25px;margin-top: 20px;">{{$t('situationMode.severInfo')}}</div>
              <el-form ref="form" :model="severRec" :rules="rules" label-position="left" label-width="11rem" style="margin-left: 45px;">
                <el-form-item prop="severIP" :label="$t('situationMode.severIntranetIP')" style="width:90%;margin-top: 20px;text-align:left" >
                  <el-input v-model="severRec.severIP" :placeholder="$t('placeholder.ip')" ></el-input>
                </el-form-item>
                <el-form-item prop="severNetmask" :label="$t('situationMode.severNetmask')" style="width:90%;margin-top: 20px;text-align:left" >
                  <el-input  v-model="severRec.severNetmask" :placeholder="$t('placeholder.netmask')" ></el-input>
                </el-form-item>
                <el-form-item prop="vpnPasswd" :label="$t('situationMode.vpnPw')" style="width:90%;margin-top: 20px;text-align:left" >
                  <el-input  v-model="severRec.vpnPasswd" :placeholder="$t('placeholder.password1')" ></el-input>
                </el-form-item>
              </el-form>
            </div>
            <div slot="footer" class="dialog-footer">
              <el-button class = "button" size="medium" @click="skipsever"  round>{{$t('situationMode.skip')}}</el-button>
              <el-button class = "button" size="medium" @click="createSever"  round>{{$t('situationMode.createSever')}}</el-button>
            </div>
        </el-dialog>
      </div>
      <!-- KidVPN服务器配置完成-->
      <div class="thisdialogstyle">
        <el-dialog  width="40%"  :visible="severSetted" @close='closeDialog'>
          <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
            <el-step :title= "item" v-for="item in step" :key="item"></el-step>
          </el-steps>
          <div style="margin-top: 20px;">{{$t('situationMode.severShowInfo')}}</div>
            <div class="infoSever" style="text-align: center;width: 90%;margin-left: 40px;margin-right: 40px;">
              <el-form ref="form" :model="severShow" label-position="left" label-width="11rem"  style="margin-left: 40px;">
                <el-form-item prop="severWanIP" :label="$t('situationMode.severInterIP')" style="width:90%;margin-top: 20px;text-align:left">
                  {{severShow.severWanIP}}
                </el-form-item>
                <el-form-item prop="severLanIP" :label="$t('situationMode.severIntranetIP')" style="width:90%;margin-top: 20px;text-align:left">
                  {{severShow.severLanIP}}
                </el-form-item>
                <el-form-item prop="severNetmask" :label="$t('situationMode.severNetmask')" style="width:90%;margin-top: 20px;text-align:left">
                  {{severShow.severNetmask}}
                </el-form-item>
                <el-form-item prop="severGateway" :label="$t('situationMode.severGateway')" style="width:90%;margin-top: 20px;text-align:left">
                  {{severShow.severGateway}}
                </el-form-item>
                <el-form-item prop="vpnPasswd" :label="$t('situationMode.vpnPw')" style="width:90%;margin-top: 20px;text-align:left">
                  {{severShow.vpnPasswd}}
                </el-form-item>
                <el-form-item prop="vpnKey" :label="$t('situationMode.vpnKey')"  style="width:90%;margin-top: 20px;text-align:left">
                  <div class="text-wrapper">{{severShow.vpnKey}}</div>
                </el-form-item>
              </el-form>
            </div>
            <div slot="footer" class="dialog-footer">
              <el-button class = "button" size="medium" @click="backSeverSet"  round>{{$t('situationMode.back')}}</el-button>
              <a :href ="'http://'+websocketip+'/VPN-Server-Conf.txt'" download="VPN-Server-Conf">
              <el-button class = "button" size="medium" @click="saveSeverInfo"  round>{{$t('situationMode.saveFileLocal')}}</el-button>
              </a>
              <el-button class = "button" size="medium" @click="severNext"  round>{{$t('situationMode.next')}}</el-button>
            </div>
        </el-dialog>
      </div>
      <!-- 内网欺骗防御-->
      <div class="thisdialogstyle">
        <el-dialog  width="40%"  :visible="defendPage" @close='closeDialog'>
          <el-steps :active="active" finish-status="success" process-status= "success" :space="200" simple>
            <el-step :title= "item" v-for="item in step" :key="item"></el-step>
          </el-steps>
          <div class="info" style="margin-top: 20px;"> 
            <el-checkbox v-model="defendStatus" style="margin-left: 40px;margin-top: 20px;margin-bottom: 20px;">{{$t('situationMode.arpDefense')}}</el-checkbox>
          </div>
          <div slot="footer" class="dialog-footer">
            <el-button class = "button" size="medium" @click="skiparp"  round>{{$t('situationMode.skip')}}</el-button>
            <el-button class = "button" size="medium" @click="startDefend"  round>{{$t('situationMode.ok')}}</el-button>
          </div>
        </el-dialog>
      </div>
      
 <!--引导配置已完成-->
    <div class="thisdialogstyle">
      <el-dialog  width="40%" :visible="allFinished" @close='closeFinished'>
        <div  style="text-align: center;">{{$t('situationMode.finished')}}</div>
          <div style="margin-bottom: 30px;text-align: center;margin-top: 20px" >
            <img src="static/netConnect.png"  height="100" width="100"  />
          </div>
          <div slot="footer" class="dialog-footer">
            <el-button class = "button" size="medium" @click="FinishConfigure"  round>{{$t('situationMode.ok')}}</el-button>
          </div>
      </el-dialog>
    </div>
  </div>
</template>


<script>

import { generateTitle } from '@/utils/i18n'
import { getMonitorInfo, logout, getPorts, websocketIpGetAction, publicNetSetAction, localNetSetAction, publicNetCancelAction, kidvpnConfigAction, kidvpnCliStatGetAction, configCompleteAction, asdefenseEnConfAction, kidvpnCancelAction, kidvpnConfigSaveAction, localDhcpValueGetAction, optionalFuncCancelAction } from '../api/api.js'
import validate from '@/utils/rules.js'
import { conversion } from '../utils/rateUnitExchange.js'
let  timerDel, wanTestTimer // 从情景模式html获得的对象与数据;删除定时器;检测wan连接状态的定时器
var ws  // websocket
export default {
  name: 'home',
  data() {
    let vpnKey = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入VPN密钥！/Please input VPN Key'))
      }
      callback()
    }
    let vpnPasswd = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入密码！/Please input password'))
      }
      callback()
    }
    let passwd = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入密码！/Please input password'))
      }
      callback()
    }
    let account = (rule, value, callback) => {
      if (value === '') {
        callback(new Error('请输入拨号账户！/Please input ADSL account'))
      }
      callback()
    }
    return {
      rules: {
        adslAccount: [
          { required: true, message: '请输入拨号账户！/Please input ADSL account' },
          {
            validator: account, trigger: 'blur'
          }
        ],
        staticIp: [
          { required: true, message: '地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        staticNetmask: [
          { required: true, message: '掩码不能为空！/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确掩码！/Please input correct netmask', '掩码不能为空！/Please input netmask'), trigger: 'blur'
          }
        ],
        staticGateway: [
          { required: true, message: '网关不能为空！/Please input gateway' },
          {
            validator: validate('ip', '请输入正确网关！/Please input correct gateway', '网关不能为空！/Please input gateway'), trigger: 'blur'
          }
        ],
        severWanIP: [
          { required: true, message: '地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        clientLanIP: [
          { required: true, message: '地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        severIP: [
          { required: true, message: '地址不能为空！/Please input IP' },
          {
            validator: validate('ip', '请输入正确IP！/Please input correct IP', '地址不能为空！/Please input IP'), trigger: 'blur'
          }
        ],
        clientNetmask: [
          { required: true, message: '内网掩码不能为空！/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确内网掩码！/Please input correct netmask', '内网掩码不能为空！/Please input netmask'), trigger: 'blur'
          }
        ],
        severNetmask: [
          { required: true, message: '内网掩码不能为空！/Please input netmask' },
          {
            validator: validate('netmask', '请输入正确内网掩码！/Please input correct netmask', '内网掩码不能为空！/Please input netmask'), trigger: 'blur'
          }
        ],

        vpnKey: [
          { required: true, message: '请输入VPN密钥！/Please input VPN Key' },
          {
            validator: vpnKey, trigger: 'blur'
          }
        ],
        adslpasswd1: [
          { required: true, message: '请输入密码！/Please input password' },
          {
            validator: passwd, trigger: 'blur'
          }
        ],
        adslpasswd2: [
          { required: true, message: '请输入密码！/Please input password' },
          {
            validator: passwd, trigger: 'blur'
          }
        ],
        vpnPasswd: [
          { required: true, message: '请输入密码！/Please input password' },
          {
            validator: vpnPasswd, trigger: 'blur'
          }
        ]

      },
      showTestButton: false,          // 测试外网连接状态页面返回上一步按钮是否显示
      connectStatusImg: '',          // 连接状态图标
      connectSeverStatus: '',            // 是否显示成功连接这段话
      lanSuccessCount: 0,             // 配置成功lan口数量
      lanCount: 0,                    // 已选择lan口数量
      wanvndid: 0,                     // 外网id号
      lanvndid: 0,                     // 内网id号
      // received_msg0: {},                // websocket发送0返回的数据
      // received_msg1: {},                // websocket发送1返回的数据
      // received_msg2: {},                // websocket发送2返回的数据
      // received_msg4: 0,                // websocket发送4返回的数据
      websocketip: '',           // websocket的ip
      nowLanPort: 0,              // 配置lan时当前选择的端口的index
      lanUselessDHCP: false,          // 只是用于lan配置折叠框左边的单选框
      lanUselessStatic: false,
      isDHCP: true,          // 是否可以配置DHCP
      isStatic: true,        // 是否可以配置static
      funcCount: 0, // 选择的功能数量
      active: 0,
      step: ['外网接入', '内网接出'], // 进行的步骤
      selected: '', // 已经被选中的端口
      wanImage: 'static/wanzh.png',
      lanImage: 'static/lanzh.png',
      defendImage: 'static/defend1zh.png', // 选择功能时的图片
      severImage: 'static/server1zh.png',
      clientImage: 'static/client1zh.png',
      DHCPIP: '', // 内网设置相关信息
      DHCPStoreIP: '',
      thisStaticIP: '',
      thisStaticNetmask: '',
      thisStaticGateway: '',
      close: 0, // 判断是关闭dialog还是跳转,1为跳转，0为关闭
      setLan: false, // 连接LAN口
      selectLanPort: false, // 选择内网端口
      clientSetted: false, // 进入连接服务器界面
      clientSet: false, // 客户端配置
      allFinished: false, // 全部配置完成界面
      lanConnected: false, // 内网配置完成
      defendPage: false, // 防御页面
      defendStatus: false, // 防御状态
      severShow: {
        // 服务器配置好的信息
        severWanIP: '',
        severLanIP: '',
        severNetmask: '',
        severGateway: '',
        vpnPasswd: '',
        vpnKey: ''
      },
      clientRec: {
        // 客户端配置收集的信息
        severWanIP: '',
        clientLanIP: '',
        clientNetmask: '',
        vpnPasswd: '',
        vpnKey: ''
      },
      severRec: {
        // 服务器配置收集的信息
        severIP: '',
        severNetmask: '',
        vpnPasswd: ''
      },
      severSetted: false, // 服务器配置完成
      severSet: false, // 配置服务器
      wanConnected: false, // 外网已连接
      wanTest: false, // 外网测试
      isadsl: 'false', // 三种外网方式互斥
      isstatic: 'false',
      staticIpInfo: {
        // 静态IP信息
        staticIp: '',
        staticNetmask: '',
        staticGateway: ''
      },

      adslInfo: {
        // adsl拨号信息
        adslAccount: '',
        passwd: '',
        adslpasswd1: '',
        adslpasswd2: ''
      },
      ports: [
        {
          id: '0',
          category: '',
          imgUrl: 'static/port3.png'
        }
      ],
      lanMethod: 0, // 已经选好的内网接入方式
      wanMethod: 0, // 已经选好的外网接入方式
      selectWanMethod: false, // 选择外网方式弹出框的可见性
      selectWanPort: false, // 选择外网接口弹出框的可见性
      func1: false,
      func2: false,
      func3: false,
      showselectFunc: false, // 选择功能1
      showFrame: true, // wyk是否显示frame
      sysName: 'OpenRT v0.1',
      collapsed: false,
      isCollapse: false,
      sysUserName: '',
      sysUserAvatar: '',
      system: [{ CPURate: '0', RAMRate: '0' }],
      data: {
        up: '',
        down: '',
        upStorage: '',
        downStorage: '',
        cpu: '',
        memory: '',
        userNum: ''
      }
    }
  },
  computed: {
    lang: {
      get() {
        console.log('get')
        console.log('in get ' + this.$store.state.app.language)
        if (this.$store.state.app.language === 'en') {
          this.Url = '/static/situationMode1.html'
          console.log('en' + this.Url)
        } else {
          this.Url = '/static/situationMode2.html'
          console.log('zh' + this.Url)
        }
        return this.$store.state.app.language
      },
      set(lang) {
        console.log('set')
        this.$i18n.locale = lang
        this.$store.dispatch('setLanguage', lang)
      }
    },
    'item.selected': function(para) {
      if (para === 'false') {
        console.log('port2')
        return 'static/port2.png'
      } else {
        console.log('port3')
        return 'static/port3.png'
      }
    }
  },

  methods: {
    skipclient() {               // 跳过客户端配置
      let para = {}
      para.kidvpn_client = 0
      optionalFuncCancelAction(para).then(res => {
        if (res.data.code === 200) {
          this.close = 1
          this.clientSet = false
          this.func1 = false
          this.setSteps()
          if (this.func2 === true) {
            this.severSet = true
          } else if (this.func3 === true) {
            this.defendPage = true
          } else {
            this.allFinished = true
          }
        } else {
          if (this.$store.state.app.language === 'en') {
            this.$message('Skip failed')
          } else {
            this.$message('跳过失败')
          }
        }
      })
    },
    skipsever() {               // 跳过服务器配置
      let para = {}
      para.kidvpn_server = 0
      optionalFuncCancelAction(para).then(res => {
        if (res.data.code === 200) {
          this.close = 1
          this.severSet = false
          this.func2 = false
          this.setSteps()
          if (this.func3 === true) {
            this.defendPage = true
          } else {
            this.allFinished = true
          }
        } else {
          if (this.$store.state.app.language === 'en') {
            this.$message('Skip failed')
          } else {
            this.$message('跳过失败')
          }
        }
      })
    },
    skiparp() {               // 跳过arp配置
      let para = {}
      para.arp_defense = 0
      optionalFuncCancelAction(para).then(res => {
        if (res.data.code === 200) {
          this.close = 1
          this.defendPage = false
          this.func3 = false
          this.setSteps()
          this.allFinished = true
        } else {
          if (this.$store.state.app.language === 'en') {
            this.$message('Skip failed')
          } else {
            this.$message('跳过失败')
          }
        }
      })
    },
    start() {                         // 进入中小企业路由开始的一系列判断，包括可否进行配置，从断点开始继续配置
      console.log('333')

      // clearInterval(timerDel)
      // this.showFrame = false
      // this.showselectFunc = true

      ws.send('0')
      ws.onmessage = function (evt) {
        var obj = JSON.parse(evt.data)
        receivedmsg0(obj)
        // this.received_msg0 = obj
      }

      let changeArea0 = function ActionReceivedmsg0(val) {
        console.log('val')
        console.log(val)
        if (val.kidvpn_client === 1) {
          this.func1 = true
        } else if (val.kidvpn_client === 0) {
          this.func1 = false
        }
        if (val.kidvpn_server === 1) {
          this.func2 = true
        } else if (val.kidvpn_server === 0) {
          this.func2 = false
        }
        if (val.arp_defense === 1) {
          this.func3 = true
        } else if (val.arp_defense === 0) {
          this.func3 = false
        }
        this.setSteps()
        if (val.status === 'Noconfig' || val.status === 'NONE') {
          console.log('4')
          ws.send('4')
          ws.onmessage = function (evt) {
            var obj = JSON.parse(evt.data)
            receivedmsg4(obj)
          }
        } else if (val.status === 'Finished') {
          if (val.position === 'Configure_Cmplete_Page') {
            // this.allFinished = true
            console.log('675')
            this.$router.push('sceneMode')
          }
        } else if (val.status === 'Configing') {
          console.log('Configing')
          if (val.kidvpn_client === 1) {
            this.func1 = true
          } else if (val.kidvpn_client === 0) {
            this.func1 = false
          }
          if (val.kidvpn_server === 1) {
            this.func2 = true
          } else if (val.kidvpn_server === 0) {
            this.func2 = false
          }
          if (val.arp_defense === 1) {
            this.func3 = true
          } else if (val.arp_defense === 0) {
            this.func3 = false
          }
          if (val.position === 'Starting-Set-Page' || val.position === 'NONE') {
            this.showselectFunc = true
          } else if (val.position === 'Connect-Submit-Page') {
            ws.send('1')
            ws.onmessage = function (evt) {
              var obj = JSON.parse(evt.data)
              receivedmsg1CSP(obj)
            }
          } else if (val.position === 'Connecting-Internet-Page') {
            this.wanTest = true
            setTimeout(() => {
              this.checkWanConnectStatus()
            }, 3000)
            // ws.send('1')
            // ws.onmessage = function (evt) {
            //   var obj = JSON.parse(evt.data)
            //   receivedmsg1CIP(obj)
            // }
          } else if (val.position === 'Intranet-Set-Page') {
            console.log(' Intranet-Set-Page')
            this.active = 1
            this.lanConnected = true
          } else if (val.position === 'Client-KidVPN-Set-Page') {
            this.active = 2
            this.clientSetted = true
            this.startGetConnectStatus()
          } else if (val.position === 'Configure_Cmplete_Page') {
            this.allFinished = true
            console.log('675')
          } else if (val.position === 'Server-KidVPN-Set-Page') {
            if (val.kidvpn_client === 1) {
              this.active = 3
            } else if (val.kidvpn_client === 0) {
              this.active = 2
            }
            ws.send('2')
            ws.onmessage = function (evt) {
              var obj = JSON.parse(evt.data)
              receivedmsg2SKS(obj)
            }
          } else if (val.position === 'Client-KidVPN-Config-Page') {
            this.active = 2
            ws.send(6)
            ws.onmessage = function (evt) {
              var obj = JSON.parse(evt.data)

              receivedmsg6CKCP(obj)
            }
            let changeArea6CKCP = function ActionReceivedmsg6CKCP(val) {
              this.clientRec.severWanIP = val.public_ip
              this.clientRec.clientLanIP = val.vnd_ip
              this.clientRec.clientNetmask = val.vnd_msk
              this.clientRec.vpnKey = val.aeskey
              this.clientRec.vpnPasswd = val.passwd
            }
            let receivedmsg6CKCP = changeArea6CKCP.bind(this)
            this.clientSet = true
          } else if (val.position === 'Server-KidVPN-Config-Page') {
            if (val.kidvpn_client === 1) {
              this.active = 3
            } else if (val.kidvpn_client === 0) {
              this.active = 2
            }
            ws.send(2)
            ws.onmessage = function (evt) {
              var obj = JSON.parse(evt.data)
              receivedmsg2SKCP(obj)
            }
            this.severSet = true
          }
        }
      }
      let changeArea2SKCP = function ActionReceivedmsg2SKCP(val) {
        this.severRec.severIP = val.vnd_ip
        this.severRec.severNetmask = val.vnd_msk

        this.severRec.vpnPasswd = val.passwd

        this.severSet = true
      }
      let changeArea2SKS = function ActionReceivedmsg2SKS(val) {
        this.severShow.severWanIP = val.public_ip
        this.severShow.severLanIP = val.vnd_ip
        this.severShow.severNetmask = val.vnd_msk
        this.severShow.severGateway = val.vnd_gw
        this.severShow.vpnPasswd = val.passwd
        this.severShow.vpnKey = val.aeskey
        this.severRec.severIP = val.vnd_ip
        this.severRec.severNetmask = val.vnd_msk

        this.severRec.vpnPasswd = val.passwd
        this.severSetted = true
      }
      // let changeArea1CIP = function ActionReceivedmsg1CIP(val) {
      //   if (val.internet === 'yes') {
      //     this.wanTestFinish()
      //     this.showTestButton = false
      //   } else if (val.internet === 'no') {
      //     // ActionReceivedmsg1CIP(val)
      //     console.log('配置失败')
      //     this.showTestButton = true
      //     // let bara = {}
      //     // bara.oper_type = val.mode
      //     // bara.webname = val.webname
      //     // if (this.func1 === true) {
      //     //   bara.kidvpn_client = 1
      //     // } else if (this.func1 === false) {
      //     //   bara.kidvpn_client = 0
      //     // }
      //     // if (this.func2 === true) {
      //     //   bara.kidvpn_server = 1
      //     // } else if (this.func2 === false) {
      //     //   bara.kidvpn_server = 0
      //     // }
      //     // if (this.func3 === true) {
      //     //   bara.arp_defense = 1
      //     // } else if (this.func3 === false) {
      //     //   bara.arp_defense = 0
      //     // }

      //     // publicNetCancelAction(bara).then(res => {
      //     //   if (res.data.code === 200) {
      //     //     console.log('取消成功')
      //     //     this.close = 1
      //     //     this.selectWanMethod = true
      //     //     this.wanTest = false
      //     //   }
      //     // })
      //   }
      // }
      let changeArea1CSP = function ActionReceivedmsg1CSP(val) {
        this.selected = val.webname
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webname === val.webname) {
            this.ports[i].selected = 'true'
            this.ports[i].imgsrc = 'static/port3.png'
          } else {
            this.ports[i].selected = 'false'
            this.ports[i].imgsrc = 'static/port2.png'
          }
        }
        if (val.mode === 1) {
          this.wanMethod = '1'
          this.adslInfo.adslAccount = val.user
          this.adslInfo.adslpasswd1 = val.pwd
          this.adslInfo.adslpasswd2 = val.pwd
        } else if (val.mode === 2) {
          this.wanMethod = '2'
        } else if (val.mode === 3) {
          this.wanMethod = '3'
          this.staticIpInfo.staticIp = val.ip
          this.staticIpInfo.staticNetmask = val.netmask
          this.staticIpInfo.staticGateway = val.gateway
        }
        this.$forceUpdate()

        this.selectWanMethod = true
      }
      let changeArea4 = function ActionReceivedmsg4(val) {
        if (val.status === 0) {
          if (this.$store.state.app.language === 'en') {
            this.$confirm(
              'Can not configure, clear the data and restart?',
              'Warning',
              {
                confirmButtonText: 'OK',
                cancelButtonText: 'Cancel',
                type: 'warning'
              }

            )
              .then(() => {
                this.$message({
                  type: 'success',
                  message: 'Restarted!'
                })

                ws.send('3')
              })
              .catch(() => {
                this.$message({
                  type: 'info',
                  message: 'Canceled'
                })
              })
          } else {
            this.$confirm(
              '不能进行情景模式引导，点击清除数据并重启',
              '提示',
              {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
              }

            )
              .then(() => {
                this.$message({
                  type: 'success',
                  message: '已重启!'
                })

                ws.send('3')
              })
              .catch(() => {
                this.$message({
                  type: 'info',
                  message: '已取消重启'
                })
              })
          }
        } else if (val.status === 1) {
          console.log('start')

          console.log('false')

          console.log('false')
          this.showselectFunc = true
          console.log('true')
        }
      }
      let receivedmsg2SKCP = changeArea2SKCP.bind(this)
      let receivedmsg2SKS = changeArea2SKS.bind(this)
      // let receivedmsg1CIP = changeArea1CIP.bind(this)
      let receivedmsg1CSP = changeArea1CSP.bind(this)
      let receivedmsg4 = changeArea4.bind(this)
      let receivedmsg0 = changeArea0.bind(this)
    },

    FinishConfigure() {            // 配置全部完成
      configCompleteAction().then(res => {
        if (res.data.code === 200) {
          console.log('全部配置完成')
          this.$router.push('sceneMode')
        } else {
          if (this.$store.state.app.language === 'en') {
            this.$message('Submit information failed, please check')
          } else {
            this.$message('提交完成状态失败，请检查')
          }
        }
      })
    },
    backSeverSet() {                         // 服务器配置完成返回上一步
      if (this.$store.state.app.language === 'en') {
        this.$confirm(
          'Are you sure to go back and revoke the current configuration?',
          'Warning',
          {
            confirmButtonText: 'OK',
            cancelButtonText: 'Cancel',
            type: 'warning'
          }
        )
          .then(() => {
            let para = {}
            para.oper_type = 0
            kidvpnCancelAction(para).then(res => {
              if (res.data.code === 200) {
                this.$message({
                  type: 'success',
                  message: 'Revoked!'
                })
                this.close = 1
                this.severSetted = false
                this.severSet = true
              } else {
                this.$message('Revoke failed')
              }
            })
          })
          .catch(() => {
            this.$message({
              type: 'info',
              message: 'Canceled'
            })
          })
      } else {
        this.$confirm(
          '返回上一步将撤销当前配置，是否确定返回?',
          '提示',
          {
            confirmButtonText: '确定',
            cancelButtonText: '取消',
            type: 'warning'
          }
        )
          .then(() => {
            let para = {}
            para.oper_type = 0
            kidvpnCancelAction(para).then(res => {
              if (res.data.code === 200) {
                this.$message({
                  type: 'success',
                  message: '已撤销!'
                })
                this.close = 1
                this.severSetted = false
                this.severSet = true
              } else {
                this.$message('撤销失败')
              }
            })
          })
          .catch(() => {
            this.$message({
              type: 'info',
              message: '已取消返回'
            })
          })
      }
    },
    changeDHCPradio() {                       // dhcp和静态路由单选框点击触发的事件
      if (this.lanUselessDHCP === false) {
        this.lanUselessStatic = true

        console.log('this.lanUselessDHCP1' + this.lanUselessDHCP)
        console.log('this.lanUselessStatic1' + this.lanUselessStatic)
      } else {
        this.lanUselessStatic = false

        console.log('this.lanUselessDHCP2' + this.lanUselessDHCP)
        console.log('this.lanUselessStatic2' + this.lanUselessStatic)
      }
    },
    changeStaticradio() {
      if (this.lanUselessStatic  === false) {
        this.lanUselessDHCP = true

        console.log('this.lanUselessStatic3' + this.lanUselessStatic)
        console.log('this.lanUselessDHCP3' + this.lanUselessDHCP)
      } else {
        this.lanUselessDHCP = false

        console.log('this.lanUselessStatic4' + this.lanUselessStatic)
        console.log('this.lanUselessDHCP4' + this.lanUselessDHCP)
      }
    },
    selectLanMethod() {
      console.log('this.lanMethod')
      console.log(this.lanMethod)
      console.log('this.setLan')
      console.log(this.setLan)
      this.setLan = true
      for (let i = 0; i < this.ports.length; i++) {
        if (this.ports[i].webindex !== this.nowLanPort && this.ports[i].webname !== this.selected) {
          this.ports[i].selected = 'false'
          this.ports[i].imgsrc = 'static/port2.png'
          console.log(this.ports[i].showtri)
        } else if (this.ports[i].webindex === this.nowLanPort || this.ports[i].webname === this.selected) {
          console.log('this.ports[i]')
          console.log(this.ports[i])
          this.ports[i].selected = 'true'
          this.ports[i].imgsrc = 'static/port3.png'
          this.isDHCP = false
          this.isStatic = false
          if (this.ports[i].webindex === this.nowLanPort) {
            this.ports[i].showtri = true
          }
          if (this.lanMethod === '2') {
            this.ports[i].lanMethod = 'DHCP'
          } else if (this.lanMethod === '3') {
            this.ports[i].lanMethod = 'Static'
          }
        }
      }
      // if (this.setLan === true) {
      if (this.lanMethod === '2') {
        console.log('choose dhcp')
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webindex === this.nowLanPort) {
            this.ports[i].lanMethod = 'DHCP'
          }
        }
      } else if (this.lanMethod === '3') {
        console.log('choose static')
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webindex === this.nowLanPort) {
            this.ports[i].lanMethod = 'Static'
          }
        }
      }
      // } else {
      //   this.lanMethod = ''
      // }
    },
    backWanTest() {        // 外网连接失败返回上一步
      this.close = 1
      this.wanConnected = false
      this.wanTest = true
    },

    clientNext() {                          // 客户端配置完成
      kidvpnCliStatGetAction().then(res => {
        if (res.data.code === 200) {
          console.log('tryconnect')
          if (res.data.status === 'ON') {
            if (this.$store.state.app.language === 'en') {
              this.connectSeverStatus = 'Connect to the server successfully'
            } else {
              this.connectSeverStatus = '本地KidVPN成功连接到服务器'
            }
            this.connectStatusImg = 'static/connect.png'
            clearInterval(timerDel)
            this.close = 1
            this.clientSetted = false
            if (this.func2 === true) {
              this.severSet = true
              this.active = 3
            } else if (this.func2 === false && this.func3 === true) {
              this.defendPage = true
              this.active = 3
            } else {
              this.allFinished = true
            }
          } else {
            if (this.$store.state.app.language === 'en') {
              this.connectSeverStatus = 'Connecting'
              this.$message('Can not connect to sever, please reconfiguration')
            } else {
              this.connectSeverStatus = '正在连接服务器'
              this.$message('未能成功连接服务器请返回重新配置')
            }

            this.connectStatusImg = 'static/disconnect.png'
          }
        } else {
          if (this.$store.state.app.language === 'en') {
            this.connectSeverStatus = 'Connecting'
            this.$message('Can not connect to sever, please reconfiguration')
          } else {
            this.$message('未能成功连接服务器请返回重新配置')
            this.connectSeverStatus = '正在连接服务器'
          }
          this.connectStatusImg = 'static/disconnect.png'
        }
        this.$forceUpdate()
      })
    },

    changeClient() {                                   // 选择功能时三张图片点击触发的事件
      if (this.$store.state.app.language === 'en') {
        if (this.clientImage === 'static/client1en.png') {
          this.clientImage = 'static/client2en.png'
        } else {
          this.clientImage = 'static/client1en.png'
        }
      } else {
        if (this.clientImage === 'static/client1zh.png') {
          this.clientImage = 'static/client2zh.png'
        } else {
          this.clientImage = 'static/client1zh.png'
        }
      }
      if (this.func1 === false) {
        this.func1 = true
      } else {
        this.func1 = false
      }
    },
    changeSever() {
      if (this.$store.state.app.language === 'en') {
        if (this.severImage === 'static/server1en.png') {
          this.severImage = 'static/server2en.png'
        } else {
          this.severImage = 'static/server1en.png'
        }
      } else {
        if (this.severImage === 'static/server1zh.png') {
          this.severImage = 'static/server2zh.png'
        } else {
          this.severImage = 'static/server1zh.png'
        }
      }
      if (this.func2 === false) {
        this.func2 = true
      } else {
        this.func2 = false
      }
    },
    changeDefend() {
      if (this.$store.state.app.language === 'en') {
        if (this.defendImage === 'static/defend1en.png') {
          this.defendImage = 'static/defend2en.png'
        } else {
          this.defendImage = 'static/defend1en.png'
        }
      } else {
        if (this.defendImage === 'static/defend1zh.png') {
          this.defendImage = 'static/defend2zh.png'
        } else {
          this.defendImage = 'static/defend1zh.png'
        }
      }
      if (this.func3 === false) {
        this.func3 = true
      } else {
        this.func3 = false
      }
    },
    closeFinished() {                     // 关闭最后配置完成的dialog
      this.allFinished = false
    },
    closeDialog() {
      console.log(this.showselectFunc)
      console.log(this.selectWanPort)
      console.log(this.selectWanMethod)
      console.log(this.wanTest)
      console.log(this.wanConnected)
      if (this.close === 0) {
        console.log('close')
        if (this.$store.state.app.language === 'en') {
          this.$confirm(
            'Quitting will delete all contents and restart the router, sure to exit?',
            'Warning',
            {
              confirmButtonText: 'OK',
              cancelButtonText: 'Cancel',
              type: 'warning'
            }
          )
            .then(() => {
              this.$message({
                type: 'success',
                message: 'Exited!'
              })
              console.log('startcall')
              this.showselectFunc = false // 关闭窗口
              this.selectWanPort = false
              this.selectWanMethod = false
              this.wanTest = false
              this.wanConnected = false
              this.selectLanPort = false
              this.lanConnected = false
              this.defendPage = false
              this.clientSet = false
              this.clientSetted = false
              this.severSetted = false
              this.severSet = false

              this.modeType = 0
              ws.send(3)
            })
            .catch(() => {
              this.$message({
                type: 'info',
                message: 'Canceled'
              })
            })
        } else {
          this.$confirm(
            '中途退出会删除所有内容，并重启路由器，是否确定退出?',
            '提示',
            {
              confirmButtonText: '确定',
              cancelButtonText: '取消',
              type: 'warning'
            }
          )
            .then(() => {
              this.$message({
                type: 'success',
                message: '已退出!'
              })
              console.log('startcall')
              this.showselectFunc = false // 关闭窗口
              this.selectWanPort = false
              this.selectWanMethod = false
              this.wanTest = false
              this.wanConnected = false
              this.selectLanPort = false
              this.lanConnected = false
              this.defendPage = false
              this.clientSet = false
              this.clientSetted = false
              this.severSetted = false
              this.severSet = false

              this.modeType = 0
              ws.send(3)
            })
            .catch(() => {
              this.$message({
                type: 'info',
                message: '已取消退出'
              })
            })
        }
      } else {
        this.close = 0
      }
    },
    connectLan() {                        // 连接内网
      console.log('change')

      if (this.setLan === true) {
        console.log('true')
        console.log(this.nowLanPort)
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webindex !== this.nowLanPort && this.ports[i].webname !== this.selected) {
            this.ports[i].selected = 'false'
            this.ports[i].imgsrc = 'static/port2.png'
            console.log(this.ports[i].showtri)
          } else if (this.ports[i].webindex === this.nowLanPort || this.ports[i].webname === this.selected) {
            console.log('this.ports[i]')
            console.log(this.ports[i])
            this.ports[i].selected = 'true'
            this.ports[i].imgsrc = 'static/port3.png'
            this.isDHCP = false
            this.isStatic = false
            if (this.ports[i].webindex === this.nowLanPort) {
              this.ports[i].showtri = true
            }
            if (this.lanMethod === '2') {
              this.ports[i].lanMethod = 'DHCP'
            } else if (this.lanMethod === '3') {
              this.ports[i].lanMethod = 'Static'
            }
          }
        }
      } else  if (this.setLan === false) {
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webindex === this.nowLanPort && this.ports[i].webname !== this.selected) {
            this.ports[i].selected = 'false'
            this.ports[i].imgsrc = 'static/port2.png'
            console.log('false')
            this.isDHCP = true
            this.isStatic = true
            this.ports[i].lanMethod = ''
            this.lanMethod = ''
          }
        }
      }
      this.$forceUpdate()
      // 点击配置Lan   可配置多个端口
      /* if (this.setLan === true) {
        console.log('true')
        this.ports[this.nowLanPort].selected = 'true'
        this.ports[this.nowLanPort].imgsrc = 'static/port3.png'
        this.isDHCP = false
        this.isStatic = false
        if (this.lanMethod === '2') {
          this.ports[this.nowLanPort].lanMethod = 'DHCP'
        } else if (this.lanMethod === '3') {
          this.ports[this.nowLanPort].lanMethod = 'Static'
        }
      } else  if (this.setLan === false && this.ports[this.nowLanPort].webname !== this.selected) {
        this.ports[this.nowLanPort].selected = 'false'
        this.ports[this.nowLanPort].imgsrc = 'static/port2.png'
        console.log('false')
        this.isDHCP = true
        this.isStatic = true
        this.ports[this.nowLanPort].lanMethod = ''
        this.lanMethod = ''
      } */
    },

    selectLanPortFinish() {                    // 选择完内网端口
      console.log('this.ports')
      console.log(this.ports)
      let para = {}

      for (let i = 0; i < this.ports.length; i++) {
        if (this.ports[i].selected === 'true' && this.ports[i].webname !== this.selected) {
          if (this.ports[i].webname === '') {
            if (this.$store.state.app.language === 'en') {
              this.$message('Please select Lan port')
            } else {
              this.$message('请选择Lan口')
            }
          } else {
            para.webname = this.ports[i].webname
            console.log('this.nowLanPort')
            console.log(this.ports[i].webname)
            console.log(this.ports[i].lanMethod)
            if (this.ports[i].lanMethod === 'DHCP' || this.ports[i].lanMethod === 'Static') {
              console.log('come')
              if (this.ports[i].lanMethod === 'DHCP') {
                para.start = this.DHCPIP
                para.end = this.DHCPStoreIP
                para.oper_type = 2
              } else if (this.ports[i].lanMethod === 'Static') {
                para.ipaddr = this.thisStaticIP
                para.netmask = this.thisStaticNetmask
                para.gateway = this.thisStaticGateway
                para.oper_type = 3
              }

              localNetSetAction(para).then(res => {
                console.log('res.data.code')
                console.log(res.data.code)
                if (res.data.code === 200) {
                  this.close = 1 // 选择完Lan口
                  this.lanConnected = true
                  this.selectLanPort = false
                } else if (res.data.code === 300) {
                  if (this.$store.state.app.language === 'en') {
                    this.$message('This IP is in the same network segment as other network ports, configure failed')
                  } else {
                    this.$message('该IP与其他网口处于同一网段，设置失败')
                  }
                } else  {
                  if (this.$store.state.app.language === 'en') {
                    this.$message('Port' + this.ports[i].webname + 'configure failed')
                  } else {
                    this.$message('端口' + this.ports[i].webname + '配置失败')
                  }
                }
              })
            } else {
              if (this.$store.state.app.language === 'en') {
                this.$message('Please select intranet method')
              } else {
                this.$message('请选择内网接入方式')
              }
            }
          }
        }
      }

      // 一次提交多个lan口
      /* for (let i = 0; i < this.ports.length; i++) {
        if (this.ports[i].selected === 'true') {
          this.lanCount = this.lanCount + 1
          let para = {}
          if (this.ports[i].webname === '') {
            this.$message('请选择Lan口')
          } else {
            para.webname = this.ports[i].webname
            console.log('this.nowLanPort')
            console.log(this.ports[i].webname)

            if (this.ports[i].lanMethod === 'DHCP' || this.ports[i].lanMethod === 'Static') {
              if (this.ports[i].lanMethod === 'DHCP') {
                para.start = '192.168.1.10'
                para.end = '192.168.1.255'
                para.oper_type = 2
              } else if (this.ports[i].lanMethod === 'Static') {
                para.ipaddr = '10.9.0.92'
                para.netmask = '255.255.0.0'
                para.gateway = '10.9.0.1'
                para.oper_type = 3
              }

              localNetSetAction(para).then(res => {
                if (res.data.code === 200) {
                  this.lanSuccessCount = this.lanSuccessCount + 1
                } else {
                  this.$message('端口' + this.ports[i].webname + '配置失败')
                }
              })
            }
          }
        }
      }
      if (this.lanSuccessCount === this.lanCount) {
        this.close = 1 // 选择完Lan口
        this.lanConnected = true
        this.selectLanPort = false
      } */

      // para.start = this.DHCPIP
      // para.end = this.DHCPStoreIP
      // para.ipaddr = this.thisStaticIP
      // para.netmask = this.thisStaticNetmask
      // para.gateway = '10.9.0.1'
    },
    severNext() {
      this.close = 1
      this.severSetted = false // 服务器配置完成跳转
      if (this.func3 === true) {
        this.defendPage = true
        this.active = 4
      } else {
        this.allFinished = true
      }
    },
    connectSever() {
      // 连接服务器
      let para = {}
      para.oper_type = 1
      para.vnd_ipaddr = this.clientRec.clientLanIP
      para.vnd_netmask = this.clientRec.clientNetmask
      para.ipaddr = this.clientRec.severWanIP
      para.passwd = this.clientRec.vpnPasswd
      para.aeskey = this.clientRec.vpnKey
      if (para.vnd_ipaddr !== '' && para.vnd_netmask !== '' && para.ipaddr !== '' && para.passwd !== '' && para.aeskey !== '') {
        kidvpnConfigAction(para).then(res => {
          if (res.data.code === 200) {
            this.close = 1
            this.clientSet = false
            this.clientSetted = true
            this.startGetConnectStatus()
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message('Configuration failed. Please reconfigure.')
            } else {
              this.$message('配置失败，请重新配置')
            }
          }
        })
      } else {
        if (this.$store.state.app.language === 'en') {
          this.$message('Please input all information')
        } else {
          this.$message('请完整输入信息')
        }
      }

      // setTimeout(this.clientFinished, 2000)
    },
    startGetConnectStatus() {
      timerDel = setInterval(this.getSeverConnectStatus, 1000)
    },
    getSeverConnectStatus() {
      kidvpnCliStatGetAction().then(res => {
        if (res.data.code === 200) {
          console.log('tryconnect')
          if (res.data.status === 'ON') {
            if (this.$store.state.app.language === 'en') {
              this.connectSeverStatus = 'Sever connected'
            } else {
              this.connectSeverStatus = '本地KidVPN成功连接到服务器'
            }
            this.connectStatusImg = 'static/connect.png'
            // clearInterval(timerDel)
          } else {
            if (this.$store.state.app.language === 'en') {
              this.connectSeverStatus = 'Sever connecting'
            } else {
              this.connectSeverStatus = '正在连接服务器'
            }

            this.connectStatusImg = 'static/disconnect.png'
          }
        } else {
          this.connectSeverStatus = '正在连接服务器'
          this.connectStatusImg = 'static/disconnect.png'
        }
        this.$forceUpdate()
      })
    },
    clientFinished() {                // 客户端配置完成
      console.log('drop')
      this.close = 1

      if (this.func2 === true) {
        this.severSet = true
      } else if (this.func2 === false && this.func3 === true) {
        this.defendPage = true
      } else if (this.func2 === false && this.func3 === false) {
        this.allFinished = true
      }
    },
    seniorSet() {
      this.active = 2
      // 内外网配置完成进入高级设置
      if (this.func1 === true) {
        this.close = 1
        this.clientSet = true
        this.lanConnected = false
      } else if (this.func1 === false && this.func2 === true) {
        this.close = 1
        this.severSet = true
        this.lanConnected = false
      } else if (
        this.func1 === false &&
        this.func2 === false &&
        this.func3 === true
      ) {
        this.close = 1
        this.defendPage = true
        this.lanConnected = false
      } else {
        this.close = 1
        this.lanConnected = false
        this.allFinished = true
      }
    },
    startDefend() {
      let para = {}
      if (this.defendStatus === true) {
        para.en = 1
        asdefenseEnConfAction(para).then(res => {
          if (res.data.code === 200) {
            this.close = 1
            this.allFinished = true
            this.defendPage = false // 开启防御
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message('Configure failed. please check')
            } else {
              this.$message('开启失败，请检查')
            }
          }
        })
      } else {
        para.en = 0
        asdefenseEnConfAction(para).then(res => {
          if (res.data.code === 200) {
            this.close = 1
            this.allFinished = true
            this.defendPage = false // 关闭防御
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message('Configure failed. please check')
            } else {
              this.$message('关闭失败，请检查')
            }
          }
        })
      }
    },
    saveSeverInfo() {                 // 配置信息保存至本地

    },
    backClientSet() {                      // 返回至客户端配置
      if (this.$store.state.app.language === 'en') {
        this.$confirm(
          'Are you sure to go back and revoke the configuration?',
          'Warning',
          {
            confirmButtonText: 'OK',
            cancelButtonText: 'Cancel',
            type: 'warning'
          }
        )
          .then(() => {
            let para = {}
            para.oper_type = 1
            kidvpnCancelAction(para).then(res => {
              if (res.data.code === 200) {
                this.$message({
                  type: 'success',
                  message: 'Revoked!'
                })
                clearInterval(timerDel)
                this.close = 1
                this.clientSet = true
                this.clientSetted = false
                ws.send(6)
                ws.onmessage = function (evt) {
                  var obj = JSON.parse(evt.data)

                  receivedmsg6BCS(obj)
                }
                let changeArea6BCS = function ActionReceivedmsg6BCS(val) {
                  this.clientRec.severWanIP = val.public_ip
                  this.clientRec.clientLanIP = val.vnd_ip
                  this.clientRec.clientNetmask = val.vnd_msk
                  this.clientRec.vpnKey = val.aeskey
                  this.clientRec.vpnPasswd = val.passwd
                }
                let receivedmsg6BCS = changeArea6BCS.bind(this)
              } else {
                this.$message('Revoke failed')
              }
            })
          })
          .catch(() => {
            this.$message({
              type: 'info',
              message: 'Canceled'
            })
          })
      } else {
        this.$confirm(
          '返回上一步将撤销当前配置，是否确定返回?',
          '提示',
          {
            confirmButtonText: '确定',
            cancelButtonText: '取消',
            type: 'warning'
          }
        )
          .then(() => {
            let para = {}
            para.oper_type = 1
            kidvpnCancelAction(para).then(res => {
              if (res.data.code === 200) {
                this.$message({
                  type: 'success',
                  message: '已撤销!'
                })
                clearInterval(timerDel)
                this.close = 1
                this.clientSet = true
                this.clientSetted = false
                ws.send(6)
                ws.onmessage = function (evt) {
                  var obj = JSON.parse(evt.data)

                  receivedmsg6BCS(obj)
                }
                let changeArea6BCS = function ActionReceivedmsg6BCS(val) {
                  this.clientRec.severWanIP = val.public_ip
                  this.clientRec.clientLanIP = val.vnd_ip
                  this.clientRec.clientNetmask = val.vnd_msk
                  this.clientRec.vpnKey = val.aeskey
                  this.clientRec.vpnPasswd = val.passwd
                }
                let receivedmsg6BCS = changeArea6BCS.bind(this)
              } else {
                this.$message('撤销失败')
              }
            })
          })
          .catch(() => {
            this.$message({
              type: 'info',
              message: '已取消返回'
            })
          })
      }
    },
    createSever() {
      // 配置服务器
      let para = {}
      para.oper_type = 0
      para.vnd_ipaddr = this.severRec.severIP
      para.vnd_netmask = this.severRec.severNetmask
      para.passwd = this.severRec.vpnPasswd
      if (para.vnd_ipaddr !== '' && para.vnd_netmask !== '' && para.passwd !== '') {
        kidvpnConfigAction(para).then(res => {
          if (res.data.code === 200) {
            this.severShow.severWanIP = res.data.ipaddr
            this.severShow.severLanIP = res.data.vnd_ipaddr
            this.severShow.severNetmask = res.data.vnd_netmask
            this.severShow.severGateway = res.data.vnd_gateway
            this.severShow.vpnPasswd = res.data.passwd
            this.severShow.vpnKey = res.data.aeskey
            ws.send('2')
            ws.onmessage = function (evt) {
              var obj = JSON.parse(evt.data)
              receivedmsg2sSI(obj)
            }
            let changeArea2sSI = function ActionReceivedmsg2sSI(val) {
              let para = {}
              para.ipaddr = val.public_ip
              para.vnd_ipaddr = val.vnd_ip
              para.vnd_netmask = val.vnd_msk
              para.vnd_gateway = val.vnd_gw
              para.passwd = val.passwd
              para.aeskey = val.aeskey
              console.log('para' + para)
              kidvpnConfigSaveAction(para).then(res => {
                if (res.data.code === 200) {

                }
              })
            }
            let receivedmsg2sSI = changeArea2sSI.bind(this)
            setTimeout(() => {
              this.gotoSeverInfo()
            }, 3000)
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message('Create failure, please reconfigure')
            } else {
              this.$message('创建失败，请重新配置')
            }
          }
        })
      } else {
        if (this.$store.state.app.language === 'en') {
          this.$message('Please input all information')
        } else {
          this.$message('请完整输入信息')
        }
      }
    },
    gotoSeverInfo() {
      this.close = 1
      this.severSetted = true
      this.severSet = false
    },
    wanFinish() {
      // 外网配置完成
      ws.send('1')
      ws.onmessage = function (evt) {
        var obj = JSON.parse(evt.data)

        getNowWanPort(obj)
      }
      let changegetNowWanPortArea = function ActiongetNowWanPort(val) {
        this.selected = val.webname

        for (let i = 0; i < this.ports.length; i++) {
          this.ports[i].imgsrc = 'static/port2.png'
          this.ports[i].selected = 'false'
          if (this.ports[i].webname === this.selected) {
            this.ports[i].imgsrc = 'static/port3.png'
            this.ports[i].selected = 'true'
          }
        }
        this.ports[0].showtri = true
        this.nowLanPort = this.ports[0].webindex
        this.$forceUpdate()
      }
      let getNowWanPort = changegetNowWanPortArea.bind(this)
      this.getPortsInfo()
      setTimeout(() => {
        this.waiteLanPortInfo()
      }, 3000)
    },
    // 查询lan口信息要等待3s等待port信息更新完毕，否则会覆盖掉选中的数据
    waiteLanPortInfo() {
      this.active = 1
      this.close = 1
      this.selectLanPort = true
      this.wanConnected = false
    },
    backSelectWanMethod() {
      // 返回功能选择
      let bara = {}

      if (this.wanMethod === '1') {
        bara.oper_type = 1
      } else if (this.wanMethod === '2') {
        bara.oper_type = 2
      } else if (this.wanMethod === '3') {
        bara.oper_type = 3
      }
      bara.webname = this.selected
      if (this.func1 === true) {
        bara.kidvpn_client = 1
      } else if (this.func1 === false) {
        bara.kidvpn_client = 0
      }
      if (this.func2 === true) {
        bara.kidvpn_server = 1
      } else if (this.func2 === false) {
        bara.kidvpn_server = 0
      }
      if (this.func3 === true) {
        bara.arp_defense = 1
      } else if (this.func3 === false) {
        bara.arp_defense = 0
      }
      publicNetCancelAction(bara).then(res => {
        if (res.data.code === 200) {
          console.log('取消成功')
          this.close = 1
          this.selectWanMethod = true
          this.wanTest = false
          ws.send(1)
          ws.onmessage = function (evt) {
            var obj = JSON.parse(evt.data)

            rebackWanMethod(obj)
          }
          let changerebackWanMethodArea = function ActionrebackWanMethod(val) {
            this.selected = val.webname
            for (let i = 0; i < this.ports.length; i++) {
              if (this.ports[i].webname === val.webname) {
                this.ports[i].selected = 'true'
                this.ports[i].imgsrc = 'static/port3.png'
              } else {
                this.ports[i].selected = 'false'
                this.ports[i].imgsrc = 'static/port2.png'
              }
            }
            if (val.mode === 1) {
              this.wanMethod = '1'
              this.adslInfo.adslAccount = val.user
              this.adslInfo.adslpasswd1 = val.pwd
              this.adslInfo.adslpasswd2 = val.pwd
            } else if (val.mode === 2) {
              this.wanMethod = '2'
            } else if (val.mode === 3) {
              this.wanMethod = '3'
              this.staticIpInfo.staticIp = val.ip
              this.staticIpInfo.staticNetmask = val.netmask
              this.staticIpInfo.staticGateway = val.gateway
            }
            this.$forceUpdate()
          }
          let rebackWanMethod = changerebackWanMethodArea.bind(this)
        } else {
          if (this.$store.state.app.language === 'en') {
            this.$message('Configuration cancel failed, please check')
          } else {
            this.$message('配置取消失败，请检查')
          }
        }
      })
    },
    wanTestFinish() {
      // 外网测试完成
      this.close = 1
      this.wanConnected = true
      this.wanTest = false
    },

    changeLabel() {
      // 选择高级功能配置
      console.log('change')
      if (this.wanMethod === 1) {
        console.log('1')
        this.isstatic = true
        this.isadsl = false
      } else if (this.wanMethod === 2) {
        console.log('2')
        this.isadsl = true
        this.isstatic = true
      } else {
        console.log('3')
        this.isadsl = true
        this.isstatic = false
      }
    },
    backSelcetPort() {
      // 返回选择端口
      this.close = 1
      this.selectWanPort = true
      this.selectWanMethod = false
    },
    checkWanConnectStatus() {
      wanTestTimer = setInterval(this.getWanConnectStatus, 1000)
    },
    selectWanMethodFinish() {
      // 选择Wan接入方式配置完成
      console.log(this.wanMethod)
      console.log(this.staticIpInfo)

      let para = {}
      para.oper_type = this.wanMethod
      para.webname = this.selected
      console.log('func1' + this.func1)
      console.log('func2' + this.func2)
      console.log('func3' + this.func3)
      if (this.func1 === true) {
        para.kidvpn_client = 1
      } else if (this.func1 === false) {
        para.kidvpn_client = 0
      }
      if (this.func2 === true) {
        para.kidvpn_server = 1
      } else if (this.func2 === false) {
        para.kidvpn_server = 0
      }
      if (this.func3 === true) {
        para.arp_defense = 1
      } else if (this.func3 === false) {
        para.arp_defense = 0
      }
      if (para.oper_type === '1' || para.oper_type === '3' || para.oper_type === '2') {
        if (para.oper_type === '1') {
          if (this.adslInfo.adslpasswd1 === this.adslInfo.adslpasswd2) {
            this.adslInfo.passwd = this.adslInfo.adslpasswd1
            para.account = this.adslInfo.adslAccount
            para.passwd = this.adslInfo.passwd
            para.oper_type = 1
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message({
                type: 'error',
                message: 'The two passwords you typed do not match!'
              })
            } else {
              this.$message({
                type: 'error',
                message: '两次输入密码不一致！'
              })
            }
          }
        } else if (para.oper_type === '3') {
          para.ipaddr = this.staticIpInfo.staticIp
          para.netmask = this.staticIpInfo.staticNetmask
          para.gateway = this.staticIpInfo.staticGateway
          para.oper_type = 3
        } else if (para.oper_type === '2') {
          para.oper_type = 2
        }
        publicNetSetAction(para).then(res => {
          if (res.data.code === 200) {
            this.close = 1
            this.wanTest = true
            this.selectWanMethod = false
            setTimeout(() => {
              this.checkWanConnectStatus()
            }, 3000)
          } else if (res.data.code === 300) {
            if (this.$store.state.app.language === 'en') {
              this.$message('This IP is in the same network segment as other network ports, configure failed')
            } else {
              this.$message('该IP与其他网口处于同一网段，设置失败')
            }
          } else {
            if (this.$store.state.app.language === 'en') {
              this.$message('Configuration information is incorrect. Please reconfigure it')
            } else {
              this.$message('配置信息有误，请重新配置')
            }
          }
        })
      } else {
        if (this.$store.state.app.language === 'en') {
          this.$message('Please select Wan port access mode')
        } else {
          this.$message('请选择Wan口接入方式')
        }
      }
    },
    getWanConnectStatus() {
      console.log('jiance')
      ws.send('1')
      ws.onmessage = function (evt) {
        var obj = JSON.parse(evt.data)

        receivedmsg1pNSA(obj)
      }
      let changeArea1pNSA = function ActionReceivedmsg1pNSA(val) {
        if (val.internet === 'yes') {
          this.wanTestFinish()
          clearInterval(wanTestTimer)

          this.showTestButton = false
        } else if (val.internet === 'no') {
          console.log('配置失败')

          this.showTestButton = true
        }
      }
      let receivedmsg1pNSA = changeArea1pNSA.bind(this)
    },
    selectWanPortFinish() {
      // 选择端口完成

      if (this.selected === '') {
        if (this.$store.state.app.language === 'en') {
          this.$message('Please select Wan port')
        } else {
          this.$message('请选择Wan口')
        }
      } else {
        this.close = 1
        this.selectWanMethod = true
        this.selectWanPort = false
      }
    },
    backSelcetFunc() {
      // 返回选择高级功能配置
      this.close = 1
      this.showselectFunc = true
      this.selectWanPort = false
    },
    pushToWanINEX(item) {
      // 获取选中Wan网口名
      console.log(this.ports)
      console.log('item')
      console.log(item)
      if (item.selected === 'false') {
        for (let i = 0; i < this.ports.length; i++) {
          this.ports[i].imgsrc = 'static/port2.png'
          this.ports[i].selected = 'false'
        }
        item.selected = 'true'
        this.selected = item.webname
        item.imgsrc = 'static/port3.png'
      } else if (item.selected === 'true') {
        item.selected = 'false'
        this.selected = ''
        item.imgsrc = 'static/port2.png'
      }
      this.$forceUpdate()
      // this.selectUrl()
    },
    pushToLanINEX(item) {
      // 获取选中Lan网口名
      if (item.webname === this.selected) {
        if (this.$store.state.app.language === 'en') {
          this.$message({
            type: 'error',
            message: 'This port has been set as WAN port. Please re-select it!'
          })
        } else {
          this.$message({
            type: 'error',
            message: '该端口已设置为WAN口，请重新选择!'
          })
        }
      } else {
        console.log('this.selected')
        console.log(this.selected)
        localDhcpValueGetAction().then(res => {
          if (res.data.code === 200) {
            for (let i = 0; i < res.data.data.length; i++) {
              if (item.webname === res.data.data[i].webname) {
                this.DHCPIP = res.data.data[i].start
                this.DHCPStoreIP = res.data.data[i].end
              }
            }
          }
        })
        if (this.ports[item.webindex].lanMethod === 'DHCP') {
          this.lanMethod = '2'
        } else if (this.ports[item.webindex].lanMethod === 'Static') {
          this.lanMethod = '3'
        } else {
          this.lanMethod = '0'
        }
        if (item.selected === 'true') {
          this.setLan = true
          this.isDHCP = false
          this.isStatic = false
        } else {
          this.setLan = false
          this.isDHCP = true
          this.isStatic = true
        }
        console.log(this.ports)
        console.log('item')
        console.log(item)
        console.log(item.webindex)
        for (let i = 0; i < this.ports.length; i++) {
          if (this.ports[i].webindex !== item.webindex) {
            this.ports[i].showtri = false
            console.log(this.ports[i].webname)
            console.log(this.ports[i].showtri)
          } else if (this.ports[i].webindex === item.webindex) {
            this.ports[i].showtri = true
            this.thisStaticIP = this.ports[i].ip
            this.thisStaticNetmask = this.ports[i].netmask
            this.thisStaticGateway = this.ports[i].gateway
            this.nowLanPort = item.webindex
          }
        }

        console.log(this.ports)

        console.log(this.ports[item.webindex].webname + this.ports[item.webindex].showtri)
        console.log(this.ports)
        // 可选择多个端口发送
        /* if (this.ports[item.webindex].lanMethod === 'DHCP') {
        this.lanMethod = '2'
      } else if (this.ports[item.webindex].lanMethod === 'Static') {
        this.lanMethod = '3'
      } else {
        this.lanMethod = '0'
      }
      if (item.selected === 'true') {
        this.setLan = true
      } else {
        this.setLan = false
      }
      console.log(this.ports)
      console.log('item')
      console.log(item)
      this.nowLanPort = item.webindex
      this.ports[item.webindex].showtri = true
      for (let i = 0; i < this.ports.length; i++) {
        if (i !== item.webindex) {
          this.ports[i].showtri = false
          console.log(this.ports[i].showtri)
        }
      } */

        this.$forceUpdate()
      }
      // this.selectUrl()
    },

    selectUrl(para) {
      console.log('777')
      // if(para === "空闲")
      if (para === 'false') {
        return 'static/port2.png'
      } else {
        return 'static/port3.png'
      }
    },
    getPortsInfo: function() {
      // getPorts().then((res) => {
      //   this.ports = res.data.ports;
      //   console.log(res);
      //   console.log(this.ports);
      // });

      getPorts().then(res => {
        let checked = res.data.code
        if (checked !== 200) {
          this.$store.dispatch('setAdaptive', checked)
          this.$router.push('ports')
        }

        this.ports = res.data.interfaces
        for (let i = 0; i < this.ports.length; i++) {
          this.ports[i].selected = 'false'
          this.ports[i].imgsrc = 'static/port2.png'

          if (this.ports[i].function === 'wan') {
            this.ports[i].selected = 'true'
            this.ports[i].imgsrc = 'static/port3.png'
            this.selected = this.ports[i].webname
          }
        }
      })
    },
    funcSubmit() {
      this.close = 1
      console.log(this.func1)
      console.log(this.func2)
      console.log(this.func3)
      this.setSteps()
      this.active = 0
      this.selectWanPort = true
      this.showselectFunc = false
      this.getPortsInfo()
    },
    setSteps() {                                 // 设置步骤条
      if (this.func1 === true && this.func2 === true && this.func3 === true) {
        this.funcCount = 3
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            ' VPNClient',
            'VPNSever',
            'ARPDefense'
          ]
        } else {
          this.step = [
            '外网接入',
            '内网接出',
            ' VPN客户端',
            'VPN服务器',
            '内网防御'
          ]
        }
      } else if (
        this.func1 === true &&
        this.func2 === false &&
        this.func3 === true
      ) {
        this.funcCount = 2
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            ' VPNClient',
            'ARPDefense'
          ]
        } else {
          this.step = [
            '外网接入',
            '内网接出',
            ' VPN客户端',
            '内网防御'
          ]
        }
      } else if (
        this.func1 === true &&
        this.func2 === false &&
        this.func3 === false
      ) {
        this.funcCount = 1
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            ' VPNClient'
          ]
        } else {
          this.step = ['外网接入', '内网接出', ' VPN客户端']
        }
      } else if (
        this.func1 === false &&
        this.func2 === true &&
        this.func3 === false
      ) {
        this.funcCount = 1
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            'VPNSever'
          ]
        } else {
          this.step = ['外网接入', '内网接出', 'VPN服务器']
        }
      } else if (
        this.func1 === false &&
        this.func2 === false &&
        this.func3 === true
      ) {
        this.funcCount = 1
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            'ARPDefense'
          ]
        } else {
          this.step = ['外网接入', '内网接出', '内网防御']
        }
      } else if (
        this.func1 === true &&
        this.func2 === true &&
        this.func3 === false
      ) {
        this.funcCount = 2
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            ' VPNClient',
            'VPNSever'
          ]
        } else {
          this.step = [
            '外网接入',
            '内网接出',
            ' VPN客户端',
            'VPN服务器'
          ]
        }
      } else if (
        this.func1 === false &&
        this.func2 === false &&
        this.func3 === false
      ) {
        this.funcCount = 0
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet'
          ]
        } else {
          this.step = ['外网接入', '内网接出']
        }
      } else if (
        this.func1 === false &&
        this.func2 === true &&
        this.func3 === true
      ) {
        this.funcCount = 1
        if (this.$store.state.app.language === 'en') {
          this.step = [
            'Internet',
            'Intranet',
            'VPNSever',
            'ARPDefense'
          ]
        } else {
          this.step = ['外网接入', '内网接出', ' VPN服务器', '内网防御']
        }
      }
    },
    // 退出登录
    logout: function() {
      let _this = this
      this.$confirm('确认退出吗?', '提示', {
        type: 'warning'
      })
        .then(() => {
          let para = sessionStorage.getItem('user')
          logout(para).then(res => {
            if (res.data.code === 200) {
              sessionStorage.removeItem('user')
              _this.$router.push('/login')
            }
          })
        })
        .catch(error => {
          console.log(error)
        })
    },
    back: function() {
      this.$router.push('/ModeChoose')
    },
    // 折叠导航栏
    collapse: function() {
      /*
        if(this.collapsed === false) {
          setTimeout((() => {
            this.collapsed = !this.collapsed;
          }), 200);
          this.isCollapse = !this.isCollapse;
        }
        else{
          this.collapsed = !this.collapsed;
          setTimeout((() => {
            this.isCollapse = !this.isCollapse;
          }), 100);
        }
        */
      this.collapsed = !this.collapsed
      this.isCollapse = !this.isCollapse
    },

    chooseLang(val) {
      this.lang = val
    },

    generateTitle,

    getInfo() {
      this.update()
      setInterval(() => {
        this.update()
        this.$store.dispatch('pushSystemData', this.data)
      }, 5000)
    },

    async update() {
      let para = {}
      para.page = 1
      para.type = 5
      await getMonitorInfo(para)
        .then(res => {
          if (res.data.code === 200) {
            this.data.upStorage = res.data.data[0].recv_rate
            this.data.downStorage = res.data.data[0].send_rate
            this.data.up = conversion(res.data.data[0].recv_rate)
            this.data.down = conversion(res.data.data[0].send_rate)
            // console.log(this.data.up)
            // this.$store.dispatch('pushSystemData', this.data)
          }
        })
        .catch(error => {
          console.log(error)
        })
      let para1 = {}
      para1.type = 6
      await getMonitorInfo(para1)
        .then(res => {
          if (res.data.code === 200) {
            this.data.cpu = res.data.cpu_usage.toFixed(0)
            this.data.memory = res.data.memory_usage.toFixed(0)
            this.data.userNum = res.data.online_users
          }
        })
        .catch(error => {
          console.log(error)
        })
      // this.$store.dispatch('pushSystemData', this.data)
    }

  },

  mounted() {
    if (this.$store.state.app.language === 'en') {
      this.severImage = 'static/server1en.png'
      this.clientImage = 'static/client1en.png'
      this.defendImage = 'static/defend1en.png'
      this.wanImage = 'static/wanen.png'
      this.lanImage = 'static/lanen.png'
    } else {
      this.severImage = 'static/server1zh.png'
      this.clientImage = 'static/client1zh.png'
      this.defendImage = 'static/defend1zh.png'
      this.wanImage = 'static/wanzh.png'
      this.lanImage = 'static/lanzh.png'
    }
    websocketIpGetAction().then(res => {
      this.websocketip = res.data.websockip
      console.log(this.websocketip)
      ws = new WebSocket('ws://' + this.websocketip + ':8001')
      ws.addEventListener('close', function(event) {
        console.log('关掉了websock')
      })
      ws.addEventListener('onopen', function(event) {
        console.log('开启了websock')
      })
      ws.onopen = function() {
        // Web Socket 已连接上，使用 send() 方法发送数据
        // ws.send(3)
      }
    })
    this.getInfo()

    this.getPortsInfo()
    setTimeout(this.start, 500)
    // 打开dialog
  },
  beforeDestroy() {
    ws.close()
    // wyk清除定时器

    // ws.onclose = function(evt) {
    //  console.log('Connection closed.')      // 关闭websock
    // }
  }
}
</script>

<style scoped lang="scss">
.text-wrapper {
  word-break: break-all;
  word-wrap: break-word
}
.container {
  position: absolute;
  top: 0;
  bottom: 0;
  width: 100%;
  .header {
    height: 60px;
    line-height: 60px;
    background: #3c8dbc;
    color: #fff;
    .userinfo {
      text-align: right;
      padding-right: 35px;
      float: right;
      .userinfo-inner {
        cursor: pointer;
        color: #fff;
        img {
          width: 40px;
          height: 40px;
          border-radius: 20px;
          margin: 10px 0 10px 10px;
          float: right;
        }
      }
    }
    .situationMode {
      right: 100px;
      padding-right: 35px;
      cursor: pointer;
      color: #fff;
    }
    .logo {
      //width:230px;
      height: 60px;
      font-size: 22px;
      padding-left: 20px;
      padding-right: 20px;
      border-color: rgba(238, 241, 146, 0.3);
      border-right-width: 1px;
      border-right-style: solid;
      img {
        width: 40px;
        float: left;
        margin: 10px 10px 10px 18px;
      }
      .txt {
        color: #fff;
      }
    }
    .logo-width {
      width: 200px;
    }
    .logo-collapse-width {
      width: 65px;
    }
    .tools {
      padding: 0 23px;
      width: 14px;
      height: 60px;
      line-height: 60px;
      cursor: pointer;
    }
    .lang {
      cursor: pointer;
      color: white;
    }
    .selected {
      color: goldenrod;
    }
  }
}
.router {
  text-align: center;
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 10px;
  margin-left: 90px;
  margin-bottom: 0px;
  background-color: lightgoldenrodyellow;
  display: flex;
  flex-wrap: wrap;
}
.defaultFunc {
  text-align: center;
width: 85%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 10px;
  margin-left: 50px;
  
  margin-bottom: 0px;
  background-color: lightgoldenrodyellow;
  display: flex;
  flex-wrap: wrap;
}
.selectFunc {
  text-align: center;
  width: 35%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 10px;
  // margin-left: 30px;
  margin-right: 50px;
  margin-bottom: 0px;
  background-color: lightgoldenrodyellow;
  display: flex;
  flex-wrap: wrap;
}
.info {
  text-align: left;
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 40px;
  margin-left: 85px;
  background-color: rgb(226, 239, 217);
  //display: flex;
  //flex-wrap: wrap;
}
.wanInfo {
  text-align: left;
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 20px;
  margin-left: 85px;
  background-color: rgb(226, 239, 217);
  //display: flex;
  //flex-wrap: wrap;
}
.lanSelectinfo {
  text-align: left;
  width: 75%;
  border: 0px solid lightgrey;
  border-radius: 10px;
  margin-top: 22px;
  margin-left: 90px;
  height: 80%;
  background-color: rgb(226, 239, 217);
  //display: flex;
  //flex-wrap: wrap;
}
.infoSever {
    text-align: left;
  width: 75%;
  border: 1px solid lightgrey;
  border-radius: 10px;
  margin-top: 20px;
  margin-left: 85px;
  background-color: rgb(226, 239, 217);
}
.laninfo{
 
 
  border: 0px solid lightgrey;
  
 border-radius: 10px;
 
  background-color: rgb(219, 219, 219);
}



.button {
  background-color: rgb(197, 90, 17);
  color: #ffffff;
}
.port {
  width: 60px;
  height: 50px;
  text-align: center;
  margin-top: 25px ;
  margin-left: 10px ;
margin-right: 10px ;
  margin-bottom: 35px ;
  flex-grow: 1;
  border-radius: 5px;
}
.tooltip-content {
  min-width: 150px;
  border: 0;
}
.text-area p {
  margin-top: 0px;
  margin-left: 0px;
  margin-bottom: 8px;
  // margin: 0;
}

.text-area .text {
  margin-top: 0px;
  margin-left: 0px;
  margin-bottom: 8px;
}
.text-areaLan p {
  margin-top: 0px;
  margin-left: 0px;
  margin-bottom: 8px;
  // margin: 0;
}

.text-areaLan .text {
  margin-top: 0px;
  margin-left: 0px;
  margin-bottom: 8px;
}
.el-step__title {
    max-width: 100%;
    word-break: break-all;
}
.middleLeft {
  margin-top: 20px;
  height: 240px;
  border: 1px solid lightgrey;
  font-family: Roboto-Thin;
  .middleLeftTop {
    height: 100px;
    padding-left: 20px;
    .middleLeftTopText {
      display: flex;
      .middleLeftTopTextChild {
        width: 25%;
        p {
          margin: 0 auto;
        }
      }
    }
  }
}

.el-dialog {
      
    background: rgb(70, 114, 196);
    
  
  .el-dialog__body{
  color: black;
  font-size: 15px;
  font-weight: 600
}
}


 .el-dialog__text{
  color: black;
  font-size: 14px;
  font-weight: 500
}
.triangular {
  border: 10px solid white;
  border-bottom-color: rgb(226, 239, 217);
  width: 0;
  height: 0;
  margin: 0 auto;
  margin-top: 0;
}
</style>
<style lang="scss">
.el-steps--simple {
    padding: 10px 1%;
    border-radius: 4px;
    background: #f5f7fa;
}


//.el-carousel__mask, .el-cascader-menu, .el-cascader-menu__item.is-disabled:hover, .el-collapse-item__header, .el-collapse-item__wrap {
//  background-color: rgb(226, 239, 217) !important;
//}
.green .el-carousel__mask{              //绿色背景
  background-color: rgb(226, 239, 217) !important;
}
.green .el-cascader-menu{
  background-color: rgb(226, 239, 217) !important;
}
.green .el-cascader-menu__item.is-disabled:hover{
  background-color: rgb(226, 239, 217) !important;
}
.green .el-collapse-item__header{
  border-bottom: 1px solid rgb(226, 239, 217);
  background-color: rgb(226, 239, 217) !important;
}
.green .el-collapse-item__wrap{
  background-color: rgb(226, 239, 217) !important;
   border-bottom: 1px solid rgb(226, 239, 217);
}
.green .el-collapse {
    border-top: 0px solid rgb(226, 239, 217);
    border-bottom: 0px solid rgb(226, 239, 217);
}
.grey .el-carousel__mask{                        //灰色背景
  background-color: rgb(219, 219, 219) !important;
}
.grey.el-dialog .el-cascader-menu{
  background-color: rgb(219, 219, 219) !important;
}
.grey .el-cascader-menu__item.is-disabled:hover{
  background-color: rgb(219, 219, 219) !important;
}
.grey .el-collapse-item__header{
  border-bottom: 1px solid rgb(219, 219, 219);
  background-color: rgb(219, 219, 219) !important;
}
.grey .el-collapse-item__wrap{
  background-color: rgb(219, 219, 219) !important;
  border-bottom: 1px solid rgb(219, 219, 219);
}
.grey .el-collapse {
    border-top: 0px solid rgb(219, 219, 219);
    border-bottom: 0px solid rgb(219, 219, 219);
}

.el-step__title {
    max-width: 80%;
    word-break: break-all;
}
.el-step.is-simple:not(:last-of-type) .el-step__title {
    max-width: 80%;
    word-break: break-all;
}
.el-step.is-simple .el-step__title {
    font-size: 13px;
    line-height: 20px;
     max-width: 100%;
}
//.thisdialogstyle .el-dialog {
 //   position: relative;
 //   margin: 0 auto 50px;
   // background: rgb(70, 114, 196);
  //  border-radius: 2px;
  //  box-shadow: 0 1px 3px rgb(0,0,0,.3);
 //   box-sizing: border-box;
 //   width: 50%;
 //   .el-dialog__body{
 // color: black;
 // font-size: 15px;
 // font-weight: 600
//}
//}

</style>