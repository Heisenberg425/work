import axios from 'axios'

// let base = 'https://10.9.0.17:80/'
let base = ''

// export const requestLogin = params => { return axios.post(`${base}/login`, params).then(res => res.data) }
// export const requestLogin = params => { return axios.post(`action/webLoginAction`, params).then(res => res.data); };
// 登陆
export const requestLogin = params => { return axios.post(`${base}action/loginAction`, params) }
export const logout = params => { return axios.post(base + 'action/logoutAction', params) }

export const getUserList = params => { return axios.get(`${base}/user/list`, { params: params }) }

export const getUserListPage = params => { return axios.get(`${base}/user/listpage`, { params: params }) }

export const removeUser = params => { return axios.get(`${base}/user/remove`, { params: params }) }

export const batchRemoveUser = params => { return axios.get(`${base}/user/batchremove`, { params: params }) }

export const editUser = params => { return axios.get(`${base}/user/edit`, { params: params }) }

export const addUser = params => { return axios.get(`${base}/user/add`, { params: params }) }

export const getSystem = params => { return axios.get(`${base}/system`, { params: params }) }

export const dialUp = params => { return axios.post('action/udialHandleAction', params) }
// export const dialUp = params => { return axios.post(`${base}/api/dialUp`, params); };
// export const dialUp = params  => {return axios.create({url:'${base}/api/dialUp', method:'post', headers:'Content-Type:text/javascript', params:params}).then(res => res.data)}

export const DHCP = params => { return axios.post('action/dhcpClientAction', params) }
export const staticIP = params => { return axios.post('action/interfaceInfoSetAction', params) }

/*******************************************************************/
// export const getPorts = data => { return axios.post(`${base}/api/ports`, { params: data }); };
export const getPorts = data => { return axios.post('action/interfaceMonitorAction', data) }

// export const getAlias = data => { return axios.post(`${base}/api/alias`, data);};
// export const editAlias = data => { return axios.post(`${base}/api/editAlias`, data);};
// export const addAlias = data => { return axios.post(`${base}/api/addAlias`, data);};
// export const delAlias = data => { return axios.post(`${base}/api/delAlias`, data);};
// export const getMapping = data => { return axios.post(`${base}/api/mapping`, data); };
// export const editMapping = data => { return axios.post(`${base}/api/editMapping`, data);};
// export const addMapping = data => { return axios.post(`${base}/api/addMapping`, data);};
// export const delMapping = data => { return axios.post(`${base}/api/delMapping`, data);};

export const getAlias = data => { return axios.post('action/natAliasGetAction', data) }
export const editAlias = data => { return axios.post('action/natAliasGetAction', data) }
export const addAlias = data => { return axios.post('action/natAliasHandleAction', data) }
export const delAlias = data => { return axios.post('action/natAliasHandleAction', data) }
export const getMapping = data => { return axios.post('action/natMapGetAction', data) }
export const editMapping = data => { return axios.post('action/natMapGetAction', data) }
export const addMapping = data => { return axios.post('action/natMapHandleAction', data) }
export const delMapping = data => { return axios.post('action/natMapHandleAction', data) }
export const sendFragment = data => { return axios.post('action/natIpFragHandleAction', data) }
export const getFragmentInfo = data => { return axios.post('action/natIpFragGetAction', data) }
export const getUsage = data => { return axios.post('action/natAssNodeUseAction', data) }
export const getDialStatus = data => { return axios.post('action/netIfInfoAction', data) }

export const sendWANLAN = data => { return axios.post('action/natWanLanAction', data) }

export const getQos = data => { return axios.post('action/qosShowAction', data) }
export const setQos = data => { return axios.post('action/qosConfigAction', data) }
export const delQos = data => { return axios.post('action/qosDeleteAction', data) }

/**********************************************************/
// export const sendSorting = data => { return axios.post(`${base}/api/intAdaHandleAction`, data);};
export const sendSorting = data => { return axios.post('action/intAdaHandleAction', data) }

/**********************************************************/
// export const sortingCancel = data => { return axios.post(`${base}/api/sortingCancel`, data);};
export const sortingCancel = data => { return axios.post('action/intAdaCancelAction', data) }

