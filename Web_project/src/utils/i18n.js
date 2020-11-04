// translate router.meta.title, be used in breadcrumb sidebar tagsview
export function generateTitle(title) {
  const hasKey = this.$te('route.' + title)
  const translatedTitle = this.$t('route.' + title) // $t :this method from vue-i18n, inject in @/lang/index.js

  if (hasKey) {
    // console.log('title1' + translatedTitle)
    return translatedTitle
  }
  // console.log('title2' + title)
  return title
}
