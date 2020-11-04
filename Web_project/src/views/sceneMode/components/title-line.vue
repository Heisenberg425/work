<template>
  <div class="title_line">
    <el-row>
      <el-col :span="7" class="router">{{ routerName }}</el-col>
      <el-col :span="1"/>
      <el-col :span="12">
        <span class="title cursor" :class="{ is_choosen: zh }" @click="chooseLang('zh')">中文</span>
        <span>/</span>
        <span class="title" :class="{ is_choosen: !zh }" @click="chooseLang('en')">EN</span>
      </el-col>
<!-- 
      <el-col :span="3">
        <span class="cursor" @click="reset">{{ $t('sceneRouter.reset') }}</span>
        wyk1113添加复位按钮
        <el-dropdown trigger="hover">
          <span class="cursor user_name">{{ $t('sceneRouter.sceneMode') }}</span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="jumpToMode">{{$t('home.professional')}}</el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
      </el-col> -->

      <el-col :span="4" class="last_row">
        <span class="cursor" style="margin-left:20px" @click="reset">{{ $t('sceneRouter.reset') }}</span>
        <!--wyk1113添加复位按钮-->
        <el-dropdown trigger="hover" style="margin-right:15px;margin-left:40px">
          <span class="cursor user_name">{{ $t('sceneRouter.sceneMode') }}</span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="jumpToMode">{{$t('home.professional')}}</el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
        <el-dropdown style="float:right;margin-right:20px">
          <span class="user_name cursor">{{ sysUserName }}</span>
          <img :src="sysUserAvatar">
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="signout">{{ $t('sceneRouter.logout') }}</el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
      </el-col>
    </el-row>
  </div>
</template>

