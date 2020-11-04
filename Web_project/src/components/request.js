/**
 * @file 用于封装http请求
 */

import * as api from '@/api/api'

// 适用于多个数据且有分页的数据请求

/**
 * @param {string} requestUrl 请求路径
 * @param {Object} para 请求参数
 * @param {number} para.page 请求的页数
 * @param {string} dataName response中包含数据的结构体名称，一般为data或interface
 * @return {Object} 返回值是一个Object，至少包括data（Object），total（number）
 */
const request = (requestUrl, para, dataName) => {
  let data, total
  if (dataName === undefined) {
    dataName = 'data'
  }
  api[requestUrl](para)
    .then(res => {
      if (res.data.code === 200) {
        if (res.data.data.length > 0) {
        // 正常有值的形况
          data = res.data[dataName]
          total = res.data.total
        } else if (res.data.total > 0) {
        // 该页无值但是total不为零的情况（一般值在上一页）
          para.page -= 1
          request(requestUrl, para, dataName)
        } else if (res.data.total === 0) {
          // 无值的情况
          data = []
          total = 0
        }
      } else {
        return res.data.code
      }
    })
    .catch(err => {
      console.log(err)
    })
  return {
    data: data,
    total: total
  }
}

export { request }