// 静态路由页面添加
export const handleRouter = data => { return axios.post('action/routeConfigAction', data) }
export const getRouterInfo = data => { return axios.post('action/routeInfoGetAction', data) }
export const vndinterfaceGetAction = data => { return axios.post('action/vndinterfaceGetAction', data) }
export const addKidVpn = data => { return axios.post('action/kidvpnHandleAction', data) }

// VND页面的请求
export const addKidVPN = data => { return axios.post('action/kidvpnCreateAction', data) }
export const addVND = data => { return axios.post('action/vndHandleAction', data) }
export const generateAESKey = data => { return axios.post('action/kidvpnAesKeyGenAction', data) }
export const getAESKey = data => { return axios.post('action/kidvpnAesKeyGetAction', data) }
export const delKidVPN = data => { return axios.post('action/kidvpnDelAction', data) }
export const getKidVPNInfo = data => { return axios.post('action/kidvpnInfoGetAction', data) }
export const saveAESKey = data => { return axios.post('action/kidvpnAesKeySaveAction', data) }
export const getLinkStatus = data => { return axios.post('action/kidvpnLinkInfoGetAction', data) }
//  请求目的/源地址
export const getCityInfo = data => { return axios.post('action/kidvpnPosInfoGetAction', data) }

// VLAN页面
export const getVLAN = data => { return axios.post('action/vlanShowAction', data) }
export const handleVLAN = data => { return axios.post('action/vlanConfigAction', data) }

// 流控页面
export const getFC = data => { return axios.post('action/flowctlShowAction', data) }
export const handleFC = data => { return axios.post('action/flowctlConfigAction', data) }

// ARP绑定
export const handleARP = data => { return axios.post('action/arpConfigAction', data) }
export const getARP = data => { return axios.post('action/arpShowAction', data) }
export const getDynamicARP = data => { return axios.post('action/arpDynamicShowAction', data) }

// 线路监控
export const getMonitorInfo = data => { return axios.post('action/monitorInfoGetAction', data) }
export const handleMonitorInfo = data => { return axios.post('action/monitorItemDisableAction', data) }

// 系统日志
export const getLog = data => { return axios.post(base + 'action/udialLogGetAction', data) }
// 操作日志
export const getOperaLog = data => { return axios.post(base + 'action/operLogGetAction', data) }
// web上下线
export const getWebInfo = data => { return axios.post(base + 'action/loginRecordQuery', data) }
// vpn 日志
export const getVPNLog = data => { return axios.post(base + 'action/vpnLogGetAction', data) }
export const udialLogDelAction = data => { return axios.post(base + 'action/udialLogDelAction', data) }
export const vpnLogDelAction = data => { return axios.post(base + 'action/vpnLogDelAction', data) }
export const operLogDelAction = data => { return axios.post(base + 'action/operLogDelAction', data) }

// DHCP
export const dhcpServerConfigAction = data => { return axios.post(base + 'action/dhcpServerConfigAction', data) }
export const dhcpServerShowAction = data => { return axios.post(base + 'action/dhcpServerShowAction', data) }
export const dhcpServerDelAction = data => { return axios.post(base + 'action/dhcpServerDelAction', data) }

// 系统信息
export const setTime = data => { return axios.post(base + 'action/timeConfigAction', data) }
export const getTime = data => { return axios.post(base + 'action/timeShowAction', data) }
export const setName = data => { return axios.post(base + 'action/devNameConfigAction', data) }
export const getName = data => { return axios.post(base + 'action/devNameGetAction', data) }
export const getpower = data => { return axios.post(base + 'action/userAuthShowAction', data) }
export const setpower = data => { return axios.post(base + 'action/userAuthHandleAction', data) }
// 密码修改
export const changePasswordAction = data => { return axios.post(base + 'action/changePasswordAction', data) }
export const queryUsersAction = data => { return axios.post(base + 'action/queryUsersAction', data) }
export const addUserAction = data => { return axios.post(base + 'action/addUserAction', data) }
export const delUserAction = data => { return axios.post(base + 'action/delUserAction', data) }
export const disableUserAction = data => { return axios.post(base + 'action/disableUserAction', data) }
// firewall
export const showInfo = data => { return axios.post(base + 'action/npfShowAction', data) }
export const handle = data => { return axios.post(base + 'action/npfConfigAction', data) }
export const enable = data => { return axios.post(base + 'action/npfEnableAction', data) }
export const addMac = data => { return axios.post(base + 'action/npfAddMacFilter', data) }
export const deleteMac = data => { return axios.post(base + 'action/npfDelMacFilter', data) }

