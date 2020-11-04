<template>
  <div class="backgroundDiv" >
    <div class="pic">
      <img src="../../static/logo.png" />
    </div>
    <el-form :model="ruleForm" :rules="rules2" ref="ruleForm" label-position="left" label-width="0" class="demo-ruleForm login-container">
      <p class="title">OpenRT</p>
      <el-form-item prop="account">
        <el-input v-on:keyup.enter.native="handleSubmit2()" style="color: #ffffff" type="text" v-model="ruleForm.account" auto-complete="off" placeholder="账号"></el-input>
      </el-form-item>
      <el-form-item prop="checkPass">
        <el-input v-on:keyup.enter.native="handleSubmit2()" type="password" v-model="ruleForm.checkPass" auto-complete="off" placeholder="密码"></el-input>
      </el-form-item>
      <div style="display: inline;">
        <!-- <el-checkbox v-model="checked" checked class="remember">记住密码</el-checkbox> -->
        <!--<router-link to="/CreatePasswd">忘记密码</router-link>-->
        <el-tooltip placement="bottom" effect="light">
          <div slot="content" style="text-align: right;">请在控制台<br/>更改密码</div>
          <el-button type="text" style="text-align: right;">忘记密码？</el-button>
        </el-tooltip>
      </div>
      <el-form-item style="width:100%;">
        <el-button type="primary" style="width:100%;" @keyup.enter.native="handleSubmit2()" @click.native.prevent="handleSubmit2" :loading="logining">登录</el-button>
        <!--<el-button @click.native.prevent="handleReset2">重置</el-button>-->
      </el-form-item>
    </el-form>
    <!--wyk导航条-->
    <div class="belowInfo" style="text-align: center;">
      <p style="display:inline-block; vertical-align:text-top;">{{"© 2018 OpenRT"}}{{" "}}{{"|"}}</p>
      <p style="display:inline-block; vertical-align:text-top;"><a href="http://www.acoinfo.com" target="_blank" >{{"官方网站"}}{{" "}}{{"|"}}</a></p>
      <p style="display:inline-block; vertical-align:text-top;"><a href="http://bbs.sylixos.com/forum.php" target="_blank">{{"官方社区"}}{{" "}}{{"|"}}</a></p>
      <p class="loginbutton" style="display:inline-block; vertical-align:text-top;"><el-button type="text" @click="getwechat"  >{{"官方微信"}}{{" "}}{{"|"}}</el-button></p>
      <p style="display:inline-block; vertical-align:text-top;">{{"常见问题"}}{{" "}}{{"|"}}</p>
      <p style="display:inline-block; vertical-align:text-top;">{{"服务热线 025-83127300"}}</p>
    </div>
    <!--微信二维码wyk-->
    <el-dialog  title="翼辉信息官方微信" width="30%" height="30%" :visible="wechat" @close='closewechat' :close-on-click-modal="false">

        <div  style="text-align: center;">微信“扫一扫”下方二维码即可添加</div>
          <div style="margin-bottom: 30px;text-align: center;margin-top: 20px" >
            <img src="static/wechat.png"  height="200" width="200"  />
          </div>
          
      </el-dialog>
  </div>
</template>

