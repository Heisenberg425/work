// 页面路由表
import situationMode from './views/situationMode.vue' // wyk914情景模式

import Login from './views/Login.vue'
import CreatePasswd from './views/CreatePasswd.vue'
import ModeChoose from './views/ModeChoose.vue'
import NotFound from './views/404.vue'
import Home from './views/Home.vue'

/**
 *systemStatus
 */
import systemStatus from './views/systemStatus/systemStatus.vue'

/**
 *status_monitor
 */

import lineMonitor from './views/statusMonitor/lineMonitor.vue'
import flowMonitor from './views/statusMonitor/flowMonitor.vue'
import rateMonitor from './views/statusMonitor/rateMonitor.vue'
import terminalMonitor from './views/statusMonitor/terminalMonitor.vue'
import performanceMonitor from './views/statusMonitor/performanceMonitor.vue'

/**
 *networkSetting
 */
import DHCP from './views/networkSetting/DHCP.vue'
import DNS from './views/networkSetting/DNS.vue'
import INEXnetwork from './views/networkSetting/INEX_network/index.vue'
import kidVPN from './views/networkSetting/kidVPN'
import NAT from './views/networkSetting/nat/'
import VLAN from './views/networkSetting/VLAN.vue'
import staticRouter from './views/networkSetting/staticRouter/staticRouter.vue'
import Qos from './views/networkSetting/Qos/index.vue'

/**
 *systemSetting
 */
import accountSetting from './views/systemSetting/accountSetting.vue'
import basicSetting from './views/systemSetting/basicSetting.vue'
import hardwareInfo from './views/systemSetting/hardwareInfo.vue'
import restartShutdown from './views/systemSetting/restartShutdown.vue'
import update from './views/systemSetting/update.vue'
import usrScript from './views/systemSetting/usrScript.vue'
import startScript from './views/systemSetting/startScript.vue'
// import powerSetting from './views/systemSetting/powerSetting'          // wyk914权限管理
// import ports from './views/systemSetting/ports.vue'         //飞腾版本去除网口定位

/**
 *flowControl
 */
import flowControl from './views/flowControl/flowControl.vue'

/**
 *behaviourManagement
 */
// import BMControl from './views/behaviourManagement/BMControl.vue'
import BMDisplay from './views/behaviourManagement/BMDisplay.vue'

/**
 *securitySetting
 */
import ARP from './views/securitySetting/ARP.vue'
import ARPDefence from './views/securitySetting/ARPDefence.vue'
import firewall from './views/securitySetting/firewall.vue'
import webBind from './views/securitySetting/webBinding.vue'
// import remoteManagement from './views/securitySetting/remoteManagement.vue'

/**
 *tools
 */
import ping from './views/tools/ping.vue'
import speed from './views/tools/speed.vue'

/**
 *log
 */
import log from './views/log/log.vue'
import operaLog from './views/log/operationLog.vue'
import vpnLog from './views/log/vpnLog.vue'
import ARPLog from './views/log/ARPLog.vue'
import DHCPLog from './views/log/DHCPLog.vue'

/**
 *shell
 */
// import ripd from './views/shell/ripd.vue'
// import ripngd from './views/shell/ripngd.vue'
// import ospfd from './views/shell/ospfd.vue'
// import isisd from './views/shell/isisd.vue'
import shell from './views/shell/bgpd.vue'
// import pimd from './views/shell/pimd.vue'

/**
 * sceneMode
 */
import sceneRouter from './views/sceneMode/router'

