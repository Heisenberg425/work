// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from './App'

import VueRouter from 'vue-router'

/**
 * 第三方组件引入
 */
// 引入vuex相关依赖
import Vuex from 'vuex'
import store from './store'

// 引入axios
import axios from 'axios'

// 引入elementUI相关依赖
import ElementUI from 'element-ui'
import 'element-ui/lib/theme-chalk/index.css' // 引用Element-ui 主题

// 引入eCharts
import VCharts from 'v-charts'

// 引入i18国际化相关依赖
import i18n from './lang'

// 引入阿里巴巴图片库图标
import './assets/iconfont/iconfont.js'
// 第三方字体图标引入 Font Awesome
import 'font-awesome/css/font-awesome.min.css'
// 引入自定义过滤器
import filters from './utils/filters.js'

// 引入polyfill
import 'babel-polyfill'

// 引入Mock模拟数据
// import Mock from './mock'

/**
 * 本地路由引入
 */
import routes from './routes'

// 使用Mock模拟数据
// Mock.bootstrap()

Vue.use(VueRouter)
Vue.use(Vuex)
Vue.use(ElementUI, {
  i18n: (key, value) => i18n.t(key, value)
})
Vue.use(VCharts)

// 注册过滤器
for (let key in filters) {
  Vue.filter(key, filters[key])
}

const router = new VueRouter({
  routes
})

Vue.config.productionTip = false

Vue.prototype.$axios = axios

// 设置一些全局函数
Vue.prototype.header = function () {
  return {
    'background-color': '#626c91',
    'color': 'white',
    'font-weight': 'normal',
    'padding': '.2rem'
  }
}

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  store,
  i18n,
  components: { App },
  template: '<App/>'
})

// 添加响应拦截器
axios.interceptors.response.use(function (response) {
  return response
}, function (error) {
  const status = error.response.status

  switch (status) {
    case 300:
      console.log('300错误')
      break
    case 400:
      console.log('400错误')
      break
    case 401:
      console.log('401错误')
      router.push('/login')
      break
    case 500:
      console.log('500错误')
      break
  }

  // 返回后给每个请求catch抓住针对错误做处理
  return Promise.reject(error)
})

// 注册导航的全局前置守卫
router.beforeEach((to, from, next) => {
  // // 用户点击退出登录后，删除用户信息并跳转至登陆页面
  // if (to.path === '/login') {
  //   sessionStorage.removeItem('user')

  //   // 关闭全局定时器
  //   store.dispatch('setTimer', false)

  //   let timer = store.state.app.timers
  //   Object.keys(timer).forEach(element => {
  //     if (element && (typeof (timer[element]) === 'number')) {
  //       clearInterval(timer[element])
  //     }
  //   })

  //   next()
  // }

  let user = JSON.parse(sessionStorage.getItem('user'))
  console.log('topath' + to.path)
  console.log('user')
  console.log(user)
  if (to.path === '/CreatePasswd') {
    // 首次登陆跳转到修改密码
    next()
    next() // warning！这里可能有问题：需要两个next（）才能跳转
  } else if (!user && to.path !== '/login') {
    console.log('login')
    next({path: '/login'})
  } else if (to.path !== '/ports' && store.state.app.adaptiveCode === 500) {
    // 验证是否已完成自适应操作
    next({path: '/ports'})
  } else if (to.path === '/') {
    console.log('systemsta')
    next({path: '/systemStatus'})
  } else {
    console.log('empty')
    next()
  }
  // next()
})
