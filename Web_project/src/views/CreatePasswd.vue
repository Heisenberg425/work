<template>
	<div class="backgroundDiv">
		<el-form ref="form" :model="form" label-position="left" label-width="0px" class="demo-ruleForm login-container">
			<h3 class="title">>{{ $t('CreatePasswd.changePasswd') }}</h3>
      <!-- wyk1011首次登陆bug-->
			<!--el-form-item prop="username">                            
				<el-input v-model="form.username" type="text" auto-complete="off" placeholder="账号"></el-input>
			</el-form-item-->
			<el-form-item prop="new_passwd">
				<el-input v-model="form.new_passwd" type="password" auto-complete="off"  :placeholder="$t('placeholder.pw2')"></el-input>
			</el-form-item>
			<el-form-item prop="new_passwd2">
				<el-input v-model="form.new_passwd2" type="password" auto-complete="off"  :placeholder="$t('placeholder.pw3')"></el-input>
			</el-form-item>
			<el-form-item style="width:100%;">
				<el-button @click="submit" type="primary" style="width:100%;" :loading="logining">{{ $t('CreatePasswd.submit') }}</el-button>
				<!--<el-button @click.native.prevent="handleReset2">重置</el-button>-->
			</el-form-item>
		</el-form>
		</div>
</template>

<script>
import { changePasswordAction } from '../api/api.js'
import md5 from 'js-md5'
export default {
  name: 'CreatePasswd',
  data() {
    return {
      form: {
        username: 'admin',
        passwd: 'admin',
        new_passwd: '',
        new_passwd2: ''
      }
    }
  },
  methods: {
    submit() {
      if (this.form.new_passwd === this.form.new_passwd2) {
        let para = Object.assign({}, this.form)
        para.passwd = md5(para.passwd)
        para.new_passwd = md5(para.new_passwd)
        changePasswordAction(para)
          .then(res => {
            if (res.data.code === 200) {
              this.$router.push('/ModeChoose')
            }
          })
          .catch(err => {
            console.log(err)
          })
        this.$refs.form.resetFields()
      } else {
        this.$message({
          message: '密码输入不一致',
          type: 'error'
        })
        this.$refs.form.resetFields()
      }
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
.login-container {
  /*box-shadow: 0 0px 8px 0 rgba(0, 0, 0, 0.06), 0 1px 0px 0 rgba(0, 0, 0, 0.02);*/
  -webkit-border-radius: 5px;
  border-radius: 5px;
  -moz-border-radius: 5px;
  background-clip: padding-box;
  margin: 180px auto;
  width: 350px;
  padding: 35px 35px 15px 35px;
  background: #fff;
  border: 1px solid #eaeaea;
  box-shadow: 0 0 25px #cac6c6;
  .title {
    margin: 0px auto 40px auto;
    text-align: center;
    color: #505458;
  }
  .remember {
    margin: 0px 0px 35px 0px;
    width: 48%;
  }
}
</style>