<script>
import { requestLogin } from '../api/api'
// import NProgress from 'nprogress'
import md5 from 'js-md5'
export default {
  created() {
    var _self = this

    document.onkeydown = function(e) {
      var key = window.event.keyCode

      if (key === 13) {
        _self.handleSubmit2()
      }
    }
  },
  beforeDestroy() {
    document.onkeydown = function(e) {
      var key = window.event.keyCode

      if (key === 13) {

      }
    }
  },
  data() {
    return {
      wechat: false,    // 官方微信二维码wyk
      logining: false,
      ruleForm: {
        account: 'admin',
        checkPass: 'admin'
      },
      rules2: {
        account: [
          { required: true, message: '请输入账号', trigger: 'blur' }
          // { validator: validaePass }
        ],
        checkPass: [
          { required: true, message: '请输入密码', trigger: 'blur' }
          // { validator: validaePass2 }
        ]
      },
      checked: true
    }
  },
  methods: {
    getwechat() {                       // wyk显示弹出框
      this.wechat = true
    },
    closewechat() {                     // wyk关闭弹出框
      this.wechat = false
    },
    handleReset2() {
      this.$refs.ruleForm.resetFields()
    },
    // handleSubmit2() {
    //   const user = [
    //     {
    //       id: 1,
    //       username: 'admin',
    //       password: '123456',
    //       // avatar: 'https://raw.githubusercontent.com/taylorchen709/markdown-images/master/vueadmin/user.png',
    //       name: 'OpenRT'
    //     }
    //   ];
    //   sessionStorage.setItem('user', JSON.stringify(user));
    //   //let _this = this;
    //   this.$refs.ruleForm.validate((valid) => {
    //     if (valid) {
    //       //_this.$router.replace('/table');
    //       this.logining = true;
    //       //NProgress.start();
    //       let loginParams = {username: this.ruleForm.account, password: this.ruleForm.checkPass};
    //       requestLogin(loginParams).then(data => {
    //         console.log("login response:"+data);
    //         this.logining = false;
    //         //NProgress.done();
    //         //let { msg, code, user } = data;
    //         let { msg, code } = data;
    //         if (code !== 200) {
    //           this.$message({
    //             message: msg,
    //             type: 'error'
    //           });
    //         }
    //         else {
    //           sessionStorage.setItem('user', JSON.stringify(user));
    //           //this.$router.push({ path: '/table' });
    //           this.$router.push({ path: '/modeChoose' });
    //         }

    //       });
    //     } else {
    //       console.log('error submit!!');
    //       return false;
    //     }
    //   });
    // },
    handleSubmit2() {
      // const user = [
      //   {
      //     id: 1,
      //     username: 'admin',
      //     password: '123456',
      //     // avatar: 'https://raw.githubusercontent.com/taylorchen709/markdown-images/master/vueadmin/user.png',
      //     name: 'OpenRT'
      //   }
      // ]
      // sessionStorage.setItem('user', JSON.stringify(user))
      // this.$router.push('/ModeChoose')

      let para = {}
      para.username = this.ruleForm.account
      // para.passwd = this.ruleForm.checkPass
      para.passwd = md5(this.ruleForm.checkPass)

      requestLogin(para)
        .then(res => {
          if (res.data.code === 200) {
            console.log(JSON.stringify(res.data))
            let user = {}
            user.id = res.data.id
            user.userName = res.data.username
            sessionStorage.setItem('user', JSON.stringify(user))
            this.$router.push('/ModeChoose')
            // this.$router.push({ path: '/systemStatus' })
          } else if (res.data.code === 1000) {
            // alert('ready to jump to createword')
            this.$router.push('/CreatePasswd')
          } else {
            this.$message({
              message: '错误',
              type: 'error'
            })
          }
        })
    }
  }
}
</script>

<style lang="scss" scoped>
.backgroundDiv {
  position: absolute;
  top: 0;
  bottom: 0;
  width: 100%;
  background: url(../../static/bg.jpg);
  background-size: 100% 100%;
}

.pic {
  opacity: 0.7;
  text-align: center;
  margin-top: 180px;
}
.belowInfo{
  font-family: 微软雅黑;
  font-size: 10px;
  color: white;
  a{text-decoration: none;color:white;}
  a:hover{text-decoration: none;color:#09f;}
  .el-button--text {
    color: white;
    background: 0 0;
    padding-left: 0;
    padding-right: 0;
}
.el-button--text:hover {
    color: #09f;
    background: 0 0;
    padding-left: 0;
    padding-right: 0;
}
.el-button {
    line-height: 0;
    white-space: nowrap;
    cursor: pointer;
    text-align: center;
    font-size: 10px;
}
}

.login-container {
  /*box-shadow: 0 0px 8px 0 rgba(0, 0, 0, 0.06), 0 1px 0px 0 rgba(0, 0, 0, 0.02);*/
  -webkit-border-radius: 5px;
  border-radius: 5px;
  -moz-border-radius: 5px;
  background-clip: padding-box;
  margin: 20px auto;
  width: 320px;
  padding: 35px 35px 15px 35px;
  //background: #fff;
  background: grey;
  border: 1px solid #eaeaea;
  box-shadow: 0 0 25px #cac6c6;
  opacity: 0.7;
  color: white;
  .title {
    margin: 20px auto;
    text-align: center;
    //color: #505458;
    color: white;
    font-size: 20px;
    font-weight: bold;
  }
  .remember {
    margin: 0 0 35px 0;
    width: 75%;
    color: white;
  }
  input:-ms-input-placeholder {
    color: white;
  }
  input::-webkit-input-placeholder {
    color: white;
  }
 
  
}
</style>
<style lang="scss">
 .loginbutton .el-button--text{
    color: #fff;
    background: 0 0;
    padding-left: 0;
    padding-right: 0;
    padding-top: 0;
}
</style>