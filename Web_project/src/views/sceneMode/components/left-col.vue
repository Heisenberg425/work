<template>
  <div>
    <div
      class="title first"
      @click="change('internetBox')">
      <div class="icon internet" />
      <div
        v-show="choosen === 'internetBox'"
        class="arrow" />
      <div class="title_name">
        {{ $t('sceneRouter.internet') }}
      </div>
    </div>

    <div class="line">
      <div class="straight-line" />
    </div>

    <div
      class="title"
      @click="change('mainBox')">
      <div class="icon router" />
      <div
        v-show="choosen === 'mainBox'"
        class="arrow" />
      <div class="title_name">
        {{ $t('sceneRouter.router') }}
      </div>
    </div>

    <div class="line">
      <div class="straight-line" />
    </div>

    <div
      class="title"
      @click="change('terminalBox')">
      <div class="icon terminal" />
      <div
        v-show="choosen === 'terminalBox'"
        class="arrow" />
      <div class="title_name">
        {{ $t('sceneRouter.terminal') }}
      </div>
    </div>

  </div>
</template>

<script>
export default {
  data () {
    return {
      choosen: 'mainBox'
    }
  },
  methods: {
    change (val) {
      this.$emit('changebox', val)
      this.choosen = val
      console.log('in left ' + val)
    }
  }
  // mounted () {
  //   setTimeout(() => {
  //     this.change('mainBox')
  //   }, 29)
  // }
}
</script>

<style scoped lang="scss">
.left_col {
  .title {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    margin: 1.5rem auto;
    .icon {
      width: 5rem;
      height: 4rem;
      margin-top: 0.5rem;
      border-radius: 0.5rem;
      background-color: rgb(70, 114, 196);
    }
    .internet {
      background-image: url("../../../../static/sceneMode/internet.png");
      background-size: 64% 80%;
      background-position: center;
      background-repeat: no-repeat;
    }
    .router {
      background-image: url("../../../../static/sceneMode/router.png");
      background-size: 64% 80%;
      background-position: center;
      background-repeat: no-repeat;
    }
    .terminal {
      background-image: url("../../../../static/sceneMode/terminal.png");
      background-size: 64% 80%;
      background-position: center;
      background-repeat: no-repeat;
    }
    .title_name {
      height: 1rem;
      line-height: 1rem;
      margin-top: 0.5rem;
      color: rgb(70, 114, 196);
      font-size: 18px;
    }
    .arrow {
      position: absolute;
      margin-top: .5rem;
      right: -20%;
      border-top: 2rem solid transparent;
      border-bottom: 2rem solid transparent;
      border-right: 2rem solid grey;
    }
  }
  .title:first-child {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    margin: 8rem auto 1.5rem auto;
  }

  .line {
    height: 8rem;
    width: 100%;
    .straight-line {
      height: 100%;
      width: 0.5rem;
      margin: auto;
      border: 0.5px solid rgb(200, 200, 200);
      background: rgb(156, 195, 229);
    }
  }
}
</style>
