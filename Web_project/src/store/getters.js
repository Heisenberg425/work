const getters = {
  sidebar: state => state.app.sidebar,
  device: state => state.app.device,
  language: state => state.app.language,
  systemData: state => state.app.systemData,
  adaptiveCode: state => state.app.adaptiveCode,
  socket: state => state.app.socket,
  timers: state => state.app.timers
}

export default getters
