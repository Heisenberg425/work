import Cookies from 'js-cookie'

const app = {
  state: {
    sidebar: {
      opened: !+Cookies.get('sideabrStatus'),
      withoutAnimation: false
    },
    device: 'desktop',
    language: Cookies.get('language') || 'zh',
    systemData: {
      up: '',
      down: '',
      cpu: '',
      memory: '',
      userNum: ''
    },
    adaptiveCode: 0,
    webSocket: {
      socket: '',
      count: 0,
      contentList: []
    },
    timers: {
      intervalOfInterface: 0,
      linecharts2: 0,
      mychart1: 0,
      inuse: false
    }
  },
  mutations: {
    TOGGLE_SIDEBAR: state => {
      if (state.sidebar.opened) {
        Cookies.set('sidebarStatus', 1)
      } else {
        Cookies.set('sidebarStatus', 0)
      }
      state.sidebar.opened = !state.sidebar.opened
      state.sidebar.withoutAnimation = false
    },
    CLOSE_SIDEBAR: (state, withoutAnimation) => {
      Cookies.set('sidebarStatus', 1)
      state.sidebar.opened = false
      state.sidebar.withoutAnimation = withoutAnimation
    },
    TOGGLE_DEVICE: (state, device) => {
      state.device = device
    },
    SET_LANGUAGE: (state, language) => {
      state.language = language
      Cookies.set('language', language)
    },
    SET_SYSDATA: (state, systemData) => {
      state.systemData.up = systemData.upStorage
      state.systemData.down = systemData.downStorage
      state.systemData.cpu = systemData.cpu
      state.systemData.memory = systemData.memory
      state.systemData.userNum = systemData.userNum
    },
    SET_ADAPTIVE: (state, adaptiveCode) => {
      state.adaptiveCode = adaptiveCode
    },
    SET_TIMER: (state, inuse) => {
      state.timers.inuse = inuse
    }

  },
  actions: {
    setLanguage({ commit }, language) {
      commit('SET_LANGUAGE', language)
    },
    pushSystemData({ commit }, systemData) {
      commit('SET_SYSDATA', systemData)
    },
    // 判断是否需要跳转去做自适应
    setAdaptive({ commit }, adaptiveCode) {
      commit('SET_ADAPTIVE', adaptiveCode)
    },
    setTimer({ commit }, inuse) {
      commit('SET_TIMER', inuse)
    }
  }
}

export default app
