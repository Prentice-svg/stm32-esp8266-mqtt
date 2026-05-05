<template>
	<view class="page">
		<web-view v-if="cameraUrl" :src="cameraUrl" :webview-styles="webviewStyles"></web-view>
		<view v-else class="empty-camera">
			<text class="empty-title">未配置摄像头地址</text>
			<text class="empty-text">请先在设置页填写 ESP32-S3-CAM IP</text>
		</view>
	</view>
</template>

<script>
export default {
	data() {
		return {
			cameraUrl: '',
			webviewStyles: {
				progress: {
					color: '#2563EB'
				}
			}
		}
	},
	onLoad(options) {
		const rawUrl = options && options.url ? decodeURIComponent(options.url) : ''
		this.cameraUrl = rawUrl
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	background: #F6F9FC;
}

web-view {
	width: 100%;
	height: 100%;
}

.empty-camera {
	min-height: 100vh;
	padding: 48rpx;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	gap: 14rpx;
	background: #F6F9FC;
	color: #0F172A;
	box-sizing: border-box;
}

.empty-title {
	font-size: 32rpx;
	font-weight: 900;
}

.empty-text {
	font-size: 24rpx;
	color: #64748B;
	text-align: center;
}
</style>
