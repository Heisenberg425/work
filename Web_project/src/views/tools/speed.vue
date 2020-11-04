<template>
  <div>
    <!--table id="Bq">
      <tr>
        <td id="stWrap"></td>
      </tr>
    </table-->
    <div class="square">
    <div class="square_body">
    <iframe
            src="http://plugin.speedtest.cn/#1d2133"
            frameborder="no"
            scrolling="auto" 
             style='overflow:scroll;overflow-x:hidden'/>
  </div>
  </div>
  </div>
</template>
<script>
export default {
  mounted () {
    if (this.$store.state.app.language === 'zh') {
      this.$route.matched[0].name = this.$route.matched[0].meta.zh
      this.$route.matched[1].name = this.$route.matched[1].meta.zh
    } else {
      this.$route.matched[0].name = this.$route.matched[0].meta.en
      this.$route.matched[1].name = this.$route.matched[1].meta.en
    }
  }
}
</script>

<style scoped>
table {
  height: 20rem;
  width: 20rem;
}

</style>
<style scoped lang="scss">
.square {
      width: 25%;
      height: 30rem;
      margin: 6rem auto;
      border-radius: 1rem;
      background: #fff;
      border: 1px solid grey;
.square_body {
        width: 100%;
        height: 100%;
        display: flex;
        flex-flow: row wrap;
        justify-content: space-around;
        iframe {
          width: 100%;
          height: 100%;
          body{
           overflow-x: hidden; 
          }
          
          // .speedtest-container{
          //   .logInformation{
          //     .logInformation[data-v-0ef51150] {
          //       margin-top: .5rem;
          //     } 
          //   }
          //   .speedButton2{
          //     .speedButton2[data-v-0ef51150] {
          //       display: -ms-flexbox;
          //       display: flex;
          //       -ms-flex-pack: center;
          //       justify-content: center;
          //       margin: -4.5rem auto;
          //     }
          //   }
          //   }
        }
        
      }}
      </style>