// 开关机
export const powerOff = data => { return axios.post(base + 'action/powerOffAction', data) }
export const restart = data => { return axios.post(base + 'action/powerRestartAction', data) }
export const getEvent = data => { return axios.post(base + 'action/powerEventShowAction', data) }
export const setEvent = data => { return axios.post(base + 'action/powerEventConfigAction', data) }

// 用户脚本
export const getusrScript = data => { return axios.post(base + 'action/usrScriptShowAction', data) }
export const setusrScript = data => { return axios.post(base + 'action/usrScriptSetAction', data) }

// ARP防御
export const setARP = data => { return axios.post(base + 'action/asdefenseEnAction', data) }
export const getARPInfo = data => { return axios.post(base + 'action/asdefenseInfoGetAction', data) }

// ping
export const ping = data => { return axios.post(base + 'action/pingTestAction', data) }

// DNS
export const getDNS = data => { return axios.post(base + 'action/dnsShowAction', data) }
export const setDNS = data => { return axios.post(base + 'action/dnsConfigAction', data) }

// 获取版本信息
export const getVersion = data => { return axios.post(base + 'action/versionShowAction', data) }

// 硬件参数
export const getHardware = data => { return axios.post(base + 'action/hwInfoGetAction', data) }

// shell 页面
export const setQuagga = data => { return axios.post(base + 'action/quaggaHandleAction', data) }
export const getQuaggaInfo = data => { return axios.post(base + 'action/quaggaStatGetAction', data) }
export const setShell = data => { return axios.post(base + 'action/quaggaConfHandleAction', data) }
export const getBtnInfo = data => { return axios.post(base + 'action/websocketIpGetAction', data) }
export const stopAllQuagga = data => { return axios.post(base + 'action/quaggaConfAllStopAction', data) }
// 获取公网连接状态
export const getExtranetStatus = data => { return axios.post(base + 'action/internetStatusAction', data) }
// 情景模式
export const websocketIpGetAction = data => { return axios.post(base + 'action/websocketIpGetAction', data) }

export const publicNetSetAction = data => { return axios.post(base + 'action/publicNetSetAction', data) }
export const localNetSetAction = data => { return axios.post(base + 'action/localNetSetAction', data) }
export const publicNetCancelAction = data => { return axios.post(base + 'action/publicNetCancelAction', data) }
export const kidvpnConfigAction = data => { return axios.post(base + 'action/kidvpnConfigAction', data) }
export const kidvpnCliStatGetAction = data => { return axios.post(base + 'action/kidvpnCliStatGetAction', data) }
export const configCompleteAction = data => { return axios.post(base + 'action/configCompleteAction', data) }
export const asdefenseEnConfAction = data => { return axios.post(base + 'action/asdefenseEnConfAction', data) }
export const kidvpnCancelAction = data => { return axios.post(base + 'action/kidvpnCancelAction', data) }
export const kidvpnConfigSaveAction = data => { return axios.post(base + 'action/kidvpnConfigSaveAction', data) }
export const localDhcpValueGetAction = data => { return axios.post(base + 'action/localDhcpValueGetAction', data) }
export const optionalFuncCancelAction = data => { return axios.post(base + 'action/optionalFuncCancelAction', data) }

// 模式选择
// 复位情景模式状态
export const situationStatResetAction = data => { return axios.post(base + 'action/situationStatResetAction', data) }
// web绑定，解绑
export const webBind = data => { return axios.post(base + 'action/webBindToNetifAction', data) }
export const webUnbind = data => { return axios.post(base + 'action/webBindToNetifCancelAction', data) }
export const getBind = data => { return axios.post(base + 'action/webBindStatusGetAction', data) }
