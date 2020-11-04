<template>
  <div>
    <div class="title">
      <div class="bottom">
        <div class="topleft">
        <p class="headline">{{ $t('newshell.line1') }}</p>
        <p class="content">{{ $t('newshell.line2') }}</p>
        <p class="headline">{{ $t('newshell.line3') }}</p>
        <p class="content">{{ $t('newshell.line4') }}</p>
        <a :href="server">
          <el-button size="large" type="primary" class="button">{{ $t('newshell.download') }}</el-button>
        </a>
        </div>
      </div>
      <div :class="zh==true?'top1':'top2'">
        <!-- <img src='../../static/PuTTYtip.png'> -->
      </div>
    </div>
    <div class="protocol-block">
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[0]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[0]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[1]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[1]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[2]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[2]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
      <div class="box-right wrap" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[3]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[3]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
    </div>
    <div class="protocol-block">
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[4]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[4]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[5]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[5]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
      <div class="box-right" style="margin-right: 80px;">
        <!-- <p>{{ item }}</p> -->
        <a :href="urlList[6]">
          <img style="width: 150px; height:150px; border-radius: 5px;" :src="drawer[6]">
          <!-- <el-button>配置{{ item }}协议</el-button> -->
        </a>
      </div>
    </div>
    <!-- <a :href="rip">
        <p>rip</p><el-button>配置协议</el-button>
      </a>
      <a :href="ripng">
        <p>ripng</p><el-button>配置协议</el-button>
      </a>
      <a :href="ospf">
        <p>ospf</p><el-button>配置协议</el-button>
      </a>
      <a :href="isis">
        <p>isis</p><el-button>配置协议</el-button>
      </a>
      <a :href="bgp">
        <p>bgd</p><el-button>配置协议</el-button>
      </a>
      <a :href="pim">
        <p>pim</p><el-button>配置协议</el-button>
    </a>-->
  </div>
</template>

<script>
import { getBtnInfo } from '@/api/api.js'
export default {
  name: 'Shell',
  data() {
    return {
      zh: true,
      url: '',
      zebra: '',
      rip: '',
      ripng: '',
      ospf: '',
      isis: '',
      bgp: '',
      pim: '1111',
      background: [
        'z-background',
        'r-background',
        'r-background',
        'o-background',
        'i-background',
        'b-background',
        'p-background'
      ],
      drawer: [
        'static/z1.jpg',
        'static/r1.jpg',
        'static/r-1.jpg',
        'static/o1.jpg',
        'static/i1.jpg',
        'static/b1.jpg',
        'static/p1.jpg'
      ],
      urlList: ['#', '#', '#', '#', '#', '#', '#']
    }
  },
  mounted() {
    // let name = sessionStorage.getItem('user')
    // name = JSON.parse(name)
    // // let para = {
    // //   usrname: name.userName
    // // }

    getBtnInfo()
      .then(res => {
        // if (res.data.code === 200) {
        this.url = res.data.websockip
        this.zebra = 'putty://telnet://' + this.url + ':2601'
        // this.urlList[0] = this.zebra
        this.rip = 'putty://telnet://' + this.url + ':2602'
        // this.urlList[1] = this.rip
        this.ripng = 'putty://telnet://' + this.url + ':2603'
        // this.urlList[2] = this.ripng
        this.ospf = 'putty://telnet://' + this.url + ':2604'
        // this.urlList[3] = this.ospf
        this.isis = 'putty://telnet://' + this.url + ':2608'
        // this.urlList[4] = this.isis
        this.bgp = 'putty://telnet://' + this.url + ':2605'
        // this.urlList[5] = this.bgp
        this.pim = 'putty://telnet://' + this.url + ':2611'
        // this.urlList[6] = this.pim

        let list = []
        list.push(this.zebra)
        list.push(this.rip)
        list.push(this.ripng)
        list.push(this.ospf)
        list.push(this.isis)
        list.push(this.bgp)
        list.push(this.pim)

        this.urlList = list

        this.server = 'http://' + this.url + '/files/putty.zip'
        console.log('url' + this.url)
        // } else if (res.data.code !== 200) {
        //   alert('Cannot get server info')
        // }
      })
      .catch(err => {
        console.log(err)
      })
    console.log('lllllllllllll')
    console.log(this.urlList)
    console.log('pin1' + this.pim3)
    if (this.$store.state.app.language === 'zh') {
      this.zh = true
      this.drawer[0] = 'static/z1.jpg'
      this.drawer[1] = 'static/r1.jpg'
      this.drawer[2] = 'static/r-1.jpg'
      this.drawer[3] = 'static/o1.jpg'
      this.drawer[4] = 'static/i1.jpg'
      this.drawer[5] = 'static/b1.jpg'
      this.drawer[6] = 'static/p1.jpg'
    } else {
      this.zh = false
      this.drawer[0] = 'static/z2.jpg'
      this.drawer[1] = 'static/r2.jpg'
      this.drawer[2] = 'static/r-2.jpg'
      this.drawer[3] = 'static/o2.jpg'
      this.drawer[4] = 'static/i2.jpg'
      this.drawer[5] = 'static/b2.jpg'
      this.drawer[6] = 'static/p2.jpg'
    }
  },
  watch: {

    '$store.state.app.language': function () {
      console.log(this.$store.state.app.language)
      if (this.$store.state.app.language === 'zh') {
        this.zh = true
        this.drawer[0] = 'static/z1.jpg'
        this.drawer[1] = 'static/r1.jpg'
        this.drawer[2] = 'static/r-1.jpg'
        this.drawer[3] = 'static/o1.jpg'
        this.drawer[4] = 'static/i1.jpg'
        this.drawer[5] = 'static/b1.jpg'
        this.drawer[6] = 'static/p1.jpg'
        console.log(this.drawer)
        this.$forceUpdate()
      } else {
        this.zh = false
        this.drawer[0] = 'static/z2.jpg'
        this.drawer[1] = 'static/r2.jpg'
        this.drawer[2] = 'static/r-2.jpg'
        this.drawer[3] = 'static/o2.jpg'
        this.drawer[4] = 'static/i2.jpg'
        this.drawer[5] = 'static/b2.jpg'
        this.drawer[6] = 'static/p2.jpg'
        console.log(this.drawer)
        this.$forceUpdate()
      }
    }
  },
  methods: {}
}
</script>