const routes = [
  {
    path: '/login',
    component: Login,
    name: '',
    hidden: true
  },
  {
    path: '/CreatePasswd',
    component: CreatePasswd,
    name: '',
    hidden: true
  },
  {
    path: '/modeChoose',
    component: ModeChoose,
    name: '',
    hidden: true
  },
  {
    path: '/sceneMode',
    component: sceneRouter,
    name: '',
    hidden: true
  },
  {
    path: '/404',
    component: NotFound,
    name: '',
    hidden: true
  },
  {
    path: '/situationMode',
    component: situationMode,
    name: '',
    hidden: true
  },
  {
    path: '/',
    component: Home,
    name: '系统概况简介',
    iconCls: 'fa fa-bar-chart',
    leaf: true, // 只有一个节点
    meta: {
      title: 'systemStatus',
      zh: '系统概况简介',
      en: 'System Status'
    },

    children: [
      {
        path: '/systemStatus',
        component: systemStatus,
        name: '系统概况',
        meta: { zh: '系统概况', en: 'System Status', title: 'systemStatus' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '状态监控',
    meta: {
      title: 'statusMonitor',
      zh: '状态监控',
      en: 'Status Monitor'
    },
    iconCls: 'fa fa-television',
    children: [
      {
        path: '/lineMonitor',
        component: lineMonitor,
        name: '线路监控',
        meta: { zh: '线路监控', en: 'Line Monitor', title: 'lineMonitor' }
      },
      {
        path: '/terminalMonitor',
        component: terminalMonitor,
        name: '终端监控',
        meta: {
          zh: '终端监控',
          en: 'Terminal Monitor',
          title: 'terminalMonitor'
        }
      },
      {
        path: '/flowMonitor',
        component: flowMonitor,
        name: '流量监控',
        meta: { zh: '流量监控', en: 'Flow Monitor', title: 'flowMonitor' }
      },
      {
        path: '/rateMonitor',
        component: rateMonitor,
        name: '速率监控',
        meta: { zh: '速率监控', en: 'Rate Monitor', title: 'rateMonitor' }
      },
      {
        path: '/performanceMonitor',
        component: performanceMonitor,
        name: '性能监控',
        meta: {
          zh: '性能监控',
          en: 'Performance Monitor',
          title: 'performanceMonitor'
        }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '网络设置',
    iconCls: 'fa fa-sliders',
    meta: {
      title: 'netSetting',
      zh: '网络设置',
      en: 'Network Settings'
    },
    children: [
      {
        path: '/INEX_network',
        component: INEXnetwork,
        name: '内外网设置',
        meta: {
          zh: '内外网设置',
          en: 'Intranet & Extranet Settings',
          title: 'INEXNetworkSetting'
        }
      },
      {
        path: '/DHCP',
        component: DHCP,
        name: 'DHCP服务器设置',
        meta: {
          zh: 'DHCP服务器设置',
          en: 'DHCP Settings',
          title: 'DHCPSetting'
        }
      },
      {
        path: '/DNS',
        component: DNS,
        name: 'DNS设置',
        meta: { zh: 'DNS设置', en: 'DNS Settings', title: 'DNSSetting' }
      },
      {
        path: '/staticRouter',
        component: staticRouter,
        name: '静态路由',
        meta: { zh: '静态路由', en: 'Static Router', title: 'staticRouter' }
      },
      // { path: '/currentRouter', component: currentRouter, name: '当前路由表' },
      {
        path: '/VLAN',
        component: VLAN,
        name: 'VLAN设置',
        meta: { zh: 'VLAN设置', en: 'VLAN Settings', title: 'VLANSetting' }
      },
      {
        path: '/kidVPN',
        component: kidVPN,
        name: 'KidVPN设置',
        meta: {
          zh: 'KidVPN设置',
          en: 'KidVPN Settings',
          title: 'KidVPNSetting'
        }
      },
      {
        path: '/NAT',
        component: NAT,
        name: 'NAT转发与端口映射',
        meta: {
          zh: 'NAT转发与端口映射',
          en: 'NAT & Port Mapping',
          title: 'NAT'
        }
      },
      {
        path: '/Qos',
        component: Qos,
        name: 'Qos设置',
        meta: { zh: 'QoS设置', en: 'QoS Settings', title: 'Qos' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '系统设置',
    iconCls: 'fa fa-cog',
    meta: {
      title: 'systemSetting',
      zh: '系统设置',
      en: 'System Settings'
    },
    children: [
      {
        path: '/basicSetting',
        component: basicSetting,
        name: '基础设置',
        meta: { zh: '基础设置', en: 'Basic Settings', title: 'basicSetting' }
      },
      {
        path: '/accountSetting',
        component: accountSetting,
        name: '账号设置',
        meta: {
          zh: '账号设置',
          en: 'Account Settings',
          title: 'accountSetting'
        }
      },
      {
        path: '/update',
        component: update,
        name: '版本升级',
        meta: { zh: '版本升级', en: 'Version Update', title: 'versionUpdate' }
      },
      {
        path: '/hardwareInfo',
        component: hardwareInfo,
        name: '硬件信息',
        meta: { zh: '硬件信息', en: 'Hardware Info', title: 'hardwareInfo' }
      },
      {
        path: '/restartShutdown',
        component: restartShutdown,
        name: '重启关机',
        meta: {
          zh: '重启关机',
          en: 'Shutdown & Restart',
          title: 'shutdownRestart'
        }
      },
      {
        path: '/usrScript',
        component: usrScript,
        name: '运行脚本',
        meta: { zh: '用户自定义运行脚本', en: 'usr Script', title: 'usrScript' }
      },
      {
        path: '/startScript',
        component: startScript,
        name: '启动脚本',
        meta: {
          zh: '用户自定义启动脚本',
          en: 'start Script',
          title: '启动脚本'
        }
      }
      // { path: '/powerSetting', component: powerSetting, name: '权限管理', meta: { title: 'powerSetting' } },          // wyk914权限管理
      // { path: '/ports', component: ports, name: '端口自适应', meta: { zh: '端口自适应', en: 'ports', title: 'ports' } }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '流控分流按钮',
    iconCls: 'fa fa-filter',
    leaf: true, // 只有一个节点
    meta: {
      title: 'flowControl',
      zh: '流控分流',
      en: 'Flow Control'
    },
    children: [
      {
        path: '/flowControl',
        component: flowControl,
        name: '流控分流',
        meta: { zh: '流控分流', en: 'Flow Control', title: 'flowControl' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '行为管理',
    iconCls: 'fa fa-universal-access',
    meta: {
      title: 'behaviourManagement',
      zh: '行为管理',
      en: 'Behaviour Management'
    },
    children: [
      // { path: '/BMControl', component: BMControl, name: '行为记录管理', meta: { title: 'recordManagement' } },
      {
        path: '/BMDisplay',
        component: BMDisplay,
        name: '行为记录显示',
        meta: {
          zh: '行为记录显示',
          en: 'Record Display',
          title: 'recordDisplay'
        }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '安全设置',
    iconCls: 'fa fa-arrows-alt',
    meta: {
      title: 'securitySetting',
      zh: '安全设置',
      en: 'Security Settings'
    },
    children: [
      {
        path: '/ARP',
        component: ARP,
        name: 'ARP绑定',
        meta: { zh: 'ARP绑定', en: 'ARP Binding', title: 'ARPBinding' }
      },
      {
        path: '/ARPDefence',
        component: ARPDefence,
        name: 'ARP防御',
        meta: { zh: 'ARP防御', en: 'ARP Defence', title: 'ARPDefence' }
      },
      // { path: '/remoteManagement', component: remoteManagement, name: '远程登陆管理', meta: { title: 'remoteLoginManagement' } },
      {
        path: '/firewall',
        component: firewall,
        name: '规则防火墙',
        meta: { zh: '规则防火墙', en: 'Firewall', title: 'firewall' }
      },
      {
        path: '/webBinding',
        component: webBind,
        name: 'Web绑定',
        meta: { zh: 'Web绑定', en: 'Web Bind', title: 'WebBind' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '系统工具',
    iconCls: 'fa fa-wrench',
    meta: {
      title: 'tools',
      zh: '系统工具',
      en: 'Tools'
    },
    children: [
      {
        path: '/ping',
        component: ping,
        name: 'Ping测试',
        meta: { zh: 'Ping测试', en: 'Ping', title: 'ping' }
      },
      {
        path: '/speed',
        component: speed,
        name: 'Speed测试',
        meta: { zh: 'Speed测试', en: 'Speed', title: 'speed' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: '日志系统',
    iconCls: 'fa fa-file-text-o',
    meta: {
      title: 'log',
      zh: '日志系统',
      en: 'Log'
    },
    children: [
      {
        path: '/log',
        component: log,
        name: '日志',
        meta: { zh: '拨号日志', en: 'Dial Log', title: 'dialLog' }
      },
      {
        path: '/operationLog',
        component: operaLog,
        name: '操作日志',
        meta: { zh: '操作日志', en: 'Opera Log', title: 'operaLog' }
      },
      {
        path: '/vpnLog',
        component: vpnLog,
        name: 'VPN日志',
        meta: { zh: 'VPN日志', en: 'VPN Log', title: 'VPNLog' }
      },
      {
        path: '/ARPLog',
        component: ARPLog,
        name: 'ARP防御日志',
        meta: { zh: 'ARP防御日志', en: 'ARP Log', title: 'ARPLog' }
      },
      {
        path: '/DHCPLog',
        component: DHCPLog,
        name: 'DHCP日志',
        meta: { zh: 'DHCP日志', en: 'DHCP Log', title: 'DHCPLog' }
      }
    ]
  },
  {
    path: '/',
    component: Home,
    name: 'shell',
    iconCls: 'fa fa-list',
    leaf: true,
    meta: {
      title: 'shell',
      zh: '动态路由',
      en: 'Shell'
    },
    children: [
      // { path: '/ripd', component: ripd, name: 'ripd页面', meta: { zh: 'Ripd页面', en: 'Ripd', title: 'ripd' } },
      // { path: '/ripngd', component: ripngd, name: 'ripngd页面', meta: { zh: 'Ripngd页面', en: 'Ripngd', title: 'ripngd' } },
      // { path: '/ospfd', component: ospfd, name: 'ospfd页面', meta: { zh: 'Ospfd页面', en: 'Ospfd', title: 'ospfd' } },
      // { path: '/isisd', component: isisd, name: 'isisd页面', meta: { zh: 'Isisd页面', en: 'Isisd', title: 'isisd' } },
      {
        path: '/bgpd',
        component: shell,
        name: 'Shell',
        meta: { zh: '动态路由', en: 'Shell', title: 'shell' }
      }
      // { path: '/pimd', component: pimd, name: 'pimd页面', meta: { zh: 'Pimd页面', en: 'Pimd', title: 'pimd' } }
    ]
  },
  {
    path: '*',
    hidden: true,
    redirect: { path: '/404' }
  }
]

export default routes
