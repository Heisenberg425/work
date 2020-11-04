export default {
  NAT: {
    dividingLine: {
      title1: 'WAN,LAN口设置',
      title2: '随机端口使用',
      title3: '网络别名',
      title4: '端口映射'
    },
    router: {
      linkStatus: '连接状态',
      IP: 'LAN IP',
      debug: '调试'
    },
    mapping: {
      add: '新增端口映射',
      assPort: '外网端口',
      localPort: '内网端口',
      IP: '内网起始IP',
      IPCnt: '内网个数',
      proto: '协议',
      placeholder: '请选择功能'
    },
    alias: {
      add: '新增网络别名',
      aliasAddress: '别名地址',
      initialAddress: 'LAN起始地址',
      endAddress: 'LAN结束地址'
    },
    WANLANSetting: {
      title: 'LAN,WAN口设置',
      chooseFunc: '选择功能',
      placeholder: '请选择内外网',
      LAN: 'LAN（内网）',
      WAN: 'WAN（外网）'
    },
    adaptive: {
      title: '端口自适应',
      text: '调试',
      portsNameTip: '请输入名称',
      tipText: '(名称示例: eth0)',
      maskTip: '请先进行网口定位操作'
    },
    selectFunction: {
      TCP: 'TCP分片包接收使能',
      UDP: 'UDP分片包接收使能',
      ICMP: 'ICMP分片包接收使能'
    }
  }
}
