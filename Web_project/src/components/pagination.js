/**
 * @file 封装分页器 换页功能
 */

import * as api from '@/api/api'

/**
 * @param {string} requestUrl 请求地址
 * @param {Object} para 请求带的参数
 * @param {number} to 跳转目标页 页数
 */
const pagination = (requestUrl, para, to) => {
  let data
  para.page = to
  api[requestUrl](para)
    .then(res => {
      data = res.data.data
    })
    .catch(err => {
      console.log(err)
    })
  return {
    data: data,
    page: to
  }
}

export { pagination }