<style scoped>
p {
  margin: 0.5rem 0;
}
.title {
  height: 20rem;
  margin-top: 1rem;
  display: flex;
}
.top1 {
  height: 110%;
  width: 37%;
   margin-top: auto ;
    margin-right:  5%;
     margin-bottom: auto ;
      /* margin-left: 5px; */
  border: lightgray 1px solid;
  background: url("../../static/PuTTYtip1.png") no-repeat center;
}
.top2 {
  height: 110%;
  width: 27%;
   margin-top: auto ;
    margin-right:  5%;
     margin-bottom: auto ;
      /* margin-left: 5px; */
  border: lightgray 1px solid;
  background: url("../../static/PuTTYtip2.png") no-repeat center;
}
.bottom {
  width: 25%;
  height: 110%;
  margin-top: auto ;
    margin-right:  5%;
     margin-bottom: auto ;
      /* margin-left: 5%; */
  padding: 0 5%;
  overflow: hidden;
  font-size: 1rem;
  border: lightgray 1px solid;
}
.topleft {
  
  margin-top: 5% ;
   
}
.bottom .headline {
  font-size: 1.2rem;
}
.bottom .content {
  text-indent: 2.5rem;
}

.button {
  margin-right: 1rem;
  float: right;
}

.title a {
  cursor: pointer;
}
.protocol-block {
  display: flex;
  flex-wrap: wrap;
  /* justify-content: space-between; */
  margin: 1rem 5%;
  margin-left: 0px;
  margin-top: 60px;
}
.box {
  width: 25%;
  display: flex;
  border: lightgrey 1px solid;
  margin: 1rem;
}
.box .box-left {
  width: 150px;
  /* height: 100%;
  background-size: auto 80%;
  background-position: 0% 0%; */
}
/* .r-background {
  background: url('../../static/r.png') no-repeat;
}
.z-background {
  background: url('../../static/z.png') no-repeat;
}
.i-background {
  background: url('../../static/i.png') no-repeat;
}
.o-background {
  background: url('../../static/o.png') no-repeat;
}
.p-background {
  background: url('../../static/p.png') no-repeat;
}
.b-background {
  background: url('../../static/b.png') no-repeat;
} */
.box .box-right {
  width: 150px;
  margin-right: 20px;
  /* display: flex;
  justify-content: flex-end;
  flex-direction: column; */
  /* padding: 1rem auto; */
}

.box .box-right p {
  text-align: right;
}
.box .box-right a {
  float: right;
}

.box a p::first-letter {
  font-size: 2rem;
  color: blue;
}
</style>
