import Vue from 'vue'
import VueI18n from 'vue-i18n'
import Cookies from 'js-cookie'
import elementEnLocale from 'element-ui/lib/locale/lang/en' // element-ui lang
import elementZhLocale from 'element-ui/lib/locale/lang/zh-CN' // element-ui lang
import enLocale from './en'
import zhLocale from './zh'
import sceneZh from './sceneZh'
import sceneEn from './sceneEn'
import situZh from './situationModeZh'
import situEn from './situationModeEn'
import staMoZh from './statusMonitorZh'
import staMoEn from './statusMonitorEn'
import operZh from './operationZh'
import operEn from './operationEn'
import homeZh from './homeZh'
import homeEn from './homeEn'
import natZh from './natZh'
import natEn from './natEn'
import INEXNetZh from './INEXNetZh'
import INEXNetEn from './INEXNetEn'
import dhcpZh from './dhcpZh'
import dhcpEn from './dhcpEn'
import kidvpnZh from './kidvpnZh'
import kidvpnEn from './kidvpnEn'
import staticRouterZh from './staticRouterZh'
import staticRouterEn from './staticRouterEn'
import systemStatusZh from './systemStatusZh'
import systemStatusEn from './systemStatusEn'
import arpZh from './arpZh'
import arpEn from './arpEn'
import logZh from './logZh'
import logEn from './logEn'
import qosZh from './qosZh'
import qosEn from './qosEn'
import systemSetZh from './systemSetZh'
import systemSetEn from './systemSetEn'
import securitySetZh from './securitySetZh'
import securitySetEn from './securitySetEn'
import powerSetZh from './powerSetZh'
import powerSetEn from './powerSetEn'
import BMZh from './BMZh'
import BMEn from './BMEn'
import basicSetZh from './basicSetZh'
import basicSetEn from './basicSetEn'
import flowConZh from './flowConZh'
import flowConEn from './flowConEn'
import adaptiveZh from './adaptiveZh'
import adaptiveEn from './adaptiveEn'
import shellZh from './shellZh'
import shellEn from './shellEn'
import usrScriptZh from './usrScriptZh'
import usrScriptEn from './usrScriptEn'
import networkSetZh from './networkSetZh'
import networkSetEn from './networkSetEn'
import pingZh from './pingZh'
import pingEn from './pingEn'
import pieChartsZh from './pieChartsZh'
import pieChartsEn from './pieChartsEn'
import placeholderZh from './placeholderZh'
import placeholderEn from './placeholderEn'
import startScriptEn from './startScriptEn'
import startScriptZh from './startScriptZh'

Vue.use(VueI18n)

const messages = {
  en: {
    ...placeholderEn,
    ...pieChartsEn,
    ...pingEn,
    ...networkSetEn,
    ...shellEn,
    ...adaptiveEn,
    ...flowConEn,
    ...basicSetEn,
    ...BMEn,
    ...powerSetEn,
    ...securitySetEn,
    ...systemSetEn,
    ...qosEn,
    ...logEn,
    ...arpEn,
    ...systemStatusEn,
    ...staticRouterEn,
    ...kidvpnEn,
    ...dhcpEn,
    ...INEXNetEn,
    ...natEn,
    ...homeEn,
    ...operEn,
    ...staMoEn,
    ...enLocale,
    ...sceneEn,
    ...situEn,
    ...elementEnLocale,
    ...usrScriptEn,
    ...startScriptEn
  },
  zh: {
    ...placeholderZh,
    ...pieChartsZh,
    ...pingZh,
    ...networkSetZh,
    ...shellZh,
    ...adaptiveZh,
    ...flowConZh,
    ...basicSetZh,
    ...BMZh,
    ...powerSetZh,
    ...securitySetZh,
    ...systemSetZh,
    ...qosZh,
    ...logZh,
    ...arpZh,
    ...systemStatusZh,
    ...staticRouterZh,
    ...kidvpnZh,
    ...dhcpZh,
    ...INEXNetZh,
    ...natZh,
    ...homeZh,
    ...operZh,
    ...staMoZh,
    ...zhLocale,
    ...sceneZh,
    ...situZh,
    ...elementZhLocale,
    ...usrScriptZh,
    ...startScriptZh
  }
}

const i18n = new VueI18n({
  locale: Cookies.get('language') || 'zh', // set locale
  messages // set locale messages
})

export default i18n