<script>
import { logout, websocketIpGetAction, getVersion } from "@/api/api.js";
var ws; // websocket
export default {
  data() {
    return {
      websocketip: "", // websocketIP
      routerName: "",
      zh: true,

      sysUserName: "default",
      sysUserAvatar: ""
    };
  },
  computed: {
    lang: {
      get() {
        // console.log('get')
        return this.$store.state.app.language;
      },
      set(lang) {
        // console.log('set')
        this.$i18n.locale = lang;
        this.$store.dispatch("setLanguage", lang);
      }
    }
  },
  methods: {
    chooseLang(val) {
      if (this.$store.state.app.language !== val) {
        this.$i18n.locale = val;
        this.$store.dispatch("setLanguage", val);
        if (val === "zh") {
          this.zh = true;
        } else {
          this.zh = false;
        }
      }
    },

    jumpToMode() {
      this.$router.push("/systemStatus");
    },
    reset() {
      // wyk1113添加重启提示
      if (this.$store.state.app.language === "zh") {
        this.$confirm(
          "复位将删除所有内容，并重启路由器，是否确定复位?",
          "提示",
          {
            confirmButtonText: "确定",
            cancelButtonText: "取消",
            type: "warning"
          }
        )
          .then(() => {
            this.$message({
              type: "success",
              message: "已复位配置!"
            });
            this.$router.push("/Login");
            setTimeout(() => {
              ws = new WebSocket("ws://" + this.websocketip + ":8001");
              ws.addEventListener("open", function(event) {
                ws.send(3);
                console.log(this.websocketip + "title");
                ws.close();
              });
            }, 3000);
          })
          .catch(() => {
            this.$message({
              type: "info",
              message: "已取消"
            });
          });
      } else {
        this.$confirm("Are you sure to reset and restart router?", "Warning", {
          confirmButtonText: "OK",
          cancelButtonText: "Cancel",
          type: "warning"
        })
          .then(() => {
            this.$message({
              type: "success",
              message: "Reset!"
            });
            this.$router.push("/Login");
            setTimeout(() => {
              ws = new WebSocket("ws://" + this.websocketip + ":8001");
              ws.addEventListener("open", function(event) {
                ws.send(3);
                console.log(this.websocketip + "title");
                ws.close();
              });
            }, 3000);
          })
          .catch(() => {
            this.$message({
              type: "info",
              message: "Canceled"
            });
          });
      }
    },
    gologin() {
      console.log("gologin");
      this.$router.push("/Login");
    },
    signout() {
      // this.$confirm('$t("sceneRouter.signoutTip")')
      //   .then(() => {
      // let para = sessionStorage.getItem('user')
      // logout(para).then(res => {
      //   if (res.data.code === 200) {
      //     sessionStorage.removeItem('user')
      //     this.$router.push('/login')
      //   }
      // })
      // })
      // .bind(this)
      console.log("logout1");
      let _this = this;
      if (this.$store.state.app.language === "zh") {
        this.$confirm("确认退出吗?", "提示", {
          type: "warning"
        })
          .then(() => {
            console.log("logout2");
            let para = sessionStorage.getItem("user");
            logout(para).then(res => {
              if (res.data.code === 200) {
                console.log("logout3");
                sessionStorage.removeItem("user");
                _this.$router.push("/login");
              }
            });
          })
          .catch(error => {
            console.log(error);
          });
      } else {
        this.$confirm("Are you sure to logout?", "Warning", {
          type: "warning"
        })
          .then(() => {
            let para = sessionStorage.getItem("user");
            logout(para).then(res => {
              if (res.data.code === 200) {
                sessionStorage.removeItem("user");
                _this.$router.push("/login");
              }
            });
          })
          .catch(error => {
            console.log(error);
          });
      }
    }
  },

  mounted() {
    console.log("this.$store.state.app.language");
    console.log(this.$store.state.app.language);
    if (this.$store.state.app.language === "zh") {
      this.zh = true;
    } else {
      this.zh = false;
    }
    getVersion().then(res => {
      if (res.data.code === 200) {
        this.routerName = "OpenRT v" + res.data.version;
      }
    });
    let user = sessionStorage.getItem("user");
    console.log("user is " + user);

    if (user) {
      user = JSON.parse(user);
      console.log("user is " + user);
      this.sysUserName = user.userName || "";
      this.sysUserAvatar = "../../../../static/acoinfo.png" || "";
    } else {
      this.sysUserAvatar = "../../../../static/acoinfo.png";
      console.log("No user!");
    }
    websocketIpGetAction().then(res => {
      // wyk1113添加websocket
      this.websocketip = res.data.websockip;

      // ws = new WebSocket('ws://' + this.websocketip + ':8001')
      // console.log(this.websocketip + 'title')
      // ws.send(7)
      // ws.onopen = function() {
      //   // Web Socket 已连接上，使用 send() 方法发送数据
      //   // ws.send(3)
      //   if (ws.readyState !== 1) {
      //     alert('duankai')
      //   }
      // }
    });
  },
  destroyed() {
    // ws.close()
    // console.log('关掉ws1')
    // wyk清除定时器
    // ws.onclose = function(evt) {
    //   console.log('Connection closed.')      // 关闭websock
    // }
  }
};
</script>

<style scoped lang="scss">
.title_line {
  margin-bottom: 1rem;
  background-color: rgb(60, 141, 188);
  font-size: 14px;
  font-weight: 500;
  color: #fff;
  .router {
    width: 230px;
    padding-left: 3rem;
    font-size: 1.5rem;
  }
  .el-col {
    height: 4rem;
    line-height: 4rem;
  }
  .title {
    cursor: pointer;
  }
  .is_choosen {
    color: orange;
  }
  .user_name {
    color: #fff;
    img {
          width: 40px;
          height: 40px;
          border-radius: 20px;
          margin: 10px 0 10px 10px;
          float: right;
        }
  }
  .last_row {
    float: right;
  }
  img {
    width: 2rem;
    height: 2rem;
    border-radius: 1.5rem;
    margin: 1rem 0.5rem;
    float: right;
  }
  .cursor {
    cursor: pointer;
  }
}
</style>
