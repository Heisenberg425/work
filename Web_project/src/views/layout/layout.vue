<template>
  <div class="app-wrapper" :class="classsObj">
    <div v-if="device==='mobile'&&sidebar.opened" class="drawer-bg" @click="handleClickOutside"></div>
  </div>
</template>