<template>
  <div class="backgroundDiv">
    <div class="pic">
      <img src="../../static/logo.png" />
    </div>
    <el-form class="login-container">
      <p class="title">{{ $t('ModeChoose.modechoose') }}</p>
      <el-form-item style="width:100%;">
        <el-button type="primary" style="width:100%;" @click.native.prevent="modeSubmit1" :loading="logining">{{ $t('home.scenemode') }}</el-button><!--wyk修改情景模式为简易模式-->
        <!--<el-button @click.native.prevent="handleReset2">重置</el-button>-->
      </el-form-item>
      <el-form-item style="width:100%;">
        <el-button type="primary" style="width:100%;" @click.native.prevent="modeSubmit2" :loading="logining">{{ $t('home.professional') }}</el-button>
        <!--<el-button @click.native.prevent="handleReset2">重置</el-button>-->
      </el-form-item>
    </el-form>
  </div>
</template>

<script>
import {situationStatResetAction} from '../api/api.js'
export default {
  name: 'ModeChoose',
  data() {
    return {
      logining: false
    }
  },
  methods: {
    modeSubmit1() {
      this.$router.push({ path: '/situationMode' })
    },
    modeSubmit2() {
      situationStatResetAction().then(res => {
        if (res.data.code === 200) {

        } else {
          if (this.$store.state.app.language === 'zh') { this.$message('情景模式状态复位失败') }  else {
            this.$message('Reset Failed')
          }      // wyk1112添加复位情景模式状态
        }
        this.$router.push({ path: '/systemStatus' })
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

  .pic{
    opacity:0.7;
    text-align: center;
    margin-top: 180px;
  }

  .login-container {
    /*box-shadow: 0 0px 8px 0 rgba(0, 0, 0, 0.06), 0 1px 0px 0 rgba(0, 0, 0, 0.02);*/
    -webkit-border-radius: 5px;
    border-radius: 5px;
    -moz-border-radius: 5px;
    background-clip: padding-box;
    margin: 20px auto;
    width: 350px;
    padding: 35px 35px 15px 35px;
    background: grey;
    border: 1px solid #eaeaea;
    box-shadow: 0 0 25px #cac6c6;
    color: white;
    opacity:0.7;
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
    }
  }
</style>
